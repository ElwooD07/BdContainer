#include "stdafx.h"
#include "ContainerAPI.h"
#include "Container.h"
#include "ContainerException.h"

dbc::ContainerGuard dbc::CreateContainer(const std::string& path, const std::string& password)
{
	try
	{
		return ContainerGuard(new Container(path, password, true));
	}
	catch (const ContainerException& ex)
	{
		throw ContainerException(ERR_DB, CANT_CREATE, ex.ErrorCode());
	}
}

dbc::ContainerGuard dbc::CreateContainer(const std::string& path, const std::string& password, IDataStorageGuard storage)
{
	try
	{
		return ContainerGuard(new Container(path, password, storage, true));
	}
	catch (const ContainerException& ex)
	{
		throw ContainerException(ERR_DB, CANT_CREATE, ex.ErrorCode());
	}
}

dbc::ContainerGuard dbc::Connect(const std::string& dbPath, const std::string& password)
{
	return ContainerGuard(new Container(dbPath, password, false));
}

dbc::ContainerGuard dbc::Connect(const std::string& dbPath, const std::string& password, IDataStorageGuard storage)
{
	return ContainerGuard(new Container(dbPath, password, storage, false));
}