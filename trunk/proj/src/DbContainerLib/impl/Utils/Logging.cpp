#include "stdafx.h"
#include "Logging.h"

void dbc::cmn::WriteLogEx(const std::string& message, const std::string& function, long int line)
{
	std::ofstream out("log.txt", std::ios::app);
	if (out.is_open())
	{
		out << "[" << __DATE__ << ' ' << __TIME__ << ", function \"" << function
			<< "\", line " << line << "]\t\t" << message << std::endl;
	}
}