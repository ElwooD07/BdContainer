#include "stdafx.h"
#include "DataStorageBinaryFile.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Crypto.h"
#include "FsUtils.h"
#include "Types.h"

namespace
{
	const size_t s_maxPasswordLen = 256;
	const size_t s_binHeaderLen = s_maxPasswordLen * 4;
	const std::string s_binFileExt = ".bin";
	const std::string s_testExpression = "Database Container Project";

	std::string GetBinFilePath(const std::string& dbPath)
	{
		return dbPath + s_binFileExt;
	}

	void GetKeyAndIvFromPassword(const std::string& password, dbc::crypting::RawData& key, dbc::crypting::RawData& iv)
	{
		assert(key.empty() && iv.empty());

		static const char s_salt[]("arbadakarba123");
		dbc::crypting::RawData hash = dbc::crypting::utils::SHA256_GetHash(dbc::crypting::utils::StringToRawData(password + s_salt));
		const unsigned int keyAndIvLen = dbc::crypting::AesCryptorBase::GetKeyAndIvLen();
		assert(hash.size() == keyAndIvLen * 2);
		key.assign(hash.begin(), hash.begin() + keyAndIvLen);
		iv.assign(hash.begin() + keyAndIvLen, hash.end());
	}

	void FillHeader(const std::string password, const dbc::crypting::RawData& key, const dbc::crypting::RawData& iv, std::ostream &out)
	{
		std::istringstream istreamPass(s_testExpression);
		dbc::crypting::AesEncryptor encryptor(key, iv);
		encryptor.Encrypt(istreamPass, out, s_testExpression.size());

		assert(s_testExpression.size() <= s_binHeaderLen);
		size_t trashLen = s_binHeaderLen - s_testExpression.size();
		if (trashLen > 0)
		{
			dbc::crypting::RawData trash(trashLen, '\0');
			dbc::crypting::utils::RandomSequence(dbc::crypting::utils::GetSeed(dbc::crypting::utils::StringToRawData(password)), trash);
			out.write(reinterpret_cast<const char*>(trash.data()), trash.size());
		}
		if (out.rdstate() != std::ios_base::goodbit)
		{
			throw dbc::ContainerException(dbc::ERR_DATA, dbc::CANT_WRITE);
		}
	}

	bool KeyAndIvAreCorrect(const dbc::crypting::RawData key, const dbc::crypting::RawData iv, std::istream& in)
	{
		std::ostringstream ostream;
		dbc::crypting::AesDecryptor decryptor(key, iv);
		uint64_t decryptedLen = decryptor.Decrypt(in, ostream, s_testExpression.size());
		ostream.flush();
		std::string decrypted = ostream.str();

		if (!in.good() || decryptedLen != s_testExpression.size())
		{
			throw dbc::ContainerException(dbc::ERR_DATA, dbc::CANT_READ);
		}
		return (s_testExpression == decrypted);
	}
}

dbc::DataStorageBinaryFile::DataStorageBinaryFile()
	: m_cryptBlockSize(crypting::AesCryptorBase::GetDefIoBlockSize())
{ }

void dbc::DataStorageBinaryFile::Open(const std::string& db_path, const std::string& password, const RawData& savedData)
{
	m_bin_file = savedData.size() > 0 ? reinterpret_cast<const char*>(savedData.data()) : GetBinFilePath(db_path);

	OpenFileStream(false);

	dbc::crypting::RawData key;
	dbc::crypting::RawData iv;
	GetKeyAndIvFromPassword(password, key, iv);

	if (!KeyAndIvAreCorrect(key, iv, m_stream))
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

	dbc::crypting::RawData key;
	dbc::crypting::RawData iv;
	GetKeyAndIvFromPassword(password, key, iv);
	FillHeader(password, key, iv, m_stream);

	m_key.swap(key);
	m_iv.swap(iv);
}

void dbc::DataStorageBinaryFile::ResetPassword(const std::string& newPassword)
{
	CheckInitialized();

	assert(!"Not implementer yet");
}

void dbc::DataStorageBinaryFile::ClearData()
{
	CheckInitialized();

	m_stream.seekg(0);
	std::string header(s_binHeaderLen, '\0');
	m_stream.read(&header[0], s_binHeaderLen);
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
	m_stream.write(header.c_str(), s_binHeaderLen);
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

	m_stream.seekp(begin, std::ios::beg);
	crypting::AesEncryptor encryptor(m_key, m_iv);
	return encryptor.Encrypt(data, m_stream, end - begin, observer);
}

uint64_t dbc::DataStorageBinaryFile::Read(std::ostream& data, uint64_t begin, uint64_t end, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	m_stream.seekg(begin, std::ios::beg);
	crypting::AesDecryptor decryptor(m_key, m_iv);
	return decryptor.Decrypt(m_stream, data, end - begin, observer);
}

uint64_t dbc::DataStorageBinaryFile::Copy(std::istream& src, std::ostream& dest, uint64_t beginSrc, uint64_t endSrc, uint64_t beginDest, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	m_stream.seekg(beginSrc, std::ios::beg);
	m_stream.seekp(beginDest, std::ios::beg);
	uint64_t maxRead = utils::TellMaxAvailable(src, endSrc - beginSrc);
	if (maxRead < endSrc - beginSrc && observer != nullptr && observer->OnWarning(ERR_DATA_SHORT_SRC) != dbc::Continue)
	{
		throw ContainerException(ERR_DATA_SHORT_SRC);
	}

	uint64_t ret = 0;
	RawData blockSrc(m_cryptBlockSize);
	RawData blockDest(m_cryptBlockSize);
	while (maxRead > 0 && !src.eof())
	{
		uint64_t copyNow = m_cryptBlockSize;
		if (maxRead < m_cryptBlockSize)
		{
			copyNow = maxRead;
			maxRead = 0;
		}
		else
		{
			maxRead -= m_cryptBlockSize;
		}
		src.read(reinterpret_cast<char*>(&blockSrc[0]), copyNow);
		if (utils::CheckStream(src, observer, CANT_READ, "Reading from input stream failed") != Continue)
		{
			return ret;
		}

		std::streamsize gcount = src.gcount();
		dest.write(reinterpret_cast<const char*>(blockDest.data()), gcount);
		if (utils::CheckStream(dest, observer, CANT_WRITE, "Writing to output stream failed") != Continue)
		{
			return ret;
		}
		ret += gcount;

		if (observer != nullptr)
		{
			observer->OnProgressUpdated(ret / static_cast<float>(maxRead));
		}
	}

	return ret;
}

uint64_t dbc::DataStorageBinaryFile::Erace(uint64_t begin, uint64_t end, dbc::IProgressObserver* observer)
{
	CheckInitialized();

	if (begin > end)
	{
		throw ContainerException(ERR_INTERNAL, WRONG_PARAMETERS);
	}

	std::vector<char> buf(20480); // 20K
	m_stream.seekp(begin, std::ios::beg);
	const uint64_t origAreaSize = end - begin;
	uint64_t areaSize = origAreaSize;
	while (areaSize > 0 && m_stream.good())
	{
		uint64_t cur_size = (areaSize < buf.size()) ? areaSize : buf.size();
		m_stream.write(buf.data(), cur_size);
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
	if (!truncate && dbc::utils::TellMaxAvailable(stream, s_binHeaderLen) < s_binHeaderLen)
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
