#include "stdafx.h"
#include "FileStreamsDefragmenter.h"
#include "FileStreamsManager.h"

dbc::FileStreamsDefragmenter::FileStreamsDefragmenter(FileStreamsManager& streamsManager, ContainerResources resources, uint64_t fileId)
	: m_streamsManager(streamsManager)
	, m_resources(resources)
	, m_fileId(fileId)
{ }

void dbc::FileStreamsDefragmenter::MergeFreeSpace()
{
	assert(m_streamsManager.GetSizeUsed() <= m_streamsManager.GetSizeAvailable());
	uint64_t freeSpace = m_streamsManager.GetSizeAvailable() - m_streamsManager.GetSizeUsed();
	if (!m_streamsManager.FreeSpaceMeetsFragmentationLevelRequirements(freeSpace))
	{
		return;
	}

	StreamsIds_st mergedStreamsIds;
	auto nextStream = m_streamsManager.GetAllStreams().begin();
	auto allStreamsEnd = m_streamsManager.GetAllStreams().end();
	do	{
		nextStream = MergeNextNeighbors(nextStream, mergedStreamsIds);
	} while (nextStream != allStreamsEnd);

	if (mergedStreamsIds.empty())
	{
		return;
	}

	// Update merged streams
	auto mergedStreamsIdsEnd = mergedStreamsIds.end();
	for (auto stream : m_streamsManager.GetAllStreams())
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
			m_streamsManager.UpdateStream(stream);
		}
	}
}

dbc::StreamsChain_vt::iterator dbc::FileStreamsDefragmenter::MergeNextNeighbors(StreamsChain_vt::iterator start, StreamsIds_st& mergedStreamsIds)
{
	if (start->used != 0)
	{
		return start + 1;
	}

	bool startWasAdded = false;
	auto allStreamsEnd = m_streamsManager.GetAllStreams().end();
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