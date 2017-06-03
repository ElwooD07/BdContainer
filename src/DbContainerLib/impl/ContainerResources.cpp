#include "stdafx.h"
#include "ContainerResourcesImpl.h"
#include "ContainerException.h"

dbc::ContaierResourcesImpl::ContaierResourcesImpl(
	Container& container,
	Connection& connection,
	IDataStorage& dataStorage)
	: m_container(container)
	, m_connection(connection)
	, m_dataStorage(dataStorage)
	, m_contaierAlive(true)
{ }

bool dbc::ContaierResourcesImpl::ContainerAlive()
{
	return m_contaierAlive;
}

dbc::Connection& dbc::ContaierResourcesImpl::GetConnection()
{
	CheckUsefulnessAndThrow(ERR_DB_NO_CONNECTION);
	return m_connection;
}

dbc::Container& dbc::ContaierResourcesImpl::GetContainer()
{
	CheckUsefulnessAndThrow(ERR_DB_NO_CONNECTION);
	return m_container;
}

dbc::IDataStorage& dbc::ContaierResourcesImpl::Storage()
{
	CheckUsefulnessAndThrow(Error(ERR_DATA, NO_ACCESS));
	return m_dataStorage;
}

dbc::ElementsSyncKeeper& dbc::ContaierResourcesImpl::GetSync()
{
	CheckUsefulnessAndThrow(CONTAINER_RESOURCES_NOT_AVAILABLE);
	return m_synkKeeper;
}

void dbc::ContaierResourcesImpl::ReportContainerDied()
{
	m_contaierAlive = false;
}

void dbc::ContaierResourcesImpl::CheckUsefulnessAndThrow(Error err)
{
	if (!m_contaierAlive)
	{
		throw ContainerException(err, OWNER_IS_MISSING);
	}
}