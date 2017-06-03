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
    : Link(resources, id)
	, m_target(s_wrongId)
{
	InitTarget();
}

dbc::DirectLink::DirectLink(ContainerResources resources, int64_t parentId, const std::string& name)
    : Link(resources, parentId, name)
{
    InitTarget();
}

dbc::ElementGuard dbc::DirectLink::Target()
{
    Refresh();
	if (m_target == s_wrongId)
	{
		return (ElementGuard(nullptr));
	}
	if (Exists(m_target))
	{
		return m_resources->GetContainer().GetElement(m_target);
	}
	return ElementGuard();
}

void dbc::DirectLink::ChangeTarget(Element& newTarget)
{
	if (!newTarget.Exists())
	{
		throw ContainerException(s_notFoundError);
	}
	m_target = GetId(newTarget);
}

dbc::Error dbc::DirectLink::IsElementReferenceable(ElementGuard element)
{
	if (element.get() == nullptr)
	{
		return WRONG_PARAMETERS;
	}
	else if (!element->Exists())
	{
		return s_notFoundError;
	}
	return SUCCESS;
}

void dbc::DirectLink::InitTarget()
{
    if (!m_props.Meta().empty())
	{
        int64_t targetTmp = utils::StringToNumber<int64_t>(m_props.Meta());
		if (targetTmp > 0)
		{
			m_target = targetTmp;
		}
	}
}
