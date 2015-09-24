#include "stdafx.h"
#include "ElementsSyncKeeper.h"

bool dbc::ElementsSyncKeeper::SetFileLock(uint64_t fileId, ReadWriteAccess access)
{
	switch (access)
	{
	case dbc::NoAccess:
		return false;
	case dbc::ReadAccess:
		return SetReadLock(fileId);
	case dbc::WriteAccess:
		return SetWriteLock(fileId);
	case dbc::AllAccess:
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
	case dbc::NoAccess:
		break;
	case dbc::ReadAccess:
		ReleaseReadLock(fileId);
		break;
	case dbc::WriteAccess:
		ReleaseWriteLock(fileId);
		break;
	case dbc::AllAccess:
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
