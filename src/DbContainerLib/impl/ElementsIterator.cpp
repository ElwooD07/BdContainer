#include "stdafx.h"
#include "ElementsIterator.h"
#include "Folder.h"
#include "File.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "IContainnerResources.h"

using namespace dbc;

namespace
{
	void GetChildrenInfo(Connection& connection, int64_t folderId, ElementInfo_vt& out)
	{
		assert(out.empty());
		int64_t tmpId;
		int64_t tmpParentId;
		int tmp_type;
		SQLQuery query(connection, "SELECT id, parent_id, type FROM FileSystem WHERE parent_id = ?;");
		query.BindInt64(1, folderId);
		while (query.Step())
		{
			tmpId = query.ColumnInt64(0);
			tmpParentId = query.ColumnInt64(1);
			tmp_type = query.ColumnInt(2);
			if (tmp_type == ElementTypeUnknown || tmp_type > ElementTypeDirectLink)
			{
				throw dbc::ContainerException(ERR_DB, IS_DAMAGED);
			}
			ElementInfo tmp_info(tmpId, tmpParentId, static_cast<ElementType>(tmp_type));
			out.push_back(tmp_info);
		}
	}
}

ElementsIterator::ElementsIterator(ContainerResources resources, int64_t folder_id)
	: m_resources(resources), m_folderId(folder_id)
{
	GetChildrenInfo(m_resources->GetConnection(), folder_id, m_info);
	m_size = m_info.size();
}

ElementGuard ElementsIterator::Next()
{
	if (!HasNext())
	{
		throw ContainerException(WRONG_PARAMETERS);
	}

	ElementInfo current = m_info[m_current++];
	return m_resources->GetContainer().CreateElementObject(current.ID, current.Type);
}