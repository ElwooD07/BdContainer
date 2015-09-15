#pragma once
#include "Types.h"

namespace dbc
{
	class IProgressObserver;

	namespace crypting
	{
		typedef unsigned char RawDataType;
		typedef std::vector<RawDataType> RawData;

		class AesCryptorBase
		{
			NONCOPYABLE(AesCryptorBase);

		public:
			AesCryptorBase(const RawData& key, const RawData& iv);
			~AesCryptorBase();
			void SetIoBlockSize(unsigned long blockSize);

			static unsigned short GetKeyAndIvLen();

		protected:
			void ErrorHandler(int ret);
			typedef int(*CryptUpdateFn)(::EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl);
			uint64_t CryptBetweenStreams(std::istream &in, std::ostream& out, uint64_t size, CryptUpdateFn updateFn, dbc::IProgressObserver* observer = nullptr);

		protected:
			RawData m_key;
			RawData m_iv;

			std::shared_ptr<::EVP_CIPHER_CTX> m_ctx;

		private:
			struct InitCrypt;
			static InitCrypt s_init;
			unsigned long m_IoBlockSize;
		};

		class AesEncryptor : public AesCryptorBase
		{
		public:
			AesEncryptor(const RawData& key, const RawData& iv);
			void Encrypt(const RawData& data, RawData& result);
			uint64_t Encrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer = nullptr);
		};

		class AesDecryptor : public AesCryptorBase
		{
		public:
			AesDecryptor(const RawData& key, const RawData& iv);
			void Decrypt(const RawData& data, RawData& result);
			uint64_t Decrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer = nullptr);
		};

		namespace utils
		{
			void RawDataAppend(const RawData& src, RawData& dest);
			RawData StringToRawData(const std::string& str);
			std::string RawDataToString(const RawData& data);

			RawData SHA3_GetHash(const RawData& message);
			void RandomSequence(unsigned int seed, RawData& sequence_out);
			unsigned int GetSeed(const RawData& sequence);
		}
	};
}