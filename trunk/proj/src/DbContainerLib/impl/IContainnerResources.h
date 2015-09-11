#pragma once
#include "Connection.h"
#include "IDataStorage.h"
#include "DataUsagePreferences.h"
#include "ElementsSyncKeeper.h"
#include <memory>

namespace dbc
{
	class IContainerResources
	{
	public:
		virtual ~IContainerResources() { }

		virtual bool ContainerAlive() = 0;
		virtual Connection& GetConnection() = 0;
		virtual IDataStorage& Storage() = 0;
		virtual DataUsagePreferences& DataUsagePrefs() = 0;
		virtual ElementsSyncKeeper& GetSync() = 0;
	};

	typedef std::shared_ptr<IContainerResources> ContainerResources;
}