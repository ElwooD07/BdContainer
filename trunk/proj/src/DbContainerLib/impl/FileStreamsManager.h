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

	class FileStreamsManager
	{
	public:
		FileStreamsManager(int64_t fileId, ContainerResources resources);

		void ReloadStreamsInfo();
		const StreamsChain_vt& GetAllStreams() const;
		void ClearStreamsInfo();

		uint64_t GetSizeAvailable() const;
		uint64_t GetSizeUsed() const;

		// Used for non-transactional write.
		// Reserves all available streams of current file, unused streams of other files and allocates new stream if necessary.
		void AllocatePlaceForDirectWrite(uint64_t sizeRequested);
		// Used for space allocation in transactional write.
		// Reserves unused streams of current file and other files and allocates new stream if necessary.
		// Returns the index of first unused stream. All reserved streams with index < then returned value are used by previous file content,
		// i.e. only streams with index >= then returned value must be used for writing.
		// The streams with index < then returned value must be erased after successfull transactional write.
		size_t AllocatePlaceForTransactionalWrite(uint64_t sizeRequested);
		// Used before finishing transactional write for deallocating previously used streams.
		void MarkStreamsAsUnused(StreamsChain_vt::const_iterator begin, StreamsChain_vt::const_iterator end);

	private:
		void ReserveExistingStreams(uint64_t requestedSize);

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
		uint64_t AllocateManyUnusedStreams(uint64_t sizeRequested);
		// Creates new big stream and allocates it for current file.
		void AllocateNewStream(uint64_t sizeRequested);

		bool CutOffPartOfUsedStream(const StreamInfo& originalStream, uint64_t sizeRequested, StreamInfo& cuttedPart);
		void AppendStream(const StreamInfo& info);
		void UpdateStream(const StreamInfo& info);

		uint64_t MaxOrder();
		uint64_t CalculateClusterMultipleSize(uint64_t sizeRequested);
		bool FreeSpaceMeetsFragmentationLevelRequirements(uint64_t freeSpace);

	private:
		const int64_t m_fileId;
		ContainerResources m_resources;

		StreamsChain_vt m_streams;
		uint64_t m_sizeAvailable;
		uint64_t m_sizeUsed;
	};
}