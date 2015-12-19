#include "stdafx.h"
#include "File.h"
#include "FileStreamsManager.h"
#include "Types.h"
#include "Container.h"
#include "SQLQuery.h"
#include "ProxyProgressObserver.h"
#include "ContainerException.h"
#include "Logging.h"

namespace
{
	class TemporarilyFileOpener
	{
	public:
		TemporarilyFileOpener(dbc::File* file, dbc::ReadWriteAccess access)
			: m_file(file)
			, m_wasTemporarilyOpened(false)
		{
			if (!file->IsOpened())
			{
				file->Open(access);
				m_wasTemporarilyOpened = true;
			}
			else if (file->Access() != dbc::AllAccess && file->Access() != access) // File is already opened in another mode
			{
				throw dbc::ContainerException(dbc::ERR_DB_FS_ALREADY_OPENED);
			}
		}
		~TemporarilyFileOpener()
		{
			if (m_wasTemporarilyOpened)
			{
				m_file->Close();
			}
		}

	private:
		dbc::File* m_file;
		bool m_wasTemporarilyOpened;
	};
}

dbc::File::File(ContainerResources resources, int64_t id)
	: Element(resources, id), m_access(NoAccess)
{	}

dbc::File::~File()
{ /*To avoid warning C4150*/}

dbc::File::File(ContainerResources resources, int64_t parent_id, const std::string &name)
	: Element(resources, parent_id, name), m_access(NoAccess)
{	}

void dbc::File::Remove()
{
	Refresh();

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET used = ? WHERE file_id = ?;");
	query.BindInt(1, 0);
	query.BindInt64(2, m_id);
	while (query.Step());

	Element::Remove();
}

dbc::ContainerFileGuard dbc::File::Clone() const
{
	return ContainerFileGuard(new File(m_resources, m_id));
}

void dbc::File::Open(ReadWriteAccess access)
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
	if (!m_resources->GetSync().SetFileLock(m_id, access))
	{
		throw ContainerException(ERR_DB_FS, IS_LOCKED);
	}
	m_access = access;
	m_streamsManager.reset(new FileStreamsManager(m_id, m_resources));
}

bool dbc::File::IsOpened() const
{
	return m_access != NoAccess;
}

dbc::ReadWriteAccess dbc::File::Access() const
{
	return m_access;
}

void dbc::File::Close()
{
	m_resources->GetSync().ReleaseFileLock(m_id, m_access);
	m_access = NoAccess;
	m_streamsManager.reset();
}

bool dbc::File::IsEmpty() const
{
	return Size() == 0;
}

uint64_t dbc::File::Size() const
{
	if (IsOpened())
	{
		return m_streamsManager->GetSizeUsed();
	}
	else
	{
		SQLQuery query(m_resources->GetConnection(), "SELECT SUM(used) from FileStreams WHERE file_id = ?;");
		query.BindInt64(1, m_id);
		query.Step();
		return query.ColumnInt64(0);
	}
}

uint64_t dbc::File::Read(std::ostream& out, uint64_t size, IProgressObserver* observer)
{
	if (!out)
	{
		throw ContainerException(ERR_DATA_CANT_OPEN_DEST);
	}

	TemporarilyFileOpener openGuard(this, ReadAccess);
	m_streamsManager->ReloadStreamsInfo();

	if (size == 0)
	{
		size = m_streamsManager->GetSizeUsed();
	}

	ProxyProgressObserver proxyObserver(observer);
	uint64_t readTotal(0);
	const StreamsChain_vt& streams = m_streamsManager->GetAllStreams();
	StreamsChain_vt::const_iterator end = streams.end();
	for (StreamsChain_vt::const_iterator it = streams.begin(); it != end && readTotal < size; ++it)
	{
		if (it->used == 0)
		{
			continue;
		}

		uint64_t sizeToReadNow = size - readTotal;
		if (sizeToReadNow == 0)
		{
			break;
		}
		else if (it->used < sizeToReadNow)
		{
			sizeToReadNow = it->used;
		}

		proxyObserver.SetRange(static_cast<float>(readTotal) / size, static_cast<float>(readTotal + sizeToReadNow) / size);
		proxyObserver.OnProgressUpdated(0);
		uint64_t read = m_resources->Storage().Read(out, it->start, it->start + sizeToReadNow, &proxyObserver);
		if (read != sizeToReadNow)
		{
			throw ContainerException(ERR_DATA, CANT_READ);
		}
		readTotal += read;
	}

	return readTotal;
}

uint64_t dbc::File::Write(std::istream& in, uint64_t size, IProgressObserver* observer)
{
	if (!in)
	{
		throw ContainerException(ERR_DATA_CANT_OPEN_SRC);
	}

	TemporarilyFileOpener openGuard(this, WriteAccess);
	m_streamsManager->ReloadStreamsInfo();

	TransactionGuard transaction = m_resources->GetConnection().StartTransaction();
	uint64_t writtenTotal = 0;
	if (m_resources->GetContainer().GetDataUsagePreferences().TransactionalWrite())
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

void dbc::File::Clear()
{
	TemporarilyFileOpener openGuard(this, WriteAccess);
	m_streamsManager->ReloadStreamsInfo();

	SQLQuery query(m_resources->GetConnection(), "UPDATE FileStreams SET used = 0 WHERE file_id = ?;");
	query.BindInt64(1, m_id);
	query.Step();

	m_streamsManager->ReloadStreamsInfo();
}

dbc::File::SpaceUsageInfo dbc::File::GetSpaceUsageInfo()
{
	SpaceUsageInfo info;
	if (IsOpened())
	{
		m_streamsManager->ReloadStreamsInfo();
		GetSpaceUsageInfoImpl(m_streamsManager.get(), info);
	}
	else
	{
		FileStreamsManager streamsManager(m_id, m_resources);
		streamsManager.ReloadStreamsInfo();
		GetSpaceUsageInfoImpl(&streamsManager, info);
	}

	return std::move(info);
}

uint64_t dbc::File::DirectWrite(std::istream& in, uint64_t size, IProgressObserver* observer)
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

	return WriteImpl(in, size, false, observer);
}

uint64_t dbc::File::TransactionalWrite(std::istream& in, uint64_t size, IProgressObserver* observer)
{
	try
	{
		m_streamsManager->AllocatePlaceForTransactionalWrite(size);
	}
	catch (const ContainerException& ex)
	{
		WriteLog("Unable to allocate place for data: " + ex.FullMessage());
		throw ContainerException(ERR_DATA, CANT_WRITE, ex.ErrType());
	}

	uint64_t writtenTotal = WriteImpl(in, size, true, observer);
	m_streamsManager->DeallocatePlaceAfterTransactionalWrite();

	return writtenTotal;
}

uint64_t dbc::File::WriteImpl(std::istream& in, uint64_t size, bool writeOnlyToUnusedStreams, IProgressObserver* observer)
{
	ProxyProgressObserver proxyObserver(observer);
	uint64_t writtenTotal = 0;
	const StreamsChain_vt& allStreams = m_streamsManager->GetAllStreams();
	const StreamsIds_st& usedStreams = m_streamsManager->GetSavedStreams();
	for (auto stream = allStreams.begin(); stream != allStreams.end() && writtenTotal < size; ++stream)
	{
		if (writeOnlyToUnusedStreams && usedStreams.find(stream->id) != usedStreams.end())
		{
			continue;
		}

		uint64_t sizeLeftToWrite(size - writtenTotal);
		uint64_t sizeToWriteNow = (sizeLeftToWrite < stream->size) ? sizeLeftToWrite : stream->size;

		proxyObserver.SetRange(static_cast<float>(writtenTotal) / size, static_cast<float>(writtenTotal + sizeToWriteNow) / size);
		proxyObserver.OnProgressUpdated(0);
		uint64_t written = m_resources->Storage().Write(in, stream->start, stream->start + sizeToWriteNow, observer);
		writtenTotal += written;
	}

	return writtenTotal;
}

void dbc::File::GetSpaceUsageInfoImpl(FileStreamsManager* streamsManager, SpaceUsageInfo& info)
{
	const StreamsChain_vt& streams = streamsManager->GetAllStreams();
	info.streamsTotal = streams.size();
	for (const StreamInfo& stream : streams)
	{
		info.spaceAvailable += stream.size;
		if (stream.used > 0)
		{
			++info.streamsUsed;
			info.spaceUsed += stream.used;
		}
	}
}
