#pragma once
#include <iosfwd>
#include <cstdint>
#include <string>
#include <memory>
#include "IProgressObserver.h"
#include "Types.h"

namespace dbc
{
	class IDataStorage
	{
	public:
		virtual ~IDataStorage() { }

		// Used by Container
		virtual void Open(const std::string& db_path, const std::string& password, const RawData& savedData) = 0;
		virtual void Create(const std::string& db_path, const std::string& password) = 0;

		virtual void ResetPassword(const std::string& newPassword) = 0;
		virtual void ClearData() = 0;
		virtual void GetDataToSave(RawData& data) = 0; // Usually called when container is going to close this storage

		// Used by binary streams
		virtual uint64_t Write(std::istream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr) = 0;
		virtual uint64_t Read(std::ostream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr) = 0;
		virtual uint64_t Copy(std::istream& src, std::ostream& dest, uint64_t beginSrc, uint64_t endSrc, uint64_t beginDest, dbc::IProgressObserver* observer = nullptr) = 0;
		virtual uint64_t Erace(uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr) = 0;

		virtual uint64_t Append(uint64_t size, uint64_t& begin, dbc::IProgressObserver* observer = nullptr) = 0;
	};

	typedef std::auto_ptr<IDataStorage> IDataStorageGuard;
}