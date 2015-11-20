#include "stdafx.h"
#include "IDataStorage.h"
#include "FileStreamsAllocator.h"
#include "FileStreamsManager.h"
#include "SQLQuery.h"
#include "ContainerException.h"

dbc::FileStreamsAllocator::FileStreamsAllocator(FileStreamsManager& streamsManager, ContainerResources resources, uint64_t fileId)
	: m_streamsManager(streamsManager)
	, m_resources(resources)
	, m_fileId(fileId)
{ }

void dbc::FileStreamsAllocator::ReserveExistingStreams(uint64_t requestedSize)
{
	// it is so difficult because of optimization for Database
	auto allStreams = m_streamsManager.GetAllStreams();
	if (allStreams.empty())
	{
		return;
	}

	uint64_t reservedTotal(0);

	uint64_t orderMaxUsed(0); // max size was used for all streams with the order less or equal than this one
	StreamInfo* streamWithCustomUsedSpace = nullptr;

	StreamsChain_vt::const_iterator end = allStreams.end();
	for (StreamsChain_vt::iterator it = allStreams.begin(); it != end; ++it)
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

void dbc::FileStreamsAllocator::AllocateUnusedAndNewStreams(uint64_t sizeRequested)
{
	uint64_t allocated = AllocateUnusedStreams(sizeRequested);
	if (allocated < sizeRequested)
	{
		uint64_t sizeAppended = sizeRequested - allocated;
		AllocateNewStream(sizeAppended);
	}
}

uint64_t dbc::FileStreamsAllocator::AllocateUnusedStreams(uint64_t sizeRequested)
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

bool dbc::FileStreamsAllocator::AllocateOneUnusedStream(uint64_t sizeRequested)
{
	uint64_t sizeForOneStream = m_streamsManager.CalculateClusterMultipleSize(sizeRequested);

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
		if (m_streamsManager.CutOffPartOfUsedStream(oldStreamInfo, sizeRequested, cuttedStream))
		{
			m_streamsManager.AppendStream(cuttedStream);
		}
		else
		{
			return false;
		}
	}
	else // It will be fully reserved
	{
		oldStreamInfo.fileId = m_fileId;
		oldStreamInfo.used = sizeRequested;
		m_streamsManager.UpdateStream(oldStreamInfo);
		m_streamsManager.GetAllStreams().push_back(oldStreamInfo);
	}
	return true;
}


uint64_t dbc::FileStreamsAllocator::AllocateUnusedStreamsFromThisFile(uint64_t sizeRequested)
{
	uint64_t allocated = 0;
	StreamsChain_vt& allStreams = m_streamsManager.GetAllStreams();
	auto end = allStreams.end();
	for (auto stream = allStreams.begin(); stream != end && allocated < sizeRequested; ++stream)
	{
		if (stream->used == 0)
		{
			uint64_t leftToAllocate = sizeRequested - allocated;
			uint64_t allocateNow = stream->size < leftToAllocate ? stream->size : leftToAllocate;
			stream->used = allocateNow;
			m_streamsManager.UpdateStream(*stream);
			allocated += allocateNow;
		}
	}
	return allocated;
}

uint64_t dbc::FileStreamsAllocator::AllocateUnusedStreamsFromAnotherFiles(uint64_t sizeRequested)
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
		newInfo.order = m_streamsManager.MaxOrder() + 1;
		newInfo.used = newUsed;
		allocated += newUsed;

		m_streamsManager.UpdateStream(newInfo); // change old stream size to size requested
		m_streamsManager.GetAllStreams().push_back(newInfo);
	}

	return allocated;
}

void dbc::FileStreamsAllocator::AllocateNewStream(uint64_t sizeRequested)
{
	uint64_t minSize = m_streamsManager.CalculateClusterMultipleSize(sizeRequested);
	uint64_t begin = 0;
	uint64_t allocated = m_resources->Storage().Append(minSize, begin);
	if (allocated != minSize)
	{
		throw ContainerException(ERR_DATA_CANT_ALLOCATE_SPACE);
	}
	StreamInfo info(0, m_fileId, 0, begin, m_streamsManager.CalculateClusterMultipleSize(sizeRequested), sizeRequested);

	m_streamsManager.AppendStream(info);
}
