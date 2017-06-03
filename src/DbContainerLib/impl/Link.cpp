#include "stdafx.h"
#include "Link.h"
#include "ContainerException.h"

dbc::Link::Link(dbc::ContainerResources resources, int64_t id)
    : Element(resources, id)
{ }

dbc::Link::Link(dbc::ContainerResources resources, int64_t parentId, const std::string& name)
    : Element(resources, parentId, name)
{ }

dbc::ElementProperties dbc::Link::GetProperties()
{
    ElementGuard target = Target();
    const std::string meta = target.get() != nullptr ? target->GetProperties().Meta() : "";
    return ElementProperties(m_props.DateCreated(), m_props.DateModified(), meta);
}

void dbc::Link::SetMetaInformation(const std::string& meta)
{
    ElementGuard target = Target();
    if (target.get() != nullptr)
    {
        target->SetMetaInformation(meta);
    }
}
