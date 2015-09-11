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
		explicit Connection(const std::string &db_path, bool create);
		~Connection();

		void Reconnect(const std::string &db_path);
		void Disconnect();

		TransactionGuard StartTransaction();
		void ExecQuery(const std::string& query);
		SQLQuery CreateQuery(const std::string& query = "");

		sqlite3* GetDB();

		static Error ConvertToDBCErr(int sqlite_err_code);

	private:
		void Connect(const std::string &db_path);
		void CheckDB();

	private:
		sqlite3 * m_db_ptr;
		TransactionsResourcesGuard m_transactionResources;
	};

}