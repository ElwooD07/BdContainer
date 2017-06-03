#pragma once
#include "StreamInfo.h"
#include "IContainnerResources.h"

namespace dbc
{
	class FileStreamsManager;

	class FileStreamsAllocator
	{
	public:
		FileStreamsAllocator(FileStreamsManager& streamsManager, ContainerResources resources, uint64_t fileId);

		// Reserve all available streams for writing to m_allStreams
		void ReserveExistingStreams(uint64_t requestedSize);
		void AllocateUnusedAndNewStreams(uint64_t sizeRequested);

	private:
		// All these functions reserve streams in streams list. If whole process failed you should reload this list.

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

	private:
		FileStreamsManager& m_streamsManager;
		ContainerResources m_resources;
		const uint64_t m_fileId;
	};
}