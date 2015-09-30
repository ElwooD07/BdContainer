#include "stdafx.h"
#include "sqlite3.h"
#include "SQLQuery.h"
#include "Connection.h"
#include "ContainerException.h"
#include "Logging.h"

dbc::SQLQuery::SQLQuery(Connection &conn, const std::string &query)
	: m_db(conn.GetDB()), m_stmt(0), m_lastRowId(0)
{
	Prepare(query);
}

dbc::SQLQuery::~SQLQuery()
{
	int res = SQLITE_OK;
	if (m_stmt)
	{
		res = sqlite3_finalize(m_stmt);
	}

	std::stringstream stream;
	stream << "- SQLQuery closed; returned code - " << res << ": " << ((res != SQLITE_OK) ? sqlite3_errmsg(m_db) : "OK");
	WriteLog(stream.str());
}

void dbc::SQLQuery::Prepare(const std::string &query)
{
	if (!m_db)
	{
		throw ContainerException(SQL_DISCONNECTED);
	}
	if (m_stmt)
	{
		sqlite3_finalize(m_stmt);
	}
	if (!query.empty())
	{
		int err = sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &m_stmt, 0);

		std::stringstream stream;
		stream << "+ SQLQuery prepared: \"" << query << "\"; returned code - " << err << ": " << ((err != SQLITE_OK) ? sqlite3_errmsg(m_db) : "OK");
		WriteLog(stream.str());

		DecideToThrow(err);
	}
}

void dbc::SQLQuery::BindBool(int column, bool value)
{
	CheckSTMT();
	DecideToThrow(sqlite3_bind_int(m_stmt, column, value ? 1 : 0));
}

void dbc::SQLQuery::BindInt(int column, int value)
{
	CheckSTMT();
	DecideToThrow(sqlite3_bind_int(m_stmt, column, value));
}

void dbc::SQLQuery::BindInt64(int column, int64_t value)
{
	CheckSTMT();
	DecideToThrow(sqlite3_bind_int64(m_stmt, column, value));
}

void dbc::SQLQuery::BindText(int column, const std::string& value)
{
	CheckSTMT();
	DecideToThrow(sqlite3_bind_text(m_stmt, column, value.c_str(), value.length() * sizeof(char), 0));
}

void dbc::SQLQuery::BindBlob(int column, const BlobData& data)
{
	CheckSTMT();
	DecideToThrow(sqlite3_bind_blob(m_stmt, column, data.data(), data.size(), 0));
}

bool dbc::SQLQuery::Step()
{
	CheckSTMT();
	int err = sqlite3_step(m_stmt);
	m_lastRowId = sqlite3_last_insert_rowid(m_db);
	DecideToThrow(err);

	return err == SQLITE_ROW;
}

void dbc::SQLQuery::Reset()
{
	if (m_stmt)
	{
		DecideToThrow(sqlite3_reset(m_stmt));
	}
}

bool dbc::SQLQuery::ColumnBool(int column)
{
	CheckSTMT();
	if (sqlite3_column_int(m_stmt, column) == 0)
	{
		return false;
	}
	return true;
}

int dbc::SQLQuery::ColumnInt(int column)
{
	CheckSTMT();
	return sqlite3_column_int(m_stmt, column);
}

int64_t dbc::SQLQuery::ColumnInt64(int column)
{
	CheckSTMT();
	return sqlite3_column_int64(m_stmt, column);
}

void dbc::SQLQuery::ColumnText(int column, std::string& out)
{
	CheckSTMT();

	const char* textRef = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, column));
	if (textRef)
	{
		out = textRef;
	}
	else
	{
		out.clear();
	}
}

void dbc::SQLQuery::ColumnBlob(int column, BlobData& data)
{
	CheckSTMT();

	const char* textRef = static_cast<const char *>(sqlite3_column_blob(m_stmt, column));
	data.clear();
	try
	{
		if (textRef != nullptr)
		{
			data.assign(textRef, textRef + sqlite3_column_bytes(m_stmt, column));
		}
	}
	catch(const std::exception& ex)
	{
		throw ContainerException(ex.what(), ERR_INTERNAL);
	}
}

int64_t dbc::SQLQuery::LastRowId()
{
	return m_lastRowId;
}

void dbc::SQLQuery::CheckSTMT()
{
	if (!m_stmt)
	{
		throw ContainerException(SQL_STMT_NOT_PREPARED);
	}
}

void dbc::SQLQuery::DecideToThrow(int errCode)
{
	if (errCode != SQLITE_OK && errCode != SQLITE_ROW && errCode != SQLITE_DONE)
	{
		ContainerException ex(ErrorString(Connection::ConvertToDBCErr(errCode)), sqlite3_errmsg(m_db));

		std::stringstream ss;
		ss << "- SQLQuery throws an exception: " << ex.FullMessage();
		WriteLog(ss.str());

		throw ex;
	}
}