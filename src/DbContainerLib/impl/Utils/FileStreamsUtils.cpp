#include "stdafx.h"
#include "FileStreamsUtils.h"

bool dbc::utils::FreeSpaceMeetsFragmentationLevelRequirements(
	uint64_t freeSpace,
	DataFragmentationLevel prefferedFragmentationLevel,
	unsigned int clusterSize)
{
	return ((prefferedFragmentationLevel == DataFragmentationLevelLarge && freeSpace >= clusterSize) ||
		(prefferedFragmentationLevel == DataFragmentationLevelNormal && freeSpace >= clusterSize * 4) ||
		(prefferedFragmentationLevel == DataFragmentationLevelMin && freeSpace >= clusterSize * 8));
}