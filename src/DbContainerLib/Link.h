#pragma once
#include "Element.h"

namespace dbc
{
    class Link : public Element
    {
    public:
        Link(ContainerResources resources, int64_t id);
        Link(ContainerResources resources, int64_t parentId, const std::string& name);

        virtual ElementProperties GetProperties() override;
        virtual void SetMetaInformation(const std::string& meta) override;

        virtual ElementGuard Target() = 0;
        virtual void ChangeTarget(Element& newTarget) = 0;
    };

    typedef std::shared_ptr<Link> LinkGuard;
}
