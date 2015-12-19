#pragma once
#include "ContainerResources.h"
#include "IContainer.h"
#include "Connection.h"
#include "IDataStorage.h"
#include "DataUsagePreferences.h"
#include "ElementsSyncKeeper.h"

namespace dbc
{
	class IContainerResources
	{
	public:
		virtual ~IContainerResources() { }

		virtual bool ContainerAlive() = 0;
		virtual IContainer& GetContainer() = 0;
		virtual Connection& GetConnection() = 0;
		virtual IDataStorage& Storage() = 0;
		virtual ElementsSyncKeeper& GetSync() = 0;
	};
}