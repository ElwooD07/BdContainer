#pragma once
#include "ElementProperties.h"
#include "ContainerResources.h"
#include "Types.h"
#include "ErrorCodes.h"
#include <memory>

namespace dbc
{
	class SQLQuery;

	class Folder;
	class File;
	class SymLink;
	class DirectLink;

	typedef std::shared_ptr<Folder> FolderGuard;
	typedef std::shared_ptr<File> FileGuard;

	class Element
	{
	public:
		Element(ContainerResources resources, int64_t id);
		Element(ContainerResources resources, int64_t parentId, const std::string& name);

		virtual bool Exists();
		virtual std::string Name();
		virtual std::string Path();
		virtual ElementType Type() const throw();

		virtual Folder* AsFolder() throw();
		virtual File* AsFile() throw();
		virtual SymLink* AsSymLink() throw();
		virtual DirectLink* AsDirectLink() throw();

		virtual bool IsTheSame(const Element& obj) const throw();
		virtual bool IsChildOf(const Element& obj);

		virtual FolderGuard GetParentEntry();

		virtual void MoveToEntry(Folder& newParent);
		virtual void Remove();
		virtual void Rename(const std::string& newName);

		virtual ElementProperties GetProperties();
		virtual void ResetProperties(const std::string& tag);

		static Error s_notFoundError;

	protected:
		void Refresh();
		bool Exists(int64_t id);
		Error Exists(int64_t parent_id, std::string name); // Returns s_errElementNotFound (see .cpp) as false and SUCCESS as true, or other error code if there was an error
		void WriteProps(time_t newDateModified, const char* tag = nullptr);
		void UpdateSpecificData(const RawData& specificData);
		int64_t GetId(const Element& element);

	protected:
		ContainerResources m_resources;

		int64_t m_id;
		int64_t m_parentId;
		ElementType m_type;
		std::string m_name;
		std::string m_propsStr;
		RawData m_specificData;

	private:
		void InitElementInfo(SQLQuery& query, int typeN, int propsN, int specificDataN);
	};

	typedef std::shared_ptr<Element> ElementGuard;
}