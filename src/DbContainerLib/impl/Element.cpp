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
    SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, type, created, modified, meta FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, id);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, s_notFoundError);
	}
	m_parentId = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
    std::string meta;
    query.ColumnText(5, meta);
    InitElementInfo(query.ColumnInt(2), query.ColumnInt64(3), query.ColumnInt64(4), meta);
}

dbc::Element::Element(ContainerResources resources, int64_t parent_id, const std::string& name)
	: m_resources(resources), m_parentId(parent_id), m_name(name)
{
    SQLQuery query(m_resources->GetConnection(), "SELECT id, type, created, modified, meta FROM FileSystem WHERE parent_id = ? AND name = ?;");
	query.BindInt64(1, parent_id);
	query.BindText(2, name);
	if (!query.Step()) // SQLITE_DONE or SQLITE_OK, but not SQLITE_ROW, which expected
	{
		throw ContainerException(s_notFoundError);
	}
	m_id = query.ColumnInt64(0);
    std::string meta;
    query.ColumnText(4, meta);
    InitElementInfo(query.ColumnInt(1), query.ColumnInt64(2), query.ColumnInt64(3), meta);
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
        UpdateModifiedAndMetaData();
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

    UpdateModifiedAndMetaData();

	m_name = newName;
}

dbc::ElementProperties dbc::Element::GetProperties()
{
	Refresh();
    return m_props;
}

void dbc::Element::SetMetaInformation(const std::string& meta)
{
	Refresh();
    UpdateModifiedAndMetaData(meta.c_str());
}

void dbc::Element::Refresh()
{
    SQLQuery query(m_resources->GetConnection(), "SELECT parent_id, name, modified, meta FROM FileSystem WHERE id = ?;");
	query.BindInt64(1, m_id);
	if (!query.Step())
	{
		throw ContainerException(s_notFoundError);
	}
	m_parentId = query.ColumnInt64(0);
	query.ColumnText(1, m_name);
    m_props.SetDateModified(query.ColumnInt64(2));
    std::string meta;
    query.ColumnText(3, meta);
    m_props.SetMeta(meta);
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

void dbc::Element::UpdateModifiedAndMetaData(const char* meta /*= nullptr*/)
{
    m_props.SetDateModified(::time(0));
    SQLQuery query(m_resources->GetConnection());
    if (meta != nullptr)
    {
        m_props.SetMeta(meta);
        query.Prepare("UPDATE FileSystem SET modified = ?, meta = ? WHERE id = ?;");
        query.BindInt64(1, m_props.DateModified());
        query.BindText(2, m_props.Meta());
        query.BindInt64(3, m_id);
    }
    else
    {
        query.Prepare("UPDATE FileSystem SET modified = ? WHERE id = ?;");
        query.BindInt64(1, m_props.DateModified());
        query.BindInt64(2, m_id);
    }
    query.Step();
}

int64_t dbc::Element::GetId(const Element& element)
{
	return element.m_id;
}

void dbc::Element::InitElementInfo(int type, int64_t created, int64_t modified, const std::string& meta)
{
    m_type = static_cast<ElementType>(type);
	if (m_type == ElementTypeUnknown)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, ERR_DB, IS_DAMAGED);
	}
    m_props = ElementProperties(created, modified, meta);
}
