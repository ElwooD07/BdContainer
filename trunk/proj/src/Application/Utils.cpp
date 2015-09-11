#include "stdafx.h"
#include "Utils.h"
#include "Menus.h"
#include <sstream>
#include <windows.h>
using namespace dbc;

extern Container * container;

const char * DEF_BORDER_SMALL = "--------------------------------------------------------------------------------";
const char * DEF_BORDER_BIG = "################################################################################";
const char * DEF_INVITATION = "Ваш выбор: ";

void Prepare(std::wstring &cont_path, std::string &pass)
{
	setlocale(LC_ALL, "Russian"); // Setting locale for cyrillic support
	system("color 0F"); // Setting default color in console window

	std::wstring wstr;
	std::string str;
	container = 0;
	if (!cont_path.empty())
	{
		if (pass.empty())
		{
			str = "Введите пароль для контейнера \"";
			str += std::string(cont_path.begin(), cont_path.end());
			str += "\":";
			GetWstring(str, wstr);
			if (!wstr.empty())
				pass.assign(wstr.begin(), wstr.end());
		}
		try
		{
			container = new Container(cont_path, pass); // Initializing wrapper
			PrintGood("Контейнер был успешно открыт\n");
		}
		catch(const ContainerException &ex)
		{
			PrintErr(ex);
		}
		Pause();
	}
	bool contin = true;
	while (contin)
	{
		contin = MenuContainer();
		if (contin)
			MenuFolder();
	}
	if (container)
	{
		container->Disconnect();
		delete container;
		container = 0;
	}
}

bool AscYN(const std::string &message)
{
	char c = 0;
	std::cout << '\n' << message << " (y/..): ";
	fflush(stdin);
	std::cin.get(c);
	return (tolower(c) == 'y');
}

int AscMenu(const std::string &name, const std::vector<std::string> &items, const std::string &invitation)
{
	int items_count = PrintList(name, items, true);
	int choice = 0;
	while (choice <= 0 || choice > items_count)
	{
		char inp[81];
		std::cout << invitation;
		fflush(stdin);
		std::cin.getline(inp, 80, '\n');
		choice = std::atoi(inp);
	}
	return choice;
}

void GetWstring(const std::string &message, std::wstring &out)
{
	out.clear();
	const unsigned short answerSize(1024);
	char answer[answerSize];
	std::cout << message;
	fflush(stdin);
	gets_s(answer, answerSize);
	int len = strlen(answer);
	out.assign(answer, answer + len);
}

void GetInt64(const std::string &message, __int64 &out)
{
	std::wstring int_wstr;
	GetWstring(message, int_wstr);
	out = _atoi64(std::string(int_wstr.begin(), int_wstr.end()).c_str());
}

int PrintList(const std::string &name, const std::vector<std::string> &items, bool enumerate)
{ // Prints the list of a given items in a pretty table, returns the real items number (except separators)
	int items_count = 0;
	std::string res = "\n";
	res += GetBorder(BORDER_BIG);
	res += "|    ";
	res += name;
	std::string spaces(DEF_BORDER_LEN - name.length() - 7, ' ');
	res += spaces;
	res += " |";
	res += GetBorder(BORDER_SMALL);
	ColoredOut(COLOR_YELLOW, res.c_str());
	res.clear();
	if (!items.empty())
	{
		for (size_t i = 0; i < items.size(); ++i)
		{
			std::stringstream ss;
			spaces.clear();
			if (items[i] == "-")
				ss << '|' << GetBorder(BORDER_SMALL, DEF_BORDER_LEN - 2) << '|';
			else
			{
				++items_count;
				ss << "| ";
				if (enumerate)
					ss << items_count << ". ";
				ss << items[i];
				spaces.insert(0, DEF_BORDER_LEN - ss.str().length() - 2, ' ');
				ss << spaces << " |";
			}
			ColoredOut(COLOR_YELLOW, ss.str().c_str());
		}
		ColoredOut(COLOR_YELLOW, GetBorder(BORDER_BIG).c_str());
	}
	return items_count;
}

void PrintErr(const dbc::ContainerException &ex)
{
	std::stringstream ss;
	ss << ex;
	PrintErr(ss.str());
}

void PrintErr(const std::exception &ex)
{
	PrintErr(std::string(ex.what()));
}

void PrintErr(const std::string &msg)
{
	std::string res(1, '\n');
	res += GetBorder(BORDER_ERROR);
	res += msg;
	res += '\n';
	res += GetBorder(BORDER_ERROR);
	res += '\n';
	ColoredOut(COLOR_RED, res.c_str(), COLOR_BACK_BLACK);
}

void PrintBad(const std::string &msg)
{
	ColoredOut(COLOR_RED, msg.c_str());
}

void PrintGood(const std::string &msg)
{
	ColoredOut(COLOR_GREEN, msg.c_str());
}

void PrintInfo(const std::string &title, const ElementInfo &info)
{
	std::vector<std::string> items;
	std::string str;
	str = "Имя: ";
	str += std::string(info.Name.begin(), info.Name.end());
	items.push_back(str);
	str = "Путь: ";
	str += std::string(info.Path.begin(), info.Path.end());
	items.push_back(str);
	str = "Тип: ";
	if (info.Type == FOLDER_TYPE)
		str += "папка";
	else if (info.Type == FILE_TYPE)
		str += "файл";
	else
		str += "неизвестен";
	items.push_back(str);
	items.push_back("-");
	items.push_back("Свойства объекта:");
	str = "Создан: ";
	str += GetTimeString(info.Props.DateCreated());
	items.push_back(str);
	str = "Изменен: ";
	str += GetTimeString(info.Props.DateModified());
	items.push_back(str);
	str = "Открыт: ";
	str += GetTimeString(info.Props.DateLastAccess());
	items.push_back(str);
	std::wstring wtag = info.Props.Tag();
	if (!wtag.empty())
	{
		str = "Тэг: ";
		str += std::string(wtag.begin(), wtag.end());
		items.push_back(str);
	}
	PrintList(title, items);
}

void PrintInfo(const std::string &title, std::vector<ElementInfo> &info)
{
	std::vector<std::string> items;
	std::string str;
	std::wstring wstr;
	std::stable_sort(info.begin(), info.end());
	for (std::vector<ElementInfo>::iterator itr = info.begin();
		itr != info.end(); ++itr)
	{
		wstr = itr->Name;
		if (itr->Type == FOLDER_TYPE)
			SlashedPath(wstr, wstr, container->PathSep());
		str.assign(wstr.begin(), wstr.end());
		items.push_back(str);
		str = "  Тип: ";
		if (itr->Type == FOLDER_TYPE)
			str += "папка";
		else if (itr->Type == FILE_TYPE)
			str += "файл";
		else
			str += "неизвестен";
		str += ";  Создан: ";
		str += GetTimeString(itr->Props.DateCreated());
		if (itr->Type == FILE_TYPE)
		{
			str += ";  Размер: ";
			str += GetSizeString(itr->Props.Size());
		}
		items.push_back(str);
		if (itr != --info.end())
			items.push_back("-");
	}
	PrintList(title, items);
}

void GetStreamInfo(const StreamInfo &info, std::vector<std::string> &lines_out)
{
	std::string str;
	str = "Имя: ";
	str += std::string(info.Name.begin(), info.Name.end());
	str += ";  Размер: ";
	str += GetSizeString(info.Size);
	lines_out.push_back(str);
}

void PrintInfo(const std::string &title, const StreamInfo &info)
{
	std::vector<std::string> lines;
	std::string str;
	str = "Файл: ";
	str += std::string(info.FilePath.begin(), info.FilePath.end());
	lines.push_back(str);
	GetStreamInfo(info, lines);
	PrintList(title, lines);
}

void PrintInfo(const std::string &title, const std::vector<StreamInfo> &info)
{
	std::vector<std::string> all_items;
	std::string str;
	str = "Файл: ";
	str += std::string(info[0].FilePath.begin(), info[0].FilePath.end());
	all_items.push_back(str);
	all_items.push_back("-");
	for (std::vector<StreamInfo>::const_iterator itr = info.begin();
		itr != info.end(); ++itr)
	{
		GetStreamInfo(*itr, all_items);
		all_items.push_back("-");
	}
	PrintList(title, all_items);
}

bool PrintFileContent()
{
	std::vector<StreamInfo> streams;
	bool ret = container->FileStreamsInfo(streams);
	if (streams.empty())
	{
		PrintBad("Данный файл не содержит потоков");
		ret = false;
	}
	else
		PrintInfo("Содержимое файла:", streams);
	std::cout << std::endl;
	return ret;
}

bool PrintFolderContent(int types)
{
	std::vector<ElementInfo> elements;
	bool ret = container->FolderChildrenInfo(elements, types);
	if (elements.empty())
	{
		PrintBad("Не найдено ни одного элемента");
		ret = false;
	}
	else
		PrintInfo("Содержимое текущей папки:", elements);
	std::cout << std::endl;
	return ret;
}

void PrintChangedElementInfo(ElementType type)
{
	std::wstring new_path;
	std::string str;
	if (type == FOLDER_TYPE)
	{
		str = "Текущая папка изменена на \"";
		container->FolderPath(new_path);
	}
	else
	{
		str = "Текущий файл изменен на \"";
		container->FilePath(new_path);
	}
	str += std::string(new_path.begin(), new_path.end());
	str += "\"\n";
	PrintGood(str);
}

void PrintWarning(const std::string &msg)
{
	ColoredOut(COLOR_YELLOW, msg.c_str());
}

std::string GetTimeString(time_t timestamp)
{
	tm * tstruct = 0;
	localtime_s(tstruct, &timestamp);
	const unsigned short strBufSize(40);
	char buf[strBufSize];
	strftime(buf, strBufSize, "%d %b %Y, %H:%M:%S", tstruct);
	return std::string(buf, strBufSize);
}

std::string GetSizeString(__int64 size_bytes)
{
	const unsigned char bufSize(10);
	char buf[bufSize];
	double d;
	if (size_bytes < 1024)
	{
		std::stringstream ss;
		ss << size_bytes << " байт(ов)";
		return ss.str();
	}
	else if (size_bytes < 1048576)
	{
		d = (static_cast<double>(size_bytes) / 1024);
		sprintf_s(buf, bufSize, "%.2f КБ", d);
	}
	else if (size_bytes < 1073741824)
	{
		d = (static_cast<double>(size_bytes) / 1048576);
		sprintf_s(buf, bufSize, "%.2f МБ", d);
	}
	else
	{
		d = (static_cast<double>(size_bytes) / 1073741824);
		sprintf_s(buf, bufSize, "%.2f ГБ", d);
	}
	std::string ret = buf;
	return ret;
}

void SlashedPath(const std::wstring &in, std::wstring &out, wchar_t separator)
{
	out.assign(in.cbegin(), in.cend());
	if (in.empty()) return;
	if (in[in.length() - 1] != separator)
		out.push_back(separator);
}

std::string GetBorder(BorderStyle border, int len)
{
	static const char * err = "/!\\";
	const char * border_arr;
	std::string ret;
	if (len <= 0)
		return ret;
	else if (len > DEF_BORDER_LEN)
		len = DEF_BORDER_LEN;
	switch(border)
	{
	case BORDER_ERROR:
		if (len < 8)
			return GetBorder(BORDER_BIG, len);
		ret += err;
		ret += ' ';
		ret += GetBorder(BORDER_BIG, len - 8);
		ret += ' ';
		ret += err;
		return ret;
	case BORDER_SMALL:
		border_arr = DEF_BORDER_SMALL;
		break;
	case BORDER_BIG:
		border_arr = DEF_BORDER_BIG;
		break;
	}
	ret.assign(border_arr, border_arr + len);
	return ret;
}

WORD GetColorAttribute(GTColor color) {
  switch (color) {
    case COLOR_RED:    return FOREGROUND_RED;
    case COLOR_GREEN:  return FOREGROUND_GREEN;
    case COLOR_YELLOW: return FOREGROUND_RED | FOREGROUND_GREEN;
	case COLOR_BACK_WHITE:	return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	case COLOR_BACK_GREY:	return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	case COLOR_BACK_BLACK:	return 0;
    default:           return 0;
  }
}

void ColoredOut(GTColor color, const char * message, GTColor back_color)
{
	const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO buffer_info;
	GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
	const WORD old_color_attrs = buffer_info.wAttributes;

	fflush(stdout);
	SetConsoleTextAttribute(stdout_handle, GetColorAttribute(color) | FOREGROUND_INTENSITY
		| GetColorAttribute(back_color)); // Back
	printf(message);

	fflush(stdout);
	
	SetConsoleTextAttribute(stdout_handle, old_color_attrs);
}

void Pause()
{
	system("pause");
}

void Cls()
{
	system("cls");
}