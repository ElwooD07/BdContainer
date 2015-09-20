#include "stdafx.h"
#include "FileStreamsManager.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "Logging.h"

namespace
{
	static const unsigned short s_minStreamOrder = 0;

	uint64_t GetTotalUnusedSize(dbc::Connection& connection)
	{
		dbc::SQLQuery query(connection, "SELECT SUM(size) FROM FileStreams WHERE used = 0;");
		query.Step();
		return query.ColumnInt64(0);
	}
}

dbc::FileStreamsManager::FileStreamsManager(int64_t fileId, ContainerResources resources)
: m_fileId(fileId), m_resources(resources), m_sizeAvailable(0), m_sizeUsed(0)
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

const dbc::StreamsChain_vt& dbc::FileStreamsManager::GetAllStreams() const
{
	return m_allStreams;
}

const dbc::StreamsIds_st& dbc::FileStreamsManager::GetUsedStreams() const
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

void dbc::FileStreamsManager::AllocatePlaceForDirectWrite(uint64_t size)
{
	ReserveExistingStreams(size);

	if (m_sizeAvailable >= size)
	{
		return;
	}
	AllocateUnusedAndNewStreams(size - m_sizeAvailable);
	UpdateSizes();
}

void dbc::FileStreamsManager::AllocatePlaceForTransactionalWrite(uint64_t size)
{
	SaveUsedStreams();
	AllocateUnusedAndNewStreams(size);
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
		MergeFreeSpace();
	}
}

void dbc::FileStreamsManager::ReserveExistingStreams(uint64_t requestedSize)
{
	// it is so difficult because of optimization for Database
	if (m_allStreams.empty())
	{
		return;
	}

	uint64_t reservedTotal(0);

	uint64_t orderMaxUsed(0); // max size was used for all streams with the order less or equal than this one
	StreamInfo* streamWithCustomUsedSpace = nullptr;

	StreamsChain_vt::const_iterator end = m_allStreams.end();
	for (StreamsChain_vt::iterator it = m_allStreams.begin(); it != end; ++it)
	{
		uint64_t reserve = 0;
		uint64_t leftToReserve = requestedSize - reservedTotal;
		if (leftToReserve > 0)
		{
			if (leftToReserve < it->size)
			{
				reserve = leftToReserve;
				streamWithCustomUsedSpace = &(*it);
			}
			else
			{
				reserve = it->size;
				orderMaxUsed = it->order;
			}
		}

		it->used = reserve;
		reservedTotal += reserve;
	}

	// Update streams with all used space
	SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET used = size WHERE file_id = ? AND stream_order <= ?;");
	query.BindInt64(1, m_fileId);
	query.BindInt64(2, orderMaxUsed);
	query.Step();

	// Update streams with 0 used space
	query.Prepare("UPDATE FileStreams SET used = 0 WHERE file_id = ? AND stream_order > ?;");
	query.BindInt64(1, m_fileId);
	query.BindInt64(2, orderMaxUsed);
	query.Step();

	// Update stream with custom used space
	if (streamWithCustomUsedSpace != nullptr)
	{
		query.Prepare("UPDATE FileStreams SET used = ? WHERE id = ?;");
		query.BindInt64(1, streamWithCustomUsedSpace->used);
		query.BindInt64(2, streamWithCustomUsedSpace->id);
		query.Step();
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

void dbc::FileStreamsManager::AllocateUnusedAndNewStreams(uint64_t sizeRequested)
{
	uint64_t allocated = AllocateUnusedStreams(sizeRequested);
	if (allocated < sizeRequested)
	{
		uint64_t sizeAppended = sizeRequested - allocated;
		AllocateNewStream(sizeAppended);
	}
}

uint64_t dbc::FileStreamsManager::AllocateUnusedStreams(uint64_t sizeRequested)
{
	uint64_t allocated = AllocateUnusedStreamsFromThisFile(sizeRequested);
	if (allocated < sizeRequested)
	{
		if (AllocateOneUnusedStream(sizeRequested - allocated))
		{
			return sizeRequested;
		}
		else
		{
			allocated += AllocateUnusedStreamsFromAnotherFiles(sizeRequested - allocated);
		}
	}
	return allocated;
}

bool dbc::FileStreamsManager::AllocateOneUnusedStream(uint64_t sizeRequested)
{
	uint64_t sizeForOneStream = CalculateClusterMultipleSize(sizeRequested);

	SQLQuery query(m_resources->GetConnection(), "SELECT id, file_id, stream_order, start, size, used FROM FileStreams WHERE (used = 0 AND size >= ?) OR (size - used >= ?) ORDER BY size;");
	query.BindInt64(1, sizeForOneStream);
	query.BindInt64(2, sizeForOneStream);
	if (!query.Step())
	{
		return false;
	}

	StreamInfo oldStreamInfo(query.ColumnInt64(0), query.ColumnInt64(1), query.ColumnInt64(2), query.ColumnInt64(3), query.ColumnInt64(4), query.ColumnInt64(5));
	if (oldStreamInfo.used != 0) // It will be truncated
	{
		StreamInfo cuttedStream;
		if (CutOffPartOfUsedStream(oldStreamInfo, sizeRequested, cuttedStream))
		{
			AppendStream(cuttedStream);
		}
	}
	else // It will be fully reserved
	{
		oldStreamInfo.fileId = m_fileId;
		oldStreamInfo.used = sizeRequested;
		UpdateStream(oldStreamInfo);
		m_allStreams.push_back(oldStreamInfo);
	}
	return true;
}


uint64_t dbc::FileStreamsManager::AllocateUnusedStreamsFromThisFile(uint64_t sizeRequested)
{
	uint64_t allocated = 0;
	auto end = m_allStreams.end();
	for (auto stream = m_allStreams.begin(); stream != end && allocated < sizeRequested; ++stream)
	{
		if (stream->used == 0)
		{
			uint64_t leftToAllocate = sizeRequested - allocated;
			uint64_t allocateNow = stream->size < leftToAllocate ? stream->size : leftToAllocate;
			stream->used = allocateNow;
			UpdateStream(*stream);
			allocated += allocateNow;
		}
	}
	return allocated;
}

uint64_t dbc::FileStreamsManager::AllocateUnusedStreamsFromAnotherFiles(uint64_t sizeRequested)
{
	uint64_t freeSpaceFound(0);
	StreamsChain_vt streamsToChange;
	SQLQuery query(m_resources->GetConnection(), "SELECT id, file_id, stream_order, start, size FROM FileStreams WHERE used = 0 AND file_id != ?;");
	query.BindInt64(1, m_fileId);
	while (query.Step() && freeSpaceFound < sizeRequested)
	{
		StreamInfo foundStream(query.ColumnInt64(0), query.ColumnInt64(1), query.ColumnInt64(2), query.ColumnInt64(3), query.ColumnInt64(4), 0);
		streamsToChange.push_back(foundStream);
		freeSpaceFound += foundStream.size;
	}
	if (freeSpaceFound == 0)
	{
		return 0;
	}

	uint64_t allocated = 0;

	StreamsChain_vt::const_iterator end = streamsToChange.end();
	for (StreamsChain_vt::const_iterator it = streamsToChange.begin(); it != end && allocated < sizeRequested; ++it)
	{
		uint64_t newUsed = (sizeRequested - allocated < it->size) ? sizeRequested - allocated : it->size;
		StreamInfo newInfo(*it);
		newInfo.fileId = m_fileId;
		newInfo.order = MaxOrder() + 1;
		newInfo.used = newUsed;
		allocated += newUsed;

		UpdateStream(newInfo); // change old stream size to size requested
		m_allStreams.push_back(newInfo);
	}

	return allocated;
}

void dbc::FileStreamsManager::AllocateNewStream(uint64_t sizeRequested)
{
	uint64_t minSize = CalculateClusterMultipleSize(sizeRequested);
	uint64_t begin = 0;
	uint64_t allocated = m_resources->Storage().Append(minSize, begin);
	if (allocated != minSize)
	{
		throw ContainerException(ERR_DATA_CANT_ALLOCATE_SPACE);
	}
	StreamInfo info(0, m_fileId, MaxOrder(), begin, CalculateClusterMultipleSize(sizeRequested), sizeRequested);

	AppendStream(info);
}

void dbc::FileStreamsManager::MergeFreeSpace()
{
	assert(m_sizeUsed <= m_sizeAvailable);
	uint64_t freeSpace = m_sizeAvailable - m_sizeUsed;
	if (!FreeSpaceMeetsFragmentationLevelRequirements(freeSpace))
	{
		return;
	}

	StreamsIds_st mergedStreamsIds;
	auto nextStream = m_allStreams.begin();
	auto allStreamsEnd = m_allStreams.end();
	do	{
		nextStream = MergeNextNeighbors(nextStream, mergedStreamsIds);
	} while (nextStream != allStreamsEnd);

	if (mergedStreamsIds.empty())
	{
		return;
	}

	// Update merged streams
	auto mergedStreamsIdsEnd = mergedStreamsIds.end();
	for (auto stream : m_allStreams)
	{
		if (mergedStreamsIds.find(stream.id) == mergedStreamsIdsEnd)
		{
			continue;
		}

		if (stream.size == 0)
		{
			SQLQuery query(m_resources->GetConnection(), "DELETE FROM FileStreams WHERE id = ?;");
			query.BindInt64(1, stream.id);
			query.Step();
		}
		else
		{
			UpdateStream(stream);
		}
	}
}

dbc::StreamsChain_vt::iterator dbc::FileStreamsManager::MergeNextNeighbors(StreamsChain_vt::iterator start, StreamsIds_st& mergedStreamsIds)
{
	if (start->used != 0)
	{
		return start + 1;
	}

	bool startWasAdded = false;
	auto allStreamsEnd = m_allStreams.end();
	auto neighbor = start + 1;
	for (; neighbor != allStreamsEnd; ++neighbor)
	{
		if (neighbor != allStreamsEnd && neighbor->used == 0)
		{
			start->size += neighbor->size;
			neighbor->size = 0;
			if (!startWasAdded)
			{
				mergedStreamsIds.insert(start->id);
				startWasAdded = true;
			}
			mergedStreamsIds.insert(neighbor->id);
		}
	}
	return neighbor;
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
	uint64_t clusterSize = m_resources->DataUsagePrefs().ClusterSize();
	uint64_t ratio = sizeRequested / clusterSize;
	if (ratio == 0 || sizeRequested % clusterSize > 0)
	{
		++ratio;
	}
	return clusterSize * ratio;
}

bool dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements(uint64_t freeSpace)
{
	DataFragmentationLevel prefferedFragmentationLevel = m_resources->DataUsagePrefs().FragmentationLevel();
	unsigned int clusterSize = m_resources->DataUsagePrefs().ClusterSize();
	return ((prefferedFragmentationLevel == DataFragmentationLevelLarge && freeSpace >= clusterSize) ||
			(prefferedFragmentationLevel == DataFragmentationLevelNormal && freeSpace >= clusterSize * 4) ||
			(prefferedFragmentationLevel == DataFragmentationLevelMin && freeSpace >= clusterSize * 8));
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