#pragma once
#include "Element.h"
#include "ElementsIterator.h"
#include "SymLink.h"
#include "DirectLink.h"

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

		FolderGuard Clone() const;
		bool IsRoot() const;
		bool HasChildren();
		ElementGuard GetChild(const std::string& name);
        ElementGuard CreateChild(const std::string& name, ElementType type, const std::string& meta = "");
        FolderGuard CreateFolder(const std::string& name, const std::string& meta = "");
        FileGuard CreateFile(const std::string& name, const std::string& meta = "");

        SymLinkGuard CreateSymLink(const std::string& name, const std::string& targetPath);
        DirectLinkGuard CreateDirectLink(const std::string& name, const ElementGuard target);

		DbcElementsIterator EnumFsEntries();

	private:
		Error RemoveFolder(int64_t folderId); // Recursive
        void CreateChildEntry(const std::string& name, ElementType type, const std::string& meta);
	};
}
