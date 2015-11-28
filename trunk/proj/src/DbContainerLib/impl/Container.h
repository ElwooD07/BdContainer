#pragma once
#include "Connection.h"
#include "IContainer.h"
#include "IDataStorage.h"
#include "IContainnerResources.h"

struct sqlite3;

namespace dbc
{
	enum ElementType;
	class ContainerElement;

	class Container: public IContainer
	{
		NONCOPYABLE(Container);

	public:
		static const int ROOT_ID; // Default id for the root folder in FileSystem table
		
		Container(const std::string& path, const std::string& password, bool create = false);
		Container(const std::string& path, const std::string& password, IDataStorageGuard storage, bool create = false);
		~Container();

		// from IContainer
		virtual void Clear();
		virtual void ResetPassword(const std::string& newPassword);
		virtual std::string GetPath() const;

		virtual ContainerFolderGuard GetRoot();
		virtual ContainerElementGuard GetElement(const std::string& path);
		virtual ContainerInfo GetInfo();

		virtual DataUsagePreferences GetDataUsagePreferences() const;
		virtual void SetDataUsagePreferences(const DataUsagePreferences& prefs);
		// ~from IContainer

	private:
		void PrepareContainer(const std::string &password, bool create);
		void ReadSets(RawData& storageData);
		void SaveStorageData();

	private:
		Connection m_connection; // Connection guard. It contains the database pointer and the path to the database file.
		std::string m_dbFile;
		IDataStorageGuard m_storage;
		DataUsagePreferences m_dataUsagePrefs;

		ContainerResources m_resources;
	};
}
