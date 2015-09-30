#pragma once
#include "ElementProperties.h"
#include <string>
#include <memory>

namespace dbc
{
	enum ElementType // Types of basic elements of container's file system
	{
		ElementTypeUnknown = 0,
		ElementTypeFolder,
		ElementTypeFile,
		ElementTypeLink // TODO: Implement this class!
	};

	class IContainerElement;
	class IContainerFolder;
	class IContainerFile;
	typedef std::shared_ptr<IContainerElement> ContainerElementGuard;
	typedef std::shared_ptr<IContainerFolder> ContainerFolderGuard;
	typedef std::shared_ptr<IContainerFile> ContainerFileGuard;

	class IContainerElement
	{
	public:
		virtual ~IContainerElement() { };

		virtual bool Exists() = 0;
		virtual std::string Name() = 0;
		virtual std::string Path() = 0;
		virtual ElementType Type() const = 0;

		virtual IContainerFolder* AsFolder() = 0;
		virtual IContainerFile* AsFile() = 0;

		virtual bool IsTheSame(const IContainerElement& obj) const = 0;
		virtual bool IsChildOf(const IContainerElement& obj) = 0;

		virtual ContainerFolderGuard GetParentEntry() = 0;

		virtual void MoveToEntry(IContainerFolder& newParent) = 0;
		virtual void Remove() = 0;
		virtual void Rename(const std::string& new_name) = 0;

		virtual void GetProperties(ElementProperties& out) = 0;
		virtual void ResetProperties(const std::string& tag) = 0;
	};
}
