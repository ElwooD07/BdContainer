#pragma once
#include "IContainer.h"
#include "IDataStorage.h"
#include "ContainerFolder.h"
#include "ContainerFile.h"
#include <string>
#include <limits>

namespace dbc
{
	// Creates empty container
	ContainerGuard CreateContainer(const std::string &path, const std::string &password);
	ContainerGuard CreateContainer(const std::string &path, const std::string &password, IDataStorageGuard storage);

	// Creates connection to the existing container
	ContainerGuard Connect(const std::string &dbpath, const std::string &password);
	ContainerGuard Connect(const std::string &dbpath, const std::string &password, IDataStorageGuard storage);
}