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

uint64_t dbc::ContainerInfoImpl::UsedSpace()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT SUM(used) FROM FileStreams;");
	query.Step();
	return query.ColumnInt64(0);
}

uint64_t dbc::ContainerInfoImpl::FreeSpace()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT SUM(size - used) FROM FileStreams;");
	query.Step();
	return query.ColumnInt64(0);
}

uint64_t dbc::ContainerInfoImpl::TotalStreams()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT COUNT(*) FROM FileStreams;");
	query.Step();
	return query.ColumnInt64(0);
}

uint64_t dbc::ContainerInfoImpl::UsedStreams()
{
	SQLQuery query(m_resources->GetConnection(), "SELECT COUNT(*) FROM FileStreams WHERE used != 0;");
	query.Step();
	return query.ColumnInt64(0);
}