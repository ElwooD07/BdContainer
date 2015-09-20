#pragma once
#include "IContainnerResources.h"
#include "StreamInfo.h"
#include "FileStreamsAllocator.h"
#include "FileStreamsDefragmenter.h"

namespace dbc
{
	class FileStreamsManager
	{
	public:
		FileStreamsManager(int64_t fileId, ContainerResources resources);

		void ReloadStreamsInfo();
		StreamsChain_vt& GetAllStreams();
		const StreamsIds_st& GetSavedStreams() const;

		uint64_t GetSizeAvailable() const;
		uint64_t GetSizeUsed() const;

		uint64_t MaxOrder();
		uint64_t CalculateClusterMultipleSize(uint64_t sizeRequested);
		bool FreeSpaceMeetsFragmentationLevelRequirements(uint64_t freeSpace);

		void AppendStream(const StreamInfo& info);
		void UpdateStream(const StreamInfo& info);
		bool CutOffPartOfUsedStream(const StreamInfo& originalStream, uint64_t sizeRequested, StreamInfo& cuttedPart);

		// Used for non-transactional write.
		// Reserves all available streams of current file, unused streams of other files and allocates new stream if necessary.
		void AllocatePlaceForDirectWrite(uint64_t sizeRequested);
		// Used for space allocation in transactional write.
		// 1. Saves previously used streams of current file
		// 2. Allocates unused streams of current file and other files and allocates new stream if necessary.
		// All unused streams in m_allStreams will be prepared for writing after running ths function.
		// All previously used streams will be saved to the m_usedStreams. Run DeallocatePlaceAfterTransactionalWrite() to free previously used streams.
		void AllocatePlaceForTransactionalWrite(uint64_t sizeRequested);
		// Used before finishing transactional write for deallocating previously used streams in m_usedStreams.
		void DeallocatePlaceAfterTransactionalWrite();

	private:
		// Used for transactional write.
		// Save all currently used streams to m_usedStreams
		void SaveUsedStreams();

		void UpdateSizes();

	private:
		const int64_t m_fileId;
		ContainerResources m_resources;
		FileStreamsAllocator m_allocator;
		FileStreamsDefragmenter m_defragmenter;

		StreamsChain_vt m_allStreams;
		StreamsIds_st m_usedStreams;
		uint64_t m_sizeAvailable;
		uint64_t m_sizeUsed;
	};
}