#include "stdafx.h"
#include "ContainerDefragmenter.h"
#include "SQLQuery.h"
#include "FileStreamsUtils.h"
#include "DefragProxyProgressObserver.h"

namespace
{
	inline void UpdateProgress(float progress, dbc::IProgressObserver* observer)
	{
		if (observer != nullptr)
		{
			observer->OnProgressUpdated(progress);
		}
	}
}

dbc::ContainerDefragmenter::ContainerDefragmenter(ContainerResources resources, IDataStorage* storage)
	: m_resources(resources)
	, m_storage(storage)
{
	assert(storage != nullptr);
}

float dbc::ContainerDefragmenter::CalculateFragmentationLevel(IProgressObserver* observer)
{
	UpdateProgress(0, observer);

	FilesIds_st allFiles;
	CollectAllFileIds(allFiles);

	if (!allFiles.empty())
	{
		return CalculateFragmentationLevel(allFiles, observer);
	}
	return 0.0;
}

float dbc::ContainerDefragmenter::CalculateFragmentationLevel(const FilesIds_st& files, IProgressObserver* observer)
{
	size_t filesTotal = files.size();
	// Kahan summation algorithm
	double sum = 0;
	double running_error = 0;
	double temp;
	double difference;
	size_t curFile = 0;
	for (uint64_t fileId : files)
	{
		++curFile;
		difference = CalculateSingleFileFragmentation(fileId);
		difference -= running_error;
		temp = sum;
		temp += difference;
		running_error = temp;
		running_error -= sum;
		running_error -= difference;
		sum = std::move(temp);
		UpdateProgress(static_cast<float>(curFile) / (filesTotal + 1), observer);
	}
	return static_cast<float>(sum) / filesTotal;
}

dbc::DataFragmentationLevel dbc::ContainerDefragmenter::InterpretFragmentationLevelValue(float fragmentation)
{
	if (fragmentation < 0.33333)
	{
		return DataFragmentationLevelMin;
	}
	else if (fragmentation < 0.66667)
	{
		return DataFragmentationLevelNormal;
	}
	else
	{
		return DataFragmentationLevelLarge;
	}
}

void dbc::ContainerDefragmenter::Defragment(DataFragmentationLevel targetQuality, IDefragProgressObserver* observer)
{
	DefragProxyProgressObserver proxyObserver(observer);
	proxyObserver.SetRange(0.0, 0.25);
	FilesIds_st allFiles;
	CollectAllFileIds(allFiles);
	proxyObserver.SetRange(0.25, 1.0);
	Defragment(allFiles, targetQuality, &proxyObserver);
}

void dbc::ContainerDefragmenter::Defragment(const FilesIds_st& files, DataFragmentationLevel targetQuality, IDefragProgressObserver* observer)
{
	float fragmentationRatio = CalculateFragmentationLevel(files, observer);
	DataFragmentationLevel fragmentationLevel = InterpretFragmentationLevelValue(fragmentationRatio);
	if (fragmentationLevel <= targetQuality)
	{
		if (observer != nullptr)
		{
			observer->OnInfo("Defragmentation is not needed: current fragmentation level corresponds to the target fragmentation quality.");
		}
	}
	else
	{
		// Do defrag
	}
}

void dbc::ContainerDefragmenter::CollectAllFileIds(FilesIds_st& files)
{
	assert(files.empty());
	SQLQuery query(m_resources->GetConnection(), "SELECT file_id FROM FileStreams ORDER BY id");
	while (query.Step())
	{
		files.insert(query.ColumnInt64(0));
	}
}

float dbc::ContainerDefragmenter::CalculateSingleFileFragmentation(uint64_t fileId)
{
	SQLQuery query(m_resources->GetConnection(), "SELECT size, used FROM FileStreams WHERE file_id = ? ORDER BY stream_order");
	query.BindInt64(1, fileId);
	size_t fragmentedStreams = 0, streamsTotal = 0;
	while (query.Step())
	{
		uint64_t size = query.ColumnInt(0);
		uint64_t used = query.ColumnInt(1);
		if (used != 0)
		{
			++fragmentedStreams;
			if (utils::FreeSpaceMeetsFragmentationLevelRequirements(size - used,
				m_resources->DataUsagePrefs().FragmentationLevel(),
				m_resources->DataUsagePrefs().ClusterSize()))
			{
				++fragmentedStreams; // Potentially fragmented
			}
		}
	};

	if (streamsTotal > 0)
	{
		return static_cast<float>(1.0 - (static_cast<float>(fragmentedStreams) / streamsTotal));
	}
	else
	{
		return 0.0;
	}
}