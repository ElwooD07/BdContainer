#include "stdafx.h"
#include "SymLink.h"
#include "CommonUtils.h"
#include "ContainerException.h"
#include "IContainnerResources.h"

dbc::SymLink::SymLink(ContainerResources resources, int64_t id)
	: Element(resources, id)
	, m_target(nullptr)
{
	if (!m_specificData.empty())
	{
		InitTarget(utils::RawDataToString(m_specificData));
	}
}

dbc::SymLink::SymLink(ContainerResources resources, int64_t parentId, const std::string& name)
	: Element(resources, parentId, name)
	, m_target(nullptr)
{
	if (!m_specificData.empty())
	{
		InitTarget(utils::RawDataToString(m_specificData));
	}
}

std::string dbc::SymLink::TargetPath() const
{
	if (m_target == nullptr)
	{
		return "";
	}
	return m_target;
}

dbc::ElementGuard dbc::SymLink::Target() const
{
	if (m_target == nullptr)
	{
		throw ContainerException(ERR_DB_FS, IS_EMPTY);
	}
	return m_resources->GetContainer().GetElement(m_target);
}

void dbc::SymLink::ChangeTarget(const std::string& newTarget)
{
	InitTarget(newTarget);
}

dbc::Error dbc::SymLink::IsTargetPathValid(const std::string& target)
{
	if (target.empty())
	{
		return WRONG_PARAMETERS;
	}
	else if (target == std::string({ dbc::PATH_SEPARATOR }))
	{
		return ACTION_IS_FORBIDDEN; // Can't create link to the root
	}
	else
	{
		return SUCCESS;
	}
}

void dbc::SymLink::InitTarget(const std::string& target)
{
	Error err = IsTargetPathValid(target);
	if (err != SUCCESS)
	{
		throw ContainerException(err);
	}
	UpdateSpecificData(utils::StringToRawData(target));
	m_target = reinterpret_cast<const char*>(m_specificData.data());
}
