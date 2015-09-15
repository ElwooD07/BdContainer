#include "stdafx.h"
#include "ContainerInfoImpl.h"
#include "Container.h"
#include "SQLQuery.h"

dbc::ContainerInfoImpl::ContainerInfoImpl(ContainerResources resources)
: m_resources(resources)
{ }

bool dbc::ContainerInfoImpl::IsEmpty()
{
	return TotalElements() <= 1; // root does not count
}

uint64_t dbc::ContainerInfoImpl::TotalElements()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem;");
	query.Step();
	return query.ColumnInt64(0);
}

uint64_t dbc::ContainerInfoImpl::TotalElements(ElementType type)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT count(*) FROM FileSystem WHERE type = ?;");
	query.BindInt(1, type);
	query.Step();
	return query.ColumnInt64(0);
}

uint64_t dbc::ContainerInfoImpl::TotalDataSize()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT SUM(used) FROM FileStreams;");
	query.Step();
	return query.ColumnInt64(0);
}
