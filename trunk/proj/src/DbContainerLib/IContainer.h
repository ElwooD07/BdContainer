#pragma once
#include "IContainerElement.h"
#include "IContainerInfo.h"
#include "DataUsagePreferences.h"
#include <string>

namespace dbc
{
	class IContainer
	{
	public:
		virtual ~IContainer() { }
		virtual void Clear() = 0;
		virtual void ResetPassword(const std::string& newPassword) = 0;

		// Working with the entire files and folders
		virtual ContainerFolderGuard GetRoot() = 0;
		virtual ContainerElementGuard GetElement(const std::string &path) = 0;
		virtual ContainerInfo GetInfo() = 0;

		virtual DataUsagePreferences GetDataUsagePreferences() = 0;
		virtual void SetDataUsagePreferences(const DataUsagePreferences& prefs) = 0;
	};

}