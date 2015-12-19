#include "stdafx.h"
#include "ContainerElement.h"
#include "ContainerAPI.h"
#include "Container.h"
#include "ContainerFolder.h"
#include "ContainerFile.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "FsUtils.h"

dbc::Error dbc::ContainerElement::s_errElementNotFound = dbc::Error(ERR_DB_FS, NOT_FOUND);

dbc::ContainerElement::ContainerElement(ContainerResources resources, int64_t id)
	: m_resources(resources), m_id(id)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, type, props, specific_data FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, id);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB_FS, NOT_FOUND);
	}
	m_parentId = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
	InitElementInfo(query, 2, 3, 4);
}

dbc::ContainerElement::ContainerElement(ContainerResources resources, int64_t parent_id, const std::string& name)
	: m_resources(resources), m_parentId(parent_id), m_name(name)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT id, type, props, specific_data FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, parent_id);
	query.BindText(2, name);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, NOT_FOUND);
	}
	m_id = query.ColumnInt64(0);
	InitElementInfo(query, 1, 2, 3);
}

bool dbc::ContainerElement::Exists()
{
	Error res = Exists(m_parentId, m_name);
	if (res == SUCCESS)
	{
		return true;
	}
	else
	{
		if (res != s_errElementNotFound)
		{
			throw ContainerException(res);
		}
		return false;
	}
}

std::string dbc::ContainerElement::Name()
{
	Refresh();

	return m_name;
}

std::string dbc::ContainerElement::Path()
{
	// Class ContainerElement does not contain such field as "m_path", because his path may change independently of it at any time. Thats why this func is not const and does queries to get the "current" path of the element.
	Refresh();

	std::string out;

	SQLQuery query(m_resources->GetConnection(), "SELECT count(*), name, parent_id FROM FileSystem WHERE id = ?;");

	for (int64_t id = m_id, parentId = m_parentId; id > 0; id = parentId, query.Reset())
	{
		std::string tmp_name;
		query.BindInt64(1, id);
		query.Step();
		int count = query.ColumnInt(0);
		if (count == 0)
		{
			throw ContainerException(ERR_DB, NOT_VALID, ERR_DB_FS, NOT_FOUND);
		}
		query.ColumnText(1, tmp_name);
		parentId = query.ColumnInt64(2);
		tmp_name = dbc::utils::SlashedPath(tmp_name);
		std::reverse(tmp_name.begin(), tmp_name.end());
		out.append(tmp_name.cbegin(), tmp_name.cend());
	}

	std::reverse(out.begin(), out.end());
	out.pop_back(); // remove last slash
	return out;
}

dbc::ElementType dbc::ContainerElement::Type() const
{
	return m_type;
}

dbc::ContainerFolder* dbc::ContainerElement::AsFolder()
{
	return dynamic_cast<ContainerFolder*>(this);
}

dbc::ContainerFile* dbc::ContainerElement::AsFile()
{
	return dynamic_cast<ContainerFile*>(this);
}

bool dbc::ContainerElement::IsTheSame(const ContainerElement& obj) const
{
	const ContainerElement& ce = dynamic_cast<const ContainerElement&>(obj);
	return (m_resources == ce.m_resources && m_id == ce.m_id);
}

bool dbc::ContainerElement::IsChildOf(const ContainerElement& obj)
{
	Refresh();

	const ContainerElement& elementObj = dynamic_cast<const ContainerElement&>(obj);
	if (elementObj.m_type == ElementTypeFile)
	{
		return false;
	}

	Error res = Exists(elementObj.m_parentId, elementObj.m_name);
	if (res != SUCCESS)
	{
		throw ContainerException(res);
	}

	int64_t targetId = elementObj.m_id;

	if (targetId == m_id)
	{
		return false;
	}

	if (targetId == m_parentId)
	{
		return true;
	}

	int64_t id = m_id;
	int64_t parentId = m_parentId;
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id FROM FileSystem WHERE id = ?;");
	while (parentId > 0)
	{
		query.Reset();
		query.BindInt64(1, id);
		query.Step();
		parentId = query.ColumnInt64(0);
		if (parentId == targetId)
		{
			return true;
		}
		id = parentId;
	}
	return false;
}

dbc::ContainerFolderGuard dbc::ContainerElement::GetParentEntry()
{
	if (m_parentId < Container::ROOT_ID)
	{
		throw ContainerException(ERR_DB_FS, NOT_FOUND);
	}

	return ContainerFolderGuard(new ContainerFolder(m_resources, m_parentId));
}

void dbc::ContainerElement::MoveToEntry(ContainerFolder& newParent)
{
	Refresh();

	if (m_id == Container::ROOT_ID || newParent.IsChildOf(*this))
	{
		throw ContainerException(ACTION_IS_FORBIDDEN);
	}

	ContainerElement& elementObj = dynamic_cast<ContainerElement&>(newParent);
	Error res = Exists(elementObj.m_id, m_name);
	if (res != s_errElementNotFound)
	{
		if (res == SUCCESS)
		{
			res = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, res);
	}

	SQLQuery query(elementObj.m_resources->GetConnection(), "UPDATE FileSystem SET parent_id = ? WHERE id = ?;");
	query.BindInt64(1, elementObj.m_id);
	query.BindInt64(2, m_id);
	query.Step();

	m_props.SetDateModified(::time(0));
	WriteProps();
}

void dbc::ContainerElement::Remove()
{
	SQLQuery query(m_resources->GetConnection(), "DELETE FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	query.Step();
}

void dbc::ContainerElement::Rename(const std::string& newName)
{
	if (newName.empty() || !dbc::utils::FileNameIsValid(newName))
	{
		throw ContainerException(ERR_DB_FS, CANT_CREATE, WRONG_PARAMETERS);
	}

	Refresh();

	Error tmp = Exists(m_parentId, newName);
	if (tmp != s_errElementNotFound)
	{
		if (tmp == SUCCESS)
		{
			tmp = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, tmp);
	}

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET name = ? WHERE id = ?;");
	query.BindText(1, newName);
	query.BindInt64(2, m_id);
	query.Step();

	m_props.SetDateModified(::time(0));
	WriteProps();

	m_name = newName;
}

void dbc::ContainerElement::GetProperties(ElementProperties& out)
{
	Refresh();

	out = m_props;
}

void dbc::ContainerElement::ResetProperties(const std::string& tag)
{
	Refresh();

	m_props.SetTag(tag);
	m_props.SetDateModified(::time(0));
	std::string propsStr;
	ElementProperties::MakeString(m_props, propsStr);

	WriteProps();
}

void dbc::ContainerElement::Refresh()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*), parent_id, name, props FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	query.Step();
	int count = query.ColumnInt(0);
	if (count == 0)
	{
		throw ContainerException(ERR_DB_FS, NOT_FOUND);
	}
	m_parentId = query.ColumnInt64(1);
	query.ColumnText(2, m_name);
	std::string props_str;
	query.ColumnText(3, props_str);
	ElementProperties::ParseString(props_str, m_props);
}

dbc::Error dbc::ContainerElement::Exists(int64_t parent_id, std::string name)
{
	int count = 0;
	try
	{
		SQLQuery query(m_resources->GetConnection(), "SELECT id FROM FileSystem WHERE parent_id = ? AND name = ?;");
		query.BindInt64(1, parent_id);
		query.BindText(2, name);
		return query.Step() ? SUCCESS : s_errElementNotFound;
	}
	catch (const ContainerException& ex)
	{
		return ex.ErrType();
	}
}

void dbc::ContainerElement::WriteProps()
{
	// Writes properties about this object to the table FileSystem in 'props' column
	// Its exceptions are managed by calling functions
	SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET props = ? WHERE id = ?;");
	std::string propsStr;
	ElementProperties::MakeString(m_props, propsStr);
	query.BindText(1, propsStr);
	query.BindInt64(2, m_id);
	query.Step();
}

void dbc::ContainerElement::InitElementInfo(SQLQuery& query, int typeN, int propsN, int specificDataN)
{
	int tmp_type = query.ColumnInt(typeN);
	m_type = static_cast<ElementType>(tmp_type);
	if (m_type == ElementTypeUnknown)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB, IS_DAMAGED);
	}

	std::string propsStr;
	query.ColumnText(propsN, propsStr);
	ElementProperties::ParseString(propsStr, m_props);

	query.ColumnBlob(specificDataN, m_specificData);
}
