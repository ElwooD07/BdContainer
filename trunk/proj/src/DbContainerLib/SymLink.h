#pragma once
#include "ContainerElement.h"

namespace dbc
{
	class SymLink : public ContainerElement
	{
	public:
		SymLink(ContainerResources resources, int64_t id);
		SymLink(ContainerResources resources, int64_t parentId, const std::string& name);

		std::string TargetPath() const;
		ContainerElementGuard Target() const;
		void ChangeTarget(const std::string& newTarget);

	private:
		void InitTarget();

	private:
		const char* m_target;
	};

	typedef std::shared_ptr<SymLink> SymLinkGuard;
}