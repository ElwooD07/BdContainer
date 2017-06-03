#include "stdafx.h"
#include "ContainerAPI.h"
#include "File.h"
#include "ContainerException.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

extern bool databaseConnected;

TEST(F_ContainerObjectsTest, ThrowAndNoThrowBehavior_Container)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root;
	EXPECT_NO_THROW(root = cont->GetRoot());
	ASSERT_NE(nullptr, root.get());

	EXPECT_NO_THROW(cont->GetInfo());
	std::string dbPath;
	EXPECT_NO_THROW(dbPath = cont->GetPath());
	EXPECT_FALSE(dbPath.empty());

	const std::string fileName = "file1";
	const std::string filePath = root->Name() + fileName;
	root->CreateFile(fileName);

	ElementGuard el;
	EXPECT_NO_THROW(el = cont->GetElement(filePath));
	EXPECT_NE(nullptr, el.get());
	el->Remove();
	EXPECT_NO_THROW(el = cont->GetElement(filePath));
	EXPECT_EQ(nullptr, el.get());

	root->CreateFile(fileName);
	EXPECT_NO_THROW(cont->Clear());
}

TEST(F_ContainerObjectsTest, ThrowAndNoThrowBehavior_Element)
{
	ASSERT_TRUE(DatabasePrepare());
	FolderGuard root = cont->GetRoot();

	struct ElementTypeData
	{
		std::string name;
		std::string path;
		ElementGuard element;
	};

	std::map<ElementType, ElementTypeData> elements;
	elements[ElementTypeFolder] = { "folder1", root->Name() + "folder1", root->CreateChild("folder1", ElementTypeFolder) };
	elements[ElementTypeFile] = { "file1", root->Name() + "file1", root->CreateChild("file1", ElementTypeFile) };
	elements[ElementTypeSymLink] = { "symlink1", root->Name() + "symlink1", root->CreateChild("symlink1", ElementTypeSymLink) };
	elements[ElementTypeDirectLink] = { "directlink1", root->Name() + "directlink1", root->CreateChild("directlink1", ElementTypeDirectLink) };
	FolderGuard folderMoveDest = root->CreateFolder("moveTo");

	for (int type = ElementTypeFolder; type <= ElementTypeDirectLink; ++type)
	{
		std::cout << "Element type = " << type << std::endl;
		ElementTypeData& data = elements[static_cast<ElementType>(type)];
		EXPECT_NO_THROW(data.element->Exists());
		EXPECT_NO_THROW(data.element->Name());
		EXPECT_NO_THROW(data.element->Path());
		EXPECT_NO_THROW(data.element->Type());
		EXPECT_NO_THROW(data.element->AsFolder());
		EXPECT_NO_THROW(data.element->AsFile());
		EXPECT_NO_THROW(data.element->AsSymLink());
		EXPECT_NO_THROW(data.element->AsDirectLink());
		EXPECT_NO_THROW(data.element->IsTheSame(*root));
		EXPECT_NO_THROW(data.element->IsChildOf(*root));
		EXPECT_NO_THROW(data.element->GetParentEntry());
		EXPECT_NO_THROW(data.element->MoveToEntry(*folderMoveDest));
		EXPECT_NO_THROW(data.element->GetProperties());
		EXPECT_NO_THROW(data.element->SetMetaInformation("tag"));
		std::string newName = data.name;
		newName.pop_back();
		EXPECT_NO_THROW(data.element->Rename(newName));
		EXPECT_NO_THROW(data.element->Remove());
		// Opterations with deleted object
		EXPECT_NO_THROW(data.element->Exists());
		EXPECT_THROW(data.element->Name(), ContainerException);
		EXPECT_THROW(data.element->Path(), ContainerException);
		EXPECT_NO_THROW(data.element->Type());
		EXPECT_NO_THROW(data.element->AsFolder());
		EXPECT_NO_THROW(data.element->AsFile());
		EXPECT_NO_THROW(data.element->AsSymLink());
		EXPECT_NO_THROW(data.element->AsDirectLink());
		EXPECT_NO_THROW(data.element->IsTheSame(*root));
		EXPECT_THROW(data.element->IsChildOf(*root), ContainerException);
		EXPECT_NO_THROW(data.element->GetParentEntry());
		EXPECT_THROW(data.element->MoveToEntry(*root), ContainerException);
		EXPECT_THROW(data.element->GetProperties(), ContainerException);
		EXPECT_THROW(data.element->SetMetaInformation("tag"), ContainerException);
		EXPECT_THROW(data.element->Rename(data.name), ContainerException);
		EXPECT_NO_THROW(data.element->Remove());
	}
}

TEST(F_ContainerObjectsTest, ExceptionalSituations_1)
{
	ASSERT_TRUE(DatabasePrepare());
	FolderGuard root = cont->GetRoot();
	
	FolderGuard ce = root->CreateFolder("folder 1");
	FileGuard ce2 = ce->CreateFile("file 1");
	EXPECT_NO_THROW(ce->Rename("file 1"));
	EXPECT_THROW(root->CreateFile("file 1"), ContainerException);

	DatabaseDisconnect();
	ASSERT_FALSE(databaseConnected);
	EXPECT_NO_THROW(ce->Type());
	EXPECT_NE(ce->AsFolder(), nullptr);
	EXPECT_EQ(ce2->AsFolder(), nullptr);
	EXPECT_EQ(ce->AsFile(), nullptr);
	EXPECT_NE(ce2->AsFile(), nullptr);
	EXPECT_NO_THROW(ce->IsTheSame(*ce));
	EXPECT_NO_THROW(ce->IsTheSame(*ce2));
	// This code should be stable without existing Container object
	EXPECT_THROW(ce->Name(), ContainerException);
	EXPECT_THROW(ce->GetProperties(), ContainerException);
	EXPECT_THROW(ce->Clone(), ContainerException);
	EXPECT_THROW(ce2->Clone(), ContainerException);
	EXPECT_THROW(ce->GetParentEntry(), ContainerException);
	EXPECT_THROW(ce->MoveToEntry(*root), ContainerException);
	EXPECT_THROW(ce->SetMetaInformation("tag"), ContainerException);
	EXPECT_THROW(ce->Exists(), ContainerException);
	EXPECT_THROW(ce->Path(), ContainerException);
}
