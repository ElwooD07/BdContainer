#pragma once
#include "TransactionGuard.h"
#include "SQLQuery.h"

struct sqlite3;

namespace dbc
{
	class Connection
	{
	public:
		Connection();
		Connection(const std::string& dbPath, bool create);
		~Connection();

		void Reconnect(const std::string& dbPath);
		void Disconnect();

		TransactionGuard StartTransaction();
		void ExecQuery(const std::string& query);
		SQLQuery CreateQuery(const std::string& query = "");

		sqlite3* GetDB();

		static Error ConvertToDBCErr(int sqliteErrCode);

	private:
		void Connect(const std::string& dbPath);
		void CheckDB();

	private:
		sqlite3* m_dbPtr;
		TransactionsResourcesGuard m_transactionResources;
	};

}