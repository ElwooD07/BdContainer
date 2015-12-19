#pragma once
#include <vector>
#include <cstdint>

namespace dbc
{
	enum ElementType // Types of basic elements of container's file system
	{
		ElementTypeUnknown = 0,
		ElementTypeFolder,
		ElementTypeFile,
		ElementTypeSymLink,
		ElementTypeDirectLink
	};

	enum ReadWriteAccess
	{
		NoAccess = 0x0,
		ReadAccess = 0x1,
		WriteAccess = 0x2,
		AllAccess = 0xf
	};

	const uint64_t WRONG_SIZE = std::numeric_limits<uint64_t>::max();
	const char PATH_SEPARATOR = '/'; // Also, this is default name for the root folder

	typedef std::vector<uint8_t> RawData;
}