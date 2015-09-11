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