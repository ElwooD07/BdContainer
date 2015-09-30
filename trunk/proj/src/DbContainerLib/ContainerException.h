#pragma once
#include "ErrorCodes.h"

namespace dbc
{
	class ContainerException: public std::exception
	{
	public:
		ContainerException(const std::string &what, Error err_type,
			const std::string &reason = "", Error reason_type = DEFAULT_ERROR);
		ContainerException(const std::string &what, const std::string &reason = "");
		ContainerException(Error err_type = DEFAULT_ERROR, Error reason_type = DEFAULT_ERROR);
		ContainerException(ErrBase base, ErrIncident incident, ErrBase reason_base, ErrIncident reason_incident);
		ContainerException(ErrBase base, ErrIncident incident, Error reason_type = DEFAULT_ERROR);

		std::string Message() const; // Main information about exception
		std::string Reason() const; // Additional information
		std::string FullMessage() const;
		Error ErrType() const;
		Error ReasonType() const;

		// Determines whether the error code result of an error, or is it just the flag
		static bool Safe(Error err_type);
		bool Safe() const;

		// DBCErr::ERROR is set as default error type for all exceptions. It means, that ERROR is undefined error type, like NULL, but not SUCCESS
		static const Error DEFAULT_ERROR;

	private:
		std::string m_what;
		Error m_err_type;
		std::string m_reason;
		Error m_reason_type;
	};
}
