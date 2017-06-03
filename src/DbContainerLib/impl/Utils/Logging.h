#pragma once

namespace dbc
{
	namespace cmn
	{
		void WriteLogEx(const std::string& message, const std::string& file, long int line);
		#define WriteLog(string_message) dbc::cmn::WriteLogEx(string_message, __FUNCTION__, __LINE__);
	}
}