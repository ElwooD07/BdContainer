#include "stdafx.h"
#include "FsUtils.h"
#include "ContainerAPI.h"

std::string dbc::utils::SlashedPath(const std::string& in)
{
	std::string out(in);
	if (!in.empty() && in[in.length() - 1] != PATH_SEPARATOR)
		out.push_back(PATH_SEPARATOR);
	return out;

}

std::string dbc::utils::UnslashedPath(const std::string& in)
{
	if (in.length() <= 1)
	{
		return in;
	}
	std::string out(in);
	while (out.size() > 1 && out.back() == PATH_SEPARATOR)
	{
		out.pop_back();
	}
	return out;
}

bool dbc::utils::FNameIsValid(const std::string &fname)
{
	return (!fname.empty() && fname.find_first_of("\\/*?\n\r") == std::string::npos);
}

bool dbc::utils::FileExists(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

uint64_t dbc::utils::TellMaxAvailable(std::istream &in, uint64_t required_size)
{
	std::ios::pos_type origin;
	origin = in.tellg();

	in.seekg(0, std::ios::end);
	std::streamoff ret = in.tellg() - origin;
	if (ret >= static_cast<int64_t>(required_size))
		ret = required_size;

	in.seekg(origin, std::ios::beg);

	return ret;
}