#include "stdafx.h"
#include "ContainerElement.h"
#include "ContainerAPI.h"
#include "Container.h"
#include "ContainerFolder.h"
#include "ContainerFile.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "FsUtils.h"

dbc::ContainerElement::ContainerElement(ContainerResources resources, int64_t id) :
m_resources(resources), m_id(id)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, type, props FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, id);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB_FS, NOT_FOUND);
	}
	m_parent_id = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
	int tmp_type = query.ColumnInt(2);
	m_type = static_cast<ElementType>(tmp_type);
	if (m_type == ElementTypeUnknown)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB, IS_DAMAGED);
	}
	std::string props_str;
	query.ColumnText(3, props_str);
	ElementProperties::ParseString(props_str, m_props);
}

dbc::ContainerElement::ContainerElement(ContainerResources resources, int64_t parent_id, const std::string &name) :
m_resources(resources), m_parent_id(parent_id), m_name(name)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT id, type, props FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, parent_id);
	query.BindText(2, name);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, NOT_FOUND);
	}
	m_id = query.ColumnInt64(0);
	int tmp_type = query.ColumnInt(1);
	m_type = static_cast<ElementType>(tmp_type);
	if (m_type == ElementTypeUnknown)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB, IS_DAMAGED);
	}
	std::string props_str;
	query.ColumnText(2, props_str);
	ElementProperties::ParseString(props_str, m_props);
}

bool dbc::ContainerElement::Exists()
{
	// Checks the existing of the element with the current information in the database
	Error res = Exists(m_parent_id, m_name);
	if (res == SUCCESS)
	{
		return true;
	}
	else
	{
		if (res != Error(ERR_DB_FS, NOT_FOUND))
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

	for (int64_t id = m_id, parent_id = m_parent_id; id > 0; id = parent_id, query.Reset())
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
		parent_id = query.ColumnInt64(2);
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

dbc::IContainerFolder* dbc::ContainerElement::AsFolder()
{
	return dynamic_cast<ContainerFolder*>(this);
}

dbc::IContainerFile* dbc::ContainerElement::AsFile()
{
	return dynamic_cast<ContainerFile*>(this);
}

bool dbc::ContainerElement::IsTheSame(const IContainerElement& obj) const
{
	const ContainerElement& ce = dynamic_cast<const ContainerElement&>(obj);
	return (
		m_resources == ce.m_resources &&
		m_id == ce.m_id
		);
}

bool dbc::ContainerElement::IsChildOf(const IContainerElement& obj)
{
	Refresh();

	const ContainerElement& el_obj = dynamic_cast<const ContainerElement&>(obj);
	if (el_obj.m_type == ElementTypeFile)
	{
		return false;
	}

	Error res = Exists(el_obj.m_parent_id, el_obj.m_name);
	if (res != SUCCESS)
	{
		throw ContainerException(res);
	}

	int64_t target_id = el_obj.m_id;

	if (target_id == m_id)
	{
		return false;
	}

	if (target_id == m_parent_id)
	{
		return true;
	}

	int64_t id = m_id;
	int64_t parent_id = m_parent_id;
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id FROM FileSystem WHERE id = ?;");
	while (parent_id > 0)
	{
		query.Reset();
		query.BindInt64(1, id);
		query.Step();
		parent_id = query.ColumnInt64(0);
		if (parent_id == target_id)
		{
			return true;
		}
		id = parent_id;
	}
	return false;
}

dbc::ContainerFolderGuard dbc::ContainerElement::GetParentEntry()
{
	if (m_parent_id < Container::ROOT_ID)
	{
		throw ContainerException(ERR_DB_FS, NOT_FOUND);
	}

	return ContainerFolderGuard(new ContainerFolder(m_resources, m_parent_id));
}

void dbc::ContainerElement::MoveToEntry(IContainerFolder& newParent)
{
	Refresh();

	if (m_id == Container::ROOT_ID || newParent.IsChildOf(*this))
	{
		throw ContainerException(ACTION_IS_FORBIDDEN);
	}

	ContainerElement& ce = dynamic_cast<ContainerElement&>(newParent);
	Error res = Exists(ce.m_id, m_name);
	if (res != Error(ERR_DB_FS, NOT_FOUND))
	{
		if (res == SUCCESS)
		{
			res = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, res);
	}

	SQLQuery query(ce.m_resources->GetConnection(), "UPDATE FileSystem SET parent_id = ? WHERE id = ?;");
	query.BindInt64(1, ce.m_id);
	query.BindInt64(2, m_id);
	query.Step();

	m_props.SetDateModified();
	WriteProps();
}

void dbc::ContainerElement::Remove()
{
	SQLQuery query(m_resources->GetConnection(), "DELETE FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	query.Step();
}

void dbc::ContainerElement::Rename(const std::string &new_name)
{
	if (new_name.empty() || !dbc::utils::FNameIsValid(new_name))
	{
		throw ContainerException(ERR_DB_FS, CANT_CREATE, WRONG_PARAMETERS);
	}

	Refresh();

	Error tmp = Exists(m_parent_id, new_name);
	if (tmp != Error(ERR_DB_FS, NOT_FOUND))
	{
		if (tmp == SUCCESS)
		{
			tmp = Error(ERR_DB_FS, ALREADY_EXISTS);
		}
		throw ContainerException(ERR_DB_FS, CANT_WRITE, tmp);
	}

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET name = ? WHERE id = ?;");
	query.BindText(1, new_name);
	query.BindInt64(2, m_id);
	query.Step();

	m_props.SetDateModified();
	WriteProps();

	m_name = new_name;
}

void dbc::ContainerElement::GetProperties(ElementProperties &out)
{
	Refresh();

	out = m_props;
}

void dbc::ContainerElement::ResetProperties(const std::string &tag)
{
	Refresh();

	m_props.SetTag(tag);
	m_props.SetDateModified();
	std::string props_str;
	ElementProperties::MakeString(m_props, props_str);

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
	m_parent_id = query.ColumnInt64(1);
	query.ColumnText(2, m_name);
	std::string props_str;
	query.ColumnText(3, props_str);
	ElementProperties::ParseString(props_str, m_props);
}

dbc::Error dbc::ContainerElement::Exists(int64_t parent_id, std::string name)
{
	// Returns DB_FS_NOT_FOUND as false and SUCCESS as true, or other error code if there was an error
	int count = 0;
	try
	{
		SQLQuery query(m_resources->GetConnection(), "SELECT id FROM FileSystem WHERE parent_id = ? AND name = ?;");
		query.BindInt64(1, parent_id);
		query.BindText(2, name);
		return query.Step() ? SUCCESS : Error(ERR_DB_FS, NOT_FOUND);
	}
	catch (const ContainerException &ex)
	{
		return ex.ErrType();
	}
}

void dbc::ContainerElement::WriteProps()
{
	// Writes properties about this object to the table FileSystem in 'props' column
	// Its exceptions is managed by calling functions
	SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET props = ? WHERE id = ?;");
	std::string props_str;
	ElementProperties::MakeString(m_props, props_str);
	query.BindText(1, props_str);
	query.BindInt64(2, m_id);
	query.Step();
}
