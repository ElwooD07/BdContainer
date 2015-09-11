#pragma once

namespace dbc
{
	namespace utils
	{
		void SplitSavingDelim(const std::string &str, char delim, std::vector<std::string> &out);

		template<typename T>
		void String2Number(const std::string& str, T& number, std::ios::fmtflags flags = std::ios::dec)
		{
			std::stringstream strm;
			strm.flags(flags);
			strm << str;
			strm >> number;
		}

		std::string BinaryToHexString(const void* data, size_t data_len);
	}
}
