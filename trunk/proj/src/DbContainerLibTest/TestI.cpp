#include "stdafx.h"
#include "ContainerAPI.h"
#include "ShittyProgressObserver.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

TEST(I_FilesPartialWrite, Transactional)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, true);

	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");
	EXPECT_EQ(0, file->Size());

	size_t dataPortion1Size = clusterSize - 20;
	std::fstream strm1(CreateStream(dataPortion1Size));
	{
		file->Write(strm1, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file->Size());
	}
	// At this point file contains 1 stream

	size_t dataPortion2Size = clusterSize + 50;
	{
		AppendStream(strm1, dataPortion2Size);
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
		std::fstream strm2(CreateStream(dataPortion3Size + 200));
		ShittyProgressObserver fakeObserver;
		strm2.seekg(200); // Just to ensure that the original sequence in file will be not the same as the new sequence in strm2, which is started from pos 200.
		EXPECT_ANY_THROW(file->Write(strm2, dataPortion3Size, &fakeObserver));
		EXPECT_NE(dataPortion3Size, file->Size());
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());

		File::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
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

TEST(I_FilesPartialWrite, Transactional_Fragmented)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, true);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");
	ContainerFileGuard file3 = cont->GetRoot()->CreateFile("file3");

	size_t dataPortion1Size = clusterSize * 2 - 20; // 2 clusters total
	{
		std::fstream strm1(CreateStream(dataPortion1Size / 2));
		file1->Write(strm1, dataPortion1Size / 2);
		EXPECT_EQ(dataPortion1Size / 2, file1->Size());
		AppendStream(strm1, dataPortion1Size / 2);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file1->Size());

		File::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1Size, info.spaceUsed);
		EXPECT_EQ(2, info.streamsTotal); // 1 stream from first write, 1 stream of 2 cluster from second one
		EXPECT_EQ(1, info.streamsUsed); // stream from first write is unused after second transactional write
		EXPECT_EQ(clusterSize * 3, info.spaceAvailable);
	}

	size_t dataPortion2Size = clusterSize * 2 + 20; // 3 clusters total
	{
		std::fstream strm2(CreateStream(dataPortion2Size / 2));
		RewindStream(strm2);
		file2->Write(strm2, dataPortion2Size / 2); // rewrite first unused stream from file1
		EXPECT_EQ(dataPortion2Size / 2, file2->Size());
		// Check file2
		File::SpaceUsageInfo info = file2->GetSpaceUsageInfo();
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

		AppendStream(strm2, dataPortion2Size / 2);
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
		std::fstream strm3(CreateStream(dataPortion3Size / 2));
		file3->Write(strm3, dataPortion3Size / 2); // rewrite two unused streams from file2
		EXPECT_EQ(dataPortion3Size / 2, file3->Size());
		// Check file3
		File::SpaceUsageInfo info = file3->GetSpaceUsageInfo();
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

		AppendStream(strm3, dataPortion3Size / 2);
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

TEST(I_FilesPartialWrite, Transactional_Fragmented_StreamsTruncating)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, true);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");
	ContainerFileGuard file3 = cont->GetRoot()->CreateFile("file3");

	size_t dataPortion1Size = clusterSize * 10 - 20; // 10 clusters total
	size_t dataPortion1CuttedSize = clusterSize + 50; // 2 clusters total
	{
		std::fstream strm1(CreateStream(dataPortion1Size));
		file1->Write(strm1, dataPortion1Size);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1CuttedSize); // This operation shoud allocate new small stream and free first large stream
		File::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
		EXPECT_EQ(clusterSize * 12, info.spaceAvailable);
		EXPECT_EQ(2, info.streamsTotal);
		EXPECT_EQ(1, info.streamsUsed);
	}

	size_t dataPortion2Size = clusterSize * 2 - 20; // 2 clusters total
	{
		std::fstream strm2(CreateStream(dataPortion2Size));
		RewindStream(strm2);
		uint64_t written = file2->Write(strm2, dataPortion2Size);
		ASSERT_EQ(written, dataPortion2Size);
		// This operation shoul allocate first unused stream of file1 for file2
	}

	size_t dataPortion3Size = clusterSize * 4 - 40; // 4 clusters total
	{
		std::fstream strm3(CreateStream(dataPortion3Size));
		RewindStream(strm3);
		uint64_t written = file3->Write(strm3, dataPortion3Size);
		ASSERT_EQ(written, dataPortion3Size);
		// If allocated stream is large enough to meet fragmentation level requirements
		// (see dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements), the new stream is cutted from file2's stream.
		// Cutted stream shoul be free and should be owned by file, which is cut this stream
		File::SpaceUsageInfo info = file3->GetSpaceUsageInfo();
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
