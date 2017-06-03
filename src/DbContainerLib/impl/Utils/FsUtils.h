#pragma once
#include "IProgressObserver.h"

namespace dbc
{
	namespace utils
	{
		std::string SlashedPath(const std::string& in);
		std::string UnslashedPath(const std::string& in);
		bool FileNameIsValid(const std::string& fname);

		bool FileExists(const std::string& fname);

		uint64_t TellMaxAvailable(std::istream& in, uint64_t requiredSize);

		template<class T>
		inline dbc::ProgressState CheckStream(T& strm, dbc::IProgressObserver* observer, dbc::ErrIncident errIncident, const char* errMsg)
		{
			dbc::Error errCode(dbc::ERR_DATA, errIncident);
			if (strm.fail())
			{
				if (observer != nullptr)
				{
					return observer->OnError(errCode);
				}
				else
				{
					throw dbc::ContainerException(errMsg, errCode);
				}
			}
			return dbc::Continue;
		}
	}
}