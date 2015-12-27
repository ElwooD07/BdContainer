#include "stdafx.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Utils.h"
#include "impl/Utils/FsUtils.h"

using namespace dbc;

extern ContainerGuard cont;

namespace
{
	std::string folder1name("1st folder");
	std::string folder2name("the 2nd folder");
	std::string file1name("first file");
	std::string file2name("other file");
	std::string folder1tag("This is the first folder, created by my Container API!");
	std::string folder2tag("And this is the 2nd folder, created by library API!");
	std::string file1tag("This is the first file, created by my Container API!");
	std::string file2tag("And this is the 2nd file,\ncreated by library API!");
}

TEST(C_FileSystemTest, Folders_Root)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root;
	ASSERT_NO_THROW(root = cont->GetRoot());
	ASSERT_NE(root.get(), nullptr);
	EXPECT_TRUE(root->IsRoot());

	std::string rootTag("This is root!\n\n\r\r");
	EXPECT_NO_THROW(root->ResetProperties(rootTag));

	std::string rootName;
	EXPECT_NO_THROW(rootName = root->Name());
	EXPECT_FALSE(rootName.empty());
	EXPECT_THROW(root->Rename("new root name"), ContainerException); // forbidden

	EXPECT_THROW(root->MoveToEntry(*root), ContainerException); // forbidden
	EXPECT_FALSE(root->HasChildren());

	ElementProperties props;
	EXPECT_NO_THROW(props = root->GetProperties());
	EXPECT_EQ(props.Tag(), rootTag);

	cont->Clear();
	EXPECT_TRUE(root->Exists());
}

TEST(C_FileSystemTest, Folders_RootAndChild)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root;
	ASSERT_NO_THROW(root = cont->GetRoot());
	ElementGuard ce;
	EXPECT_NO_THROW(ce = root->CreateChild(folder1name, ElementTypeFolder, folder1tag));
	Folder* cf = nullptr;
	EXPECT_NO_THROW(cf = ce->AsFolder());
	EXPECT_TRUE(cf->Exists());
	EXPECT_TRUE(ce->Exists());
	EXPECT_TRUE(root->HasChildren());
	EXPECT_THROW(root->MoveToEntry(*cf), ContainerException); // forbidden
	EXPECT_THROW(root->GetParentEntry(), ContainerException); // there is no parent for the root

	EXPECT_NO_THROW(cf->ResetProperties(folder1tag));
	ElementProperties props;
	EXPECT_NO_THROW(props = ce->GetProperties());
	EXPECT_EQ(props.Tag(), folder1tag);
	EXPECT_EQ(cf->Name(), ce->Name());
	EXPECT_TRUE(ce->IsTheSame(*cf));
	EXPECT_TRUE(cf->IsTheSame(*ce));
}

TEST(C_FileSystemTest, Folders_RootAndChildrenCreate)
{
	ASSERT_TRUE(DatabasePrepare());
	
	FolderGuard root;
	ElementGuard ce1;
	ElementGuard ce2;
	ElementGuard ce3;

	EXPECT_NO_THROW(root = cont->GetRoot());
	EXPECT_NO_THROW(ce1 = root->CreateChild(folder1name, ElementTypeFolder, folder1tag));
	EXPECT_NO_THROW(ce2 = root->CreateChild(file1name, ElementTypeFile, file1tag));

	Folder* cfold = nullptr;
	EXPECT_NO_THROW(ce3 = root->GetChild(folder1name));
	EXPECT_NO_THROW(cfold = ce3->AsFolder());
	ASSERT_NE(cfold, nullptr);
	EXPECT_EQ(ce3->Name(), cfold->Name());
	EXPECT_EQ(folder1name, cfold->Name());
	EXPECT_TRUE(ce1->IsTheSame(*ce3));
	EXPECT_TRUE(ce1->IsTheSame(*cfold));

	ElementProperties props1;
	EXPECT_NO_THROW(props1 = cfold->GetProperties());
	EXPECT_EQ(folder1tag, props1.Tag());
	std::string folder1path(root->Name() + folder1name);
	EXPECT_EQ(folder1path, cfold->Path());

	File* cfile = nullptr;
	EXPECT_NO_THROW(ce3 = root->GetChild(file1name));
	EXPECT_NO_THROW(cfile = ce3->AsFile());
	ASSERT_NE(cfile, nullptr);
	EXPECT_TRUE(ce2->IsTheSame(*ce3));
	EXPECT_TRUE(ce2->IsTheSame(*cfile));
	EXPECT_EQ(ce3->Name(), cfile->Name());
	EXPECT_EQ(file1name, cfile->Name());

	ElementProperties props2;
	EXPECT_NO_THROW(props2 = cfile->GetProperties());
	EXPECT_EQ(file1tag, props2.Tag());
	std::string file1path(root->Name() + file1name);
	EXPECT_EQ(file1path, cfile->Path());

	EXPECT_FALSE(ce1->IsTheSame(*ce2));
	EXPECT_FALSE(ce2->IsTheSame(*ce1));
}

TEST(C_FileSystemTest, Folders_RootAndChildrenMove)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root;
	FolderGuard ceFold1;
	FolderGuard ceFold2;
	FileGuard ceFile1;
	FileGuard ceFile2;

	EXPECT_NO_THROW(root = cont->GetRoot());
	EXPECT_NO_THROW(ceFold1 = root->CreateFolder(folder1name, folder1tag));
	EXPECT_NO_THROW(ceFold2 = ceFold1->CreateFolder(folder2name, folder2tag));
	EXPECT_NO_THROW(ceFile1 = ceFold1->CreateFile(file1name, file1tag));
	EXPECT_NO_THROW(ceFile2 = ceFold2->CreateFile(file2name, file2tag));
	
	// the path of first child folder was checked in previous test
	std::string folder2path1(root->Name() + folder1name + dbc::PATH_SEPARATOR + folder2name);
	EXPECT_EQ(folder2path1, ceFold2->Path());
	std::string file1path1(root->Name() + folder1name + dbc::PATH_SEPARATOR + file1name);
	EXPECT_EQ(file1path1, ceFile1->Path());
	std::string file2path1(root->Name() + folder1name + dbc::PATH_SEPARATOR + folder2name + dbc::PATH_SEPARATOR + file2name);
	EXPECT_EQ(file2path1, ceFile2->Path());

	EXPECT_TRUE(ceFile1->IsChildOf(*ceFold1));
	EXPECT_NO_THROW(ceFile1->MoveToEntry(*root)); // move ceFile1 to the root
	std::string file1path2(root->Name() + file1name);
	EXPECT_EQ(file1path2, ceFile1->Path());
	EXPECT_FALSE(ceFile1->IsChildOf(*ceFold1));

	EXPECT_THROW(ceFold1->MoveToEntry(*ceFold2), ContainerException); // forbidden to move parent folder to child folder
	EXPECT_THROW(ceFold2->MoveToEntry(*ceFold1), ContainerException); // forbidden to move folder to the same parent folder

	EXPECT_TRUE(ceFold2->IsChildOf(*ceFold1));
	EXPECT_TRUE(ceFile2->IsChildOf(*ceFold1));
	EXPECT_NO_THROW(ceFold2->MoveToEntry(*root)); // move ceFold2 to the root
	std::string folder2path2(root->Name() + folder2name);
	EXPECT_EQ(folder2path2, ceFold2->Path());
	std::string file2path2(utils::SlashedPath(folder2path2) + file2name);
	EXPECT_EQ(file2path2, ceFile2->Path());
	EXPECT_FALSE(ceFold2->IsChildOf(*ceFold1));
	EXPECT_FALSE(ceFile2->IsChildOf(*ceFold1));

	ElementProperties propsFile1 = ceFile1->GetProperties();
	ElementProperties propsFile2 = ceFile2->GetProperties();
	ElementProperties propsFold1 = ceFold1->GetProperties();
	ElementProperties propsFold2 = ceFold2->GetProperties();
	EXPECT_EQ(file1tag, propsFile1.Tag());
	EXPECT_EQ(file2tag, propsFile2.Tag());
	EXPECT_EQ(folder1tag, propsFold1.Tag());
	EXPECT_EQ(folder2tag, propsFold2.Tag());
}