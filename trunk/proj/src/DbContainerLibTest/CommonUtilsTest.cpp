#include "stdafx.h"
#include "ContainerAPI.h"
#include "impl/Utils/FsUtils.h"

TEST(FsUtilsTest, SlashedPathSimple)
{
	const std::string origPath1("c:\\file");
	std::string resPath;
	resPath = dbc::utils::SlashedPath(origPath1);
	EXPECT_EQ(origPath1 + dbc::PATH_SEPARATOR, resPath);

	const std::string origPath2("c:\\file\\file2/file3" + dbc::PATH_SEPARATOR);
	resPath = dbc::utils::SlashedPath(origPath2);
	EXPECT_EQ(origPath2 + dbc::PATH_SEPARATOR, resPath);

	const std::string origPath3("c:\\file\\file2/file3");
	resPath = dbc::utils::SlashedPath(origPath3);
	EXPECT_EQ(origPath3 + dbc::PATH_SEPARATOR, resPath);

	const std::string emptyPath;
	resPath = dbc::utils::SlashedPath(emptyPath);
	EXPECT_EQ(emptyPath, resPath);
}

TEST(FsUtilsTest, UnslashedPath)
{
	const std::string origPath1("c:\\file");
	std::string resPath;
	resPath = dbc::utils::UnslashedPath(origPath1 + dbc::PATH_SEPARATOR);
	EXPECT_EQ(origPath1, resPath);

	resPath = dbc::utils::UnslashedPath(origPath1);
	EXPECT_EQ(origPath1, resPath);

	const std::string origPath2("qwerty" + std::string(5, dbc::PATH_SEPARATOR));
	resPath = dbc::utils::UnslashedPath(origPath2);
	EXPECT_EQ("qwerty", resPath);
	
	const std::string origPath3("qwerty\\\\\\");
	resPath = dbc::utils::UnslashedPath(origPath3);
	EXPECT_EQ(origPath3, resPath);

	const std::string emptyPath;
	resPath = dbc::utils::UnslashedPath(emptyPath);
	EXPECT_EQ(emptyPath, resPath);

	const std::string rootPath(1, dbc::PATH_SEPARATOR);
	resPath = dbc::utils::UnslashedPath(rootPath);
	EXPECT_EQ(rootPath, resPath);

	const std::string rootPath2(5, dbc::PATH_SEPARATOR);
	resPath = dbc::utils::UnslashedPath(rootPath2);
	EXPECT_EQ(std::string(1, dbc::PATH_SEPARATOR), resPath);
}

TEST(FsUtilsTest, FNameIsValid)
{
	EXPECT_TRUE(dbc::utils::FNameIsValid("file"));
	EXPECT_TRUE(dbc::utils::FNameIsValid("filefilefilefile ''''filefilefilefile"));
	EXPECT_TRUE(dbc::utils::FNameIsValid("!@#$%^&()[]{}<>+_-"));
	EXPECT_FALSE(dbc::utils::FNameIsValid(""));
	EXPECT_FALSE(dbc::utils::FNameIsValid("fdd\\sfsd")); // slashes are delimeters in the path
	EXPECT_FALSE(dbc::utils::FNameIsValid("/"));
	EXPECT_FALSE(dbc::utils::FNameIsValid("\n")); // just because it isn't look good
	EXPECT_FALSE(dbc::utils::FNameIsValid("\r")); // same situation
	EXPECT_FALSE(dbc::utils::FNameIsValid("*")); // These symbols are reserved for the possibility to search by name or by path
	EXPECT_FALSE(dbc::utils::FNameIsValid("?"));
}