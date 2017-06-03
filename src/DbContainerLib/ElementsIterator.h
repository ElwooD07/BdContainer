#pragma once
#include "Iterator.h"
#include "Element.h"
#include "Types.h"

namespace dbc
{
	struct ElementInfo
	{
	public:
		int64_t ID;
		int64_t ParentID;
		ElementType Type;

		ElementInfo()
			: ID(-1), ParentID(-1), Type(ElementTypeUnknown)
		{ }

		ElementInfo(int64_t id, int64_t parent_id, ElementType type)
			: ID(id), ParentID(parent_id), Type(type)
		{ }
	};

	typedef std::vector<ElementInfo> ElementInfo_vt;

	class ElementsIterator: public Iterator<ElementGuard>
	{
	public:
		ElementsIterator(ContainerResources resources, int64_t folder_id);

		virtual ElementGuard Next();

	private:
		int64_t m_folderId;
		ContainerResources m_resources;
		ElementInfo_vt m_info;
	};

    using DbcElementsIterator = std::unique_ptr<Iterator<ElementGuard>>;
}
