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
	strm.seekp(0, std::ios::end);
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

void RewindStream(std::istream& strm)
{
	strm.clear();
	strm.seekg(0);
	ASSERT_EQ(strm.tellg(), std::streamoff(0));
	ASSERT_FALSE(strm.bad());
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

TEST(H_FilesPartialWrite, NonTransactional_Success)
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
		RewindStream(strm);
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

TEST(H_FilesPartialWrite, NonTransactional_Failed)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, false);
	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	{
		std::fstream strm(CreateData(dataPortion1Size));
		file->Write(strm, dataPortion1Size);
	}

	size_t dataPortion2Size = clusterSize;
	{
		std::fstream strm(CreateData(dataPortion1Size + dataPortion2Size));
		RewindStream(strm);
		ShittyProgressObserver fakeObserver;
		uint64_t written = 0;
		uint64_t oldSize = file->Size();
		EXPECT_ANY_THROW(written = file->Write(strm, dataPortion1Size + dataPortion2Size, &fakeObserver));
		EXPECT_NE(written, dataPortion1Size + dataPortion2Size);
		EXPECT_NE(written, oldSize);
		EXPECT_EQ(oldSize, file->Size());
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
		RewindStream(strm1);
		file->Write(strm1, dataPortion1Size + dataPortion2Size);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());
	}
	// At this point file contains at least 2 streams: 1 from first write and 1 from second write (1 old + 1 for transactional write)
	std::string originalData(dataPortion1Size + dataPortion2Size, '\0');
	RewindStream(strm1);
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

TEST(H_FilesPartialWrite, Transactional_Fragmented)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, true);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");
	ContainerFileGuard file3 = cont->GetRoot()->CreateFile("file3");

	size_t dataPortion1Size = clusterSize * 2 - 20; // 2 clusters total
	{
		std::fstream strm1(CreateData(dataPortion1Size / 2));
		file1->Write(strm1, dataPortion1Size / 2);
		EXPECT_EQ(dataPortion1Size / 2, file1->Size());
		AppendData(strm1, dataPortion1Size / 2);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file1->Size());

		IContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1Size, info.spaceUsed);
		EXPECT_EQ(2, info.streamsTotal); // 1 stream from first write, 1 stream of 2 cluster from second one
		EXPECT_EQ(1, info.streamsUsed); // stream from first write is unused after second transactional write
		EXPECT_EQ(clusterSize * 3, info.spaceAvailable);
	}

	size_t dataPortion2Size = clusterSize * 2 + 20; // 3 clusters total
	{
		std::fstream strm2(CreateData(dataPortion2Size / 2));
		RewindStream(strm2);
		file2->Write(strm2, dataPortion2Size / 2); // rewrite first unused stream from file1
		EXPECT_EQ(dataPortion2Size / 2, file2->Size());
		// Check file2
		IContainerFile::SpaceUsageInfo info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size / 2, info.spaceUsed);
		EXPECT_EQ(2, info.streamsTotal); // 2 streams from first write - first unused from file1 and one newly allocated
		EXPECT_EQ(2, info.streamsUsed);
		EXPECT_EQ(clusterSize * 2, info.spaceAvailable);
		// Check file1
		info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1Size, info.spaceUsed);
		EXPECT_EQ(1, info.streamsTotal); // 1 stream from second write, first unused stream now used by file2
		EXPECT_EQ(1, info.streamsUsed);
		EXPECT_EQ(clusterSize * 2, info.spaceAvailable);

		AppendData(strm2, dataPortion2Size / 2);
		RewindStream(strm2);
		file2->Write(strm2, dataPortion2Size);
		EXPECT_EQ(dataPortion2Size, file2->Size());

		info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size, info.spaceUsed);
		EXPECT_EQ(3, info.streamsTotal); // 2 streams from first write, 1 stream of 3 clusters from second one
		EXPECT_EQ(1, info.streamsUsed); // Used only new allocated stream
		EXPECT_EQ(clusterSize * 5, info.spaceAvailable); // 2 streams of 2 clusters from first write, 1 stream of 3 clusters from second
	}

	size_t dataPortion3Size = clusterSize * 3 + 40; // 3 clusters total
	{
		std::fstream strm3(CreateData(dataPortion3Size / 2));
		file3->Write(strm3, dataPortion3Size / 2); // rewrite two unused streams from file2
		EXPECT_EQ(dataPortion3Size / 2, file3->Size());
		// Check file3
		IContainerFile::SpaceUsageInfo info = file3->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion3Size / 2, info.spaceUsed);
		EXPECT_EQ(2, info.streamsTotal);
		EXPECT_EQ(2, info.streamsUsed);
		EXPECT_EQ(clusterSize * 2, info.spaceAvailable);
		// Check file2
		info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size, info.spaceUsed);
		EXPECT_EQ(1, info.streamsTotal); // 1 stream left after write to the file3
		EXPECT_EQ(1, info.streamsUsed);
		EXPECT_EQ(clusterSize * 3, info.spaceAvailable); // 1 stream of 3 clusters left after write to the file3

		AppendData(strm3, dataPortion3Size / 2);
		RewindStream(strm3);
		file3->Write(strm3, dataPortion3Size); // allocate 1 stream of 3 clusters for new data
		EXPECT_EQ(dataPortion3Size, file3->Size());

 		info = file3->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion3Size, info.spaceUsed);
		EXPECT_EQ(3, info.streamsTotal); // 2 streams from first write, 1 stream of 4 clusters from second one
		EXPECT_EQ(1, info.streamsUsed); // Used only new allocated stream
		EXPECT_EQ(clusterSize * 6, info.spaceAvailable); // 1 cluster + 1 cluster + 4 clusters
	}
}

TEST(H_FilesPartialWrite, NonTransactional_Fragmented_StreamsTruncating)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, false);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");

	size_t dataPortion1Size = clusterSize * 10 - 20; // 10 clusters total
	size_t dataPortion1CuttedSize = clusterSize + 50; // 2 clusters total
	{
		std::fstream strm1(CreateData(dataPortion1Size));
		file1->Write(strm1, dataPortion1Size);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1CuttedSize);
		IContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
		EXPECT_EQ(clusterSize * 10, info.spaceAvailable);
		EXPECT_EQ(1, info.streamsTotal);
		// Now file1 has 1 stream with available space > clusterSize * 8.
		// This should be enough for writing small portion of data for another file.
		// See dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements for details
	}

	size_t dataPortion2Size = clusterSize * 4 - 20; // 4 clusters total
	{
		std::fstream strm1(CreateData(dataPortion2Size));
		file2->Write(strm1, dataPortion2Size);
		IContainerFile::SpaceUsageInfo info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size, info.spaceUsed); // < clusterSize * 4
		EXPECT_EQ(clusterSize * 8, info.spaceAvailable);
		EXPECT_EQ(1, info.streamsTotal);
		// file2 content should be placed in the cutted free space in large stream of file1.
		// Now the part of file1's big stream is the one stream for file2
	}

	// Check first file
	IContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
	EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
	EXPECT_EQ(clusterSize * 2, info.spaceAvailable);
	EXPECT_EQ(1, info.streamsTotal);
}

TEST(H_FilesPartialWrite, Transactional_Fragmented_StreamsTruncating)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, true);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");
	ContainerFileGuard file3 = cont->GetRoot()->CreateFile("file3");

	size_t dataPortion1Size = clusterSize * 10 - 20; // 10 clusters total
	size_t dataPortion1CuttedSize = clusterSize + 50; // 2 clusters total
	{
		std::fstream strm1(CreateData(dataPortion1Size));
		file1->Write(strm1, dataPortion1Size);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1CuttedSize); // This operation shoud allocate new small stream and free first large stream
		IContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
		EXPECT_EQ(clusterSize * 12, info.spaceAvailable);
		EXPECT_EQ(2, info.streamsTotal);
		EXPECT_EQ(1, info.streamsUsed);
	}

	size_t dataPortion2Size = clusterSize * 2 - 20; // 2 clusters total
	{
		std::fstream strm2(CreateData(dataPortion2Size));
		RewindStream(strm2);
		uint64_t written = file2->Write(strm2, dataPortion2Size);
		ASSERT_EQ(written, dataPortion2Size);
		// This operation shoul allocate first unused stream of file1 for file2
	}

	size_t dataPortion3Size = clusterSize * 4 - 40; // 4 clusters total
	{
		std::fstream strm3(CreateData(dataPortion3Size));
		RewindStream(strm3);
		uint64_t written = file3->Write(strm3, dataPortion3Size);
		ASSERT_EQ(written, dataPortion3Size);
		// If allocated stream is large enough to meet fragmentation level requirements
		// (see dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements), the new stream is cutted from file2's stream.
		// Cutted stream shoul be free and should be owned by file, which is cut this stream
		IContainerFile::SpaceUsageInfo info = file3->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion3Size, info.spaceUsed);
		EXPECT_EQ(clusterSize * 8, info.spaceAvailable);
		EXPECT_EQ(1, info.streamsTotal);
		EXPECT_EQ(1, info.streamsUsed);

		// Check file2
		info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size, info.spaceUsed);
		EXPECT_EQ(clusterSize * 2, info.spaceAvailable); // Now first stream consists only from 2 clusters - the other part was cutted off by file3
		EXPECT_EQ(1, info.streamsTotal);
		EXPECT_EQ(1, info.streamsUsed);
	}
}

TEST(H_FilesPartialWrite, Transactional_Fragmented_FreeSpaceMerge)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForThisTest(cont, true);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");

	size_t dataPortion1Size = clusterSize * 10 - 20; // 10 clusters total
	size_t totalSize = 0;
	std::fstream strm1(CreateData(dataPortion1Size));
	// First stream will very big
	for (int i = 0; i < 10; ++i)
	{
		AppendData(strm1, clusterSize);
		RewindStream(strm1);
		size_t sizeToWriteNow = dataPortion1Size + clusterSize * i - 10 * i;
		uint64_t written = file1->Write(strm1, sizeToWriteNow);
		EXPECT_EQ(sizeToWriteNow, written);
		totalSize = sizeToWriteNow;
	}

	std::fstream strm2("testfile2.txt", std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out);
	ASSERT_TRUE(strm2.is_open());
	uint64_t read = file1->Read(strm2, totalSize);
	EXPECT_EQ(totalSize, read);

	// Ensure, that the content is OK
	std::string expectedContent(totalSize, '\0');
	RewindStream(strm1);
	strm1.read(&expectedContent[0], totalSize);
	std::string actualContent(totalSize, '\0');
	RewindStream(strm2);
	strm2.read(&actualContent[0], totalSize);
	EXPECT_EQ(expectedContent, actualContent);

	// Check file1 fragmentation
	// There is should be 3 streams allocated for file1:
	// 1 empty stream with previous content
	// 1 fully occupied stream with the main portion of data
	// 1 small partly occupied stream with the last appended portion of data
	IContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
	EXPECT_EQ(totalSize, info.spaceUsed);
	uint64_t oldSpaceAvailable = info.spaceAvailable;
	EXPECT_EQ(3, info.streamsTotal);
	EXPECT_EQ(2, info.streamsUsed);

	// Rewrite all file with much smaller content
	// All unusd streams should be merged or deleted after this operation
	RewindStream(strm1);
	size_t dataPortion2Size = clusterSize + 200; // 2 clusters total
	file1->Write(strm1, dataPortion2Size);
	info = file1->GetSpaceUsageInfo();
	EXPECT_EQ(dataPortion2Size, info.spaceUsed);
	EXPECT_EQ(oldSpaceAvailable, info.spaceAvailable);
	EXPECT_EQ(2, info.streamsTotal);
	EXPECT_EQ(1, info.streamsUsed);

	strm2.seekp(0);
	read = file1->Read(strm2);
	EXPECT_EQ(dataPortion2Size, read);
}