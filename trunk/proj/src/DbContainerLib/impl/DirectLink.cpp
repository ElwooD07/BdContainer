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
		return (ElementGuard(nullptr));
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
	if (!element.Exists())
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
		if (targetTmp > 0)
		{
			m_target = targetTmp;
		}
	}
}
