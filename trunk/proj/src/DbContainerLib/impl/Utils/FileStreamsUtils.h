#pragma once
#include "DataUsagePreferences.h"

namespace dbc
{
	namespace utils
	{
		bool FreeSpaceMeetsFragmentationLevelRequirements(
			uint64_t freeSpace,
			DataFragmentationLevel prefferedFragmentationLevel,
			unsigned int clusterSize);
	}
}