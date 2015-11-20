#pragma once
#include "TypesInternal.h"

namespace dbc
{
	class ElementsSyncKeeper
	{
	public:
		bool SetFileLock(uint64_t fileId, ReadWriteAccess access);
		void ReleaseFileLock(uint64_t fileId, ReadWriteAccess access);

	private:
		bool SetWriteLock(uint64_t fileId);
		void ReleaseWriteLock(uint64_t fileId);
		bool SetReadLock(uint64_t fileId);
		void ReleaseReadLock(uint64_t fileId);
		bool SetReadWriteLock(uint64_t fileId);
		void ReleaseReadWriteLock(uint64_t fileId);

	private:
		std::mutex m_mutLocks;
		std::set<uint64_t> m_writeLocks; // fileId
		std::map<uint64_t, long int> m_readLocks; // fileId, locks count
	};
}