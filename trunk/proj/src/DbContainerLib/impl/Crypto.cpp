#include "stdafx.h"

#include "sha3.h"
#include "modes.h"
#include "aes.h"
#include "filters.h"
#include "files.h"

#include "Crypto.h"
#include "FsUtils.h"
#include "ContainerException.h"

namespace
{
	const unsigned long int DEF_IO_BLOCK_SIZE = 65536; // 64K
	const unsigned long int MIN_IO_BLOCK_SIZE = 256; // 256b
	const unsigned long int MAX_IO_BLOCK_SIZE = 67108864; // 64M
	static unsigned long int s_IoBlockSize = DEF_IO_BLOCK_SIZE;

	uint64_t FeedSTFilter(CryptoPP::StreamTransformationFilter * filter, std::istream &in, uint64_t size)
	{
		// Only for binary streams! Using text streams here is forbidden!
		std::streamoff ret = 0;

		uint64_t max_size = dbc::utils::TellMaxAvailable(in, size);
		uint64_t block_size = s_IoBlockSize;
		std::vector<byte> buf(s_IoBlockSize);
		while(max_size > 0)
		{
			if (max_size < s_IoBlockSize)
			{
				block_size = max_size;
				max_size = 0;
			}
			else
			{
				max_size -= s_IoBlockSize;
			}
			in.read(reinterpret_cast<char*>(&buf[0]), block_size);
			ret += in.gcount();
			filter->Put(&buf[0], static_cast<size_t>(in.gcount()));
		}
		filter->MessageEnd();
		return ret; // the count of processed symbols
	}
}

const int dbc::crypto::AES_KEY_LENGHT = CryptoPP::AES::DEFAULT_KEYLENGTH;
const int dbc::crypto::AES_BLOCK_SIZE = CryptoPP::AES::BLOCKSIZE;
const int dbc::crypto::SHA3_DIGEST_LENGHT = CryptoPP::SHA3_256::DIGESTSIZE;

std::string dbc::crypto::SHA3_GetHash(const std::string &message)
{
	CryptoPP::SHA3_256 hash;
	byte digest[CryptoPP::SHA3_256::DIGESTSIZE];

	hash.CalculateDigest(digest, reinterpret_cast<const byte*>(message.c_str()), message.length());
	std::string ret(digest, digest + SHA3_DIGEST_LENGHT);
	return ret;
}

uint64_t dbc::crypto::AES_Encrypt(const std::string &key, const std::string &iv, std::istream &message, std::ostream &out, uint64_t size)
{
	if (key.length() != AES_KEY_LENGHT || iv.length() != AES_KEY_LENGHT)
		throw ContainerException("", WRONG_PARAMETERS, "Key or IV lenght is invalid");

	CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption ofbEncryption((byte*)key.c_str(), key.length(), (byte *)iv.c_str());

	CryptoPP::StreamTransformationFilter filter(ofbEncryption, new CryptoPP::FileSink(out));

	return FeedSTFilter(&filter, message, size);
}

uint64_t dbc::crypto::AES_Decrypt(const std::string &key, const std::string &iv, std::istream &message, std::ostream &out, uint64_t size)
{
	if (key.length() != AES_KEY_LENGHT || iv.length() != AES_KEY_LENGHT)
		throw ContainerException("", WRONG_PARAMETERS, "Key or IV lenght is invalid");

	CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption ofbDecryption((byte*)key.c_str(), key.length(), (byte *)iv.c_str());

	CryptoPP::StreamTransformationFilter filter(ofbDecryption, new CryptoPP::FileSink(out));

	return FeedSTFilter(&filter, message, size);
}

void dbc::crypto::RandomSequence(unsigned int seed, std::string &sequence_out)
{
	srand(seed);
	for (std::string::iterator itr = sequence_out.begin();
		itr != sequence_out.end(); ++itr)
	{
		*itr = (char)(rand() % 256);
	}
}

unsigned int dbc::crypto::GetSeed(const std::string &sequence)
{
	if (sequence.empty())
		return 0;
	unsigned int res = 0;
	for (std::string::const_iterator itr = sequence.cbegin();
		itr != sequence.cend(); ++itr)
		res += *itr;
	res += static_cast<unsigned int>(time(0));
	return res;
}

void dbc::crypto::SetIOBlockSize(long int size)
{
	if (size >= MIN_IO_BLOCK_SIZE && size <= MAX_IO_BLOCK_SIZE)
		s_IoBlockSize = size;
}

unsigned long int dbc::crypto::GetIOBlockSize()
{
	return s_IoBlockSize;
}

void dbc::crypto::ResetIOBlockSize()
{
	s_IoBlockSize = DEF_IO_BLOCK_SIZE;
}