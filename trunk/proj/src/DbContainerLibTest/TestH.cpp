#include "stdafx.h"
#include "ContainerAPI.h"
#include "Utils.h"

using namespace dbc;

namespace
{
	class ShittyProgressObserver: public dbc::IProgressObserver
	{
	public:
		virtual ProgressState OnProgressUpdated(float progress)
		{
			//if (progress > 0.1)
			//{
				throw std::runtime_error("Unexpected error");
			//}
		}

		virtual ProgressState OnWarning(Error errCode)
		{
			return dbc::Stop;
		}

		virtual ProgressState OnError(Error errCode)
		{
			return dbc::Stop;
		}
	};

	unsigned int PrepareContainerForThisTest(dbc::ContainerGuard container, bool transactionalWrite) // returns cluster size
	{
		DataUsagePreferences& prefs = container->GetDataUsagePreferences();
		prefs.SetTransactionalWrite(transactionalWrite);
		prefs.SetClusterSizeLevel(DataUsagePreferences::CLUSTER_SIZE_MIN);
		unsigned int clusterSize = prefs.ClusterSize();
		container->SetDataUsagePreferences(prefs);
		return clusterSize;
	}
}

extern ContainerGuard cont;

void AppendData(std::ostream& strm, size_t size)
{
	const std::string s_smallExpression("0123456789abcdefghijklmnopqrstuvwxyz!");
	for (size_t i = 0; i < size;)
	{
		size_t appended = 0;
		if (i + s_smallExpression.size() > size)
		{
			appended = size - i;
		}
		else
		{
			appended = s_smallExpression.size();
		}
		strm.write(s_smallExpression.data(), appended);
		
		i += appended;
	}
	strm.flush();
}

std::fstream CreateData(size_t size)
{
	std::fstream strm("testfile.txt", std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
	AppendData(strm, size);
	return std::move(strm);
}

TEST(H_FilesInfo, SpaceUsageInfo)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = cont->GetDataUsagePreferences().ClusterSize();

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	std::fstream strm(CreateData(dataPortion1Size));
	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");

	IContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
	EXPECT_EQ(0, fileUsage.streamsTotal);
	EXPECT_EQ(0, fileUsage.streamsUsed);
	EXPECT_EQ(0, fileUsage.spaceAvailable);
	EXPECT_EQ(0, fileUsage.spaceUsed);
	file->Write(strm, dataPortion1Size);
	fileUsage = file->GetSpaceUsageInfo();
	EXPECT_EQ(1, fileUsage.streamsTotal);
	EXPECT_EQ(1, fileUsage.streamsUsed);
	EXPECT_EQ(clusterSize * 2, fileUsage.spaceAvailable);
	EXPECT_EQ(dataPortion1Size, fileUsage.spaceUsed);
}

TEST(H_FilesPartialWrite, NonTransactional)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, false);

	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");
	EXPECT_EQ(0, file->Size());

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	{
		std::fstream strm(CreateData(dataPortion1Size));
		file->Write(strm, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file->Size());
		IContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(1, fileUsage.streamsTotal);
		EXPECT_EQ(1, fileUsage.streamsUsed);
		EXPECT_EQ(clusterSize * 2, fileUsage.spaceAvailable);
		EXPECT_EQ(dataPortion1Size, fileUsage.spaceUsed);
	}

	size_t dataPortion2Size = clusterSize;
	{
		std::fstream strm(CreateData(dataPortion1Size + dataPortion2Size));
		strm.seekg(0);
		file->Write(strm, dataPortion1Size + dataPortion2Size);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());
		IContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(2, fileUsage.streamsTotal);
		EXPECT_EQ(2, fileUsage.streamsUsed);
		EXPECT_EQ(clusterSize * 3, fileUsage.spaceAvailable);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, fileUsage.spaceUsed);
	}

	size_t dataPortion3Size = clusterSize - 10;
	{
		std::fstream strm(CreateData(dataPortion1Size + dataPortion2Size));
		std::string testExpressionExpected(dataPortion3Size, '\0');
		strm.seekg(clusterSize);
		strm.read(&testExpressionExpected[0], dataPortion3Size);
		ASSERT_EQ(dataPortion3Size, strm.gcount());
		strm.seekg(clusterSize);
		strm.clear();

		file->Write(strm, dataPortion3Size);
		EXPECT_EQ(dataPortion3Size, file->Size());
		IContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(2, fileUsage.streamsTotal);
		EXPECT_EQ(1, fileUsage.streamsUsed);
		EXPECT_EQ(clusterSize * 3, fileUsage.spaceAvailable);
		EXPECT_EQ(dataPortion3Size, fileUsage.spaceUsed);

		std::stringstream strmActual;
		file->Read(strmActual, dataPortion3Size);
		std::string testExpressionActual = strmActual.str();
		EXPECT_EQ(dataPortion3Size, testExpressionActual.size());
		EXPECT_EQ(testExpressionExpected, testExpressionActual);
	}
}

TEST(H_FilesPartialWrite, Transactional)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, true);

	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");
	EXPECT_EQ(0, file->Size());

	size_t dataPortion1Size = clusterSize - 20;
	std::fstream strm1(CreateData(dataPortion1Size));
	{
		file->Write(strm1, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file->Size());
	}
	// At this point file contains 1 stream

	size_t dataPortion2Size = clusterSize + 50;
	{
		AppendData(strm1, dataPortion2Size);
		strm1.seekg(0);
		file->Write(strm1, dataPortion1Size + dataPortion2Size);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());
	}
	// At this point file contains at least 2 streams: 1 from first write and 1 from second write (1 old + 1 for transactional write)
	std::string originalData(dataPortion1Size + dataPortion2Size, '\0');
	strm1.seekg(0);
	strm1.read(&originalData[0], dataPortion1Size + dataPortion2Size);
	strm1.close();

	size_t dataPortion3Size = clusterSize + 200;
	{
		std::fstream strm2(CreateData(dataPortion3Size + 200));
		ShittyProgressObserver fakeObserver;
		strm2.seekg(200); // Just to ensure that the original sequence in file will be not the same as the new sequence in strm2, which is started from pos 200.
		EXPECT_ANY_THROW(file->Write(strm2, dataPortion3Size, &fakeObserver));
		EXPECT_NE(dataPortion3Size, file->Size());
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());

		IContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, fileUsage.spaceUsed);
	}

	// Now check the file content
	{
		std::stringstream strm3;
		uint64_t readLen = 0;
		EXPECT_NO_THROW(readLen = file->Read(strm3, originalData.size()));
		EXPECT_EQ(originalData.size(), readLen);
		EXPECT_EQ(originalData, strm3.str());
	}
}
