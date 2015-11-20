#include "stdafx.h"
#include "Utils.h"
#include "Menus.h"

using namespace dbc;

int main(int argc, char* argv[])
{
	std::wstring path;
	std::string pass;
	if (argc > 1)
	{
		int len = wcslen((wchar_t *)(argv[1]));
		path.assign(argv[1], argv[1] + len);
		if (argc > 2)
			pass.assign(argv[2], argv[2] + wcslen((wchar_t *)(argv[1])));
	}
	Prepare(path, pass);
	system("pause");
	return 0;
}