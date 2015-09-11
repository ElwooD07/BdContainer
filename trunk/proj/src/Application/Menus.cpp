#include "stdafx.h"
#include "Menus.h"
#include "Utils.h"
#include "Wrapper.h"

extern Container * container;

bool MenuContainer()
{
	std::vector<std::string> items;
	bool success = false;
	while (!success)
	{
		items.clear();
		items.push_back("Создать новый контейнер");
		items.push_back("Открыть существующий контейнер");
		if (container)
		{
			items.push_back("-");
			items.push_back("Продолжить работу с открытым контейнером");
			items.push_back("Закрыть контейнер");
			items.push_back("Очистить открытый контейнер");
		}
		items.push_back("-");
		items.push_back("Выход");
		Cls();
		int ch = AscMenu("Начало работы", items);
		switch(ch)
		{
		case 1:
			success = MenuCreate();
			break;
		case 2:
			success = MenuConnect();
			break;
		case 3:
			if (container)
			{
				success = true;
				break;
			}
			else
				return false;
		case 4:
			MenuDisconnect();
			continue;
		case 5:
			MenuClear();
			continue;
		case 6:
			return false;
		}
		if (!success)
			Pause();
	}
	PrintGood("Контейнер успешно открыт. Вам предоставлено управление из корневого каталога.\n");
	Pause();
	return true;
}

bool MenuCreate()
{
	std::cout << "Создание нового контейнера.\n" << std::endl;
	std::wstring path;
	std::wstring pass;
	GetWstring("Введите путь или имя контейнера: ", path);
	GetWstring("Введите пароль для создаваемого контейнера: ", pass);
	if (!Container::Create(path, std::string(pass.begin(), pass.end())))
		return false;
	if (!container && AscYN("Желаете открыть только что созданный контейнер?"))
	{
		try
		{
			container = new Container(path, std::string(pass.begin(), pass.end()));
			return true;
		}
		catch(const ContainerException &ex)
		{
			PrintErr(ex);
		}
	}
	return false;
}

bool MenuConnect()
{
	if (container)
	{
		if (AscYN("Соединение еще существует. Разорвать его?"))
		{
			SubMenuDisconnect();
		}
	}
	if (!container)
	{
		std::wstring path;
		std::string pass;
		std::wstring wpass;
		GetWstring("Введите путь или имя контейнера: ", path);
		GetWstring("Введите пароль: ", wpass);
		if (!wpass.empty())
			pass.assign(wpass.begin(), wpass.end());
		try
		{
			container = new Container(path, pass); // Initializing wrapper
			return true;
		}
		catch(const ContainerException &ex)
		{
			PrintErr(ex);
		}
	}
	return false;
}

void MenuDisconnect()
{
	if (!AscYN("Вы де1йствительно хотите разорвать связь с открытым контейнером?"))
		return;
	SubMenuDisconnect();
}

void SubMenuDisconnect()
{
	if (container->Disconnect())
	{
		delete container;
		container = 0;
		PrintGood("\nСоединение было успешно разорвано\n");
	}
}

void MenuClear()
{
	if (!AscYN("Вы уверены, что хотите удалить все данные из открытого контейнера?"))
		return;
	if (container->Clear())
	{
		PrintGood("Контейнер успешно очищен\n");
		Pause();
	}
}

void MenuFolder()
{
	if (!container)
		return;

	bool success = true;
	bool exit = false;

	while(!exit)
	{
		ElementInfo info;
		success = container->FolderInfo(info);
		if (!success)
			return;
		std::wstring invitation;
		success = container->FolderPath(invitation);
		if (!success)
			return;
		invitation.push_back('>');
		std::vector<std::string> items;
		items.push_back("Показать содержимое");
		items.push_back("-");
		items.push_back("Выбрать папку");
		items.push_back("Выбрать файл");
		items.push_back("-");
		items.push_back("Создать папку");
		items.push_back("Создать файл");
		items.push_back("Перейти к другой папке (путь)");
		items.push_back("-");
		items.push_back("Информация о текущей папке");
		items.push_back("Редактировать текущую папку");
		items.push_back("-");
		items.push_back("Вернуться в главное меню контейнера");
		Cls();
		int choice = AscMenu("Меню текущей папки:", items,
			std::string(invitation.begin(), invitation.end()));
		if (choice >= 2 && choice <= 5)
		{
			Cls();
			int types = 0;
			if (choice == 2 || choice == 4)
				types = FOLDER_TYPE;
			else
				types = FILE_TYPE;
			success = PrintFolderContent(types);
			if (!success && (choice == 2 || choice == 3))
			{
				Pause();
				continue;
			}
		}
		std::wstring name;
		switch(choice)
		{
		case 1:
			PrintFolderContent();
			Pause();
			continue;
		case 2:
			GetWstring("Введите имя выбранной папки: ", name);
			success = container->FolderChoose(name);
			if (success)
				PrintChangedElementInfo(FOLDER_TYPE);
			break;
		case 3:
			GetWstring("Введите имя выбранного файла: ", name);
			success = container->FileChoose(name);
			if (success)
				MenuFile();
			break;
		case 4:
			GetWstring("Введите имя новой папки: ", name);
			success = container->FolderCreate(name);
			if (success)
				PrintGood("Папка успешно создана\n");
			break;
		case 5:
			GetWstring("Введите имя нового файла: ", name);
			success = container->FileCreate(name);
			if (success)
				PrintGood("Файл был успешно создан\n");
			break;
		case 6:
			GetWstring("Введите путь папки для перехода: ", name);
			success = container->FolderGoTo(name);
			if (success)
				PrintChangedElementInfo(FOLDER_TYPE);
			break;
		case 7:
			success = container->FolderInfo(info);
			if (success)
				PrintInfo("Информация о текущей папке: ", info);
			break;
		case 8:
			MenuElementEdit(info);
			break;
		case 9:
			exit = true;
			PrintGood("Сейчас вы будете перемещены в меню контейнера\n");
			break;
		}
		Pause();
	}
}

void MenuElementEdit(const ElementInfo &info)
{
	bool success;
	std::string str;
	std::wstring name;
	Cls();
	PrintInfo("Информация о выбранном объекте:", info);
	std::vector<std::string> items;
	items.push_back("Переместить в ... (путь)");
	items.push_back("Переименовать");
	items.push_back("Удалить");
	items.push_back("-");
	items.push_back("Вернуться");
	int choice = AscMenu("Варианты редактирования объекта:", items);
	switch(choice)
	{
	case 1:
		str = "Введите путь существующей папки, в которую следует переместить ";
		str += (info.Type == FOLDER_TYPE) ? "текущую папку:\n" : "текущий файл:\n";
		GetWstring(str, name);
		if (info.Type == FOLDER_TYPE)
			success = container->FolderMoveTo(name);
		else
			success = container->FileMoveTo(name);
		if (success)
			PrintChangedElementInfo(info.Type);
		break;
	case 2:
		GetWstring("Введите новое имя: ", name);
		if (info.Type == FOLDER_TYPE)
			success = container->FolderRename(name);
		else
			success = container->FileRename(name);
		if (success)
			PrintChangedElementInfo(info.Type);
		break;
	case 3:
		if (info.Type == FOLDER_TYPE)
		{
			success = container->FolderRemove();
			str = "Папка была успешно удалена\n";
		}
		else
		{
			success = container->FileRemove();
			str = "Файл был успешно удален\n";
		}
		if (success)
			PrintGood(str);
		break;
	case 4:
		str = "Сейчас вы будете перемещены в меню редактирования объекта\n\"";
		str += std::string(info.Path.begin(), info.Path.end());
		str += "\"\n";
		PrintGood(str);
		break;
	}
}

void MenuFile()
{
	if (!container)
		return;

	std::wstring name;
	bool success = true;

	__int64 size = 0;

	while(true)
	{
		ElementInfo info;
		success = container->FileInfo(info);
		if (!success)
			return;
		std::wstring invitation;
		success = container->FilePath(invitation);
		if (!success)
			return;
		invitation.push_back('>');
		std::vector<std::string> items;
		items.push_back("Показать доступные потоки");
		items.push_back("-");
		items.push_back("Открыть поток");
		items.push_back("Создать новый поток");
		items.push_back("Удалить поток");
		items.push_back("-");
		items.push_back("Информация о текущем файле");
		items.push_back("Редактировать текущий файл");
		items.push_back("-");
		items.push_back("Вернуться в меню папок");
		Cls();
		int choice = AscMenu("Меню выбранного файла:",
			items, std::string(invitation.begin(), invitation.end()));
		if (choice >= 1 && choice <= 5)
		{
			Cls();
			success = PrintFileContent();
		}
		switch(choice)
		{
		case 1:
			Pause();
			continue;
		case 2:
			if (success)
			{
				GetWstring("Введите имя потока: ", name);
				success = container->StreamChoose(name);
				if (success)
					MenuStream();
			}
			break;
		case 3:
			GetWstring("Введите имя создаваемого потока: ", name);
			GetInt64("Введите размер создаваемого потока в байтах: ", size);
			success = container->StreamCreate(name, size);
			if (success)
				PrintGood("Поток для выбранного файла был успешно создан\n");
			break;
		case 4:
			if (success)
			{
				GetWstring("Введите имя удаляемого потока: ", name);
				success = container->StreamRemove(name);
				if (success)
					PrintGood("Выбранный поток был удален\n");
			}
			break;
		case 5:
			success = container->FileInfo(info);
			if (success)
				PrintInfo("Информация о текущем файле: ", info);
			break;
		case 6:
			MenuElementEdit(info);
			break;
		case 7:
			PrintGood("Сейчас вы будете перемещены в меню папок\n");
			return;
		}
		Pause();
	}
}

void MenuStream()
{
	if (!container)
		return;

	std::wstring fname;
	bool success = true;

	__int64 size = 0;

	while(true)
	{
		StreamInfo info;
		success = container->StreamGetInfo(info);
		if (!success)
			return;
		Cls();
		PrintInfo("Информация о потоке:", info);
		std::wstring invitation = info.FilePath;
		invitation += L":";
		invitation += info.Name;
		invitation.push_back('>');
		std::vector<std::string> items;
		items.push_back("Записать данные в поток");
		items.push_back("Считать данные из потока");
		items.push_back("-");
		items.push_back("Удалить поток");
		items.push_back("-");
		items.push_back("Вернуться в меню файла");
		int choice = AscMenu("Меню работы с данными потока:",
			items, std::string(invitation.begin(), invitation.end()));
		switch(choice)
		{
		case 1:
			GetWstring("Введите имя файла-источника: ", fname);
			GetInt64("Введите размер записываемых данных: ", size);
			success = container->StreamWrite(fname, size);
			if (success)
				PrintGood("Данные были успешно записаны в поток\n");
			break;
		case 2:
			GetWstring("Введите имя файла-назначения: ", fname);
			GetInt64("Введите размер считываемых данных: ", size);
			success = container->StreamRead(fname, size);
			if (success)
				PrintGood("Данные были успешно считаны из потока\n");
			break;
		case 3:
			success = container->StreamRemove();
			if (success)
			{
				PrintGood("Поток был успешно удален\n");
				return;
			}
			break;
		case 4:
			PrintGood("Сейчас вы будете перемещены в меню файла\n");
			return;
		}
		Pause();
	}
}