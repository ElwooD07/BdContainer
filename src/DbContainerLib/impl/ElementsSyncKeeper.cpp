#include "stdafx.h"
#include "ElementsSyncKeeper.h"
#include "Types.h"

bool dbc::ElementsSyncKeeper::SetFileLock(uint64_t fileId, ReadWriteAccess access)
{
	switch (access)
	{
	case NoAccess:
		return false;
	case ReadAccess:
		return SetReadLock(fileId);
	case WriteAccess:
		return SetWriteLock(fileId);
	case AllAccess:
		return SetReadWriteLock(fileId);
	default:
		assert(!"Unknown file lock type");
		return false;
	}
}

void dbc::ElementsSyncKeeper::ReleaseFileLock(uint64_t fileId, ReadWriteAccess access)
{
	switch (access)
	{
	case NoAccess:
		break;
	case ReadAccess:
		ReleaseReadLock(fileId);
		break;
	case WriteAccess:
		ReleaseWriteLock(fileId);
		break;
	case AllAccess:
		ReleaseReadWriteLock(fileId);
		break;
	default:
		assert(!"Unknown file lock type");
	}
}

bool dbc::ElementsSyncKeeper::SetWriteLock(uint64_t fileId)
{
	MutexLock lock(m_mutLocks);
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
	MutexLock lock(m_mutLocks);
	m_writeLocks.erase(fileId);
}

bool dbc::ElementsSyncKeeper::SetReadLock(uint64_t fileId)
{
	MutexLock lock(m_mutLocks);
	if (m_writeLocks.find(fileId) == m_writeLocks.end())
	{
		++m_readLocks[fileId];
		return true;
	}
	return false;
}

void dbc::ElementsSyncKeeper::ReleaseReadLock(uint64_t fileId)
{
	MutexLock lock(m_mutLocks);
	--m_readLocks[fileId];
	assert(m_readLocks[fileId] >= 0);
}

bool dbc::ElementsSyncKeeper::SetReadWriteLock(uint64_t fileId)
{
	MutexLock lock(m_mutLocks);
	if (m_readLocks[fileId] == 0
		&& m_writeLocks.find(fileId) == m_writeLocks.end())
	{
		++m_readLocks[fileId];
		m_writeLocks.insert(fileId);
		return true;
	}
	return false;
}

void dbc::ElementsSyncKeeper::ReleaseReadWriteLock(uint64_t fileId)
{
	MutexLock lock(m_mutLocks);
	m_writeLocks.erase(fileId);
	--m_readLocks[fileId];
	assert(m_readLocks[fileId] >= 0);
}
