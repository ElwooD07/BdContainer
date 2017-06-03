#pragma once
#include "Link.h"

namespace dbc
{
    class DirectLink : public Link
	{
	public:
		DirectLink(ContainerResources resources, int64_t id);
		DirectLink(ContainerResources resources, int64_t parentId, const std::string& name);

        // Link
        virtual ElementGuard Target() override;
        virtual void ChangeTarget(Element& newTarget) override;

		static Error IsElementReferenceable(ElementGuard element);

	private:
		void InitTarget();

	private:
		int64_t m_target;
	};

	typedef std::shared_ptr<DirectLink> DirectLinkGuard;
}
