#pragma once
#include "Element.h"
#include "IProgressObserver.h"

namespace dbc
{
	class Container;
	union Error;
	class FileStreamsManager;

	class File: public Element
	{
	public:
		File(ContainerResources resources, int64_t id);
		File(ContainerResources resources, int64_t parentId, const std::string& name);
		~File();

		virtual void Remove();

		FileGuard Clone() const;

		void Open(ReadWriteAccess access);
		bool IsOpened() const;
		ReadWriteAccess Access() const;
		void Close();

		bool IsEmpty() const;
		uint64_t Size() const;
		uint64_t Read(std::ostream& out, uint64_t size = 0, IProgressObserver* observer = nullptr);
		uint64_t Write(std::istream& in, uint64_t size, IProgressObserver* observer = nullptr);
		void Clear();

		struct SpaceUsageInfo
		{
			SpaceUsageInfo(uint64_t streamsTotal = 0, uint64_t streamsUsed = 0, uint64_t spaceAvailable = 0, uint64_t spaceUsed = 0)
				: streamsTotal(streamsTotal), streamsUsed(streamsUsed), spaceAvailable(spaceAvailable), spaceUsed(spaceUsed)
			{ }

			uint64_t streamsTotal;
			uint64_t streamsUsed;
			uint64_t spaceAvailable;
			uint64_t spaceUsed;
		};

		SpaceUsageInfo GetSpaceUsageInfo();

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