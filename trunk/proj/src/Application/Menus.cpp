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
		items.push_back("������� ����� ���������");
		items.push_back("������� ������������ ���������");
		if (container)
		{
			items.push_back("-");
			items.push_back("���������� ������ � �������� �����������");
			items.push_back("������� ���������");
			items.push_back("�������� �������� ���������");
		}
		items.push_back("-");
		items.push_back("�����");
		Cls();
		int ch = AscMenu("������ ������", items);
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
	PrintGood("��������� ������� ������. ��� ������������� ���������� �� ��������� ��������.\n");
	Pause();
	return true;
}

bool MenuCreate()
{
	std::cout << "�������� ������ ����������.\n" << std::endl;
	std::wstring path;
	std::wstring pass;
	GetWstring("������� ���� ��� ��� ����������: ", path);
	GetWstring("������� ������ ��� ������������ ����������: ", pass);
	if (!Container::Create(path, std::string(pass.begin(), pass.end())))
		return false;
	if (!container && AscYN("������� ������� ������ ��� ��������� ���������?"))
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
		if (AscYN("���������� ��� ����������. ��������� ���?"))
		{
			SubMenuDisconnect();
		}
	}
	if (!container)
	{
		std::wstring path;
		std::string pass;
		std::wstring wpass;
		GetWstring("������� ���� ��� ��� ����������: ", path);
		GetWstring("������� ������: ", wpass);
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
	if (!AscYN("�� ��1����������� ������ ��������� ����� � �������� �����������?"))
		return;
	SubMenuDisconnect();
}

void SubMenuDisconnect()
{
	if (container->Disconnect())
	{
		delete container;
		container = 0;
		PrintGood("\n���������� ���� ������� ���������\n");
	}
}

void MenuClear()
{
	if (!AscYN("�� �������, ��� ������ ������� ��� ������ �� ��������� ����������?"))
		return;
	if (container->Clear())
	{
		PrintGood("��������� ������� ������\n");
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
		items.push_back("�������� ����������");
		items.push_back("-");
		items.push_back("������� �����");
		items.push_back("������� ����");
		items.push_back("-");
		items.push_back("������� �����");
		items.push_back("������� ����");
		items.push_back("������� � ������ ����� (����)");
		items.push_back("-");
		items.push_back("���������� � ������� �����");
		items.push_back("������������� ������� �����");
		items.push_back("-");
		items.push_back("��������� � ������� ���� ����������");
		Cls();
		int choice = AscMenu("���� ������� �����:", items,
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
			GetWstring("������� ��� ��������� �����: ", name);
			success = container->FolderChoose(name);
			if (success)
				PrintChangedElementInfo(FOLDER_TYPE);
			break;
		case 3:
			GetWstring("������� ��� ���������� �����: ", name);
			success = container->FileChoose(name);
			if (success)
				MenuFile();
			break;
		case 4:
			GetWstring("������� ��� ����� �����: ", name);
			success = container->FolderCreate(name);
			if (success)
				PrintGood("����� ������� �������\n");
			break;
		case 5:
			GetWstring("������� ��� ������ �����: ", name);
			success = container->FileCreate(name);
			if (success)
				PrintGood("���� ��� ������� ������\n");
			break;
		case 6:
			GetWstring("������� ���� ����� ��� ��������: ", name);
			success = container->FolderGoTo(name);
			if (success)
				PrintChangedElementInfo(FOLDER_TYPE);
			break;
		case 7:
			success = container->FolderInfo(info);
			if (success)
				PrintInfo("���������� � ������� �����: ", info);
			break;
		case 8:
			MenuElementEdit(info);
			break;
		case 9:
			exit = true;
			PrintGood("������ �� ������ ���������� � ���� ����������\n");
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
	PrintInfo("���������� � ��������� �������:", info);
	std::vector<std::string> items;
	items.push_back("����������� � ... (����)");
	items.push_back("�������������");
	items.push_back("�������");
	items.push_back("-");
	items.push_back("���������");
	int choice = AscMenu("�������� �������������� �������:", items);
	switch(choice)
	{
	case 1:
		str = "������� ���� ������������ �����, � ������� ������� ����������� ";
		str += (info.Type == FOLDER_TYPE) ? "������� �����:\n" : "������� ����:\n";
		GetWstring(str, name);
		if (info.Type == FOLDER_TYPE)
			success = container->FolderMoveTo(name);
		else
			success = container->FileMoveTo(name);
		if (success)
			PrintChangedElementInfo(info.Type);
		break;
	case 2:
		GetWstring("������� ����� ���: ", name);
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
			str = "����� ���� ������� �������\n";
		}
		else
		{
			success = container->FileRemove();
			str = "���� ��� ������� ������\n";
		}
		if (success)
			PrintGood(str);
		break;
	case 4:
		str = "������ �� ������ ���������� � ���� �������������� �������\n\"";
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
		items.push_back("�������� ��������� ������");
		items.push_back("-");
		items.push_back("������� �����");
		items.push_back("������� ����� �����");
		items.push_back("������� �����");
		items.push_back("-");
		items.push_back("���������� � ������� �����");
		items.push_back("������������� ������� ����");
		items.push_back("-");
		items.push_back("��������� � ���� �����");
		Cls();
		int choice = AscMenu("���� ���������� �����:",
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
				GetWstring("������� ��� ������: ", name);
				success = container->StreamChoose(name);
				if (success)
					MenuStream();
			}
			break;
		case 3:
			GetWstring("������� ��� ������������ ������: ", name);
			GetInt64("������� ������ ������������ ������ � ������: ", size);
			success = container->StreamCreate(name, size);
			if (success)
				PrintGood("����� ��� ���������� ����� ��� ������� ������\n");
			break;
		case 4:
			if (success)
			{
				GetWstring("������� ��� ���������� ������: ", name);
				success = container->StreamRemove(name);
				if (success)
					PrintGood("��������� ����� ��� ������\n");
			}
			break;
		case 5:
			success = container->FileInfo(info);
			if (success)
				PrintInfo("���������� � ������� �����: ", info);
			break;
		case 6:
			MenuElementEdit(info);
			break;
		case 7:
			PrintGood("������ �� ������ ���������� � ���� �����\n");
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
		PrintInfo("���������� � ������:", info);
		std::wstring invitation = info.FilePath;
		invitation += L":";
		invitation += info.Name;
		invitation.push_back('>');
		std::vector<std::string> items;
		items.push_back("�������� ������ � �����");
		items.push_back("������� ������ �� ������");
		items.push_back("-");
		items.push_back("������� �����");
		items.push_back("-");
		items.push_back("��������� � ���� �����");
		int choice = AscMenu("���� ������ � ������� ������:",
			items, std::string(invitation.begin(), invitation.end()));
		switch(choice)
		{
		case 1:
			GetWstring("������� ��� �����-���������: ", fname);
			GetInt64("������� ������ ������������ ������: ", size);
			success = container->StreamWrite(fname, size);
			if (success)
				PrintGood("������ ���� ������� �������� � �����\n");
			break;
		case 2:
			GetWstring("������� ��� �����-����������: ", fname);
			GetInt64("������� ������ ����������� ������: ", size);
			success = container->StreamRead(fname, size);
			if (success)
				PrintGood("������ ���� ������� ������� �� ������\n");
			break;
		case 3:
			success = container->StreamRemove();
			if (success)
			{
				PrintGood("����� ��� ������� ������\n");
				return;
			}
			break;
		case 4:
			PrintGood("������ �� ������ ���������� � ���� �����\n");
			return;
		}
		Pause();
	}
}