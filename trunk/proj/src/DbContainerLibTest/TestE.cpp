#include "stdafx.h"
#include "ContainerAPI.h"
#include "IContainerFile.h"
#include "ContainerException.h"
#include "Utils.h"

using namespace dbc;

extern ContainerGuard cont;

namespace
{
	const unsigned int contentRepeats(50);
	std::string fname = "testfile.txt";
	std::string sname = "stream 1";
}

void MakeLongString(const std::string& baseContent, size_t repeats, std::string& result)
{
	assert(result.empty());
	for (size_t i = 0; i < repeats; ++i)
	{
		result.append(baseContent);
	}
}

bool WriteTestFile(const std::string &content)
{
	std::ofstream out(fname, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out)
		return false;
	if (content.empty())
	{
		out.close();
		return true;
	}
	std::string longString;
	MakeLongString(content, contentRepeats, longString);
	out.write(longString.c_str(), longString.length());
	return out.good();
}

TEST(E_FileSystemTest, Files_CreateRemove)
{
	ASSERT_TRUE(DatabasePrepare());
	ContainerFolderGuard root = cont->GetRoot();

	ContainerElementGuard ce;
	EXPECT_NO_THROW(ce = root->CreateChild(fname, ElementTypeFile));
	ASSERT_NE(ce->AsFile(), nullptr);
	ContainerFileGuard cfile;
	EXPECT_NO_THROW(cfile = ce->AsFile()->Clone());
	
	DbcElementsIterator ei;
	EXPECT_NO_THROW(ei = root->EnumFsEntries());
	EXPECT_EQ(1, ei->Count());

	ContainerFileGuard cfile2;
	EXPECT_THROW(ce = root->CreateChild(fname, ElementTypeFile), ContainerException);
	EXPECT_NO_THROW(ce = root->CreateChild(fname + " 2", ElementTypeFile));
	EXPECT_NO_THROW(cfile2 = ce->AsFile()->Clone());

	EXPECT_NO_THROW(ei = root->EnumFsEntries());
	EXPECT_EQ(2, ei->Count());
	EXPECT_TRUE(ei->HasNext());
	EXPECT_NO_THROW(ce = ei->Next());

	EXPECT_TRUE(cfile->IsTheSame(*ce));
	EXPECT_NO_THROW(cfile->Rename(fname + " 1"));
	EXPECT_TRUE(cfile->IsTheSame(*ce));
	EXPECT_NO_THROW(ce->Remove());
	EXPECT_FALSE(ce->Exists());
	EXPECT_FALSE(cfile->Exists());
	EXPECT_THROW(ce->Remove(), ContainerException);
	EXPECT_THROW(cfile->Remove(), ContainerException);

	EXPECT_TRUE(ei->HasNext());
	EXPECT_NO_THROW(ce = ei->Next());
	EXPECT_TRUE(ce->IsTheSame(*cfile2));
}

TEST(E_FileSystemTest,  Files_Open)
{
	ASSERT_TRUE(DatabasePrepare());
	cont->Clear();
	ContainerFolderGuard root = cont->GetRoot();

	ContainerFileGuard cfile;
	EXPECT_NO_THROW(cfile = root->CreateFile(fname));

	std::string ostr;
	std::ostringstream ostrstream(ostr);
	std::string istr("0123456789");
	std::istringstream istrstream(istr);

	EXPECT_FALSE(cfile->IsOpened());
	EXPECT_EQ(NoAccess, cfile->Access());
	EXPECT_THROW(cfile->Size(), ContainerException);
	EXPECT_THROW(cfile->Read(ostrstream, istr.size()), ContainerException);
	EXPECT_THROW(cfile->Write(istrstream, istr.size()), ContainerException);

	EXPECT_NO_THROW(cfile->Open(WriteAccess));
	EXPECT_TRUE(cfile->IsOpened());
	EXPECT_EQ(WriteAccess, cfile->Access());
	EXPECT_THROW(cfile->Read(ostrstream, istr.size()), ContainerException);
	EXPECT_NO_THROW(cfile->Write(istrstream, istr.size()));
	EXPECT_THROW(cfile->Open(AllAccess), ContainerException); // already opened
	EXPECT_NO_THROW(cfile->Size());
	ASSERT_NO_THROW(cfile->Close());

	EXPECT_NO_THROW(cfile->Open(ReadAccess));
	EXPECT_TRUE(cfile->IsOpened());
	EXPECT_EQ(ReadAccess, cfile->Access());
	EXPECT_THROW(cfile->Write(istrstream, istr.size()), ContainerException);
	EXPECT_NO_THROW(cfile->Read(ostrstream, istr.size()));
	EXPECT_NO_THROW(cfile->Size());

	istrstream.seekg(0);
	ostrstream.clear();

	ASSERT_NO_THROW(cfile->Close());
	EXPECT_NO_THROW(cfile->Open(AllAccess));
	EXPECT_TRUE(cfile->IsOpened());
	EXPECT_EQ(AllAccess, cfile->Access());
	EXPECT_NO_THROW(cfile->Clear());
	EXPECT_NO_THROW(cfile->Write(istrstream, istr.size()));
	EXPECT_NO_THROW(cfile->Read(ostrstream, istr.size()));
	ASSERT_NO_THROW(cfile->Close());

	EXPECT_NO_THROW(cfile->Remove());
}

TEST(E_FileSystemTest,  Files_Write)
{
	ASSERT_TRUE(DatabasePrepare());

	const std::string baseContent("0123456789");
	const uint64_t dataSize(contentRepeats * baseContent.size());
	ASSERT_TRUE(WriteTestFile(baseContent));
	std::ifstream tfile_istream(fname);
	ASSERT_TRUE(tfile_istream.is_open());

	ContainerFileGuard cfile = cont->GetRoot()->CreateFile(fname);
	EXPECT_THROW(cfile->Size(), ContainerException);

	ASSERT_NO_THROW(cfile->Open(WriteAccess));
	EXPECT_EQ(0, cfile->Size());
	uint64_t writen(0);
	EXPECT_NO_THROW(writen = cfile->Write(tfile_istream, dataSize));
	EXPECT_EQ(dataSize, writen);
}

TEST(E_FileSystemTest, Files_Read)
{
	ASSERT_TRUE(DatabasePrepare());

	std::string origContent;
	size_t dataSize = 0;
	ContainerFileGuard cfile = cont->GetRoot()->CreateFile(fname);
	{
		const std::string baseContent("0123456789");
		dataSize = contentRepeats * baseContent.size();
		std::stringstream streamOrigContent;
		for (size_t i = 0; i < contentRepeats; ++i)
		{
			streamOrigContent << baseContent;
		}
		// Write database test file. This action performed in previous test
		cfile->Open(WriteAccess);
		cfile->Write(streamOrigContent, dataSize);
		cfile->Close();
		streamOrigContent.seekg(0);
		origContent = streamOrigContent.str();
	}

	ASSERT_NO_THROW(cfile->Open(ReadAccess));
	uint64_t read = 0;
	std::fstream streamSavedContent(fname, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
	ASSERT_TRUE(streamSavedContent.is_open());
	EXPECT_NO_THROW(read = cfile->Read(streamSavedContent, dataSize));
	EXPECT_EQ(dataSize, read);
	
	streamSavedContent.seekg(0);
	std::string savedContent(dataSize, '\0');
	streamSavedContent.read(&savedContent[0], dataSize);
	EXPECT_EQ(origContent.size(), savedContent.size());
	EXPECT_EQ(origContent, savedContent);
}