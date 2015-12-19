#pragma once
#include "IContainnerResources.h"
#include "ElementsSyncKeeper.h"

namespace dbc
{
	union Error;

	class ContaierResourcesImpl: public IContainerResources
	{
	public:
		ContaierResourcesImpl(IContainer& container, Connection& connection, IDataStorage& dataStorage);

		virtual bool ContainerAlive();
		virtual IContainer& GetContainer();
		virtual Connection& GetConnection();
		virtual IDataStorage& Storage();
		virtual ElementsSyncKeeper& GetSync();

		void ReportContainerDied() throw();

	private:
		void CheckUsefulnessAndThrow(Error err);

	private:
		IContainer& m_container;
		Connection& m_connection;
		IDataStorage& m_dataStorage;
		ElementsSyncKeeper m_synkKeeper;
		bool m_contaierAlive;
	};
}