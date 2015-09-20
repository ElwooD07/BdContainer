#pragma once
#include "StreamInfo.h"
#include "IContainnerResources.h"

namespace dbc
{
	class FileStreamsManager;

	class FileStreamsDefragmenter
	{
	public:
		FileStreamsDefragmenter(FileStreamsManager& streamsManager, ContainerResources resources, uint64_t fileId);

		// Used after unused streams deallocation. See its call in DeallocatePlaceAfterTransactionalWrite()
		void MergeFreeSpace();

	private:
		StreamsChain_vt::iterator MergeNextNeighbors(StreamsChain_vt::iterator start, StreamsIds_st& mergedStreamsIds);

	private:
		FileStreamsManager& m_streamsManager;
		ContainerResources m_resources;
		const uint64_t m_fileId;
	};
}