#include "stdafx.h"
#include "DataStorageBinaryFile.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Crypto.h"
#include "FsUtils.h"
#include "Types.h"

namespace
{
	static const size_t MAX_PASSWORD_LEN = 256;
	static const size_t BIN_HEADER_LEN = MAX_PASSWORD_LEN * 4;
	static const std::string BIN_FILE_EXT = ".bin";

	std::string GetBinFilePath(const std::string& db_path)
	{
		return db_path + BIN_FILE_EXT;
	}

	std::string CalculateAdaptedHash(const std::string& data, const char* salt, size_t outLen)
	{
		std::string hash(dbc::crypto::SHA3_GetHash(data + salt));
		hash.resize(outLen);
		return hash;
	}

	std::string GetKeyFromPassword(const std::string& password)
	{
		static const char s_salt[]("arbadakarba123");
		return CalculateAdaptedHash(password, s_salt, dbc::crypto::AES_BLOCK_SIZE);
	}

	std::string GetIvFromPassword(const std::string& password)
	{
		static const char s_salt[]("tmdfiycyhdsmwiietyhccyneotitn");
		return CalculateAdaptedHash(password, s_salt, dbc::crypto::AES_BLOCK_SIZE);
	}

	void FillHeader(const std::string password, std::ostream &out)
	{
		std::string key = GetKeyFromPassword(password); // AES key
		std::string iv = GetIvFromPassword(password); // AES IV

		std::istringstream istreamPass(password);
		dbc::crypto::AES_Encrypt(key, iv, istreamPass, out, password.length()); // Writing encrypted password

		assert(password.length() <= BIN_HEADER_LEN);
		size_t trashLen = BIN_HEADER_LEN - password.length();
		if (trashLen > 0)
		{
			std::string trash(trashLen, '\0');
			dbc::crypto::RandomSequence(dbc::crypto::GetSeed(password), trash);
			out.write(trash.c_str(), trash.length());
		}
		if (out.rdstate() != std::ios_base::goodbit)
		{
			throw dbc::ContainerException(dbc::ERR_DATA, dbc::CANT_WRITE);
		}
	}
}

dbc::DataStorageBinaryFile::DataStorageBinaryFile()
{ }

void dbc::DataStorageBinaryFile::Open(const std::string& db_path, const std::string& password, const RawData& savedData)
{
	m_bin_file = savedData.size() > 0 ? reinterpret_cast<const char*>(savedData.data()) : GetBinFilePath(db_path);

	OpenFileStream(false);

	std::string key = GetKeyFromPassword(password);
	std::string iv = GetIvFromPassword(password);
	std::ostringstream ostream;
	uint64_t decryptedLen = crypto::AES_Decrypt(key, iv, m_stream, ostream, password.length());
	ostream.flush();
	std::string decrypted = ostream.str();
	if (!m_stream.good() || decryptedLen != password.length())
	{
		throw ContainerException(ERR_DATA, CANT_READ);
	}
	if (decrypted != password)
	{
		throw ContainerException(INVALID_PASSWORD);
	}

	m_key.swap(key);
	m_iv.swap(iv);
}

void dbc::DataStorageBinaryFile::Create(const std::string& db_path, const std::string& password)
{
	m_bin_file = GetBinFilePath(db_path);
	ClearFile();
	OpenFileStream(true);
	FillHeader(password, m_stream);
}

void dbc::DataStorageBinaryFile::ResetPassword(const std::string& newPassword)
{
	CheckInitialized();

	assert(!"Not implementer yet");
}

void dbc::DataStorageBinaryFile::ClearData()
{
	CheckInitialized();

	dbc::MutexLock readGuard(m_mutexRead);
	dbc::MutexLock writeGuard(m_mutexWrite);

	m_stream.seekg(0);
	std::string header(BIN_HEADER_LEN, '\0');
	m_stream.read(&header[0], BIN_HEADER_LEN);
	if (!m_stream.good())
	{
		throw ContainerException(ERR_DATA, CANT_READ);
	}

	m_stream.close();
	m_stream.open(m_bin_file, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
	if (!m_stream.is_open())
	{
		throw ContainerException(ERR_DATA, CANT_OPEN);
	}

	m_stream.seekp(0);
	m_stream.write(header.c_str(), BIN_HEADER_LEN);
	if (!m_stream.good())
	{
		throw ContainerException(ERR_DATA, CANT_WRITE);
	}
}

void dbc::DataStorageBinaryFile::GetDataToSave(RawData& data)
{
	// TODO: Save some data to the database!
}

uint64_t dbc::DataStorageBinaryFile::Write(std::istream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	dbc::MutexLock writeGuard(m_mutexWrite);

	m_stream.seekp(begin, std::ios::beg);
	return crypto::AES_Encrypt(m_key, m_iv, data, m_stream, end - begin);
}

uint64_t dbc::DataStorageBinaryFile::Read(std::ostream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	dbc::MutexLock readGuard(m_mutexRead);

	m_stream.seekg(begin, std::ios::beg);
	return crypto::AES_Decrypt(m_key, m_iv, m_stream, data, end - begin);
}

uint64_t dbc::DataStorageBinaryFile::Copy(std::istream& src, std::ostream& dest, uint64_t beginSrc, uint64_t endSrc, uint64_t beginDest, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	dbc::MutexLock readGuard(m_mutexRead);
	dbc::MutexLock writeGuard(m_mutexWrite);

	m_stream.seekg(beginSrc, std::ios::beg);
	m_stream.seekp(beginDest, std::ios::beg);
	// TODO: Implement this!!!
	return 0;
}

uint64_t dbc::DataStorageBinaryFile::Erace(uint64_t begin, uint64_t end, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	if (begin > end)
	{
		throw ContainerException(ERR_INTERNAL, WRONG_PARAMETERS);
	}

	dbc::MutexLock writeGuard(m_mutexWrite);

	const unsigned int buf_size = 20480; // 20K
	char buf[buf_size];
	memset(buf, 0, buf_size);
	m_stream.seekp(begin, std::ios::beg);
	const uint64_t origAreaSize = end - begin;
	uint64_t areaSize = origAreaSize;
	while (areaSize > 0 && m_stream.good())
	{
		uint64_t cur_size = (areaSize < buf_size) ? areaSize : buf_size;
		m_stream.write(buf, cur_size);
		areaSize -= cur_size;

		if (observer != nullptr)
		{
			observer->OnProgressUpdated(static_cast<float>(areaSize) / (end - begin));
		}
	}
	return origAreaSize - areaSize;
}

uint64_t dbc::DataStorageBinaryFile::Append(uint64_t size, uint64_t& begin, dbc::IProgressObserver* observer)
{
	m_stream.seekg(0, std::ios::end);
	if (!m_stream.good())
	{
		return WRONG_SIZE;
	}
	begin = m_stream.tellg();
	return Erace(begin, begin + size, observer);
}

void dbc::DataStorageBinaryFile::OpenFileStream(bool truncate)
{
	dbc::MutexLock readGuard(m_mutexRead);
	dbc::MutexLock writeGuard(m_mutexWrite);

	if (m_stream.is_open())
	{
		m_stream.close();
	}

	int flags = std::ios::binary | std::ios::in | std::ios::out;
	flags |= truncate ? std::ios::trunc : 0;
	std::fstream stream(m_bin_file, flags);
	if (!stream)
	{
		throw ContainerException(ERR_DATA, CANT_OPEN);
	}
	if (!truncate && dbc::utils::TellMaxAvailable(stream, BIN_HEADER_LEN) < BIN_HEADER_LEN)
	{
		throw ContainerException(ERR_DATA, IS_DAMAGED);
	}
	m_stream.swap(stream);
}

void dbc::DataStorageBinaryFile::CheckInitialized()
{
	if (m_key.empty() || m_iv.empty())
	{
		throw ContainerException(ERR_DATA_NOT_INITIALIZED);
	}
	if (!m_stream.is_open())
	{
		throw ContainerException(ERR_DATA, NO_ACCESS);
	}
}

void dbc::DataStorageBinaryFile::ClearFile()
{
	std::ofstream bfile(m_bin_file, std::ios::out | std::ios::trunc);
	bool erased = !bfile.fail();
	bfile.close();

	if (!erased)
	{
		throw dbc::ContainerException(ERR_FS, CANT_REMOVE);
	}
}
