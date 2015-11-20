#include "stdafx.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Utils.h"
#include "impl/Utils/FsUtils.h"

using namespace dbc;

extern ContainerGuard cont;

namespace
{
	std::string fold_name_base = "folder ";
	std::string non_conflict_name = "arbadakarba";

	typedef std::vector<ContainerFolderGuard> Folders_vt;
	typedef std::vector<std::string> FoldersNames_vt;

	void CreateFoldNames(size_t count, FoldersNames_vt& names)
	{
		for (size_t i = 0; i < count; ++i)
		{
			std::string fold_name(fold_name_base);
			fold_name.push_back(static_cast<char>(i + 97));
			names.push_back(fold_name);
		}
	}
}

TEST(D_FileSystemTest, Folders_Create_1)
{
	ASSERT_TRUE(DatabasePrepare());

	FoldersNames_vt foldsNames;
	const unsigned char foldsInPack = 3;
	CreateFoldNames(foldsInPack, foldsNames);
	ASSERT_FALSE(foldsNames.empty());

	ContainerFolderGuard root = cont->GetRoot();
	ContainerElementGuard ce;

	Folders_vt folds;
	for (int i = 0; i < foldsInPack; ++i)
	{
		EXPECT_NO_THROW(root->CreateChild(foldsNames[i], ElementTypeFolder));
		EXPECT_NO_THROW(ce = root->GetChild(foldsNames[i]));
		EXPECT_EQ(foldsNames[i], ce->Name());
		EXPECT_EQ(ElementTypeFolder, ce->Type());
		ContainerFolder* cf = ce->AsFolder();
		ASSERT_NE(cf, nullptr);
		folds.push_back(cf->AsFolder()->Clone());
	}
	for (int i = 0; i < foldsInPack; ++i)
	{
		EXPECT_THROW(root->CreateChild(foldsNames[i], ElementTypeFolder), ContainerException);
		EXPECT_THROW(root->CreateChild(foldsNames[i], ElementTypeFile), ContainerException);
		EXPECT_TRUE(folds[i]->Exists());
	}
}

TEST(D_FileSystemTest, Folders_Create_2_Ex)
{
	ASSERT_TRUE(DatabasePrepare());

	FoldersNames_vt foldsNames;
	const unsigned char foldsInPack = 3;
	CreateFoldNames(foldsInPack, foldsNames);
	ASSERT_FALSE(foldsNames.empty());

	ContainerFolderGuard root;
	ASSERT_NO_THROW(root = cont->GetRoot());
	ContainerFolderGuard new_root;
	ASSERT_NO_THROW(new_root = root->CreateFolder("new root"));
	
	ASSERT_NE(new_root.get(), nullptr);
	Folders_vt folds;

	for (int i = 0; i < foldsInPack; ++i)
	{
		ASSERT_NO_THROW(folds.push_back(new_root->CreateFolder(foldsNames[i])));
	}

	EXPECT_NO_THROW(new_root->Rename(non_conflict_name));
	EXPECT_THROW(new_root->Rename(non_conflict_name), ContainerException);
	std::string compared_path(root->Name() + non_conflict_name + dbc::PATH_SEPARATOR + foldsNames[foldsInPack - 1]);
	EXPECT_EQ(compared_path, folds[foldsInPack - 1]->Path());

	ContainerElementGuard ce;
	DbcElementsIterator itr;
	ASSERT_NO_THROW(itr = new_root->EnumFsEntries());
	for (int i = 0; i < foldsInPack; ++i)
	{
		ASSERT_TRUE(itr->HasNext());
		EXPECT_NO_THROW(ce = itr->Next());
		EXPECT_EQ(ce->Name(), foldsNames[i]);
	}
	EXPECT_FALSE(itr->HasNext());
	EXPECT_NO_THROW(itr->Rewind());
	ASSERT_TRUE(itr->HasNext());
}

TEST(D_FileSystemTest, Folders_Create_3)
{
	ASSERT_TRUE(DatabasePrepare());

	FoldersNames_vt foldsNames;
	const unsigned char foldsInPack = 5;
	CreateFoldNames(foldsInPack, foldsNames);
	ASSERT_FALSE(foldsNames.empty());
	ContainerFolderGuard root;
	ASSERT_NO_THROW(root = cont->GetRoot());

	for (int i = 1; i < foldsInPack; ++i)
	{
		ContainerFolderGuard new_root;
		ASSERT_NO_THROW(new_root = root->CreateFolder(foldsNames[i]));

		for (int j = 0; j < foldsInPack; ++j)
		{
			ContainerElementGuard ce;
			EXPECT_NO_THROW(new_root->CreateChild(foldsNames[j], ElementTypeFolder));
			EXPECT_NO_THROW(ce = new_root->GetChild(foldsNames[j]));
			EXPECT_EQ(foldsNames[j], ce->Name());
			EXPECT_EQ(ElementTypeFolder, ce->Type());
			ContainerFolder* cf = ce->AsFolder();
			ASSERT_NE(cf, nullptr);

			EXPECT_THROW(new_root->CreateChild(foldsNames[j], ElementTypeFolder), ContainerException);
			EXPECT_THROW(new_root->CreateChild(foldsNames[j], ElementTypeFile), ContainerException);
			std::string compared_path(utils::SlashedPath(new_root->Path()) + foldsNames[j]);
			EXPECT_EQ(compared_path, cf->Path());
		}
	}
}

TEST(D_FileSystemTest, Folders_Create_Delete)
{
	ASSERT_TRUE(DatabasePrepare());
	ContainerFolderGuard root = cont->GetRoot();

	FoldersNames_vt foldsNames;
	const unsigned char foldsInPack = 5;
	CreateFoldNames(foldsInPack, foldsNames);
	ASSERT_FALSE(foldsNames.empty());
	
	ContainerFolderGuard cfold;
	for (size_t i = 0; i < foldsInPack; ++i)
	{
		ASSERT_NO_THROW(cfold = root->CreateChild(foldsNames[i], ElementTypeFolder)->AsFolder()->Clone());
		for (size_t j = 0; j < foldsInPack; ++j)
		{
			EXPECT_NO_THROW(cfold->CreateChild(foldsNames[j], ElementTypeFile));
		}
	}
	
	std::string deletedName = foldsNames[foldsInPack / 2];
	ContainerElementGuard ce;
	
	EXPECT_NO_THROW(ce = root->GetChild(deletedName));
	ASSERT_NE(ce->AsFolder(), nullptr);
	EXPECT_NO_THROW(cfold = ce->AsFolder()->Clone());
	EXPECT_NO_THROW(ce = cfold->GetChild(deletedName));
	EXPECT_NO_THROW(cfold->Remove());
	EXPECT_FALSE(ce->Exists());
	EXPECT_THROW(cfold->GetChild(deletedName), ContainerException);
}

TEST(D_FileSystemTest, FoldersIterator)
{
	ASSERT_TRUE(DatabasePrepare());
	ContainerFolderGuard root = cont->GetRoot();

	const size_t foldsCount = 5;
	FoldersNames_vt foldNames;
	CreateFoldNames(foldsCount, foldNames);
	ASSERT_FALSE(foldNames.empty());
	Folders_vt folds;
	for (FoldersNames_vt::iterator it = foldNames.begin(); it != foldNames.end(); ++it)
	{
		ASSERT_NO_THROW(folds.push_back(root->CreateFolder(*it)));
	}

	DbcElementsIterator ei;
	ASSERT_NO_THROW(ei = root->EnumFsEntries());
	EXPECT_FALSE(ei->Empty());
	ContainerElementGuard ce;
	for (size_t i = 0; ei->HasNext(); ++i)
	{
		ce = ei->Next();
		EXPECT_EQ(ElementTypeFolder, ce->Type());
		EXPECT_TRUE(ce->IsTheSame(*folds[i]));
		EXPECT_TRUE(folds[i]->Exists());
		EXPECT_NO_THROW(ce->Remove());
		EXPECT_FALSE(ce->Exists());
		EXPECT_FALSE(folds[i]->Exists());
	}
	ei->Rewind();
	EXPECT_TRUE(ei->HasNext());
	EXPECT_THROW(ei->Next(), ContainerException);
	EXPECT_NO_THROW(ei = root->EnumFsEntries());
	EXPECT_TRUE(ei->Empty());
}