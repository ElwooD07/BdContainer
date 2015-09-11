// class ContainerFolder
#include "stdafx.h"
#include "ContainerFolder.h"
#include "ContainerFile.h"
#include "Container.h"
#include "SQLQuery.h"
#include "ElementsIterator.h"
#include "FsUtils.h"
#include "ContainerException.h"

dbc::ContainerFolder::ContainerFolder(ContainerResources resources, int64_t id) :
ContainerElement(resources, id)
{	}

dbc::ContainerFolder::ContainerFolder(ContainerResources resources, int64_t parent_id, const std::string &name) :
ContainerElement(resources, parent_id, name)
{	}

std::string dbc::ContainerFolder::Name()
{
	if (IsRoot())
	{
		return m_name;
	}
	return ContainerElement::Name();
}

std::string dbc::ContainerFolder::Path()
{
	if (IsRoot())
	{
		return m_name;
	}
	return ContainerElement::Path();
}

void dbc::ContainerFolder::Remove()
{
	Refresh();

	Error err = RemoveFolder(m_resources->GetConnection(), m_id);
	if (err != SUCCESS)
	{
		throw ContainerException(err);
	}
}

void dbc::ContainerFolder::Rename(const std::string &new_name)
{
	if (!IsRoot())
	{
		ContainerElement::Rename(new_name);

		m_props.SetDateModified();
		WriteProps();
	}
	else
	{
		throw ContainerException(ACTION_IS_FORBIDDEN);
	}
}

dbc::ContainerFolderGuard dbc::ContainerFolder::Clone() const
{
	return ContainerFolderGuard(new ContainerFolder(m_resources, m_id));
}

bool dbc::ContainerFolder::IsRoot() const
{
	return (m_id == dbc::Container::ROOT_ID
		&& m_name.size() == 1
		&& m_name[0] == PATH_SEPARATOR);
}

bool dbc::ContainerFolder::HasChildren()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem WHERE parent_id = ? LIMIT 1;");
	query.BindInt64(1, m_id);
	query.Step();
	return query.ColumnInt(0) > 0;
}

dbc::ContainerElementGuard dbc::ContainerFolder::GetChild(const std::string &name)
{
	Refresh();

	SQLQuery query(m_resources->GetConnection(), "SELECT count(*), id, type FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, m_id);
	query.BindText(2, name);
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
	int64_t id = query.ColumnInt64(1);
	int tmp_type = query.ColumnInt(2);

	switch (tmp_type)
	{
	case ElementTypeFolder:
		return ContainerElementGuard(new ContainerFolder(m_resources, id));
	case ElementTypeFile:
		return ContainerElementGuard(new ContainerFile(m_resources, id));
	default:
		throw ContainerException(ERR_DB, IS_DAMAGED);
	}
}

dbc::ContainerElementGuard dbc::ContainerFolder::CreateChild(const std::string &name, ElementType type, const std::string &tag)
{
	CreateChildEntry(name, type, tag);
	switch (type)
	{
	case ElementTypeFolder:
		return ContainerElementGuard(new ContainerFolder(m_resources, m_id, name));
	case ElementTypeFile:
		return ContainerElementGuard(new ContainerFile(m_resources, m_id, name));
	default:
		throw ContainerException(ERR_INTERNAL);
	}
}

dbc::ContainerFolderGuard dbc::ContainerFolder::CreateFolder(const std::string& name, const std::string& tag)
{
	CreateChildEntry(name, ElementTypeFolder, tag);
	return ContainerFolderGuard(new ContainerFolder(m_resources, m_id, name));
}
dbc::ContainerFileGuard dbc::ContainerFolder::CreateFile(const std::string& name, const std::string& tag)
{
	CreateChildEntry(name, ElementTypeFile, tag);
	return ContainerFileGuard(new ContainerFile(m_resources, m_id, name));
}

dbc::DbcElementsIterator dbc::ContainerFolder::EnumFsEntries()
{
	Refresh();
	return DbcElementsIterator(new ElementsIterator(m_resources, m_id));
}

dbc::Error dbc::ContainerFolder::RemoveFolder(Connection& connection, int64_t folder_id)
{
	if (folder_id <= 1)
	{
		throw ContainerException(ACTION_IS_FORBIDDEN); //  Deleting is forbidden for the root;
	}

	Error ret = SUCCESS;
	try
	{
		SQLQuery query(connection, "SELECT id FROM FileSystem WHERE parent_id = ?;");
		int64_t id;
		query.BindInt64(1, folder_id);
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
		query.BindInt64(1, folder_id);
		query.Step();
	}
	catch (const ContainerException &ex)
	{
		ret = ex.ErrType();
	}
	return ret;
}

void dbc::ContainerFolder::CreateChildEntry(const std::string& name, ElementType type, const std::string& tag)
{
	Refresh();

	if (name.empty() || !dbc::utils::FNameIsValid(name) || (type == ElementTypeUnknown))
	{
		throw ContainerException(ERR_DB_FS, CANT_CREATE, WRONG_PARAMETERS);
	}

	Error tmp = Exists(m_id, name);
	if (tmp != Error(ERR_DB_FS, NOT_FOUND))
	{
		if (tmp == SUCCESS)
		{
			tmp = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, tmp);
	}

	SQLQuery query(m_resources->GetConnection(), "INSERT INTO FileSystem(parent_id, name, type, props) VALUES (?, ?, ?, ?);");
	query.BindInt64(1, m_id);
	query.BindText(2, name);
	query.BindInt(3, type);
	ElementProperties props;
	ElementProperties::SetCurrentTime(props);
	props.SetTag(tag);
	std::string props_str;
	ElementProperties::MakeString(props, props_str);
	query.BindText(4, props_str);
	query.Step();
}