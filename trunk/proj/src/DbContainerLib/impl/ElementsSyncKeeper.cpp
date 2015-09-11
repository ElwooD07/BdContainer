#include "stdafx.h"
#include "ElementsSyncKeeper.h"

bool dbc::ElementsSyncKeeper::SetWriteLock(uint64_t fileId)
{
	MutexLock lockWrite(m_mutWrite);
	MutexLock lockRead(m_mutRead);
	if (m_readLocks[fileId] == 0 &&
		m_writeLocks.find(fileId) == m_writeLocks.end())
	{
		m_writeLocks.insert(fileId);
		return true;
	}
	return false;
}

void dbc::ElementsSyncKeeper::ReleaseWriteLock(uint64_t fileId)
{
	MutexLock lock(m_mutWrite);
	m_writeLocks.erase(fileId);
}

bool dbc::ElementsSyncKeeper::SetReadLock(uint64_t fileId)
{
	MutexLock lock(m_mutRead);
	if (m_writeLocks.find(fileId) == m_writeLocks.end())
	{
		++m_readLocks[fileId];
		return true;
	}
	return false;
}

void dbc::ElementsSyncKeeper::ReleaseReadLock(uint64_t fileId)
{
	MutexLock lock(m_mutRead);
	if (m_readLocks[fileId] > 0)
	{
		--m_readLocks[fileId];
	}
}
