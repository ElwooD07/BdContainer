#pragma once
#include "IContainerFile.h"
#include "ContainerElement.h"
#include "FileStreamsManager.h"

namespace dbc
{
	class Container;
	union Error;

	class ContainerFile: public IContainerFile, public ContainerElement
	{
	public:
		ContainerFile(ContainerResources resources, int64_t id);
		ContainerFile(ContainerResources resources, int64_t parent_id, const std::string &name);

		virtual void Remove();

		virtual ContainerFileGuard Clone() const;

		virtual void Open(ReadWriteAccess access);
		virtual bool IsOpened() const;
		virtual ReadWriteAccess Access() const;
		virtual void Close();

		virtual bool IsEmpty() const;
		virtual uint64_t Size() const;
		virtual uint64_t Read(std::ostream& out, uint64_t size = 0, IProgressObserver* observer = nullptr);
		virtual uint64_t Write(std::istream& in, uint64_t size, IProgressObserver* observer = nullptr);
		virtual void Clear();

		virtual SpaceUsageInfo GetSpaceUsageInfo();

	private:
		uint64_t DirectWrite(std::istream& in, uint64_t size, IProgressObserver* observer);
		uint64_t TransactionalWrite(std::istream& in, uint64_t size, IProgressObserver* observer);
		uint64_t WriteImpl(std::istream& in, uint64_t size, bool writeOnlyToUnusedStreams, IProgressObserver* observer);
		void GetSpaceUsageInfoImpl(FileStreamsManager* streamsManager, SpaceUsageInfo& info);

	private:
		std::auto_ptr<FileStreamsManager> m_streamsManager;
		ReadWriteAccess m_access;
	};
}