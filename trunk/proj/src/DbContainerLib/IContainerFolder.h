#pragma once
#include "IContainerElement.h"
#include "Iterator.h"
#include <string>

namespace dbc
{
	typedef std::auto_ptr<Iterator<ContainerElementGuard> > DbcElementsIterator;

	class IContainerFolder : public virtual IContainerElement
	{
	public:
		virtual ContainerFolderGuard Clone() const = 0;
		virtual bool IsRoot() const = 0;
		virtual bool HasChildren() = 0;
		virtual ContainerElementGuard GetChild(const std::string& name) = 0;
		virtual ContainerElementGuard CreateChild(const std::string& name, ElementType type, const std::string& tag = "") = 0;
		virtual ContainerFolderGuard CreateFolder(const std::string& name, const std::string& tag = "") = 0;
		virtual ContainerFileGuard CreateFile(const std::string& name, const std::string& tag = "") = 0;

		virtual DbcElementsIterator EnumFsEntries() = 0;
	};
}