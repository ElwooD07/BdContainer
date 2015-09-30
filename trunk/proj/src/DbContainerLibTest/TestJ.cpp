// This test is not more like a unit test than all other tests.
// This "Autotest" based on writing/reading random portions of data to/from random count of files
// Sometimes it helps me to find very dangerous bugs :)
#include "stdafx.h"
#include "ContainerAPI.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

void RandomWriteReadImpl()
{
	// Create data portions set
	srand(static_cast<unsigned int>(::time(0)));
	const unsigned char filesCount = 5 + (rand() % 5);
	const unsigned char portionsCount = 5 + (rand() % 3);
	std::map<unsigned char, std::vector<size_t> > dataPortions;
	std::vector<size_t> sizesTotal(filesCount);
	for (unsigned char i = 0; i < filesCount; ++i)
	{
		sizesTotal[i] = 0;
		for (unsigned char j = 0; j < portionsCount; ++j)
		{
			size_t portionSize = 1000 + (rand() % 40000);
			dataPortions[i].push_back(portionSize);
			sizesTotal[i] += portionSize;
		}
	}

	// Create files in container and real streams for data
	std::vector<ContainerFileGuard> files;
	std::vector<std::fstream> fstreams;
	const std::string fileNameBase("file");
	for (unsigned char i = 0; i < filesCount; ++i)
	{
		std::string fileName(fileNameBase);
		fileName.push_back(49 + i); // 49 is '1' in ASCII table
		files.push_back(cont->GetRoot()->CreateFile(fileName));
		fstreams.push_back(CreateStream(0, fileName));
	}

	// Write to container's files and streams
	for (unsigned char f = 0; f < filesCount; ++f)
	{
		uint64_t writtenTotal = 0;
		for (unsigned int p = 0; p < portionsCount; ++p)
		{
			size_t portionSize = dataPortions[f][p];
			AppendStream(fstreams[f], portionSize);
			RewindStream(fstreams[f]);
			writtenTotal += portionSize;
			uint64_t written = files[f]->Write(fstreams[f], writtenTotal);
			ASSERT_EQ(writtenTotal, written);
		}
		ASSERT_EQ(sizesTotal[f], files[f]->Size());
	}

	// Read and check files content
	for (unsigned char f = 0; f < filesCount; ++f)
	{
		std::string expectedContent(sizesTotal[f], '\0');
		RewindStream(fstreams[f]);
		fstreams[f].read(&expectedContent[0], sizesTotal[f]);
		std::stringstream strm;
		files[f]->Read(strm);
		std::string actualContent = strm.str();
		auto miss = std::mismatch(expectedContent.begin(), expectedContent.end(), actualContent.begin());
		EXPECT_TRUE(miss.first == expectedContent.end());
		if (miss.first != expectedContent.end())
		{
			std::cout << "contents mismatch in position " << std::distance(expectedContent.begin(), miss.first) << std::endl;
		}
	}

	// Delete files
	for (unsigned char f = 0; f < filesCount; ++f)
	{
		std::string fileName(fileNameBase);
		fileName.push_back(49 + f); // 49 is '1' in ASCII table
		fstreams[f].close();
		remove(fileName.c_str());
	}
}

TEST(J_FilesPartialWrite, Autotest_NonTransactional_RandomWriteRead)
{
	ASSERT_TRUE(DatabasePrepare());
	PrepareContainerForPartialWriteTest(cont, false);

	RandomWriteReadImpl();
}

TEST(J_FilesPartialWrite, Autotest_Transactional_RandomWriteRead)
{
	ASSERT_TRUE(DatabasePrepare());
	PrepareContainerForPartialWriteTest(cont, true);

	RandomWriteReadImpl();
}