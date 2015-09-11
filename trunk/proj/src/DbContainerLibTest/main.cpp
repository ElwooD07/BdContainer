#include "stdafx.h"
#include "Utils.h"
#include "ContainerAPI.h"
#include "ContainerException.h"

using namespace dbc;

std::string db_path = "dbtest.db";
std::string bin_path = "dbtest.db.bin";
std::string pass = "23"; // SHA3 hash of this password has 0

ContainerGuard cont = nullptr;

bool databaseCreated = true;
bool databaseConnected = false;

void ExceptionMessages();

int main(int argc, char* argv[])
{
	RefreshLog();
	DatabaseRemove();

	testing::InitGoogleTest(&argc, argv); 
    RUN_ALL_TESTS();

	std::cout << std::endl << "Print all possible error messages? (y/...): ";
	char answ;
	std::cin.get(answ);
	if (tolower(answ) == 'y')
	{
		ShowExceptionMessages();
	}
	system("PAUSE");
}