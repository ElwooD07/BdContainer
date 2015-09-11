#pragma once

namespace dbc
{
	namespace crypto
	{
		std::string SHA3_GetHash(const std::string &message);
		uint64_t AES_Encrypt(const std::string &key, const std::string &iv, std::istream &message, std::ostream &out, uint64_t size);
		uint64_t AES_Decrypt(const std::string &key, const std::string &iv, std::istream &message, std::ostream &out, uint64_t size);

		void RandomSequence(unsigned int seed, std::string &sequence_out);
		unsigned int GetSeed(const std::string &sequence);

		void SetIOBlockSize(long int size);
		unsigned long int GetIOBlockSize();
		void ResetIOBlockSize();
		extern const int AES_KEY_LENGHT;
		extern const int AES_BLOCK_SIZE;
		extern const int SHA3_DIGEST_LENGHT;
	};
}