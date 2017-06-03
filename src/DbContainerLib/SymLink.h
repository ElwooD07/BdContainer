#pragma once
#include "Link.h"

namespace dbc
{
    class SymLink : public Link
	{
	public:
		SymLink(ContainerResources resources, int64_t id);
		SymLink(ContainerResources resources, int64_t parentId, const std::string& name);

        // Link
        virtual ElementGuard Target() override;
        virtual void ChangeTarget(Element& newTarget) override;

        void ChangeTarget(const std::string& newTarget);
        std::string TargetPath() const;
		static Error IsTargetPathValid(const std::string& target);

	private:
		void InitTarget(const std::string& target);

	private:
		const char* m_target;
	};

	typedef std::shared_ptr<SymLink> SymLinkGuard;
}
