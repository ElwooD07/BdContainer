#include "stdafx.h"
#include "CommonUtils.h"

void dbc::utils::SplitSavingDelim(const std::string& str, char delim, std::vector<std::string>& out)
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

std::string dbc::utils::BinaryToHexString(const void* data, size_t dataLen)
{
	std::string result(dataLen * 2, '\0');
	for (size_t i = 0; i < dataLen; ++i)
	{
		sprintf_s(&result[0] + (i * 2), result.size(), "%02x", *(static_cast<const unsigned char*>(data) + i));
	}
	return result;
}

dbc::RawData dbc::utils::StringToRawData(const std::string& str)
{
	const dbc::RawData::value_type* strPtr = reinterpret_cast<const dbc::RawData::value_type*>(str.c_str());
	return std::move(dbc::RawData(strPtr, strPtr + str.size()));
}

std::string dbc::utils::RawDataToString(const RawData& data)
{
	std::string res;
	if (!data.empty())
	{
		res.assign(reinterpret_cast<const char*>(data.data()), data.size());
	}
	return std::move(res);
}
