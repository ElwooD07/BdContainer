#pragma once
#include "Element.h"

namespace dbc
{
	class DirectLink : public Element
	{
	public:
		DirectLink(ContainerResources resources, int64_t id);
		DirectLink(ContainerResources resources, int64_t parentId, const std::string& name);

		ElementGuard Target();
		void ChangeTarget(Element& newTarget);

		static Error IsElementReferenceable(Element& element);

	private:
		void InitTarget();

	private:
		int64_t m_target;
	};

	typedef std::shared_ptr<DirectLink> DirectLinkGuard;
}