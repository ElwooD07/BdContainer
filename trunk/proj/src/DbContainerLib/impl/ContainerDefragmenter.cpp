#include "stdafx.h"
#include "ContainerDefragmenter.h"
#include "SQLQuery.h"
#include "FileStreamsUtils.h"
#include "DefragProxyProgressObserver.h"
#include "StreamInfo.h"

namespace
{
	inline void UpdateProgress(float progress, dbc::IProgressObserver* observer)
	{
		if (observer != nullptr)
		{
			observer->OnProgressUpdated(progress);
		}
	}

	inline void ReportInfo(const char* info, dbc::IProgressObserver* observer)
	{
		if (observer != nullptr)
		{
			observer->OnInfo(info);
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
	FilesFragmentation_mp fragmentation;
	CreateFragmentationLevelsMap(files, fragmentation, observer);
	return CalculateAverageFragmentation(fragmentation);
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
	proxyObserver.SetRange(static_cast<float>(0.0), static_cast<float>(0.05));
	proxyObserver.OnInfo("Collecting files info");

	FilesIds_st allFiles;
	CollectAllFileIds(allFiles);
	proxyObserver.SetRange(static_cast<float>(0.05), static_cast<float>(1.0));
	Defragment(allFiles, targetQuality, &proxyObserver);
}

void dbc::ContainerDefragmenter::Defragment(const FilesIds_st& files, DataFragmentationLevel targetQuality, IDefragProgressObserver* observer)
{
	DefragProxyProgressObserver proxyObserver(observer);
	proxyObserver.SetRange(static_cast<float>(0.0), static_cast<float>(0.1));
	proxyObserver.OnInfo("Collecting files fragmentation info");

	FilesFragmentation_mp fragmentationInfo;
	CreateFragmentationLevelsMap(files, fragmentationInfo, observer);
	float fragmentationRatio = CalculateAverageFragmentation(fragmentationInfo);
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
		proxyObserver.SetRange(static_cast<float>(0.1), static_cast<float>(1.0));
		DefragImpl(fragmentationInfo, targetQuality, &proxyObserver);
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

void dbc::ContainerDefragmenter::CreateFragmentationLevelsMap(const FilesIds_st& filesIds, FilesFragmentation_mp& fragmentation, IProgressObserver* observer)
{
	assert(fragmentation.empty());
	size_t curFile = 0;
	size_t filesTotal = filesIds.size();
	for (uint64_t fileId : filesIds)
	{
		fragmentation[fileId] = CalculateSingleFileFragmentation(fileId);
		UpdateProgress(static_cast<float>(++curFile) / filesTotal + 1, observer);
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

float dbc::ContainerDefragmenter::CalculateAverageFragmentation(const FilesFragmentation_mp& fragmentation)
{
	size_t filesTotal = fragmentation.size();
	// Kahan summation algorithm
	double sum = 0;
	double running_error = 0;
	double temp;
	double difference;
	size_t curFile = 0;
	for (auto fileFragmentation : fragmentation)
	{
		++curFile;
		difference = fileFragmentation.second;
		difference -= running_error;
		temp = sum;
		temp += difference;
		running_error = temp;
		running_error -= sum;
		running_error -= difference;
		sum = std::move(temp);
	}
	return static_cast<float>(sum) / filesTotal;
}

void dbc::ContainerDefragmenter::DefragImpl(FilesFragmentation_mp& fragmentationInfo, DataFragmentationLevel targetQuality, IDefragProgressObserver* observer)
{
	ReportInfo("Starting defragmentation", observer);

	FilesIds_st skippedFiles;
	StreamsChain_vt emptyStreams;
	SQLQuery query(m_resources->GetConnection(), "SELECT id, file_id, stream_order, start, size, used FROM FileSreams ORDER BY start");
	while (query.Step())
	{
		StreamInfo stream(query.ColumnInt64(0), query.ColumnInt64(1), query.ColumnInt64(2), query.ColumnInt64(4), query.ColumnInt64(5), query.ColumnInt64(6));
		if (stream.used == 0)
		{
			emptyStreams.push_back(stream);
			continue;
		}
		if (skippedFiles.count(stream.fileId) > 0) // this file was skipped for some reason
		{
			continue;
		}
		if (fragmentationInfo.count(stream.fileId) == 0) // file is not present in defrag list
		{
			continue;
		}
		if (InterpretFragmentationLevelValue(fragmentationInfo[stream.fileId]) < targetQuality)
		{
			skippedFiles.insert(stream.fileId);
			ReportInfo("File fragmentation level corresponds to the target fragmentation quality.", observer);
			continue;
		}
	}
}