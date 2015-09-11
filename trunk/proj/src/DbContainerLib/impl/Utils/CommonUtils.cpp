#include "stdafx.h"
#include "CommonUtils.h"

void dbc::utils::SplitSavingDelim(const std::string &str, char delim, std::vector<std::string> &out)
{
	out.clear();
	std::string::const_iterator last = str.begin();
	std::string::const_iterator end = str.end();
	for (std::string::const_iterator i = last; i != end; ++i)
	{
		if (*i == delim)
		{
			out.push_back(std::string(last, i + 1));
			last = i + 1;
		}
	}
	if (last != end)
	{
		out.push_back(std::string(last, end));
	}
}

std::string dbc::utils::BinaryToHexString(const void* data, size_t data_len)
{
	std::string result(data_len * 2, '\0');
	for (size_t i = 0; i < data_len; ++i)
	{
		sprintf_s(&result[0] + (i * 2), result.size(), "%02x", *(static_cast<const unsigned char*>(data) + i));
	}
	return result;
}
