#pragma once
#include "IContainerElement.h"
#include "IProgressObserver.h"
#include <string>
#include <iosfwd>

namespace dbc
{
	enum ReadWriteAccess
	{
		NoAccess = 0x0,
		ReadAccess = 0x1,
		WriteAccess = 0x2,
		AllAccess = 0xf
	};

	class IContainerFile : public virtual IContainerElement
	{
	public:
		virtual ContainerFileGuard Clone() const = 0;

		virtual void Open(ReadWriteAccess access) = 0;
		virtual bool IsOpened() const = 0;
		virtual ReadWriteAccess Access() const = 0;
		virtual void Close() = 0;

		virtual bool IsEmpty() const = 0;
		virtual uint64_t Size() const = 0;
		virtual uint64_t Read(std::ostream& out, uint64_t size = 0, IProgressObserver* observer = nullptr) = 0; // size = 0 means Read/Write all data
		virtual uint64_t Write(std::istream& in, uint64_t size, IProgressObserver* observer = nullptr) = 0;
		virtual void Clear() = 0;
	};
}