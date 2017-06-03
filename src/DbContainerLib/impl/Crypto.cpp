#include "stdafx.h"
#include "Crypto.h"
#include "FsUtils.h"
#include "ContainerException.h"

namespace
{
	const unsigned long int DEF_IO_BLOCK_SIZE = 512;
	const unsigned long int MIN_IO_BLOCK_SIZE = 128;
	const unsigned long int MAX_IO_BLOCK_SIZE = 65536; // 64K
}

static const EVP_CIPHER* s_cryptCipher = EVP_aes_128_ofb();
static const unsigned short s_cryptKeyLen = EVP_CIPHER_key_length(s_cryptCipher);

struct dbc::crypto::AesCryptorBase::CryptoResourcesGuard
{
	CryptoResourcesGuard()
	{
		ERR_load_crypto_strings();
		OpenSSL_add_all_algorithms();
	}

	~CryptoResourcesGuard()
	{
		EVP_cleanup();
		ERR_free_strings();
	}
};

dbc::crypto::AesCryptorBase::CryptoResourcesGuard dbc::crypto::AesCryptorBase::s_cryptoResources = dbc::crypto::AesCryptorBase::CryptoResourcesGuard();

dbc::crypto::AesCryptorBase::AesCryptorBase(const RawData& key, const RawData& iv, CryptInitFn initFn, CryptUpdateFn updateFn)
	: m_key(key)
	, m_iv(iv)
	, m_cryptInitFn(initFn)
	, m_cryptUpdateFn(updateFn)
	, m_ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free)
	, m_IoBlockSize(DEF_IO_BLOCK_SIZE)
{
	if (m_key.size() != s_cryptKeyLen || m_iv.size() != s_cryptKeyLen)
	{
		throw ContainerException("Invalid key or iv data length", WRONG_PARAMETERS);
	}
}

dbc::crypto::AesCryptorBase::~AesCryptorBase()
{
	// To avoid warning from "std::auto_ptr<CtxImpl> m_ctx" in private members of this class
}

unsigned long dbc::crypto::AesCryptorBase::GetIoBlockSize()
{
	return m_IoBlockSize;
}

void dbc::crypto::AesCryptorBase::SetIoBlockSize(unsigned long blockSize)
{
	if (blockSize < MIN_IO_BLOCK_SIZE || blockSize > MAX_IO_BLOCK_SIZE)
	{
		throw ContainerException("IO block size is out of range", WRONG_PARAMETERS);
	}
	m_IoBlockSize = blockSize;
}

unsigned long dbc::crypto::AesCryptorBase::GetDefIoBlockSize()
{
	return DEF_IO_BLOCK_SIZE;
}

unsigned short dbc::crypto::AesCryptorBase::GetKeyAndIvLen()
{
	return s_cryptKeyLen;
}

void dbc::crypto::AesCryptorBase::CryptRawData(const RawData& src, RawData& dest, dbc::IProgressObserver* observer)
{
	size_t srcSize = src.size();
	size_t processed = 0;
	RawData srcBlockTmp(m_IoBlockSize);
	RawData destBlockTmp(m_IoBlockSize);
	RawData resultTmp;
	if (!src.empty())
	{
		for (size_t offset = 0; processed < srcSize;)
		{
			if (observer != nullptr)
			{
				observer->OnProgressUpdated(static_cast<float>(offset) / srcSize);
			}
			size_t processNow = offset + m_IoBlockSize > srcSize ? srcSize - offset : m_IoBlockSize;
			srcBlockTmp.assign(src.begin() + offset, src.begin() + offset + processNow);
			processed += CryptPortion(srcBlockTmp, destBlockTmp, processNow, observer);
			resultTmp.insert(resultTmp.begin() + offset, destBlockTmp.begin(), destBlockTmp.begin() + processNow);
		}
	}

	std::swap(dest, resultTmp);
}

uint64_t dbc::crypto::AesCryptorBase::CryptBetweenStreams(std::istream &in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer)
{
	// Only for binary streams! Using text streams here is forbidden!
	uint64_t block_size = m_IoBlockSize;
	RawData bufIn(m_IoBlockSize);
	RawData bufOut(m_IoBlockSize);
	std::streamoff ret = 0;

	uint64_t max_size = dbc::utils::TellMaxAvailable(in, size);
	if (observer && max_size < size)
	{
		observer->OnWarning(ERR_DATA_SHORT_SRC);
	}

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
		if (dbc::utils::CheckStream(in, observer, CANT_READ, "Reading from input stream failed") != Continue)
		{
			return ret;
		}

		size_t updated = CryptPortion(bufIn, bufOut, static_cast<size_t>(in.gcount()), observer);
        if (updated < static_cast<size_t>(in.gcount()))
		{
			throw ContainerException("Encryption/Decryption error", ERR_INTERNAL);
		}

		out.write(reinterpret_cast<const char*>(bufOut.data()), updated);
		if (dbc::utils::CheckStream(out, observer, CANT_WRITE, "Writing to output stream failed") != Continue)
		{
			return ret;
		}
		ret += updated;

		if (observer != nullptr)
		{
			observer->OnProgressUpdated(ret / static_cast<float>(size));
		}
	}
	
	return ret;
}

void dbc::crypto::AesCryptorBase::CheckUpdateFn(int ret)
{
	if (ret != 1)
	{
		std::shared_ptr<BIO> bio(BIO_new(BIO_s_mem()), ::BIO_free);
		ERR_print_errors(bio.get());
		char* buf = nullptr;
		BIO_get_mem_data(bio.get(), &buf);
		throw ContainerException(std::string("Encryption/Decryption error: ") + buf, ERR_INTERNAL);
	}
}

size_t dbc::crypto::AesCryptorBase::CryptPortion(const RawData& src, RawData& dest, size_t size, dbc::IProgressObserver* observer)
{
	InitCtx(observer);
	if (size > src.size())
	{
		size = src.size();
	}
	int updated = 0;
	CheckUpdateFn(m_cryptUpdateFn(m_ctx.get(), &dest[0], &updated, &src[0], static_cast<int>(size)));
	ClearCtx(observer);
	return static_cast<size_t>(updated);
}

void dbc::crypto::AesCryptorBase::InitCtx(dbc::IProgressObserver* observer)
{
	if (!m_cryptInitFn(m_ctx.get(), s_cryptCipher, 0, &m_key[0], &m_iv[0]))
	{
		if (observer != nullptr && observer->OnWarning(ERR_INTERNAL) == dbc::Continue)
		{
			return;
		}
		throw ContainerException("Encryption/Decryption error", ERR_INTERNAL);
	}
}

void dbc::crypto::AesCryptorBase::ClearCtx(dbc::IProgressObserver* observer)
{
	if (!::EVP_CIPHER_CTX_cleanup(m_ctx.get()))
	{
		if (observer != nullptr && observer->OnWarning(ERR_INTERNAL) == dbc::Continue)
		{
			return;
		}
		throw ContainerException("Encryption/Decryption error", ERR_INTERNAL);
	}
}

dbc::crypto::AesEncryptor::AesEncryptor(const RawData& key, const RawData& iv)
	: AesCryptorBase(key, iv, &::EVP_EncryptInit_ex, &::EVP_EncryptUpdate)
{ }

void dbc::crypto::AesEncryptor::Encrypt(const RawData& data, RawData& result, dbc::IProgressObserver* observer)
{
	CryptRawData(data, result, observer);
}

uint64_t dbc::crypto::AesEncryptor::Encrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer)
{
	return CryptBetweenStreams(in, out, size, observer);
}

dbc::crypto::AesDecryptor::AesDecryptor(const RawData& key, const RawData& iv)
	: AesCryptorBase(key, iv, &::EVP_DecryptInit_ex, &::EVP_DecryptUpdate)
{ }

void dbc::crypto::AesDecryptor::Decrypt(const RawData& data, RawData& result, dbc::IProgressObserver* observer)
{
	CryptRawData(data, result, observer);
}

uint64_t dbc::crypto::AesDecryptor::Decrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer)
{
	return CryptBetweenStreams(in, out, size, observer);
}

dbc::RawData dbc::crypto::utils::SHA256_GetHash(const dbc::RawData& message)
{
	RawData hash(SHA256_DIGEST_LENGTH);
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, message.data(), message.size());
	SHA256_Final(&hash[0], &sha256);
	return hash;
}

void dbc::crypto::utils::RandomSequence(unsigned int seed, dbc::RawData& sequence_out)
{
	srand(seed);
	for (RawData::iterator itr = sequence_out.begin(); itr != sequence_out.end(); ++itr)
	{
		*itr = (char)(rand() % 256);
	}
}

unsigned int dbc::crypto::utils::GetSeed(const dbc::RawData& sequence)
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
