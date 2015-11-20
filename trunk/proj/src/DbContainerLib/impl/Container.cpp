#include "stdafx.h"
#include "Container.h"
#include "ContainerAPI.h"
#include "DataStorageBinaryFile.h"
#include "ContainerElement.h"
#include "ContainerFolder.h"
#include "ContainerFile.h"
#include "ContainerResourcesImpl.h"
#include "ContainerException.h"
#include "Crypto.h"
#include "ContainerInfoImpl.h"
#include "CommonUtils.h"
#include "FsUtils.h"

const int dbc::Container::ROOT_ID = 1;

using namespace dbc;

namespace
{
	void ClearDB(Connection& connection)
	{
		const int tables_count = 3;
		std::string tables[tables_count] = { "Sets", "FileSystem", "FileStreams" };

		dbc::SQLQuery query = connection.CreateQuery();
		std::string dropCommand("DROP TABLE ");
		for (size_t i = 0; i < tables_count; ++i)
		{
			query.Prepare(dropCommand + tables[i] + ";");
			query.Step();
		}

		query.Prepare("VACUUM");
		query.Step();
	}

	void WriteTables(Connection& connection)
	{
		// Creation of the database tables
		Error ret(SUCCESS);
		SQLQuery query = connection.CreateQuery();

		std::list<std::string> tables;
		tables.push_back("CREATE TABLE Sets(id INTEGER PRIMARY KEY NOT NULL, storage_data_size INTEGER, storage_data BLOB);");
		tables.push_back("CREATE TABLE FileSystem(id INTEGER PRIMARY KEY NOT NULL, parent_id INTEGER, name TEXT, type INTEGER, props TEXT);");
		tables.push_back("CREATE TABLE FileStreams(id INTEGER PRIMARY KEY NOT NULL, file_id INTEGER NOT NULL, stream_order INTEGER, start INTEGER, size INTEGER, used INTEGER);");

		for (std::list<std::string>::const_iterator itr = tables.begin(); itr != tables.end(); ++itr)
		{
			try
			{
				query.Prepare(*itr);
				query.Step();
			}
			catch (const dbc::ContainerException& ex)
			{
				throw ContainerException(ERR_DB, CANT_CREATE, ex.ErrType());
			}
		}
	}

	void WriteRoot(Connection& connection)
	{
		// Creating root folder
		SQLQuery query = connection.CreateQuery("INSERT INTO FileSystem(parent_id, name, type, props) VALUES (?, ?, ?, ?);");

		query.BindInt(1, 0);
		std::string tmp_name(1, dbc::PATH_SEPARATOR);
		query.BindText(2, tmp_name);
		query.BindInt(3, static_cast<int>(dbc::ElementTypeFolder));
		ElementProperties elProps;
		ElementProperties::SetCurrentTime(elProps);
		std::string propsStr;
		ElementProperties::MakeString(elProps, propsStr);
		query.BindText(4, propsStr);
		query.Step();
	}

	void WriteSets(Connection& connection)
	{
		//SQLQuery query(db, L"INSERT INTO Sets(path_sep) VALUES (?);");
		// TODO: Write another sets
	}

	void BuildDB(Connection& connection)
	{
		WriteTables(connection);
		WriteRoot(connection);
		WriteSets(connection);
	}

	/*Error DBIsEmpty(sqlite3 * db)
	{
		Error ret(ERR_DB, IS_EMPTY); // Expected = default
		const int queries_count = 3;
		const std::string tables[queries_count] = { "Sets", "FileSystem", "FileStreams" };

		try
		{
			SQLQuery query(db, "SELECT count(name) FROM sqlite_master where name=?;");
			for (int i = 0; i < queries_count; ++i)
			{
				query.Reset();
				query.BindText(tables[i], 1);
				query.Step();
				if (query.ColumnInt(1) != 0) // Not expected
				{
					ret = Error(ERR_DB, ALREADY_EXISTS);
					break;
				}
			}
		}
		catch (const dbc::ContainerException &ex)
		{
			ret = ex.ErrType();
		}
		return ret;
	}*/

	bool CheckDBValidy(dbc::Connection &db)
	{
		return true;
		// TODO: implement proper DB validation
	}

	void SetDBPragma(dbc::Connection &db)
	{
		dbc::SQLQuery query(db, "PRAGMA auto_vacuum = FULL;");
		query.Step();
	}
}

dbc::Container::Container(const std::string& path, const std::string& password, bool create)
	: m_dbFile(path), m_connection(path, create), m_storage(new dbc::DataStorageBinaryFile)
{
	PrepareContainer(password, create);
}

dbc::Container::Container(const std::string& path, const std::string& password, IDataStorageGuard storage, bool create)
	: m_dbFile(path), m_connection(path, create), m_storage(storage)
{
	PrepareContainer(password, create);
}

dbc::Container::~Container()
{
	ContaierResourcesImpl* resources = dynamic_cast<ContaierResourcesImpl*>(m_resources.get());
	assert(resources != nullptr && "Wrong container resources implementation class is used");

	resources->ReportContainerDied();
}

void dbc::Container::Clear()
{
	try
	{
		ClearDB(m_connection);
		m_storage->ClearData();
	}
	catch (const ContainerException &ex)
	{
		throw ContainerException(ERR_DB, CANT_REMOVE, ex.ErrType());
	}

	try
	{
		BuildDB(m_connection);
	}
	catch (const ContainerException &ex)
	{
		throw ContainerException(ERR_DB, CANT_CREATE, ex.ErrType());
	}
}

void dbc::Container::ResetPassword(const std::string& newPassword)
{
	m_storage->ResetPassword(newPassword);
}

dbc::ContainerFolderGuard dbc::Container::GetRoot()
{
	return ContainerFolderGuard(new ContainerFolder(m_resources, ROOT_ID));
}

dbc::ContainerElementGuard dbc::Container::GetElement(const std::string& path)
{
	if (path.empty())
	{
		throw ContainerException(WRONG_PARAMETERS);
	}

	std::vector<std::string> names;
	dbc::utils::SplitSavingDelim(path, PATH_SEPARATOR, names);

	int64_t parentId = 0;
	int elementType = ElementTypeUnknown;

	SQLQuery query(m_connection, "SELECT count(*), id, type FROM FileSystem WHERE parent_id = ? AND name = ?;");
	for (std::vector<std::string>::iterator itr = names.begin(); itr != names.end(); ++itr, query.Reset())
	{
		query.BindInt64(1, parentId);
		*itr = dbc::utils::UnslashedPath(*itr);
		query.BindText(2, *itr);
		query.Step();
		int count = query.ColumnInt(0);
		if (count == 0)
		{
			throw ContainerException(ERR_DB_FS, NOT_FOUND);
		}
		else if (count > 1)
		{
			throw ContainerException(ERR_DB, IS_DAMAGED);
		}
		parentId = query.ColumnInt64(1);
		elementType = query.ColumnInt(2);
	}

	switch (elementType)
	{
	case ElementTypeFolder:
		return ContainerElementGuard(new ContainerFolder(m_resources, parentId));
	case ElementTypeFile:
		return ContainerElementGuard(new ContainerFile(m_resources, parentId));
	default:
		throw ContainerException(ERR_INTERNAL);
	}
}

ContainerInfo dbc::Container::GetInfo()
{
	return ContainerInfo(new ContainerInfoImpl(m_resources));
}

dbc::DataUsagePreferences dbc::Container::GetDataUsagePreferences()
{
	return m_dataUsagePrefs;
}

void dbc::Container::SetDataUsagePreferences(const DataUsagePreferences& prefs)
{
	m_dataUsagePrefs = prefs;
}

void dbc::Container::PrepareContainer(const std::string& password, bool create)
{
	assert(m_storage.get() != nullptr);
	if (create) // create DB and storage
	{
		BuildDB(m_connection);
		m_storage->Create(m_dbFile, password);
	}
	else 
	{
		if (!CheckDBValidy(m_connection)) // check existing DB
		{
			throw ContainerException(ERR_DB, CANT_OPEN, ERR_DB, NOT_VALID);
		}
		// check Data
		RawData storageData;
		ReadSets(storageData);
		m_storage->Open(m_dbFile, password, storageData);
		// TODO: Parse storage data
	}
	SetDBPragma(m_connection);

	m_resources.reset(new ContaierResourcesImpl(m_connection, *m_storage, m_dataUsagePrefs));
}

void dbc::Container::ReadSets(RawData& storageData)
{
	assert(storageData.empty());
	SQLQuery query(m_connection, "SELECT storage_data FROM Sets WHERE id = 1;");
	query.Step();
	query.ColumnBlob(1, storageData);
}

void dbc::Container::SaveStorageData()
{
	assert(m_storage.get() != nullptr);
	RawData storageData;
	m_storage->GetDataToSave(storageData);
	if (storageData.size() > 0)
	{
		SQLQuery query(m_connection, "UPDATE Sets SET storage_data = ? WHERE id = 1;");
		query.BindBlob(1, storageData);
	}
}
