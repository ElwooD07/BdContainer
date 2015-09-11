#pragma once

namespace dbc
{
	namespace utils
	{
		std::string SlashedPath(const std::string& in);
		std::string UnslashedPath(const std::string& in);
		bool FNameIsValid(const std::string &fname);

		bool FileExists(const std::string &fname);

		uint64_t TellMaxAvailable(std::istream &in, uint64_t required_size);
	}
}