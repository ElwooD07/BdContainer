#include "stdafx.h"
#include "SymLink.h"
#include "CommonUtils.h"
#include "FsUtils.h"
#include "ContainerException.h"
#include "IContainnerResources.h"

dbc::SymLink::SymLink(ContainerResources resources, int64_t id)
    : Link(resources, id)
	, m_target(nullptr)
{
    if (!m_props.Meta().empty())
	{
        InitTarget(m_props.Meta());
	}
}

dbc::SymLink::SymLink(ContainerResources resources, int64_t parentId, const std::string& name)
    : Link(resources, parentId, name)
	, m_target(nullptr)
{
    if (!m_props.Meta().empty())
	{
        InitTarget(m_props.Meta());
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

dbc::ElementGuard dbc::SymLink::Target()
{
    Refresh();
	if (m_target == nullptr)
	{
		return (ElementGuard(nullptr));
	}
    return m_resources->GetContainer().GetElement(m_target);
}

void dbc::SymLink::ChangeTarget(dbc::Element& newTarget)
{
    InitTarget(newTarget.Path());
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
	else if (target.front() != dbc::PATH_SEPARATOR)
	{
		return ACTION_IS_FORBIDDEN; // Path is relative
	}
	std::vector<std::string> names;
	utils::SplitWithoutDelim(target, dbc::PATH_SEPARATOR, names);
	for (auto& name : names)
	{
		if (!utils::FileNameIsValid(name))
		{
			return WRONG_PARAMETERS;
		}
	}
	return SUCCESS;
}

void dbc::SymLink::InitTarget(const std::string& target)
{
	Error err = IsTargetPathValid(target);
	if (err != SUCCESS)
	{
		throw ContainerException(err);
	}
    UpdateModifiedAndMetaData(target.c_str());
    m_target = m_props.Meta().data();
}
