#pragma once
#include "IContainnerResources.h"

namespace dbc
{
	union Error;

	class ContaierResourcesImpl: public IContainerResources
	{
	public:
		ContaierResourcesImpl(Connection& connection, IDataStorage& dataStorage, DataUsagePreferences& dataUsagePrefs);

		virtual bool ContainerAlive();
		virtual Connection& GetConnection();
		virtual IDataStorage& Storage();
		virtual DataUsagePreferences& DataUsagePrefs();
		virtual ElementsSyncKeeper& GetSync();

		void ReportContainerDied() throw();

	private:
		void CheckUsefulnessAndThrow(Error err);

	private:
		Connection& m_connection;
		IDataStorage& m_dataStorage;
		DataUsagePreferences& m_dataUsagePrefs;
		ElementsSyncKeeper m_synkKeeper;
		bool m_contaierAlive;
	};
}