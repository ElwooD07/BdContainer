#pragma once
#include "Types.h"

namespace dbc
{
	class ElementsSyncKeeper
	{
	public:
		MutexLockGuard LockFileStreamsChanging();
		bool SetWriteLock(uint64_t fileId);
		void ReleaseWriteLock(uint64_t fileId);

	private:
		std::mutex m_mutChangeFileStreams;
		std::set<uint64_t> m_writeLocks;
	};
}