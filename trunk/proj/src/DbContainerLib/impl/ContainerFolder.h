#pragma once
#include "IContainerFolder.h"
#include "ContainerElement.h"
#include "IContainnerResources.h"

namespace dbc
{
	class Connection;

	class ContainerFolder: public IContainerFolder, public ContainerElement
	{
	public:
		ContainerFolder(ContainerResources resources, int64_t id);
		ContainerFolder(ContainerResources resources, int64_t parent_id, const std::string &name);

		virtual std::string Name(); // optimized for the root
		virtual std::string Path(); // optimized for the root
		virtual void Remove();
		virtual void Rename(const std::string &new_name);

		virtual ContainerFolderGuard Clone() const;
		virtual bool IsRoot() const;
		virtual bool HasChildren();
		virtual ContainerElementGuard GetChild(const std::string &name);
		virtual ContainerElementGuard CreateChild(const std::string& name, ElementType type, const std::string& tag);
		virtual ContainerFolderGuard CreateFolder(const std::string& name, const std::string& tag);
		virtual ContainerFileGuard CreateFile(const std::string& name, const std::string& tag);

		virtual DbcElementsIterator EnumFsEntries();

	private:
		Error RemoveFolder(Connection& owner, int64_t folder_id); // Recursive
		void CreateChildEntry(const std::string& name, ElementType type, const std::string& tag);
	};
}