#include "stdafx.h"
#include "FileStreamsManager.h"
#include "SQLQuery.h"
#include "FileStreamsUtils.h"
#include "ContainerException.h"

namespace
{
	static const unsigned short s_minStreamOrder = 0;
}

dbc::FileStreamsManager::FileStreamsManager(int64_t fileId, ContainerResources resources)
	: m_fileId(fileId)
	, m_resources(resources)
	, m_allocator(*this, resources, fileId)
	, m_sizeAvailable(0)
	, m_sizeUsed(0)
{ }

void dbc::FileStreamsManager::ReloadStreamsInfo()
{
	m_allStreams.clear();
	m_usedStreams.clear();
	m_sizeAvailable = 0;
	m_sizeUsed = 0;

	SQLQuery query(m_resources->GetConnection(), "SELECT id, stream_order, start, size, used FROM FileStreams WHERE file_id = ? ORDER BY stream_order;");
	query.BindInt64(1, m_fileId);
	while (query.Step())
	{
		StreamInfo info(query.ColumnInt64(0), m_fileId, query.ColumnInt64(1), query.ColumnInt64(2), query.ColumnInt64(3), query.ColumnInt64(4));
		m_allStreams.push_back(info);
		m_sizeAvailable += info.size;
		m_sizeUsed += info.used;
	}
}

dbc::StreamsChain_vt& dbc::FileStreamsManager::GetAllStreams()
{
	return m_allStreams;
}

const dbc::StreamsIds_st& dbc::FileStreamsManager::GetSavedStreams() const
{
	return m_usedStreams;
}

uint64_t dbc::FileStreamsManager::GetSizeAvailable() const
{
	return m_sizeAvailable;
}

uint64_t dbc::FileStreamsManager::GetSizeUsed() const
{
	return m_sizeUsed;
}

uint64_t dbc::FileStreamsManager::MaxOrder()
{
	if (m_allStreams.empty())
	{
		return s_minStreamOrder;
	}
	else
	{
		return m_allStreams.back().order;
	}
}

uint64_t dbc::FileStreamsManager::CalculateClusterMultipleSize(uint64_t sizeRequested)
{
	uint64_t clusterSize = m_resources->GetContainer().GetDataUsagePreferences().ClusterSize();
	uint64_t ratio = sizeRequested / clusterSize;
	if (ratio == 0 || sizeRequested % clusterSize > 0)
	{
		++ratio;
	}
	return clusterSize * ratio;
}

bool dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements(uint64_t freeSpace)
{
	return utils::FreeSpaceMeetsFragmentationLevelRequirements(freeSpace,
		m_resources->GetContainer().GetDataUsagePreferences().FragmentationLevel(),
		m_resources->GetContainer().GetDataUsagePreferences().ClusterSize());
}

void dbc::FileStreamsManager::AppendStream(const StreamInfo& info)
{
	StreamInfo newAppendedStream(info);
	newAppendedStream.fileId = m_fileId;
	newAppendedStream.order = MaxOrder() + 1;

	SQLQuery query(m_resources->GetConnection(), "INSERT INTO FileStreams(file_id, stream_order, start, size, used) VALUES (?, ?, ?, ?, ?);");
	query.BindInt64(1, newAppendedStream.fileId);
	query.BindInt64(2, newAppendedStream.order);
	query.BindInt64(3, newAppendedStream.start);
	query.BindInt64(4, newAppendedStream.size);
	query.BindInt64(5, newAppendedStream.used);
	query.Step();
	newAppendedStream.id = query.LastRowId();

	m_allStreams.push_back(newAppendedStream);
}

void dbc::FileStreamsManager::UpdateStream(const dbc::StreamInfo& info)
{
	dbc::SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET file_id = ?, stream_order = ?, start = ?, size = ?, used = ? WHERE id = ?;");
	query.BindInt64(1, info.fileId);
	query.BindInt64(2, info.order);
	query.BindInt64(3, info.start);
	query.BindInt64(4, info.size);
	query.BindInt64(5, info.used);
	query.BindInt64(6, info.id);
	query.Step();
}

bool dbc::FileStreamsManager::CutOffPartOfUsedStream(const StreamInfo& originalStream, uint64_t sizeRequested, StreamInfo& cuttedPart)
{
	assert(!originalStream.IsEmpty() && cuttedPart.IsEmpty());

	uint64_t dataUsedClusterMultiple = CalculateClusterMultipleSize(originalStream.used);
	if (dataUsedClusterMultiple >= originalStream.size) // This might be happened if cluster size is changed
	{
		return false;
	}
	uint64_t dataFree = originalStream.size - dataUsedClusterMultiple;
	if (FreeSpaceMeetsFragmentationLevelRequirements(dataFree))
	{
		StreamInfo newStreamInfo1(originalStream.id, originalStream.fileId, originalStream.order, originalStream.start, dataUsedClusterMultiple, originalStream.used);
		StreamInfo newStreamInfo2(originalStream.id, m_fileId, originalStream.order + 1, originalStream.start + dataUsedClusterMultiple, dataFree, sizeRequested);

		UpdateStream(newStreamInfo1);
		std::swap(cuttedPart, newStreamInfo2);
		return true;
	}
	return false;
}

void dbc::FileStreamsManager::AllocatePlaceForDirectWrite(uint64_t size)
{
	m_allocator.ReserveExistingStreams(size);

	if (m_sizeAvailable >= size)
	{
		return;
	}
	m_allocator.AllocateUnusedAndNewStreams(size - m_sizeAvailable);
	UpdateSizes();
}

void dbc::FileStreamsManager::AllocatePlaceForTransactionalWrite(uint64_t size)
{
	SaveUsedStreams();
	m_allocator.AllocateUnusedAndNewStreams(size);
}

void dbc::FileStreamsManager::DeallocatePlaceAfterTransactionalWrite()
{
	if (!m_usedStreams.empty())
	{
		SQLQuery query(m_resources->GetConnection());
		auto allStreamsEnd = m_allStreams.end();
		for (auto stream = m_allStreams.begin(); stream != allStreamsEnd; ++stream)
		{
			if (m_usedStreams.find(stream->id) != m_usedStreams.end())
			{
				query.Prepare("UPDATE FileStreams SET used = 0 WHERE id = ?;");
				query.BindInt64(1, stream->id);
				query.Step();
				stream->used = 0;
			}
		}
		UpdateSizes();
	}
}

void dbc::FileStreamsManager::SaveUsedStreams()
{
	m_usedStreams.clear();
	for (auto stream : m_allStreams)
	{
		if (stream.used != 0)
		{
			m_usedStreams.insert(stream.id);
		}
	}
}

void dbc::FileStreamsManager::UpdateSizes()
{
	m_sizeUsed = 0;
	m_sizeAvailable = 0;
	for (auto stream : m_allStreams)
	{
		m_sizeUsed += stream.used;
		m_sizeAvailable += stream.size;
	}
}