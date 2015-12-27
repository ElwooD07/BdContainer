#include "stdafx.h"
#include "CommonUtils.h"

namespace
{
	void SplitImpl(const std::string& str, char delim, dbc::Strings_vt& out, bool saveDelim)
	{
		assert(out.empty());
		out.clear();
		std::string::const_iterator last = str.begin();
		std::string::const_iterator end = str.end();
		for (std::string::const_iterator i = last; i != end; ++i)
		{
			if (*i == delim)
			{
				std::string::const_iterator to = saveDelim ? i + 1 : i;
				std::string token(last, to);
				last = i + 1;
				if (!saveDelim && token.empty())
				{
					continue;
				}
				out.push_back(token);
			}
		}
		if (last != end)
		{
			out.push_back(std::string(last, end));
		}
	}
}

void dbc::utils::SplitSavingDelim(const std::string& str, char delim, Strings_vt& out)
{
	SplitImpl(str, delim, out, true);
}

void dbc::utils::SplitWithoutDelim(const std::string& str, char delim, Strings_vt& out)
{
	SplitImpl(str, delim, out, false);
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
	dbc::RawData out(strPtr, strPtr + str.size());
	if (out.back() != 0)
	{
		out.push_back(0);
	}
	return std::move(out);
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
