#include "stdafx.h"
#include "ContainerAPI.h"
#include "Utils.h"
#include "impl/Utils/FsUtils.h"

using namespace dbc;

extern ContainerGuard cont;

TEST(B_ContainerFuncTests, ElementProperties)
{
    ElementProperties props(3213, 0, "daskdhjkasdhjkasd|dasjdhjasdh|dasjkdhas");
	EXPECT_EQ(props.DateCreated(), 3213);
	EXPECT_EQ(props.DateModified(), 0);
	EXPECT_EQ(props.Meta(), "daskdhjkasdhjkasd|dasjdhjasdh|dasjkdhas");
    std::string largeMeta(ElementProperties::s_MaxTagLength+ 10, '0');
    props.SetMeta(largeMeta);
	EXPECT_EQ(std::string(ElementProperties::s_MaxTagLength, '0'), props.Meta());
}

TEST(B_ContainerFuncTests, GetRoot)
{
	ASSERT_TRUE(DatabasePrepare());

	FolderGuard root;
	ASSERT_NO_THROW(root = cont->GetRoot());
	EXPECT_EQ(ElementTypeFolder, root->Type());
	EXPECT_TRUE(root->IsRoot());
}

TEST(B_ContainerFuncTests, Clear)
{
	ASSERT_TRUE(DatabasePrepare());
	FolderGuard root = cont->GetRoot();

	std::string ce_name1("some name");
	std::string ce_name2("some other name");
	// Work with children entries tested in other files
	ElementGuard ce1 = root->CreateChild(ce_name1, ElementTypeFolder);
	ElementGuard ce2 = root->CreateChild(ce_name2, ElementTypeFile);
	EXPECT_TRUE(ce1->Exists());
	EXPECT_TRUE(ce2->Exists());
	EXPECT_NO_THROW(cont->Clear());
	EXPECT_FALSE(ce1->Exists());
	EXPECT_FALSE(ce2->Exists());

	ElementProperties rootProps = root->GetProperties();
	EXPECT_TRUE(rootProps.Meta().empty());
}

TEST(B_ContainerFuncTests, GetElement)
{
	ASSERT_TRUE(DatabasePrepare());
	cont->Clear();
	FolderGuard root = cont->GetRoot();

	const std::string child1name("folder 1");
	const std::string child2name("nested folder");
	const std::string child3name("last entry - the file");
	// Work with children entries tested in other files
	ASSERT_NO_THROW(root->CreateFolder(child1name)->CreateFolder(child2name)->CreateFile(child3name));
	
	const std::string child1path(root->Name() + child1name);
	const std::string child2path(utils::SlashedPath(child1path) + child2name);
	const std::string child3path(utils::SlashedPath(child2path) + child3name);
	
	ElementGuard ce;
	EXPECT_NO_THROW(ce = cont->GetElement(child3path));
	EXPECT_EQ(child3name, ce->Name());
	EXPECT_EQ(child3path, ce->Path());
	ASSERT_NO_THROW(ce->Remove());

	EXPECT_NO_THROW(ce = cont->GetElement(child2path));
	EXPECT_EQ(child2name, ce->Name());
	EXPECT_EQ(child2path, ce->Path());
	ASSERT_NO_THROW(ce->Remove());

	EXPECT_NO_THROW(ce = cont->GetElement(child1path));
	EXPECT_EQ(child1name, ce->Name());
	EXPECT_EQ(child1path, ce->Path());
	ASSERT_NO_THROW(ce->Remove());
	
	EXPECT_NO_THROW(ce = cont->GetElement(child1path));
	EXPECT_EQ(nullptr, ce.get());

	EXPECT_NO_THROW(ce = cont->GetElement(child2path));
	EXPECT_EQ(nullptr, ce.get());

	EXPECT_NO_THROW(ce = cont->GetElement(child3path));
	EXPECT_EQ(nullptr, ce.get());
}
