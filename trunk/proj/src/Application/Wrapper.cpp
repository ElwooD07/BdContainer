#include "stdafx.h"
#include "Wrapper.h"
#include "Utils.h"

using namespace dbc;

Container * container;

bool operator<(const ElementInfo &left, const ElementInfo &right)
{
	return (left.Type == FOLDER_TYPE && right.Type != FOLDER_TYPE);
}

bool operator==(const ElementInfo &left, const ElementInfo &right)
{
	return (left.Type == right.Type);
}

Container::Container(const std::wstring &dbpath, const std::string &password)
{
	m_cont = Connect(dbpath, password); // Using of the public function that returns IContainer interface
	m_root = m_cont->GetRoot();
	m_fold = dynamic_cast<IContainerFolder *>(m_root->Clone());
	m_file = 0;
	m_stream = 0;
	m_path_sep = m_cont->PathSep();
}

bool Container::Disconnect()
{
	try { m_cont->Disconnect(); }
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::Clear()
{
	try { m_cont->Clear(); }
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::IsConnected()
{
	return m_cont != 0;
}

bool Container::FolderGoTo(const std::wstring &path)
{
	try
	{
		IContainerElement * ice = m_cont->GetElement(path);
		if (ice->GetElementType() != FOLDER_TYPE)
			throw ContainerException(DB_FS_NOT_FOUND);
		m_fold = dynamic_cast<IContainerFolder *>(ice);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderChoose(const std::wstring &child_name)
{
	try
	{
		IContainerElement * ice = m_fold->GetChild(child_name);
		if (ice->GetElementType() != FOLDER_TYPE)
			throw ContainerException(DB_FS_NOT_FOUND);
		IContainerFolder * m_fold_tmp = dynamic_cast<IContainerFolder *>(ice);
		delete m_fold;
		m_fold = m_fold_tmp;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderCreate(const std::wstring &name)
{
	try
	{
		IContainerElement * ice = m_fold->CreateChild(name, FOLDER_TYPE);
		delete ice;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderRename(const std::wstring &new_name)
{
	try
	{
		m_fold->Rename(new_name);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderMoveTo(const std::wstring &new_path)
{
	return ElementMoveTo(new_path, m_fold);
}

bool Container::FolderRemove()
{
	try
	{
		IContainerFolder * parent = 0;
		if (!m_fold->SameEntry(m_root))
			parent = m_fold->GetParentEntry(); // This method will raise an exception "element not found", if it's a root
		m_fold->Remove(); // This method will raise an exception "action is forbidden", if it's a root
		m_fold = parent;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderInfo(ElementInfo &out)
{
	try
	{
		out.Name = m_fold->GetName();
		out.Path = m_fold->GetPath();
		out.Type = m_fold->GetElementType();
		m_fold->GetProperties(out.Props);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderPath(std::wstring &out)
{
	out.clear();
	try
	{
		out = m_fold->GetPath();
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FolderChildrenInfo(std::vector<ElementInfo> &out, int types)
{
	out.clear();
	try
	{
		if (m_fold->HasChildren())
		{
			IElementsIterator * ei = m_fold->EnumFsEntries();
			while(ei->HasCurrent())
			{
				IContainerElement * ice = ei->Current();
				ElementProperties props;
				ice->GetProperties(props);
				if (ice->GetElementType() & types)
				{
					ElementInfo info(ice->GetName(), ice->GetPath(),
						ice->GetElementType(), props);
					delete ice;
					out.push_back(info);
				}
				ei->MoveNext();
			}
		}
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileGoTo(const std::wstring &path)
{
	try
	{
		IContainerElement * ice = 0;
		ice = m_cont->GetElement(path);
		if (ice->GetElementType() != FILE_TYPE)
			throw ContainerException(DB_FS_NOT_FOUND);
		m_file = dynamic_cast<IContainerFile *>(ice);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileChoose(const std::wstring &child_name)
{
	try
	{
		IContainerElement * ice = m_fold->GetChild(child_name);
		if (ice->GetElementType() != FILE_TYPE)
			throw ContainerException(DB_FS_NOT_FOUND);
		IContainerFile * m_file_tmp = dynamic_cast<IContainerFile *>(ice);
		delete m_file;
		m_file = m_file_tmp;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileCreate(const std::wstring &name)
{
	try
	{
		IContainerElement * ice = m_fold->CreateChild(name, FILE_TYPE);
		delete ice;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileRename(const std::wstring &new_name)
{
	try
	{
		m_file->Rename(new_name);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileMoveTo(const std::wstring &new_path)
{
	return ElementMoveTo(new_path, m_file);
}

bool Container::FileRemove()
{
	try
	{
		m_file->Remove();
		delete m_file;
		m_file = 0;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileInfo(ElementInfo &out)
{
	try
	{
		out.Name = m_file->GetName();
		out.Path = m_file->GetPath();
		out.Type = m_file->GetElementType();
		m_file->GetProperties(out.Props);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FilePath(std::wstring &out)
{
	out.clear();
	try
	{
		out = m_file->GetPath();
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::FileStreamsInfo(std::vector<StreamInfo> &out)
{
	out.clear();
	try
	{
		if (m_file->GetBinaryCount() > 0)
		{
			std::wstring fpath = m_file->GetPath();
			IStreamsIterator * si = m_file->EnumBinaryStreams();
			for (IBinaryStream * bs = si->Current();
				si->HasCurrent(); si->MoveNext(), bs = si->Current())
			{
				StreamInfo info(fpath, bs->GetName(),
					bs->GetSize());
				delete bs;
				out.push_back(info);
			}
		}
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamChoose(const std::wstring &stream_name)
{
	try
	{
		if (m_stream)
		{
			delete m_stream;
			m_stream = 0;
		}
		m_stream = m_file->OpenBinaryStream(stream_name, AllAccess);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamCreate(const std::wstring &stream_name, __int64 size)
{
	try
	{
		m_file->CreateBinaryStream(stream_name, size);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamRemove()
{
	bool ret = StreamRemove(m_stream->GetName());
	if (ret)
	{
		delete m_stream;
		m_stream = 0;
	}
	return ret;
}

bool Container::StreamRemove(const std::wstring &stream_name)
{
	try
	{
		m_file->RemoveBinaryStream(stream_name);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamWrite(const std::wstring &file_in, __int64 size)
{
	try
	{
		std::ifstream in(file_in, std::ios::in | std::ios::binary);
		m_stream->Write(in, size);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamRead(const std::wstring &file_out, __int64 size)
{
	try
	{
		std::ofstream out(file_out, std::ios::out | std::ios::binary);
		m_stream->Read(out, size);
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::StreamGetInfo(StreamInfo &out)
{
	try
	{
		out.FilePath = m_file->GetPath();
		out.Name = m_stream->GetName();
		out.Size = m_stream->GetSize();
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

wchar_t Container::PathSep()
{
	return m_path_sep;
}

// ########## STATICS ########## //
bool Container::Create(const std::wstring &dbpath, const std::string &password)
{
	try
	{
		ContainerProps props(password);
		CreateContainer(dbpath, props);
		PrintGood("Контейнер успешно создан!\n");
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}

bool Container::Clear(const std::wstring &dbpath, const std::string &password)
{
	try
	{
		IContainer * cont = Connect(dbpath, password);
		cont->Clear();
		cont->Disconnect();
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}
// ########## ~STATICS~ ########## //

Container::~Container()
{
	if (m_cont) delete m_cont;
	if (m_root) delete m_root;
	if (m_file) delete m_file;
	if (m_stream) delete m_stream;
}

bool Container::ElementMoveTo(const std::wstring &to_path, IContainerElement * element)
{
	try
	{
		IContainerElement * ice = m_cont->GetElement(to_path);
		if (ice->GetElementType() != FOLDER_TYPE)
			throw ContainerException(DB_FS_NOT_FOUND);
		IContainerFolder * cfold = dynamic_cast<IContainerFolder *>(ice);
		element->MoveToEntry(cfold);
		delete cfold;
	}
	catch(const ContainerException &ex)
	{
		PrintErr(ex);
		return false;
	}
	return true;
}