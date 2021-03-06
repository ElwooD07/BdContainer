#pragma once
#include "IContainerInfo.h"
#include "Element.h"
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
		virtual std::string GetPath() const = 0;

		// Working with the entire files and folders
		virtual FolderGuard GetRoot() = 0;
		virtual ElementGuard GetElement(const std::string &path) = 0;
		virtual ContainerInfo GetInfo() = 0;

		virtual DataUsagePreferences GetDataUsagePreferences() const = 0;
		virtual void SetDataUsagePreferences(const DataUsagePreferences& prefs) = 0;
	};

	typedef std::shared_ptr<IContainer> ContainerGuard;
}