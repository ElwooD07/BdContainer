#pragma once
#include "impl/TypesInternal.h"

namespace dbc
{
	namespace utils
	{
		void SplitSavingDelim(const std::string& str, char delim, std::vector<std::string>& out);

		template<typename T>
		T StringToNumber(const std::string& str, std::ios::fmtflags flags = std::ios::dec)
		{
			std::stringstream strm;
			strm.flags(flags);
			strm << str;
			T number;
			strm >> number;
			return number;
		}

		template<typename T>
		std::string NumberToString(const T& number, std::ios::fmtflags flags = std::ios::dec)
		{
			std::stringstream strm;
			strm.flags(flags);
			strm << number;
			std::string str;
			strm >> str;
			return str;
		}

		std::string BinaryToHexString(const void* data, size_t data_len);

		RawData StringToRawData(const std::string& str);
		std::string RawDataToString(const RawData& data);
	}
}
