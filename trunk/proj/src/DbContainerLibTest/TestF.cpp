#include "stdafx.h"
#include "ContainerAPI.h"
#include "File.h"
#include "ContainerException.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

extern bool databaseConnected;

TEST(F_ContainerObjectsTest, ExceptionalSituations_1)
{
	ASSERT_TRUE(DatabasePrepare());
	ContainerFolderGuard root = cont->GetRoot();
	
	ContainerFolderGuard ce = root->CreateFolder("folder 1");
	ContainerFileGuard ce2 = ce->CreateFile("file 1");
	EXPECT_NO_THROW(ce->Rename("file 1"));
	EXPECT_THROW(root->CreateFile("file 1"), ContainerException);

	DatabaseDisconnect();
	ASSERT_FALSE(databaseConnected);
	ElementProperties props;
	EXPECT_NO_THROW(ce->Type());
	EXPECT_NE(ce->AsFolder(), nullptr);
	EXPECT_EQ(ce2->AsFolder(), nullptr);
	EXPECT_EQ(ce->AsFile(), nullptr);
	EXPECT_NE(ce2->AsFile(), nullptr);
	EXPECT_NO_THROW(ce->IsTheSame(*ce));
	EXPECT_NO_THROW(ce->IsTheSame(*ce2));
	// This code should be stable without existing Container object
	EXPECT_THROW(ce->Name(), ContainerException);
	EXPECT_THROW(ce->GetProperties(props), ContainerException);
	EXPECT_THROW(ce->Clone(), ContainerException);
	EXPECT_THROW(ce2->Clone(), ContainerException);
	EXPECT_THROW(ce->GetParentEntry(), ContainerException);
	EXPECT_THROW(ce->MoveToEntry(*root), ContainerException);
	EXPECT_THROW(ce->ResetProperties("tag"), ContainerException);
	EXPECT_THROW(ce->Exists(), ContainerException);
	EXPECT_THROW(ce->Path(), ContainerException);
}