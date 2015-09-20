#include "stdafx.h"
#include "ContainerAPI.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

TEST(G_ContainerInfoTests, IsEmpty)
{
	ASSERT_TRUE(DatabasePrepare());
	
	ContainerInfo info = cont->GetInfo();
	cont->Clear();
	EXPECT_TRUE(info->IsEmpty());

	ContainerFolderGuard root = cont->GetRoot();
	ContainerElementGuard ce = root->CreateChild("folder", ElementTypeFolder);
	EXPECT_FALSE(info->IsEmpty());

	ce->Remove();
	EXPECT_TRUE(info->IsEmpty());
}

TEST(G_ContainerInfoTests, TotalElements)
{
	ASSERT_TRUE(DatabasePrepare());
	cont->Clear();
	ContainerInfo info = cont->GetInfo();

	EXPECT_EQ(1, info->TotalElements()); // root should necessarily be in the empty container
	EXPECT_EQ(1, info->TotalElements(ElementTypeFolder));
	EXPECT_EQ(0, info->TotalElements(ElementTypeFile));
	ContainerFolderGuard root = cont->GetRoot();
	ContainerFolderGuard cfold = root->CreateFolder("folder1");
	EXPECT_EQ(2, info->TotalElements());

	cfold->CreateChild("file1", ElementTypeFile);
	EXPECT_EQ(3, info->TotalElements());

	EXPECT_EQ(2, info->TotalElements(ElementTypeFolder));
	EXPECT_EQ(1, info->TotalElements(ElementTypeFile));

	root->CreateFolder("folder2")->CreateFile("file2");

	EXPECT_EQ(5, info->TotalElements());
	EXPECT_EQ(3, info->TotalElements(ElementTypeFolder));
	EXPECT_EQ(2, info->TotalElements(ElementTypeFile));

	cfold->Remove();
	ASSERT_FALSE(cfold->Exists());

	EXPECT_EQ(3, info->TotalElements());
	EXPECT_EQ(2, info->TotalElements(ElementTypeFolder));
	EXPECT_EQ(1, info->TotalElements(ElementTypeFile));

	cont->Clear();
	EXPECT_EQ(1, info->TotalElements());
	EXPECT_EQ(1, info->TotalElements(ElementTypeFolder));
}

TEST(G_ContainerInfoTests, TotalDataSize)
{
	ASSERT_TRUE(DatabasePrepare());
	cont->Clear();
	ContainerInfo info = cont->GetInfo();
	
	ContainerFolderGuard root = cont->GetRoot();

	std::vector<std::string> data;
	data.push_back("0123456789");
	data.push_back("01234567890123456789");
	data.push_back("01234567890123456789");
	data.push_back("012345678901234");
	data.push_back("01234");
	unsigned totalSize(0);
	const std::string fileBaseName("file");
	for (size_t i = 0; i < data.size(); ++i)
	{
		ContainerFileGuard cf = root->CreateFile(fileBaseName + std::string(1, i + 97));
		EXPECT_EQ(0, cf->Size());
		std::stringstream strm;
		strm << data[i];
		cf->Write(strm, data[i].size());
		
		totalSize += data[i].size();
		EXPECT_EQ(totalSize, info->UsedSpace());
	}
	for (size_t i = 0; i < data.size(); ++i)
	{
		ContainerElementGuard ce = root->GetChild(fileBaseName + std::string(1, i + 97));
		EXPECT_EQ(data[i].size(), ce->AsFile()->Size());
		ASSERT_NO_THROW(ce->Remove());
		totalSize -= data[i].size();
		EXPECT_EQ(totalSize, info->UsedSpace());
	}
	EXPECT_EQ(0, info->UsedSpace());
}

TEST(H_FilesInfoTest, SpaceUsageInfo)
{
	ASSERT_TRUE(DatabasePrepare());
	unsigned int clusterSize = cont->GetDataUsagePreferences().ClusterSize();

	size_t dataPortion1Size = clusterSize + clusterSize / 2;
	std::fstream strm(CreateStream(dataPortion1Size));
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