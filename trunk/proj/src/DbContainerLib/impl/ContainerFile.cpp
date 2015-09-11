#include "stdafx.h"
#include "ContainerFile.h"
#include "FileStreamsManager.h"
#include "SQLQuery.h"
#include "ContainerException.h"
#include "Logging.h"

dbc::ContainerFile::ContainerFile(ContainerResources resources, int64_t id) :
ContainerElement(resources, id), m_access(NoAccess)
{	}

dbc::ContainerFile::ContainerFile(ContainerResources resources, int64_t parent_id, const std::string &name) :
ContainerElement(resources, parent_id, name), m_access(NoAccess)
{	}

void dbc::ContainerFile::Remove()
{
	Refresh();

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET used = ? WHERE file_id = ?;");
	query.BindInt(1, 0);
	query.BindInt64(2, m_id);
	while (query.Step());

	ContainerElement::Remove();
}

dbc::ContainerFileGuard dbc::ContainerFile::Clone() const
{
	return ContainerFileGuard(new ContainerFile(m_resources, m_id));
}

void dbc::ContainerFile::Open(ReadWriteAccess access)
{
	if (access == NoAccess)
	{
		throw ContainerException(ERR_DB_FS, CANT_OPEN, WRONG_PARAMETERS);
	}
	// Already opened by this object
	if (m_access != NoAccess)
	{
		throw ContainerException(ERR_DB_FS_ALREADY_OPENED);
	}
	// Opened in another object for writing
	if ((access == WriteAccess && !m_resources->GetSync().SetWriteLock(m_id)) ||
		(access == ReadAccess && !m_resources->GetSync().SetReadLock(m_id)))
	{
		throw ContainerException(ERR_DB_FS, IS_LOCKED);
	}
	m_access = access;
	m_streamsManager.reset(new FileStreamsManager(m_id, m_resources));
}

bool dbc::ContainerFile::IsOpened()
{
	return m_access != NoAccess;
}

dbc::ReadWriteAccess dbc::ContainerFile::Access()
{
	return m_access;
}

void dbc::ContainerFile::Close()
{
	if (m_access == WriteAccess)
	{
		m_resources->GetSync().ReleaseWriteLock(m_id);
	}
	else if (m_access == ReadAccess)
	{
		m_resources->GetSync().ReleaseReadLock(m_id);
	}
	m_access = NoAccess;
	m_streamsManager.reset();
}

bool dbc::ContainerFile::IsEmpty() const
{
	return Size() == 0;
}

uint64_t dbc::ContainerFile::Size() const
{
	CheckIsOpened();
	return m_streamsManager->GetSizeUsed();
}

uint64_t dbc::ContainerFile::Read(std::ostream& out, uint64_t size, IProgressObserver* observer)
{
	CheckIsOpened();
	CheckAccess(ReadAccess);

	if (!out)
	{
		throw ContainerException(ERR_DATA_CANT_OPEN_DEST);
	}

	m_streamsManager->ReloadStreamsInfo();

	uint64_t readTotal(0);
	const StreamsChain_vt& streams = m_streamsManager->GetAllStreams();
	StreamsChain_vt::const_iterator end = streams.end();
	for (StreamsChain_vt::const_iterator it = streams.begin(); it != end; ++it)
	{
		if (it->used == 0)
		{
			continue;
		}

		uint64_t read = m_resources->Storage().Read(out, it->start, it->start + it->used, observer);
		if (read != it->used)
		{
			throw ContainerException(ERR_DATA, CANT_READ);
		}
		readTotal += read;
	}

	return readTotal;
}

uint64_t dbc::ContainerFile::Write(std::istream& in, uint64_t size, IProgressObserver* observer)
{
	CheckIsOpened();
	CheckAccess(WriteAccess);

	if (!in)
	{
		throw ContainerException(ERR_DATA_CANT_OPEN_SRC);
	}

	TransactionGuard transaction = m_resources->GetConnection().StartTransaction();
	uint64_t writtenTotal = 0;
	if (m_resources->DataUsagePrefs().TransactionalWrite())
	{
		writtenTotal = TransactionalWrite(in, size, observer);
	}
	else
	{
		writtenTotal = DirectWrite(in, size, observer);
	}
	transaction->Commit();

	return writtenTotal;
}

void dbc::ContainerFile::Clear()
{
	CheckIsOpened();
	CheckAccess(WriteAccess);

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET used = 0 WHERE file_id = ?;");
	query.BindInt64(1, m_id);
	query.Step();

	m_streamsManager->ReloadStreamsInfo();
}

void dbc::ContainerFile::CheckIsOpened() const
{
	if (m_access == NoAccess)
	{
		throw ContainerException(ERR_DB_FS_NOT_OPENED);
	}
}

void dbc::ContainerFile::CheckAccess(ReadWriteAccess access) const
{
	if (!(m_access & access))
	{
		throw ContainerException(ERR_DB_FS, NO_ACCESS);
	}
}

uint64_t dbc::ContainerFile::DirectWrite(std::istream& in, uint64_t size, IProgressObserver* observer)
{
	try
	{
		m_streamsManager->AllocatePlaceForDirectWrite(size);
	}
	catch (const ContainerException& ex)
	{
		WriteLog("Unable to allocate place for data: " + ex.FullMessage());
		throw ContainerException(ERR_DATA, CANT_WRITE, ex.ErrType());
	}

	StreamsChain_vt::const_iterator begin = m_streamsManager->GetAllStreams().begin();
	StreamsChain_vt::const_iterator end = m_streamsManager->GetAllStreams().end();
	return WriteImpl(in, begin, end, size, observer);
}

uint64_t dbc::ContainerFile::TransactionalWrite(std::istream& in, uint64_t size, IProgressObserver* observer)
{
	uint64_t firstUnusedStreamIndex = 0;
	try
	{
		firstUnusedStreamIndex = m_streamsManager->AllocatePlaceForTransactionalWrite(size);
	}
	catch (const ContainerException& ex)
	{
		WriteLog("Unable to allocate place for data: " + ex.FullMessage());
		throw ContainerException(ERR_DATA, CANT_WRITE, ex.ErrType());
	}

	StreamsChain_vt::const_iterator firstUnusedStream = m_streamsManager->GetAllStreams().begin();
	StreamsChain_vt::const_iterator end = m_streamsManager->GetAllStreams().end();
	std::advance(firstUnusedStream, firstUnusedStreamIndex);
	
	uint64_t writtenTotal = WriteImpl(in, firstUnusedStream, end, size, observer);
	m_streamsManager->MarkStreamsAsUnused(m_streamsManager->GetAllStreams().begin(), firstUnusedStream);

	return writtenTotal;
}

uint64_t dbc::ContainerFile::WriteImpl(std::istream& in, StreamsChain_vt::const_iterator begin, StreamsChain_vt::const_iterator end, uint64_t size, IProgressObserver* observer)
{
	uint64_t writtenTotal = 0;
	for (begin; begin != end && writtenTotal < size; ++begin)
	{
		uint64_t sizeLeftToWrite(size - writtenTotal);
		uint64_t sizeToWriteNow = (sizeLeftToWrite < begin->size) ? sizeLeftToWrite : begin->size;

		uint64_t written = m_resources->Storage().Write(in, begin->start, begin->start + sizeToWriteNow, observer);
		if (written != sizeToWriteNow)
		{
			Clear();
			throw ContainerException(ERR_DATA, CANT_WRITE);
		}
		writtenTotal += written;
	}

	return writtenTotal;
}
