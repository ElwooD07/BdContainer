#pragma once
#include "Element.h"

namespace dbc
{
	class SymLink : public Element
	{
	public:
		SymLink(ContainerResources resources, int64_t id);
		SymLink(ContainerResources resources, int64_t parentId, const std::string& name);

		std::string TargetPath() const;
		ElementGuard Target() const;
		void ChangeTarget(const std::string& newTarget);

		static Error IsTargetPathValid(const std::string& target);

	private:
		void InitTarget(const std::string& target);

	private:
		const char* m_target;
	};

	typedef std::shared_ptr<SymLink> SymLinkGuard;
}