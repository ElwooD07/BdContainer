#pragma once
#include "Types.h"

namespace dbc
{
	class ElementsSyncKeeper
	{
	public:
		bool SetWriteLock(uint64_t fileId);
		void ReleaseWriteLock(uint64_t fileId);
		bool SetReadLock(uint64_t fileId);
		void ReleaseReadLock(uint64_t fileId);

	private:
		std::mutex m_mutWrite;
		std::mutex m_mutRead;
		std::set<uint64_t> m_writeLocks; // fileId
		std::map<uint64_t, size_t> m_readLocks; // fileId, locks count
	};
}