#include "stdafx.h"
#include "Utils.h"
#include "impl/Utils/FsUtils.h"
#include "ContainerAPI.h"

using namespace dbc;

extern std::string db_path;
extern std::string bin_path;
extern std::string pass;

extern ContainerGuard cont;

extern bool databaseCreated;
extern bool databaseConnected;

void RefreshLog()
{
	remove("log.txt");
}

void DatabaseCreate()
{
	if (dbc::utils::FileExists(db_path))
	{
		databaseCreated = true;
		DatabaseRemove();
	}
	if (databaseCreated) return;
	try
	{
		CreateContainer(db_path, pass);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Cant create container " << db_path << ": " << ex.what();
		return;
	}
	databaseCreated = true;
}

void DatabaseRemove()
{
	if (databaseConnected)
	{
		DatabaseDisconnect();
	}
	remove(db_path.c_str());
	databaseCreated = dbc::utils::FileExists(db_path);
}

void DatabaseConnect()
{
	if (databaseConnected) return;
	try
	{
		cont = Connect(db_path, pass);
		databaseConnected = true;
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Can't connect to the " << db_path << ": " << ex.what();
		return;
	}
}

void DatabaseDisconnect()
{
	if (!databaseConnected) return;
	if (cont.get() != nullptr)
	{
		cont.reset();
	}
	databaseConnected = false;
}

bool DatabasePrepare()
{
	try
	{
		DatabaseCreate();
		if (databaseCreated)
		{
			DatabaseConnect();
			if (databaseConnected)
			{
				cont->Clear();
				return true;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "\nCan't prepare database: " << ex.what() << std::endl;
	}
	return false;
}

void AppendStream(std::ostream& strm, size_t size)
{
	const std::string s_smallExpression("0123456789abcdefghijklmnopqrstuvwxyz!");
	strm.seekp(0, std::ios::end);
	for (size_t i = 0; i < size;)
	{
		size_t appended = 0;
		if (i + s_smallExpression.size() > size)
		{
			appended = size - i;
		}
		else
		{
			appended = s_smallExpression.size();
		}
		strm.write(s_smallExpression.data(), appended);

		i += appended;
	}
	strm.flush();
}

std::fstream CreateStream(size_t size)
{
	std::fstream strm("testfile.txt", std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
	AppendStream(strm, size);
	return std::move(strm);
}

void RewindStream(std::istream& strm)
{
	strm.clear();
	strm.seekg(0);
	ASSERT_EQ(strm.tellg(), std::streamoff(0));
	ASSERT_FALSE(strm.bad());
}

unsigned int PrepareContainerForPartialWriteTest(dbc::ContainerGuard container, bool transactionalWrite) // returns cluster size
{
	DataUsagePreferences& prefs = container->GetDataUsagePreferences();
	prefs.SetTransactionalWrite(transactionalWrite);
	prefs.SetClusterSizeLevel(DataUsagePreferences::CLUSTER_SIZE_MIN);
	unsigned int clusterSize = prefs.ClusterSize();
	container->SetDataUsagePreferences(prefs);
	return clusterSize;
}

void ShowExceptionMessages()
{
	std::cout << "\nContainerException Error messages:\n\n";
	std::cout << "   Commons:\n";
	std::cout << ErrorString(SUCCESS) << std::endl;
	std::cout << ErrorString(WRONG_PARAMETERS) << std::endl;
	std::cout << ErrorString(CANT_ALLOC_MEMORY) << std::endl;
	std::cout << ErrorString(INVALID_PASSWORD) << std::endl;
	std::cout << ErrorString(OWNER_IS_MISSING) << std::endl;
	std::cout << ErrorString(ACTION_IS_FORBIDDEN) << std::endl;

	std::cout << "   Standard messages:\n";
	for (unsigned int i = ERR_SQL; i < (ERR_BASE_COUNT & 0xff00); i += 0x0100) // iterate base
	{
		for (unsigned int j = INCIDENT_NONE; j < INCIDENT_COUNT; ++j)
		{
			std::cout << ErrorString(i | j) << std::endl;
		}
	}

	std::cout << "   Extended messages:\n";
	std::cout << ErrorString(SQL_WRONG_QUERY) << std::endl;
	std::cout << ErrorString(SQL_DISCONNECTED) << std::endl;
	std::cout << ErrorString(SQL_STMT_NOT_PREPARED) << std::endl;
	std::cout << ErrorString(SQL_CANT_PREPARE) << std::endl;
	std::cout << ErrorString(SQL_CANT_STEP) << std::endl;
	std::cout << ErrorString(SQL_NO_ACCESS) << std::endl;
	std::cout << ErrorString(SQL_BUSY) << std::endl;
	std::cout << ErrorString(SQL_ROW) << std::endl;
	std::cout << ErrorString(SQL_DONE) << std::endl;
	std::cout << ErrorString(ERR_DB_NO_CONNECTION) << std::endl;
	std::cout << ErrorString(ERR_DATA_CANT_OPEN_SRC) << std::endl;
	std::cout << ErrorString(ERR_DATA_CANT_OPEN_DEST) << std::endl;
	std::cout << ErrorString(ERR_DATA_NOT_INITIALIZED) << std::endl;

	/*
	// Database entirely
	std::cout << ErrorString(DB_NOT_FOUND) << std::endl; // = 0x0201,
	std::cout << ErrorString(DB_ALREADY_EXISTS) << std::endl; // = 0x0202,
	std::cout << ErrorString(DB_CANT_OPEN) << std::endl; // = 0x0203,
	std::cout << ErrorString(DB_CANT_CREATE) << std::endl; // = 0x0204,
	std::cout << ErrorString(DB_CANT_READ) << std::endl; // = 0x0205,
	std::cout << ErrorString(DB_CANT_WRITE) << std::endl; // = 0x0206,
	std::cout << ErrorString(DB_CANT_REMOVE) << std::endl; // = 0x207,
	std::cout << ErrorString(DB_NOT_VALID) << std::endl; // = 0x0208,
	std::cout << ErrorString(DB_IS_EMPTY) << std::endl; // = 0x0209,
	std::cout << ErrorString(DB_IS_DAMAGED) << std::endl; // = 0x20a,
	std::cout << ErrorString(DB_NO_CONNECTION) << std::endl; // = 0x20b,

	std::cout << ErrorString(DB_ERROR) << std::endl; // = 0x02ff,

	// File system ("real")
	std::cout << ErrorString(FS_NOT_FOUND) << std::endl; // = 0x0301,
	std::cout << ErrorString(FS_ALREADY_EXISTS) << std::endl; // = 0x0302,
	std::cout << ErrorString(FS_CANT_OPEN) << std::endl; // = 0x0303,
	std::cout << ErrorString(FS_CANT_CREATE) << std::endl; // = 0x0304,
	std::cout << ErrorString(FS_CANT_READ) << std::endl; // = 0x0305,
	std::cout << ErrorString(FS_CANT_WRITE) << std::endl; // = 0x0306,
	std::cout << ErrorString(FS_CANT_REMOVE) << std::endl; // = 0x0307,

	std::cout << ErrorString(FS_ERROR) << std::endl; // = 0x03ff,

	// File system in the database ("virtual")
	std::cout << ErrorString(DB_FS_NOT_FOUND) << std::endl; // = 0x0401,
	std::cout << ErrorString(DB_FS_ALREADY_EXISTS) << std::endl; // = 0x0402,
	std::cout << ErrorString(DB_FS_CANT_OPEN) << std::endl; // = 0x0403,
	std::cout << ErrorString(DB_FS_CANT_CREATE) << std::endl; // = 0x0404,
	std::cout << ErrorString(DB_FS_CANT_READ) << std::endl; // = 0x0405,
	std::cout << ErrorString(DB_FS_CANT_WRITE) << std::endl; // = 0x0406,
	std::cout << ErrorString(DB_FS_CANT_REMOVE) << std::endl; // = 0x0407,

	std::cout << ErrorString(DB_FS_ERROR) << std::endl; // = 0x04ff,

	// Data in binary file
	std::cout << ErrorString(DATA_NOT_FOUND) << std::endl; // = 0x0501,
	std::cout << ErrorString(DATA_ALREADY_EXISTS) << std::endl; // = 0x0502,
	std::cout << ErrorString(DATA_CANT_OPEN) << std::endl; // = 0x0503,
	std::cout << ErrorString(DATA_CANT_CREATE) << std::endl; // = 0x0504,
	std::cout << ErrorString(DATA_CANT_READ) << std::endl; // = 0x0505,
	std::cout << ErrorString(DATA_CANT_WRITE) << std::endl; // = 0x0506,
	std::cout << ErrorString(DATA_CANT_REMOVE) << std::endl; // = 0x0507,
	std::cout << ErrorString(DATA_IS_DAMAGED) << std::endl; // = 0x0508,
	std::cout << ErrorString(DATA_NO_ACCESS) << std::endl; // = 0x0509,

	std::cout << ErrorString(DATA_CANT_OPEN_SRC) << std::endl; // = 0x050a, // extended
	std::cout << ErrorString(DATA_CANT_OPEN_DEST) << std::endl; // = 0x050b, // extended

	std::cout << ErrorString(DATA_ERROR) << std::endl; // = 0x05ff,

	std::cout << ErrorString(INTERNAL_ERROR) << std::endl; // = 0xfffe,
	std::cout << ErrorString(ERROR) << std::endl; // = 0xffff
	std::cout << std::endl;
	*/
}
