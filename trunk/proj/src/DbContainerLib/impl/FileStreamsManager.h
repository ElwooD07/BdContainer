#pragma once
#include "IContainnerResources.h"
#include <vector>

namespace dbc
{
	struct StreamInfo
	{
		StreamInfo(int64_t id = 0, int64_t fileId = 0, uint64_t order = 0, uint64_t start = 0, uint64_t size = 0, uint64_t used = 0)
		: id(id), fileId(fileId), order(order), start(start), size(size), used(used)
		{ }

		int64_t id;
		int64_t fileId;
		uint64_t order;
		uint64_t start;
		uint64_t size;
		uint64_t used;

		bool IsEmpty() const
		{
			return (id == 0 && fileId == 0 && order == 0 && start == 0 && size == 0 && used == 0);
		}
	};

	typedef std::vector<StreamInfo> StreamsChain_vt;
	typedef std::set<uint64_t> StreamsIds_st;

	class FileStreamsManager
	{
	public:
		FileStreamsManager(int64_t fileId, ContainerResources resources);

		void ReloadStreamsInfo();
		const StreamsChain_vt& GetAllStreams() const;
		const StreamsIds_st& GetUsedStreams() const;
		void ClearStreamsInfo();

		uint64_t GetSizeAvailable() const;
		uint64_t GetSizeUsed() const;

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
		// Reserve all available streams for writing to m_allStreams
		void ReserveExistingStreams(uint64_t requestedSize);
		// Used for transactional write.
		// Save all currently used streams to m_usedStreams
		void SaveUsedStreams();

		void AllocateUnusedAndNewStreams(uint64_t sizeRequested);
		// ### Space allocation
		// All these functions reserved streams in private streams list. If whole process failed you should reload this list.

		// Allocates available unused space in container. Result is the total size of allocated space.
		uint64_t AllocateUnusedStreams(uint64_t sizeRequested);

		// Gets any first large unused stream from DB and allocates it for current file.
		// Returns true if the single stream with requested size was found. Resulted stream will be the last in the private streams list.
		bool AllocateOneUnusedStream(uint64_t sizeRequested);

		// Gets all unused streams from DB. Allocates found unused streams to current file, which summary size is >= sizeRequested.
		// The rest of found unused streams will be untouched.
		uint64_t AllocateUnusedStreamsFromThisFile(uint64_t sizeRequested);
		uint64_t AllocateUnusedStreamsFromAnotherFiles(uint64_t sizeRequested);

		// Creates new big stream and allocates it for current file.
		void AllocateNewStream(uint64_t sizeRequested);

		// Used after unused streams deallocation. See its call in DeallocatePlaceAfterTransactionalWrite()
		void MergeFreeSpace();
		StreamsChain_vt::iterator MergeNextNeighbors(StreamsChain_vt::iterator start, StreamsIds_st& mergedStreamsIds);

		bool CutOffPartOfUsedStream(const StreamInfo& originalStream, uint64_t sizeRequested, StreamInfo& cuttedPart);
		void AppendStream(const StreamInfo& info);
		void UpdateStream(const StreamInfo& info);

		uint64_t MaxOrder();
		uint64_t CalculateClusterMultipleSize(uint64_t sizeRequested);
		bool FreeSpaceMeetsFragmentationLevelRequirements(uint64_t freeSpace);
		// Updates m_sizeUsed and m_sizeAvailable according to m_allStreams content
		void UpdateSizes();

	private:
		const int64_t m_fileId;
		ContainerResources m_resources;

		StreamsChain_vt m_allStreams;
		StreamsIds_st m_usedStreams;
		uint64_t m_sizeAvailable;
		uint64_t m_sizeUsed;
	};
}