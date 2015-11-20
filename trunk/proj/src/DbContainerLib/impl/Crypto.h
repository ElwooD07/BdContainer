#pragma once
#include "TypesInternal.h"

namespace dbc
{
	class IProgressObserver;

	namespace crypto
	{
		class AesCryptorBase
		{
			NONCOPYABLE(AesCryptorBase);

		public:
			typedef int(*CryptInitFn)(::EVP_CIPHER_CTX* ctx, const ::EVP_CIPHER* cipher, ::ENGINE* impl, const unsigned char* key, const unsigned char* iv);
			typedef int(*CryptUpdateFn)(::EVP_CIPHER_CTX* ctx, unsigned char* out, int* outl, const unsigned char* in, int inl);

			AesCryptorBase(const RawData& key, const RawData& iv, CryptInitFn initFn, CryptUpdateFn updateFn);
			~AesCryptorBase();
			unsigned long GetIoBlockSize();
			void SetIoBlockSize(unsigned long blockSize);

			static unsigned short GetKeyAndIvLen();
			static unsigned long GetDefIoBlockSize();

		protected:
			void CryptRawData(const RawData& src, RawData& dest, dbc::IProgressObserver* observer);
			uint64_t CryptBetweenStreams(std::istream &in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer = nullptr);

		protected:
			RawData m_key;
			RawData m_iv;

			std::shared_ptr<::EVP_CIPHER_CTX> m_ctx;

		private:
			void CheckUpdateFn(int ret);
			size_t CryptPortion(const RawData& src, RawData& dest, size_t size, dbc::IProgressObserver* observer);
			void InitCtx(dbc::IProgressObserver* observer);
			void ClearCtx(dbc::IProgressObserver* observer);

		private:
			struct CryptoResourcesGuard;
			static CryptoResourcesGuard s_cryptoResources;
			unsigned long m_IoBlockSize;

			CryptInitFn m_cryptInitFn;
			CryptUpdateFn m_cryptUpdateFn;
		};

		class AesEncryptor : public AesCryptorBase
		{
		public:
			AesEncryptor(const RawData& key, const RawData& iv);
			void Encrypt(const RawData& data, RawData& result, dbc::IProgressObserver* observer = nullptr);
			uint64_t Encrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer = nullptr);
		};

		class AesDecryptor : public AesCryptorBase
		{
		public:
			AesDecryptor(const RawData& key, const RawData& iv);
			void Decrypt(const RawData& data, RawData& result, dbc::IProgressObserver* observer = nullptr);
			uint64_t Decrypt(std::istream& in, std::ostream& out, uint64_t size, dbc::IProgressObserver* observer = nullptr);
		};

		namespace utils
		{
			RawData StringToRawData(const std::string& str);
			std::string RawDataToString(const RawData& data);

			RawData SHA256_GetHash(const RawData& message);
			void RandomSequence(unsigned int seed, RawData& sequence_out);
			unsigned int GetSeed(const RawData& sequence);
		}
	};
}