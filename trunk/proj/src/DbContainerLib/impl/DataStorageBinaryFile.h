#pragma once
#include <mutex>
#include "IDataStorage.h"

namespace dbc
{
	class DataStorageBinaryFile: public IDataStorage
	{
	public:
		DataStorageBinaryFile();

		virtual void Open(const std::string& db_path, const std::string& password, const RawData& savedData);
		virtual void Create(const std::string& db_path, const std::string& password);

		virtual void ResetPassword(const std::string& newPassword);
		virtual void ClearData();
		virtual void GetDataToSave(RawData& data);

		virtual uint64_t Write(std::istream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr);
		virtual uint64_t Read(std::ostream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr);
		virtual uint64_t Copy(std::istream& src, std::ostream& dest, uint64_t beginSrc, uint64_t endSrc, uint64_t beginDest, dbc::IProgressObserver* observer = nullptr);
		virtual uint64_t Erace(uint64_t begin, uint64_t end, dbc::IProgressObserver* observer = nullptr);

		virtual uint64_t Append(uint64_t size, uint64_t& begin, dbc::IProgressObserver* observer);

	private:
		void OpenFileStream(bool truncate);
		void CheckInitialized();
		void ClearFile();

	private:
		std::string m_key;
		std::string m_iv;
		std::string m_bin_file;
		std::fstream m_stream;
		std::mutex m_mutexWrite;
		std::mutex m_mutexRead;
	};
}