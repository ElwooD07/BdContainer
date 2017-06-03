#include "stdafx.h"
#include "Connection.h"
#include "sqlite3.h"
#include "ContainerException.h"
#include "SQLQuery.h"
#include "FsUtils.h"
#include "Logging.h"

dbc::Connection::Connection()
	: m_dbPtr(nullptr)
{
	m_transactionResources.reset(new TransactionsResources(this));
}

dbc::Connection::Connection(const std::string& dbPath, bool create)
	: m_dbPtr(nullptr)
{
	if (create && dbc::utils::FileExists(dbPath))
	{
		throw ContainerException(ERR_DB, ALREADY_EXISTS);
	}
	Connect(dbPath);

	m_transactionResources.reset(new TransactionsResources(this));
}

dbc::Connection::~Connection()
{
	Disconnect();
}

void dbc::Connection::Reconnect(const std::string& dbPath)
{
	if (!dbPath.empty())
	{
		Disconnect();
		Connect(dbPath);
	}
}

void dbc::Connection::Disconnect()
{
	int retCode = SQLITE_OK;
	if (m_dbPtr)
	{
		retCode = sqlite3_close(m_dbPtr);
		m_dbPtr = nullptr;
	}
	m_transactionResources.reset();

	std::stringstream ss;
	ss << "-- Connection closed: returned code = " << retCode << ": " << sqlite3_errstr(retCode);
	WriteLog(ss.str());
}

dbc::TransactionGuard dbc::Connection::StartTransaction()
{
	CheckDB();

	return TransactionGuard(new TransactionGuardImpl(m_transactionResources));
}

void dbc::Connection::ExecQuery(const std::string& query)
{
	CheckDB();

	char* errStr = 0;
	if (sqlite3_exec(m_dbPtr, query.c_str(), 0, 0, &errStr) != SQLITE_OK)
	{	
		throw ContainerException(ErrorString(ERR_SQL, CANT_EXEC), errStr);
	}
}

dbc::SQLQuery dbc::Connection::CreateQuery(const std::string& query /*= ""*/)
{
	CheckDB();

	return SQLQuery(*this, query);
}

sqlite3* dbc::Connection::GetDB()
{
	CheckDB();

	return m_dbPtr;
}

dbc::Error dbc::Connection::ConvertToDBCErr(int sqlite_err_code)
{
	switch (sqlite_err_code)
	{
	case SQLITE_OK:
	case SQLITE_DONE:
		return SUCCESS;
	case SQLITE_ROW:
		return SQL_ROW;
	case SQLITE_PERM:
	case SQLITE_LOCKED:
	case SQLITE_READONLY:
		return SQL_NO_ACCESS;
	case SQLITE_BUSY:
		return SQL_BUSY;
	case SQLITE_NOMEM:
		return CANT_ALLOC_MEMORY;
	case SQLITE_IOERR:
		return ERR_FS;
	case SQLITE_CORRUPT:
		return Error(ERR_DB, IS_DAMAGED);
	case SQLITE_CANTOPEN:
		return Error(ERR_DB, CANT_OPEN);
	case SQLITE_EMPTY:
		return Error(ERR_DB, IS_EMPTY);
	case SQLITE_CONSTRAINT:
	case SQLITE_INTERNAL:
		return ERR_INTERNAL;
	case SQLITE_NOTADB:
		return Error(ERR_DB, NOT_VALID);
	default:
		return ERR_SQL;
	}
}

void dbc::Connection::Connect(const std::string &db_path)
{
	int retCode = sqlite3_open_v2(db_path.c_str(), &m_dbPtr, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_PRIVATECACHE, NULL);

	std::stringstream ss;
	ss << "\n++ Connection opened: DBfile \"" << db_path << "\", returned code = " << retCode;
	WriteLog(ss.str());

	if (retCode != SQLITE_OK)
	{
		throw ContainerException(ERR_DB, CANT_OPEN, ConvertToDBCErr(retCode));
	}
}

void dbc::Connection::CheckDB()
{
	if (m_dbPtr == nullptr)
	{
		throw ContainerException(SQL_DISCONNECTED);
	}
}
