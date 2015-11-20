#include "stdafx.h"
#include "TransactionGuard.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "Logging.h"
#include "TypesInternal.h"

void dbc::detail::CreateNewSavepointName(std::string& name)
{
	static const char minLetter = 'a';
	static const char maxLetter = 'a';

	if (name.empty())
	{
		name = minLetter;
	}
	else
	{
		char& lastLetter = name.back();
		if (name.back() != maxLetter)
		{
			++lastLetter;
		}
		else
		{
			name += minLetter;
		}
	}
}

dbc::TransactionsResources::TransactionsResources(Connection* connection)
	: m_connection(connection)
{ }

dbc::Connection* dbc::TransactionsResources::GetConnection()
{
	return m_connection;
}

std::string dbc::TransactionsResources::NextTransactionName()
{
	dbc::MutexLock lock(m_changeNameMutex);
	detail::CreateNewSavepointName(m_lastTransactionName);
	return m_lastTransactionName;
}

dbc::TransactionGuardImpl::TransactionGuardImpl(TransactionsResourcesGuard resources)
	: m_resources(resources)
	, m_committed(false)
{
	CheckResources();
	m_transactionName = m_resources->NextTransactionName();
	TransactionQueryImpl("SAVEPOINT " + m_transactionName + ";");
}

dbc::TransactionGuardImpl::~TransactionGuardImpl()
{
	try
	{
		if (!m_committed)
		{
			TransactionQueryImpl("ROLLBACK TO SAVEPOINT " + m_transactionName + ";");
		}
	}
	catch(const ContainerException& ex)
	{
		WriteLog(ex.FullMessage());
	}
	catch(const std::exception& ex)
	{
		WriteLog(ex.what());
	}
}

void dbc::TransactionGuardImpl::Commit()
{
	TransactionQueryImpl("RELEASE SAVEPOINT " + m_transactionName + ";");
	m_committed = true;
}

void dbc::TransactionGuardImpl::CheckResources()
{
	if (m_resources.get() == nullptr)
	{
		throw ContainerException(OWNER_IS_MISSING);
	}
}

void dbc::TransactionGuardImpl::TransactionQueryImpl(const std::string& queryStr)
{
	SQLQuery query(*m_resources->GetConnection(), queryStr);
	query.Step();
}
