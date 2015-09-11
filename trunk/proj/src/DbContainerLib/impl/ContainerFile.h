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
		virtual bool IsOpened();
		virtual ReadWriteAccess Access();
		virtual void Close();

		virtual bool IsEmpty() const;
		virtual uint64_t Size() const;
		virtual uint64_t Read(std::ostream& out, uint64_t size, IProgressObserver* observer);
		virtual uint64_t Write(std::istream& in, uint64_t size, IProgressObserver* observer);
		virtual void Clear();

	private:
		void CheckIsOpened() const;
		void CheckAccess(ReadWriteAccess access) const;
		uint64_t DirectWrite(std::istream& in, uint64_t size, IProgressObserver* observer);
		uint64_t TransactionalWrite(std::istream& in, uint64_t size, IProgressObserver* observer);
		uint64_t WriteImpl(std::istream& in, StreamsChain_vt::const_iterator begin, StreamsChain_vt::const_iterator end, uint64_t size, IProgressObserver* observer);

	private:
		std::auto_ptr<FileStreamsManager> m_streamsManager;
		ReadWriteAccess m_access;
	};
}