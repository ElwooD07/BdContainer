#pragma once
#include "TypesInternal.h"

namespace dbc
{
	class Connection;

	namespace detail
	{
		void CreateNewSavepointName(std::string& name);
	}

	class TransactionsResources
	{
		public:
			TransactionsResources(Connection* connection);
			Connection* GetConnection();
			std::string NextTransactionName();

		private:
			std::string m_lastTransactionName;
			std::mutex m_changeNameMutex;
			Connection* m_connection;
	};

	typedef std::shared_ptr<TransactionsResources> TransactionsResourcesGuard;

	class TransactionGuardImpl
	{
		NONCOPYABLE(TransactionGuardImpl);

	public:
		explicit TransactionGuardImpl(TransactionsResourcesGuard resources);
		~TransactionGuardImpl();
		void Commit();

	private:
		void CheckResources();
		void TransactionQueryImpl(const std::string& query);

	private:
		TransactionsResourcesGuard m_resources;
		bool m_committed;
		std::string m_transactionName;
	};

	typedef std::auto_ptr<TransactionGuardImpl> TransactionGuard;
}