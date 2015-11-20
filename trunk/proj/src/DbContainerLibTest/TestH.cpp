#include "stdafx.h"
#include "ContainerAPI.h"
#include "Utils.h"
#include "ShittyProgressObserver.h"

using namespace dbc;

extern ContainerGuard cont;

TEST(H_FilesPartialWrite, NonTransactional_Success)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, false);

	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");
	EXPECT_EQ(0, file->Size());

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	{
		std::fstream strm(CreateStream(dataPortion1Size));
		file->Write(strm, dataPortion1Size);
		EXPECT_EQ(dataPortion1Size, file->Size());
		ContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(1, fileUsage.streamsTotal);
		EXPECT_EQ(1, fileUsage.streamsUsed);
		EXPECT_EQ(clusterSize * 2, fileUsage.spaceAvailable);
		EXPECT_EQ(dataPortion1Size, fileUsage.spaceUsed);
	}

	size_t dataPortion2Size = clusterSize;
	{
		std::fstream strm(CreateStream(dataPortion1Size + dataPortion2Size));
		RewindStream(strm);
		file->Write(strm, dataPortion1Size + dataPortion2Size);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, file->Size());
		ContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
		EXPECT_EQ(2, fileUsage.streamsTotal);
		EXPECT_EQ(2, fileUsage.streamsUsed);
		EXPECT_EQ(clusterSize * 3, fileUsage.spaceAvailable);
		EXPECT_EQ(dataPortion1Size + dataPortion2Size, fileUsage.spaceUsed);
	}

	size_t dataPortion3Size = clusterSize - 10;
	{
		std::fstream strm(CreateStream(dataPortion1Size + dataPortion2Size));
		std::string testExpressionExpected(dataPortion3Size, '\0');
		strm.seekg(clusterSize);
		strm.read(&testExpressionExpected[0], dataPortion3Size);
		ASSERT_EQ(dataPortion3Size, strm.gcount());
		strm.seekg(clusterSize);
		strm.clear();

		file->Write(strm, dataPortion3Size);
		EXPECT_EQ(dataPortion3Size, file->Size());
		ContainerFile::SpaceUsageInfo fileUsage = file->GetSpaceUsageInfo();
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
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, false);
	ContainerFileGuard file = cont->GetRoot()->CreateFile("file1");

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	{
		std::fstream strm(CreateStream(dataPortion1Size));
		file->Write(strm, dataPortion1Size);
	}

	size_t dataPortion2Size = clusterSize;
	{
		std::fstream strm(CreateStream(dataPortion1Size + dataPortion2Size));
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

TEST(H_FilesPartialWrite, NonTransactional_Fragmented)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, false);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");

	size_t totalSize1 = 0;
	std::fstream strm1(CreateStream(totalSize1));
	size_t totalSize2 = 0;
	std::fstream strm2(CreateStream(totalSize2));

	for (int i = 0; i < 10; ++i)
	{
		size_t appendedSize = clusterSize - 10;
		totalSize1 += appendedSize;
		totalSize2 += appendedSize;
		AppendStream(strm1, totalSize1);
		AppendStream(strm2, totalSize2);

		RewindStream(strm1);
		file1->Write(strm1, totalSize1);
		RewindStream(strm2);
		file2->Write(strm2, totalSize2);

		ContainerFile::SpaceUsageInfo info1 = file1->GetSpaceUsageInfo();
		EXPECT_EQ(totalSize1, info1.spaceUsed);
		ContainerFile::SpaceUsageInfo info2 = file2->GetSpaceUsageInfo();
		EXPECT_EQ(totalSize2, info2.spaceUsed);
		EXPECT_EQ(info1.spaceAvailable, info2.spaceAvailable);
		EXPECT_EQ(i + 1, info1.streamsTotal);
		EXPECT_EQ(i + 1, info1.streamsUsed);
		EXPECT_EQ(i + 1, info2.streamsTotal);
		EXPECT_EQ(i + 1, info2.streamsUsed);
	}

	// file2 become smaller on its half.
	// file 1 become larger on half of file2->Size().
	size_t newTotalSize2 = totalSize2 / 2;
	size_t newTotalSize1 = totalSize1 + newTotalSize2;
	
	RewindStream(strm2);
	ASSERT_NO_THROW(file2->Write(strm2, newTotalSize2));
	AppendStream(strm1, newTotalSize1 - totalSize1);
	RewindStream(strm1);
	ASSERT_NO_THROW(file1->Write(strm1, newTotalSize1));
	// Now files' streams should be reallocated: file 2 takes its own 5 streams and file1 takes 10 own streams + 5 streams fried by file2.
	// Note, that its possible only if file2 is rewritten earlier than file1. In other case file1 will allocate 1 new stream.

	ContainerFile::SpaceUsageInfo info1 = file1->GetSpaceUsageInfo();
	EXPECT_EQ(newTotalSize1, info1.spaceUsed);
	ContainerFile::SpaceUsageInfo info2 = file2->GetSpaceUsageInfo();
	EXPECT_EQ(newTotalSize2, info2.spaceUsed);
	EXPECT_NE(info1.spaceAvailable, info2.spaceAvailable);
	uint64_t spaceAvailableBeforeTruncating = info1.spaceAvailable;
	EXPECT_NE(info1.streamsTotal, info2.streamsTotal);
	EXPECT_NE(info1.streamsUsed, info2.streamsUsed);

	EXPECT_EQ(20, info1.streamsTotal + info2.streamsTotal);
	EXPECT_EQ(15, info1.streamsTotal);
	EXPECT_EQ(15, info1.streamsUsed);
	EXPECT_EQ(5, info2.streamsTotal);
	EXPECT_EQ(5, info2.streamsUsed);
}

TEST(H_FilesPartialWrite, NonTransactional_Fragmented_StreamsTruncating)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = PrepareContainerForPartialWriteTest(cont, false);
	ContainerFileGuard file1 = cont->GetRoot()->CreateFile("file1");
	ContainerFileGuard file2 = cont->GetRoot()->CreateFile("file2");

	size_t dataPortion1Size = clusterSize * 10 - 20; // 10 clusters total
	size_t dataPortion1CuttedSize = clusterSize + 50; // 2 clusters total
	{
		std::fstream strm1(CreateStream(dataPortion1Size));
		file1->Write(strm1, dataPortion1Size);
		RewindStream(strm1);
		file1->Write(strm1, dataPortion1CuttedSize);
		ContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
		EXPECT_EQ(clusterSize * 10, info.spaceAvailable);
		EXPECT_EQ(1, info.streamsTotal);
		// Now file1 has 1 stream with available space > clusterSize * 8.
		// This should be enough for writing small portion of data for another file.
		// See dbc::FileStreamsManager::FreeSpaceMeetsFragmentationLevelRequirements for details
	}

	size_t dataPortion2Size = clusterSize * 4 - 20; // 4 clusters total
	{
		std::fstream strm1(CreateStream(dataPortion2Size));
		file2->Write(strm1, dataPortion2Size);
		ContainerFile::SpaceUsageInfo info = file2->GetSpaceUsageInfo();
		EXPECT_EQ(dataPortion2Size, info.spaceUsed); // < clusterSize * 4
		EXPECT_EQ(clusterSize * 8, info.spaceAvailable);
		EXPECT_EQ(1, info.streamsTotal);
		// file2 content should be placed in the cutted free space in large stream of file1.
		// Now the part of file1's big stream is the one stream for file2
	}

	// Check first file
	ContainerFile::SpaceUsageInfo info = file1->GetSpaceUsageInfo();
	EXPECT_EQ(dataPortion1CuttedSize, info.spaceUsed);
	EXPECT_EQ(clusterSize * 2, info.spaceAvailable);
	EXPECT_EQ(1, info.streamsTotal);
}
