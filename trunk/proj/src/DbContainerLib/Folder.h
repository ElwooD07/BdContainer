#pragma once
#include "Element.h"
#include "ElementsIterator.h"
#include "SymLink.h"

namespace dbc
{
	class Connection;

	class Folder: public Element
	{
	public:
		Folder(ContainerResources resources, int64_t id);
		Folder(ContainerResources resources, int64_t parentId, const std::string& name);

		virtual std::string Name(); // optimized for the root
		virtual std::string Path(); // optimized for the root
		virtual void Remove();
		virtual void Rename(const std::string& newName);

		ContainerFolderGuard Clone() const;
		bool IsRoot() const;
		bool HasChildren();
		ContainerElementGuard GetChild(const std::string& name);
		ContainerElementGuard CreateChild(const std::string& name, ElementType type, const std::string& tag = "");
		ContainerFolderGuard CreateFolder(const std::string& name, const std::string& tag = "");
		ContainerFileGuard CreateFile(const std::string& name, const std::string& tag = "");

		SymLinkGuard CreateSymLink(const std::string& name, const std::string& targetPath, const std::string& tag = "");

		DbcElementsIterator EnumFsEntries();

	private:
		Error RemoveFolder(Connection& owner, int64_t folderId); // Recursive
		void CreateChildEntry(const std::string& name, ElementType type, const std::string& tag, const std::string& specificData = "");
	};
}