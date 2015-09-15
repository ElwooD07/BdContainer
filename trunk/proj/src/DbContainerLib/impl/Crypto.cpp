#include "stdafx.h"
#include "Crypto.h"
#include "FsUtils.h"
#include "ContainerException.h"
#include "IProgressObserver.h"

namespace
{
	const unsigned long int DEF_IO_BLOCK_SIZE = 65536; // 64K
	const unsigned long int MIN_IO_BLOCK_SIZE = 256; // 256b
	const unsigned long int MAX_IO_BLOCK_SIZE = 67108864; // 64M

	template<class T>
	inline dbc::ProgressState CheckStream(T& strm, dbc::IProgressObserver* observer, dbc::ErrIncident errIncident, const char* errMsg)
	{
		dbc::Error errCode(dbc::ERR_DATA, errIncident);
		if (strm.fail())
		{
			if (observer != nullptr)
			{
				return observer->OnError(errCode);
			}
			else
			{
				throw dbc::ContainerException(errMsg, errCode);
			}
		}
		return dbc::Continue;
	}
}

static const EVP_CIPHER* s_cryptCipher = EVP_aes_128_ofb();
static const unsigned short s_cryptKeyLen = EVP_CIPHER_key_length(s_cryptCipher);

namespace
{
	inline void CheckResultLen(size_t origLen, int resLen)
	{
		if (origLen != resLen)
		{
			throw dbc::ContainerException("Encryption/Decryption error", dbc::ERR_INTERNAL);
		}
	}
}

struct dbc::crypting::AesCryptorBase::InitCrypt
{
	InitCrypt()
	{
		ERR_load_crypto_strings();
		OpenSSL_add_all_algorithms();
	}

	~InitCrypt()
	{
		EVP_cleanup();
		ERR_free_strings();
	}
};

dbc::crypting::AesCryptorBase::InitCrypt dbc::crypting::AesCryptorBase::s_init = dbc::crypting::AesCryptorBase::InitCrypt();

dbc::crypting::AesCryptorBase::AesCryptorBase(const RawData& key, const RawData& iv)
	: m_key(key)
	, m_iv(iv)
	, m_ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free)
	, m_IoBlockSize(DEF_IO_BLOCK_SIZE)
{
	if (m_key.size() != s_cryptKeyLen || m_iv.size() != s_cryptKeyLen)
	{
		throw ContainerException("Invalid key or iv data length", WRONG_PARAMETERS);
	}
}

dbc::crypting::AesCryptorBase::~AesCryptorBase()
{
	// To avoid warning from "std::auto_ptr<CtxImpl> m_ctx" in private members of this class
}

void dbc::crypting::AesCryptorBase::SetIoBlockSize(unsigned long blockSize)
{
	if (blockSize < MIN_IO_BLOCK_SIZE || blockSize > MAX_IO_BLOCK_SIZE)
	{
		throw ContainerException("IO block size is out of range", WRONG_PARAMETERS);
	}
	m_IoBlockSize = blockSize;
}

unsigned short dbc::crypting::AesCryptorBase::GetKeyAndIvLen()
{
	return s_cryptKeyLen;
}

void dbc::crypting::AesCryptorBase::ErrorHandler(int ret)
{
	if (ret != 1)
	{
		std::shared_ptr<BIO> bio(BIO_new(BIO_s_mem()), ::BIO_free);
		ERR_print_errors(bio.get());
		char* buf = NULL;
		BIO_get_mem_data(bio.get(), &buf);
		throw ContainerException("Encryption/Decryption error", ERR_INTERNAL);
	}
}

uint64_t dbc::crypting::AesCryptorBase::CryptBetweenStreams(std::istream &in, std::ostream& out, uint64_t size, CryptUpdateFn updateFn, dbc::IProgressObserver* observer)
{
	// Only for binary streams! Using text streams here is forbidden!
	uint64_t block_size = m_IoBlockSize;
	RawData bufIn(m_IoBlockSize);
	RawData bufOut(m_IoBlockSize);
	std::streamoff ret = 0;

	uint64_t max_size = dbc::utils::TellMaxAvailable(in, size);
	while (max_size > 0 && !in.eof())
	{
		if (max_size < m_IoBlockSize)
		{
			block_size = max_size;
			max_size = 0;
		}
		else
		{
			max_size -= m_IoBlockSize;
		}
		in.read(reinterpret_cast<char*>(&bufIn[0]), block_size);
		if (CheckStream(in, observer, CANT_READ, "Reading from input stream failed") != Continue)
		{
			return ret;
		}

		long long gcount = in.gcount();
		int updated = 0;
		ErrorHandler(updateFn(m_ctx.get(), &bufOut[0], &updated, &bufIn[0], static_cast<int>(gcount)));

		out.write(reinterpret_cast<const char*>(bufOut.data()), updated);
		if (CheckStream(out, observer, CANT_WRITE, "Writing to output stream failed") != Continue)
		{
			return ret;
		}
		ret += gcount;

		if (observer != nullptr)
		{
			observer->OnProgressUpdated(ret / static_cast<float>(size));
		}
	}
	
	return ret;
}

dbc::crypting::AesEncryptor::AesEncryptor(const RawData& key, const RawData& iv)
	: AesCryptorBase(key, iv)
{
	EVP_EncryptInit_ex(m_ctx.get(), s_cryptCipher, 0, &m_key[0], &m_iv[0]);
}

void dbc::crypting::AesEncryptor::Encrypt(const RawData& data, RawData& result)
{
	RawData resultTmp(data.size());
	if (!data.empty())
	{
		int len(0);
		ErrorHandler(EVP_EncryptUpdate(m_ctx.get(), &resultTmp[0], &len, &data[0], data.size()));
		CheckResultLen(data.size(), len);
	}

	std::swap(result, resultTmp);
}

uint64_t dbc::crypting::AesEncryptor::Encrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer)
{
	return CryptBetweenStreams(in, out, size, EVP_EncryptUpdate, observer);
}

dbc::crypting::AesDecryptor::AesDecryptor(const RawData& key, const RawData& iv)
	: AesCryptorBase(key, iv)
{
	EVP_DecryptInit_ex(m_ctx.get(), s_cryptCipher, 0, &m_key[0], &m_iv[0]);
}

void dbc::crypting::AesDecryptor::Decrypt(const RawData& data, RawData& result)
{
	RawData resultTmp(data.size());
	if (!data.empty())
	{
		int len(0);
		ErrorHandler(EVP_DecryptUpdate(m_ctx.get(), &resultTmp[0], &len, &data[0], data.size()));
		CheckResultLen(data.size(), len);
	}

	std::swap(result, resultTmp);
}

uint64_t dbc::crypting::AesDecryptor::Decrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer)
{
	return CryptBetweenStreams(in, out, size, &EVP_DecryptUpdate, observer);
}

void dbc::crypting::utils::RawDataAppend(const RawData& src, RawData& dest)
{
	dest.insert(dest.end(), src.begin(), src.end());
}


dbc::crypting::RawData dbc::crypting::utils::StringToRawData(const std::string& str)
{
	const dbc::crypting::RawDataType* strPtr = reinterpret_cast<const dbc::crypting::RawDataType*>(str.c_str());
	return std::move(dbc::crypting::RawData(strPtr, strPtr + str.size()));
}

std::string dbc::crypting::utils::RawDataToString(const RawData& data)
{
	std::string res;
	if (!data.empty())
	{
		res.assign(reinterpret_cast<const char*>(data.data()), data.size());
	}
	return std::move(res);
}

dbc::crypting::RawData dbc::crypting::utils::SHA3_GetHash(const dbc::crypting::RawData& message)
{
	RawData hash(SHA256_DIGEST_LENGTH);
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, message.data(), message.size());
	SHA256_Final(&hash[0], &sha256);
	return hash;
}

void dbc::crypting::utils::RandomSequence(unsigned int seed, dbc::crypting::RawData& sequence_out)
{
	srand(seed);
	for (RawData::iterator itr = sequence_out.begin(); itr != sequence_out.end(); ++itr)
	{
		*itr = (char)(rand() % 256);
	}
}

unsigned int dbc::crypting::utils::GetSeed(const dbc::crypting::RawData& sequence)
{
	if (sequence.empty())
	{
		return 0;
	}
	unsigned int res = 0;
	for (RawData::const_iterator itr = sequence.cbegin(); itr != sequence.cend(); ++itr)
	{
		res += *itr;
	}
	res += static_cast<unsigned int>(time(0));
	return res;
}
