#pragma once
#include "IContainnerResources.h"
#include "Container.h"

namespace dbc
{
	class ContainerElement: public virtual IContainerElement
	{
	public:
		ContainerElement(ContainerResources resources, int64_t id);
		ContainerElement(ContainerResources resources, int64_t parentId, const std::string& name);

		// from IContainerElement

		virtual bool Exists();
		virtual std::string Name();
		virtual std::string Path();
		virtual ElementType Type() const;

		virtual IContainerFolder* AsFolder();
		virtual IContainerFile* AsFile();

		virtual bool IsTheSame(const IContainerElement& obj) const;
		virtual bool IsChildOf(const IContainerElement& obj);

		virtual ContainerFolderGuard GetParentEntry();

		virtual void MoveToEntry(IContainerFolder& newParent);
		virtual void Remove();
		virtual void Rename(const std::string& newName);

		virtual void GetProperties(ElementProperties& out);
		virtual void ResetProperties(const std::string& tag);

	protected:
		ContainerResources m_resources;

		int64_t m_id;
		int64_t m_parentId;
		ElementType m_type;
		std::string m_name;
		ElementProperties m_props;

		static Error s_errElementNotFound;

	protected:
		void Refresh();
		Error Exists(int64_t parent_id, std::string name); // Returns s_errElementNotFound (see .cpp) as false and SUCCESS as true, or other error code if there was an error
		void WriteProps();
	};
}