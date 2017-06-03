#include "stdafx.h"
#include "ContainerAPI.h"
#include "ContainerException.h"
#include "Utils.h"

using namespace dbc;

extern std::string db_path;
extern std::string pass;

extern ContainerGuard cont;

extern bool databaseCreated;
extern bool databaseConnected;

TEST(A_EnvironmentUtilsTest, DB_CreateConnectDisconnectRemove)
{
	DatabaseRemove();
	EXPECT_FALSE(databaseCreated);
	DatabaseCreate();
	EXPECT_TRUE(databaseCreated);
	DatabaseConnect();
	EXPECT_TRUE(databaseConnected);
	DatabaseDisconnect();
	EXPECT_FALSE(databaseConnected);
	DatabaseRemove();
	EXPECT_FALSE(databaseCreated);
}

TEST(A_DBEntireTests, CreateDB)
{
	DatabaseRemove();
	ASSERT_FALSE(databaseCreated);
	EXPECT_THROW(CreateContainer("1:\\>?*&\\..\\dsd.db", "123"), ContainerException);
	ASSERT_NO_THROW(CreateContainer(db_path, pass));
	EXPECT_THROW(CreateContainer(db_path, pass), ContainerException);
	DatabaseRemove();
}

TEST(A_DBEntireTests, Connect)
{
	DatabaseCreate();
	ASSERT_TRUE(databaseCreated);
	ASSERT_NO_THROW(cont = Connect(db_path, pass));
	DatabaseDisconnect();
}

TEST(A_DBEntireTests, ConnectThrowing)
{
	DatabaseCreate();
	ASSERT_TRUE(databaseCreated);
	DatabaseDisconnect();
	ASSERT_FALSE(databaseConnected);
	EXPECT_THROW(cont = Connect("no such DB", pass), ContainerException);
	EXPECT_THROW(cont = Connect(db_path, "Invalid password"), ContainerException);
	ASSERT_NO_THROW(cont = Connect(db_path, pass));
}

TEST(A_DBEntireTests, Reconnect)
{
	ASSERT_TRUE(DatabasePrepare());
	cont.reset();
	EXPECT_EQ(cont.get(), nullptr);
	EXPECT_NO_THROW(cont = Connect(db_path, pass));
	EXPECT_NE(cont.get(), nullptr);
}
