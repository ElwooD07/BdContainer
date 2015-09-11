#pragma once

#include "stdafx.h"

using namespace dbc;

struct ElementInfo
{
public:
	ElementInfo(std::wstring name = std::wstring(L""),
		std::wstring path = std::wstring(L""),
		ElementType type = UNKNOWN_TYPE,
		ElementProperties props = ElementProperties()
		)
	{
		Name = name;
		Path = path;
		Type = type;
		Props = props;
	}

	std::wstring Name;
	std::wstring Path;
	ElementType Type;
	ElementProperties Props;
};

bool operator<(const ElementInfo &left, const ElementInfo &right);
bool operator==(const ElementInfo &left, const ElementInfo &right);

struct StreamInfo
{
public:
	StreamInfo(std::wstring fpath = std::wstring(L""),
		std::wstring name = std::wstring(L""),
		__int64 size = 0)
	{
		FilePath = fpath;
		Name = name;
		Size = size;
	}

	std::wstring FilePath;
	std::wstring Name;
	__int64 Size;
};

class Container
{
public:
	Container(const std::wstring &dbpath, const std::string &password);
	bool Disconnect();
	bool Clear();
	bool IsConnected();

	bool FolderGoTo(const std::wstring &path);
	bool FolderChoose(const std::wstring &child_name);
	bool FolderCreate(const std::wstring &name);
	bool FolderRename(const std::wstring &new_name);
	bool FolderMoveTo(const std::wstring &new_path);
	bool FolderRemove();

	bool FolderInfo(ElementInfo &out);
	bool FolderPath(std::wstring &out);
	bool FolderChildrenInfo(std::vector<ElementInfo> &out, int types = FOLDER_TYPE | FILE_TYPE);

	bool FileGoTo(const std::wstring &path);
	bool FileChoose(const std::wstring &child_name);
	bool FileCreate(const std::wstring &name);
	bool FileRename(const std::wstring &new_name);
	bool FileMoveTo(const std::wstring &new_path);
	bool FileRemove();

	bool FileInfo(ElementInfo &out);
	bool FilePath(std::wstring &out);
	bool FileStreamsInfo(std::vector<StreamInfo> &out);

	bool StreamChoose(const std::wstring &stream_name);
	bool StreamCreate(const std::wstring &stream_name, __int64 size);
	bool StreamRemove();
	bool StreamRemove(const std::wstring &stream_name);
	bool StreamWrite(const std::wstring &file_in, __int64 size = 0);
	bool StreamRead(const std::wstring &file_out, __int64 size = 0);

	bool StreamGetInfo(StreamInfo &out);

	wchar_t PathSep();

	static bool Create(const std::wstring &dbpath, const std::string &password);
	static bool Clear(const std::wstring &dbpath, const std::string &password);

	~Container();

private:
	bool ElementMoveTo(const std::wstring &to_path, IContainerElement * element);

	IContainer * m_cont; // IContainer interface of the connected container object
	IContainerFolder * m_root; // Root folder
	IContainerFolder * m_fold; // Current folder
	IContainerFile * m_file; // Current file (may be 0)
	IBinaryStream * m_stream; // Current stream of the current file (may be 0 too)
	wchar_t m_path_sep;
};