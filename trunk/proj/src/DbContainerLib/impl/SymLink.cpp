#include "stdafx.h"
#include "SymLink.h"
#include "CommonUtils.h"
#include "ContainerException.h"
#include "IContainnerResources.h"

dbc::SymLink::SymLink(ContainerResources resources, int64_t id)
	: Element(resources, id)
	, m_target(nullptr)
{
	InitTarget();
}

dbc::SymLink::SymLink(ContainerResources resources, int64_t parentId, const std::string& name)
	: Element(resources, parentId, name)
	, m_target(nullptr)
{
	InitTarget();
}

std::string dbc::SymLink::TargetPath() const
{
	return m_target;
}

dbc::ContainerElementGuard dbc::SymLink::Target() const
{
	if (m_target == nullptr)
	{
		throw ContainerException(ERR_DB_FS, IS_EMPTY);
	}
	return m_resources->GetContainer().GetElement(m_target);
}

void dbc::SymLink::ChangeTarget(const std::string& newTarget)
{
	if (newTarget.empty())
	{
		throw ContainerException(WRONG_PARAMETERS);
	}
	m_specificData = utils::StringToRawData(newTarget);
	InitTarget();
}

void dbc::SymLink::InitTarget()
{
	if (!m_specificData.empty())
	{
		m_target = reinterpret_cast<const char*>(m_specificData.data());
	}
}
