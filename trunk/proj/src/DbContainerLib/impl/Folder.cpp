#include "stdafx.h"
#include "Folder.h"
#include "File.h"
#include "Container.h"
#include "IContainnerResources.h"
#include "SQLQuery.h"
#include "FsUtils.h"
#include "CommonUtils.h"
#include "ContainerException.h"

dbc::Folder::Folder(ContainerResources resources, int64_t id)
	: Element(resources, id)
{	}

dbc::Folder::Folder(ContainerResources resources, int64_t parent_id, const std::string &name)
	: Element(resources, parent_id, name)
{	}

std::string dbc::Folder::Name()
{
	if (IsRoot())
	{
		return m_name;
	}
	return Element::Name();
}

std::string dbc::Folder::Path()
{
	if (IsRoot())
	{
		return m_name;
	}
	return Element::Path();
}

void dbc::Folder::Remove()
{
	Refresh();

	Error err = RemoveFolder(m_resources->GetConnection(), m_id);
	if (err != SUCCESS)
	{
		throw ContainerException(err);
	}
}

void dbc::Folder::Rename(const std::string& newName)
{
	if (!IsRoot())
	{
		Element::Rename(newName);

		m_props.SetDateModified(::time(0));
		WriteProps();
	}
	else
	{
		throw ContainerException(ACTION_IS_FORBIDDEN);
	}
}

dbc::FolderGuard dbc::Folder::Clone() const
{
	return FolderGuard(new Folder(m_resources, m_id));
}

bool dbc::Folder::IsRoot() const
{
	if (m_id == dbc::Container::ROOT_ID)
	{
		assert(m_name.size() == 1 && m_name[0] == PATH_SEPARATOR && "Wrong root name");
		return true;
	}
	return false;
}

bool dbc::Folder::HasChildren()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem WHERE parent_id = ? LIMIT 1;");
	query.BindInt64(1, m_id);
	query.Step();
	return query.ColumnInt(0) > 0;
}

dbc::ElementGuard dbc::Folder::GetChild(const std::string& name)
{
	Refresh();

	SQLQuery query(m_resources->GetConnection(), "SELECT count(*), id, type FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, m_id);
	query.BindText(2, name);
	query.Step();
	int count = query.ColumnInt(0);
	if (count == 0)
	{
		throw ContainerException(notFoundError);
	}
	else if (count > 1)
	{
		throw ContainerException(ERR_DB, IS_DAMAGED);
	}
	int64_t id = query.ColumnInt64(1);
	int tmp_type = query.ColumnInt(2);

	switch (tmp_type)
	{
	case ElementTypeFolder:
		return ElementGuard(new Folder(m_resources, id));
	case ElementTypeFile:
		return ElementGuard(new File(m_resources, id));
	case ElementTypeSymLink:
		return ElementGuard(new SymLink(m_resources, id));
	default:
		assert(!"Unknown element type specified");
		throw ContainerException(ERR_DB, IS_DAMAGED);
	}
}

dbc::ElementGuard dbc::Folder::CreateChild(const std::string& name, ElementType type, const std::string& tag /*= 0*/)
{
	CreateChildEntry(name, type, tag);
	switch (type)
	{
	case ElementTypeFolder:
		return ElementGuard(new Folder(m_resources, m_id, name));
	case ElementTypeFile:
		return ElementGuard(new File(m_resources, m_id, name));
	case ElementTypeSymLink:
		return ElementGuard(new SymLink(m_resources, m_id, name));
	default:
		assert(!"Unknown element type specified");
		throw ContainerException(ERR_INTERNAL);
	}
}

dbc::FolderGuard dbc::Folder::CreateFolder(const std::string& name, const std::string& tag)
{
	CreateChildEntry(name, ElementTypeFolder, tag);
	return FolderGuard(new Folder(m_resources, m_id, name));
}
dbc::FileGuard dbc::Folder::CreateFile(const std::string& name, const std::string& tag)
{
	CreateChildEntry(name, ElementTypeFile, tag);
	return FileGuard(new File(m_resources, m_id, name));
}

dbc::SymLinkGuard dbc::Folder::CreateSymLink(const std::string& name, const std::string& targetPath, const std::string& tag /*= ""*/)
{
	Error err = SymLink::IsTargetPathValid(targetPath);
	if (err != SUCCESS)
	{
		throw ContainerException(err);
	}
	CreateChildEntry(name, ElementTypeSymLink, tag, targetPath);
	return SymLinkGuard(new SymLink(m_resources, m_id, name));
}

dbc::DbcElementsIterator dbc::Folder::EnumFsEntries()
{
	Refresh();
	return DbcElementsIterator(new ElementsIterator(m_resources, m_id));
}

dbc::Error dbc::Folder::RemoveFolder(Connection& connection, int64_t folderId)
{
	if (folderId <= 1)
	{
		throw ContainerException(ACTION_IS_FORBIDDEN); //  Deleting is forbidden for the root;
	}

	Error ret = SUCCESS;
	try
	{
		TransactionGuard transaction = m_resources->GetConnection().StartTransaction();

		SQLQuery query(connection, "SELECT id FROM FileSystem WHERE parent_id = ?;");
		int64_t id;
		query.BindInt64(1, folderId);
		while (query.Step())
		{
			id = query.ColumnInt64(0);
			ret = RemoveFolder(connection, id);
			if (ret != SUCCESS)
			{
				break;
			}
		}
		query.Prepare("DELETE FROM FileSystem WHERE id = ?;");
		query.BindInt64(1, folderId);
		query.Step();

		transaction->Commit();
	}
	catch (const ContainerException &ex)
	{
		ret = ex.ErrType();
	}
	return ret;
}

void dbc::Folder::CreateChildEntry(const std::string& name, ElementType type, const std::string& tag, const std::string& specificData)
{
	Refresh();

	if (name.empty() || !dbc::utils::FileNameIsValid(name) || (type == ElementTypeUnknown))
	{
		throw ContainerException(ERR_DB_FS, CANT_CREATE, WRONG_PARAMETERS);
	}

	Error tmp = Exists(m_id, name);
	if (tmp != notFoundError)
	{
		if (tmp == SUCCESS)
		{
			tmp = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, tmp);
	}

	SQLQuery query(m_resources->GetConnection(), "INSERT INTO FileSystem(parent_id, name, type, props, specific_data) VALUES (?, ?, ?, ?, ?);");
	query.BindInt64(1, m_id);
	query.BindText(2, name);
	query.BindInt(3, type);
	ElementProperties props;
	ElementProperties::SetCurrentTime(props);
	props.SetTag(tag);
	std::string propsStr;
	ElementProperties::MakeString(props, propsStr);
	query.BindText(4, propsStr);
	RawData specificDataBlob(utils::StringToRawData(specificData));
	query.BindBlob(5, specificDataBlob);
	query.Step();
}