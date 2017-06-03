#include "stdafx.h"
#include "Element.h"
#include "ContainerAPI.h"
#include "Container.h"
#include "Folder.h"
#include "File.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "FsUtils.h"
#include "IContainnerResources.h"

dbc::Error dbc::Element::s_notFoundError = dbc::Error(ERR_DB_FS, NOT_FOUND);

dbc::Element::Element(ContainerResources resources, int64_t id)
	: m_resources(resources), m_id(id)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, type, props, specific_data FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, id);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, s_notFoundError);
	}
	m_parentId = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
	InitElementInfo(query, 2, 3, 4);
}

dbc::Element::Element(ContainerResources resources, int64_t parent_id, const std::string& name)
	: m_resources(resources), m_parentId(parent_id), m_name(name)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT id, type, props, specific_data FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, parent_id);
	query.BindText(2, name);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(s_notFoundError);
	}
	m_id = query.ColumnInt64(0);
	InitElementInfo(query, 1, 2, 3);
}

bool dbc::Element::Exists()
{
	return Exists(m_id);
}

std::string dbc::Element::Name()
{
	Refresh();

	return m_name;
}

std::string dbc::Element::Path()
{
	// Class Element does not contain such field as "m_path", because his path may change independently of it at any time. Thats why this func is not const and does queries to get the "current" path of the element.
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
			throw ContainerException(ERR_DB, NOT_VALID, s_notFoundError);
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

dbc::ElementType dbc::Element::Type() const
{
	return m_type;
}

dbc::Folder* dbc::Element::AsFolder()
{
	return dynamic_cast<Folder*>(this);
}

dbc::File* dbc::Element::AsFile()
{
	return dynamic_cast<File*>(this);
}

dbc::SymLink* dbc::Element::AsSymLink()
{
	return dynamic_cast<SymLink*>(this);
}

dbc::DirectLink* dbc::Element::AsDirectLink()
{
	return dynamic_cast<DirectLink*>(this);
}

bool dbc::Element::IsTheSame(const Element& obj) const
{
	return (m_resources.get() == obj.m_resources.get() && m_id == obj.m_id);
}

bool dbc::Element::IsChildOf(const Element& obj)
{
	Refresh();

	const Element& elementObj = dynamic_cast<const Element&>(obj);
	if (elementObj.m_type == ElementTypeFile)
	{
		return false;
	}

	int64_t targetId = elementObj.m_id;
	if (!Exists(targetId))
	{
		throw ContainerException(s_notFoundError);
	}

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

dbc::FolderGuard dbc::Element::GetParentEntry()
{
	if (m_parentId < Container::ROOT_ID)
	{
		throw ContainerException(s_notFoundError);
	}

	return FolderGuard(new Folder(m_resources, m_parentId));
}

void dbc::Element::MoveToEntry(Folder& newParent)
{
	Refresh();

	if (m_id == Container::ROOT_ID || newParent.IsChildOf(*this))
	{
		throw ContainerException(ACTION_IS_FORBIDDEN);
	}

	Element& elementObj = dynamic_cast<Element&>(newParent);
	try
	{
		Error res = Exists(elementObj.m_id, m_name);
		if (res != s_notFoundError)
		{
			if (res == SUCCESS)
			{
				res = Error(ERR_DB_FS, ALREADY_EXISTS);
			}
			throw ContainerException(res);
		}

		SQLQuery query(elementObj.m_resources->GetConnection(), "UPDATE FileSystem SET parent_id = ? WHERE id = ?;");
		query.BindInt64(1, elementObj.m_id);
		query.BindInt64(2, m_id);
		query.Step();
		WriteProps(::time(0));
	}
	catch (const ContainerException& ex)
	{
		throw ContainerException(ERR_DB_FS, CANT_WRITE, ex.ErrorCode());
	}
}

void dbc::Element::Remove()
{
	SQLQuery query(m_resources->GetConnection(), "DELETE FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	query.Step();
}

void dbc::Element::Rename(const std::string& newName)
{
	if (newName.empty() || !dbc::utils::FileNameIsValid(newName))
	{
		throw ContainerException(ERR_DB_FS, CANT_CREATE, WRONG_PARAMETERS);
	}

	Refresh();
	Error tmp = Exists(m_parentId, newName);
	if (tmp != s_notFoundError)
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

	WriteProps(::time(0));

	m_name = newName;
}

dbc::ElementProperties dbc::Element::GetProperties()
{
	Refresh();
	ElementProperties props;
	ElementProperties::ParseString(m_propsStr, props);
	return std::move(props);
}

void dbc::Element::ResetProperties(const std::string& tag)
{
	Refresh();
	WriteProps(::time(0), tag.c_str());
}

void dbc::Element::Refresh()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, props FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	if (!query.Step())
	{
		throw ContainerException(s_notFoundError);
	}
	m_parentId = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
	query.ColumnText(2, m_propsStr);
}

bool dbc::Element::Exists(int64_t id)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, id);
	query.Step();
	int count = query.ColumnInt(0);
	return count != 0;
}

dbc::Error dbc::Element::Exists(int64_t parent_id, std::string name)
{
	int count = 0;
	try
	{
		SQLQuery query(m_resources->GetConnection(), "SELECT id FROM FileSystem WHERE parent_id = ? AND name = ?;");
		query.BindInt64(1, parent_id);
		query.BindText(2, name);
		return query.Step() ? SUCCESS : s_notFoundError;
	}
	catch (const ContainerException& ex)
	{
		return ex.ErrorCode();
	}
}

void dbc::Element::WriteProps(time_t newDateModified, const char* tag /*= nullptr*/)
{
	ElementProperties props;
	ElementProperties::ParseString(m_propsStr, props);
	if (newDateModified != props.DateModified() || tag != nullptr)
	{
		props.SetDateModified(newDateModified);
		if (tag != nullptr)
		{
			props.SetTag(tag);
		}
		ElementProperties::MakeString(props, m_propsStr);
		SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET props = ? WHERE id = ?;");
		query.BindText(1, m_propsStr);
		query.BindInt64(2, m_id);
		query.Step();
	}
}

void dbc::Element::UpdateSpecificData(const RawData& specificData)
{
	m_specificData.reserve(specificData.size());
	SQLQuery query(m_resources->GetConnection(), "UPDATE FileSystem SET specific_data = ? WHERE id = ?");
	query.BindBlob(1, specificData);
	query.BindInt64(2, m_id);
	query.Step();
	m_specificData.assign(specificData.begin(), specificData.end());
}

int64_t dbc::Element::GetId(const Element& element)
{
	return element.m_id;
}

void dbc::Element::InitElementInfo(SQLQuery& query, int typeN, int propsN, int specificDataN)
{
	int tmpType = query.ColumnInt(typeN);
	m_type = static_cast<ElementType>(tmpType);
	if (m_type == ElementTypeUnknown)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB, IS_DAMAGED);
	}

	std::string propsStr;
	query.ColumnText(propsN, m_propsStr);
	query.ColumnBlob(specificDataN, m_specificData);
}
