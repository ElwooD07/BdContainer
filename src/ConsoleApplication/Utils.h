#pragma once
#include "Wrapper.h"

enum GTColor {
  COLOR_DEFAULT,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BACK_WHITE,
  COLOR_BACK_GREY,
  COLOR_BACK_BLACK
};

enum BorderStyle
{
	BORDER_SMALL,
	BORDER_BIG,
	BORDER_ERROR
};

const int DEF_BORDER_LEN = 80;
// All this is in the Utils.cpp
extern const char * DEF_BORDER_SMALL;
extern const char * DEF_BORDER_BIG;
extern const char * DEF_INVITATION;

// Prepare main menu loop
void Prepare(std::wstring &cont_path = std::wstring(L""), std::string &pass = std::string(""));

bool AscYN(const std::string &message);
int AscMenu(const std::string &name, const std::vector<std::string> &items,
	const std::string &invitation = std::string(DEF_INVITATION));
void GetWstring(const std::string &message, std::wstring &out);
void GetInt64(const std::string &message, __int64 &out);

int PrintList(const std::string &name, const std::vector<std::string> &items, bool enumerate = false);
void PrintErr(const dbc::ContainerException &ex);
void PrintErr(const std::exception &ex);
void PrintErr(const std::string &msg);
void PrintBad(const std::string &msg);
void PrintGood(const std::string &msg);

void PrintInfo(const std::string &title, const ElementInfo &info);
void PrintInfo(const std::string &title, std::vector<ElementInfo> &info); // Not "const" because of sorting
void GetStreamInfo(const StreamInfo &info, std::vector<std::string> &lines_out);
void PrintInfo(const std::string &title, const StreamInfo &info);
void PrintInfo(const std::string &title, const std::vector<StreamInfo> &info);
bool PrintFileContent();
bool PrintFolderContent(int types = FOLDER_TYPE | FILE_TYPE);
void PrintChangedElementInfo(ElementType type);
bool PrintFileStreams();

std::string GetTimeString(time_t timestamp);
std::string GetSizeString(__int64 size_bytes);
void SlashedPath(const std::wstring &in, std::wstring &out, wchar_t separator);

std::string GetBorder(BorderStyle border, int len = DEF_BORDER_LEN);
void ColoredOut(GTColor color, const char * message, GTColor back_color = COLOR_BACK_BLACK);
void Pause();
void Cls();