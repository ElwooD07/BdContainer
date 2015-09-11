#pragma once
#include "IContainer.h"
#include "IContainerFolder.h"
#include "IContainerFile.h"
#include "IDataStorage.h"
#include <string>
#include <memory>
#include <limits>

namespace dbc
{
	const uint64_t WRONG_SIZE = std::numeric_limits<uint64_t>::max();
	const char PATH_SEPARATOR = '/'; // Also, this is default name for the root folder

	typedef std::shared_ptr<IContainer> ContainerGuard;

	// Creates empty container
	ContainerGuard CreateContainer(const std::string &path, const std::string &password);
	ContainerGuard CreateContainer(const std::string &path, const std::string &password, IDataStorageGuard storage);

	// Creates connection to the existing container
	ContainerGuard Connect(const std::string &dbpath, const std::string &password);
	ContainerGuard Connect(const std::string &dbpath, const std::string &password, IDataStorageGuard storage);
}