#pragma once
#include "ContainerResources.h"
#include "impl/Container.h"
#include "impl/Connection.h"
#include "IDataStorage.h"
#include "impl/ElementsSyncKeeper.h"

namespace dbc
{
	class IContainerResources
	{
	public:
		virtual ~IContainerResources() { }

		virtual bool ContainerAlive() = 0;
		virtual Container& GetContainer() = 0;
		virtual Connection& GetConnection() = 0;
		virtual IDataStorage& Storage() = 0;
		virtual ElementsSyncKeeper& GetSync() = 0;
	};
}