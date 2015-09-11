#include "stdafx.h"
#include "ElementsSyncKeeper.h"

dbc::MutexLockGuard dbc::ElementsSyncKeeper::LockFileStreamsChanging()
{
	return MutexLockGuard(new MutexLock(m_mutChangeFileStreams));
}

bool dbc::ElementsSyncKeeper::SetWriteLock(uint64_t fileId)
{
	if (m_writeLocks.find(fileId) == m_writeLocks.end())
	{
		m_writeLocks.insert(fileId);
		return true;
	}
	return false;
}

void dbc::ElementsSyncKeeper::ReleaseWriteLock(uint64_t fileId)
{
	m_writeLocks.erase(fileId);
}
