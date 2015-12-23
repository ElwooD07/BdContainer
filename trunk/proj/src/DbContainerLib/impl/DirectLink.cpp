#include "stdafx.h"
#include "DirectLink.h"
#include "Folder.h"
#include "IContainnerResources.h"
#include "CommonUtils.h"
#include "ContainerException.h"

namespace
{
	const int64_t s_wrongId = -1;
}

dbc::DirectLink::DirectLink(ContainerResources resources, int64_t id)
	: Element(resources, id)
	, m_target(s_wrongId)
{
	InitTarget();
}

dbc::DirectLink::DirectLink(ContainerResources resources, int64_t parentId, const std::string& name)
	: Element(resources, parentId, name)
{
	InitTarget();
}

dbc::ElementGuard dbc::DirectLink::Target()
{
	if (m_target == s_wrongId)
	{
		throw ContainerException(LINK_IS_EMPTY);
	}
	else
	{
		SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem WHERE id = ?;");
		query.BindInt64(1, m_target);
		query.Step();
		if (query.ColumnInt(1) == 0)
		{
			throw ContainerException(LINK_IS_NOT_REFERENCEABLE);
		}
	}
	return m_resources->GetContainer().GetElement(m_target);
}

void dbc::DirectLink::ChangeTarget(Element& newTarget)
{
	if (!newTarget.Exists())
	{
		throw ContainerException(notFoundError);
	}
	m_target = GetId(newTarget);
}

dbc::Error dbc::DirectLink::IsElementReferenceable(Element& element)
{
	if (element.Type() == ElementTypeFolder && element.AsFolder()->IsRoot())
	{
		return ACTION_IS_FORBIDDEN;
	}
	else if (!element.Exists())
	{
		return notFoundError;
	}
	return SUCCESS;
}

void dbc::DirectLink::InitTarget()
{
	if (!m_specificData.empty())
	{
		std::string targetStr = utils::RawDataToString(m_specificData);
		int64_t targetTmp = utils::StringToNumber<int64_t>(targetStr);
		if (targetTmp != 0)
		{
			m_target = targetTmp;
		}
	}
}
