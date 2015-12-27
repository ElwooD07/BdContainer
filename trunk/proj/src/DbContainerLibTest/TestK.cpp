#include "stdafx.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

TEST(LinksTest, SymLink_Basics_TargetName)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string linkName = "symlink1";
	const std::string linkPath = root->Name() + linkName;

	SymLinkGuard link;
	EXPECT_THROW(root->CreateSymLink("testLink1", ""), ContainerException); // Empty target
	EXPECT_THROW(root->CreateSymLink("testLink1", "/folder/*"), ContainerException); // Forbidden file name in target
	EXPECT_THROW(root->CreateSymLink("testLink1", "/folder1/fold*er2/file"), ContainerException); // Forbidden file name in target
	EXPECT_THROW(root->CreateSymLink("testLink1", "/folder1/folder2/fi*le"), ContainerException); // Forbidden file name in target
	ASSERT_NO_THROW(link = root->CreateSymLink(linkName, linkPath), ContainerException); // Link to itself
	ASSERT_NO_THROW(link->Remove());
	ASSERT_NO_THROW(link = root->CreateSymLink(linkName, "/abc")); // Target not exists
	EXPECT_NE(nullptr, link.get());
	EXPECT_THROW(link->ChangeTarget(""), ContainerException); // Empty target
	EXPECT_THROW(link->ChangeTarget("/folder/*"), ContainerException); // Forbidden file name in target
	EXPECT_THROW(link->ChangeTarget("/folder1/fold*er2/file"), ContainerException); // Forbidden file name in target
	EXPECT_THROW(link->ChangeTarget("/folder1/folder2/fi*le"), ContainerException); // Forbidden file name in target
	EXPECT_NO_THROW(link->ChangeTarget(linkPath), ContainerException); // Link to itself

	ElementGuard element = cont->GetElement(linkPath);
	EXPECT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*link));

	EXPECT_NO_THROW(link->ChangeTarget(root->Name() + "someFakeFile2"));
	ElementGuard target;
	EXPECT_NO_THROW(target = link->Target());
	EXPECT_EQ(nullptr, target.get());

	element = root->CreateFile("someFakeFile2");
	EXPECT_NO_THROW(target = link->Target());
	EXPECT_NE(nullptr, target.get());
	EXPECT_TRUE(element->IsTheSame(*element));

	EXPECT_NO_THROW(link->Remove());
	EXPECT_FALSE(link->Exists());
	EXPECT_EQ(nullptr, root->GetChild(linkName).get());
}

TEST(LinksTest, SymLink_Basics_TargetExistance)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string linkName = "symlink1";
	const std::string linkPath = root->Name() + linkName;
	const std::string targetName = "target";
	const std::string targetPath = root->Name() + targetName;

	SymLinkGuard link;
	ASSERT_NO_THROW(link = root->CreateSymLink(linkName, "someFakeFile"));
	ElementGuard element = cont->GetElement(linkPath);
	EXPECT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*link));

	EXPECT_NO_THROW(link->ChangeTarget(targetPath));
	ElementGuard target;
	EXPECT_NO_THROW(target = link->Target());
	EXPECT_EQ(nullptr, target.get());

	element = root->CreateFile(targetName);
	EXPECT_NO_THROW(target = link->Target());
	EXPECT_NE(nullptr, target.get());
	EXPECT_TRUE(element->IsTheSame(*element));

	EXPECT_NO_THROW(link->Remove());
	EXPECT_FALSE(link->Exists());
	EXPECT_EQ(nullptr, root->GetChild(linkName).get());
}

TEST(LinksTest, SymLink_ToFile)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string fileName = "file1";
	const std::string targetPath = root->Name() + fileName;

	SymLinkGuard symlink = root->CreateSymLink("symlink1", targetPath);
	EXPECT_EQ(targetPath, symlink->TargetPath());
	ElementGuard element;
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_EQ(nullptr, element.get());

	FileGuard file = root->CreateFile(fileName);
	ASSERT_EQ(targetPath, file->Path());
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*file));

	ASSERT_NO_THROW(file->Remove());
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_EQ(nullptr, element.get());
}

TEST(LinksTest, SymLink_ToFolder)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string folderName = "folder1";
	const std::string targetPath = root->Name() + folderName;

	SymLinkGuard symlink = root->CreateSymLink("symlink1", targetPath);
	EXPECT_EQ(targetPath, symlink->TargetPath());
	ElementGuard element;
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_EQ(nullptr, element.get());

	FolderGuard folder = root->CreateFolder(folderName);
	ASSERT_EQ(targetPath, folder->Path());
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*folder));

	ASSERT_NO_THROW(folder->Remove());
	EXPECT_NO_THROW(element = symlink->Target());
	EXPECT_EQ(nullptr, element.get());
}

TEST(LinksTest, SymLink_ToSymLink)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string linkName1 = "symlink1";
	const std::string linkName2 = "symlink2";
	const std::string targetPath = root->Name() + linkName2;

	SymLinkGuard symlink1 = root->CreateSymLink("symlink1", targetPath);
	EXPECT_EQ(targetPath, symlink1->TargetPath());
	ElementGuard element;
	EXPECT_NO_THROW(element = symlink1->Target());
	EXPECT_EQ(nullptr, element.get());

	SymLinkGuard symlink2 = root->CreateSymLink(linkName2, "/abc");
	ASSERT_EQ(targetPath, symlink2->Path());
	EXPECT_NO_THROW(element = symlink1->Target());
	EXPECT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*symlink2));

	ElementGuard secondTarget = root->CreateFile("targetFile");
	EXPECT_NO_THROW(symlink2->ChangeTarget(secondTarget->Path()));
	ASSERT_NE(nullptr, symlink2->Target().get());
	EXPECT_NO_THROW(element = symlink1->Target()->AsSymLink()->Target());
	ASSERT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*secondTarget));

	ASSERT_NO_THROW(symlink2->Remove());
	EXPECT_NO_THROW(element = symlink1->Target());
	EXPECT_EQ(nullptr, element.get());
}

TEST(LinksTest, SymLink_ToDirectLink)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root = cont->GetRoot();
	const std::string symLinkName = "symlink";
	const std::string directLinkName = "directlink";
	const std::string targetPath = root->Name() + directLinkName;

	SymLinkGuard symLink = root->CreateSymLink(symLinkName, targetPath);
	EXPECT_EQ(targetPath, symLink->TargetPath());

	ElementGuard secondTarget = root->CreateFile("targetFile");
	DirectLinkGuard directLink = root->CreateDirectLink(directLinkName, secondTarget);
	ASSERT_NE(nullptr, directLink.get());

	ElementGuard element;
	EXPECT_NO_THROW(element = symLink->Target());
	ASSERT_NE(nullptr, element.get());
	EXPECT_TRUE(element->IsTheSame(*directLink));

	ASSERT_NO_THROW(directLink->Remove());
	ASSERT_NO_THROW(element = symLink->Target());
	EXPECT_EQ(nullptr, element.get());
}