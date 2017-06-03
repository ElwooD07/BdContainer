#include "stdafx.h"
#include "impl/Crypto.h"
#include "ContainerException.h"
#include "impl/Utils/CommonUtils.h"

using namespace dbc;
using namespace dbc::crypto;
using namespace dbc::utils;

RawData keyNormal = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
RawData ivNormal = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };

void FillStream(std::iostream& strm, RawData& expression)
{
	static const std::string s_smallExpression("0123456789abcdefghijklmnopqrstuvwxyz");
	static const int iterationsCount = 10;

	for (int i = 0; i < iterationsCount; ++i)
	{
		strm << s_smallExpression;
	}
	size_t expressionSize = iterationsCount * s_smallExpression.size();
	expression.resize(expressionSize);
	strm.read(reinterpret_cast<char*>(&expression[0]), expressionSize);
	strm.seekg(0);
}

TEST(CryptingTest, CryptorsInitFailed_WrongKeyAndIvSize)
{
	ASSERT_NE(AesCryptorBase::GetKeyAndIvLen(), 0);

	RawData keySmall = { 0x0, 0x1, 0x2 };
	RawData ivSmall = { 0x4, 0x3, 0x2, 0x1, 0x0 };
	RawData keyLarge(AesCryptorBase::GetKeyAndIvLen() + 5);
	RawData ivLarge(AesCryptorBase::GetKeyAndIvLen() + 3);
	
	EXPECT_THROW(AesEncryptor encryptor1(keySmall, ivSmall), ContainerException);
	EXPECT_THROW(AesDecryptor decryptor1(keySmall, ivNormal), ContainerException);
	EXPECT_THROW(AesEncryptor encryptor2(keyNormal, ivSmall), ContainerException);
	EXPECT_THROW(AesDecryptor decryptor2(keyLarge, ivLarge), ContainerException);
	EXPECT_THROW(AesEncryptor encryptor3(keyLarge, ivNormal), ContainerException);
	EXPECT_THROW(AesDecryptor decryptor3(keyNormal, ivLarge), ContainerException);
	EXPECT_THROW(AesEncryptor encryptor4(keySmall, ivLarge), ContainerException);
	EXPECT_THROW(AesDecryptor decryptor4(ivSmall, keyLarge), ContainerException);
}

TEST(CryptingTest, CryptorsInitFailed_NormalKeyAndIvSize)
{
	EXPECT_NO_THROW(AesEncryptor encryptor1(keyNormal, ivNormal));
	EXPECT_NO_THROW(AesDecryptor decryptor1(keyNormal, ivNormal));
}

TEST(CryptingTest, EncryptString)
{
	RawData expression(StringToRawData("1234567890"));
	AesEncryptor encryptor(keyNormal, ivNormal);
	RawData encrypted;
	ASSERT_NO_THROW(encryptor.Encrypt(expression, encrypted));
	EXPECT_EQ(encrypted.size(), expression.size());
	EXPECT_NE(encrypted, expression);
}

TEST(CryptingTest, DecryptString)
{
	RawData expression(StringToRawData("1234567890"));
	AesEncryptor encryptor(keyNormal, ivNormal);
	RawData encrypted;
	uint64_t encryptedSize = 0;
	encryptor.Encrypt(expression, encrypted); // This test case must be passed above

	RawData decrypted;
	AesDecryptor decryptor(keyNormal, ivNormal);
	ASSERT_NO_THROW(decryptor.Decrypt(encrypted, decrypted));

	EXPECT_EQ(expression, decrypted);
}

TEST(CryptingTest, EncryptStream)
{
	std::stringstream strmIn;
	RawData expression;
	FillStream(strmIn, expression);

	std::stringstream strmOut;
	AesEncryptor encryptor(keyNormal, ivNormal);
	uint64_t encryptedSize = 0;
	ASSERT_NO_THROW(encryptedSize = encryptor.Encrypt(strmIn, strmOut, expression.size()));
	ASSERT_EQ(encryptedSize, expression.size());

	EXPECT_EQ(encryptedSize, expression.size());
	RawData encrypted(static_cast<unsigned int>(encryptedSize));
	strmOut.read(reinterpret_cast<char*>(&encrypted[0]), encryptedSize);
	EXPECT_NE(encrypted, expression);
}

TEST(CryptingTest, DecryptStream)
{
	std::stringstream strmIn;
	RawData expression;
	FillStream(strmIn, expression);

	std::stringstream strmEncrypted;
	AesEncryptor encryptor(keyNormal, ivNormal);
	encryptor.Encrypt(strmIn, strmEncrypted, expression.size()); // This test case must be passed above

	std::stringstream strmDecrypted;
	AesDecryptor decryptor(keyNormal, ivNormal);
	uint64_t decryptedSize = 0;
	ASSERT_NO_THROW(decryptedSize = decryptor.Decrypt(strmEncrypted, strmDecrypted, expression.size()));
	ASSERT_EQ(decryptedSize, expression.size());

	RawData decrypted(static_cast<unsigned int>(decryptedSize));
	strmDecrypted.read(reinterpret_cast<char*>(&decrypted[0]), decryptedSize);
	EXPECT_EQ(decrypted, expression);
}