#pragma once
#include "TypesPublic.h"

struct sqlite3;
struct sqlite3_stmt;

namespace dbc
{
	class Connection;
	union Error;

	class SQLQuery
	{
	public:
		explicit SQLQuery(Connection& conn, const std::string& query = "");
		~SQLQuery();

		void Prepare(const std::string& query);
		void BindBool(int column, bool value);
		void BindInt(int column, int value);
		void BindInt64(int column, int64_t value);
		void BindText(int column, const std::string& value);
		void BindBlob(int column, const RawData& data);

		bool Step(); // Returns true if sqlite API returns SQLITE_ROW
		void Reset();

		bool ColumnBool(int column);
		int ColumnInt(int column);
		int64_t ColumnInt64(int column);
		void ColumnText(int column, std::string& out);
		void ColumnBlob(int column, RawData& data);

		int64_t LastRowId();

	private:
		void CheckSTMT();
		void DecideToThrow(int errCode);

	private:
		::sqlite3* m_db;
		::sqlite3_stmt* m_stmt;
		int64_t m_lastRowId;
	};
}