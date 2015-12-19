#pragma once
#include "ElementProperties.h"
#include "ContainerResources.h"
#include "Types.h"
#include "ErrorCodes.h"
#include <memory>

namespace dbc
{
	class SQLQuery;

	class ContainerFolder;
	class ContainerFile;

	typedef std::shared_ptr<ContainerFolder> ContainerFolderGuard;
	typedef std::shared_ptr<ContainerFile> ContainerFileGuard;

	class ContainerElement
	{
	public:
		ContainerElement(ContainerResources resources, int64_t id);
		ContainerElement(ContainerResources resources, int64_t parentId, const std::string& name);

		virtual bool Exists();
		virtual std::string Name();
		virtual std::string Path();
		virtual ElementType Type() const;

		virtual ContainerFolder* AsFolder();
		virtual ContainerFile* AsFile();

		virtual bool IsTheSame(const ContainerElement& obj) const;
		virtual bool IsChildOf(const ContainerElement& obj);

		virtual ContainerFolderGuard GetParentEntry();

		virtual void MoveToEntry(ContainerFolder& newParent);
		virtual void Remove();
		virtual void Rename(const std::string& newName);

		virtual void GetProperties(ElementProperties& out);
		virtual void ResetProperties(const std::string& tag);

	protected:
		void Refresh();
		Error Exists(int64_t parent_id, std::string name); // Returns s_errElementNotFound (see .cpp) as false and SUCCESS as true, or other error code if there was an error
		void WriteProps();

	protected:
		ContainerResources m_resources;

		int64_t m_id;
		int64_t m_parentId;
		ElementType m_type;
		std::string m_name;
		ElementProperties m_props;
		RawData m_specificData;

		static Error s_errElementNotFound;

	private:
		void InitElementInfo(SQLQuery& query, int typeN, int propsN, int specificDataN);
	};

	typedef std::shared_ptr<ContainerElement> ContainerElementGuard;
}