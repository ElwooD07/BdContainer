#include "stdafx.h"
#include "ContainerException.h"

const dbc::Error dbc::ContainerException::DEFAULT_ERROR = ERR_UNDEFINED;

dbc::ContainerException::ContainerException(const std::string &what, Error err_type,
const std::string &reason, Error reason_type)
: std::exception((!what.empty()) ? what.c_str() : ErrorString(err_type).c_str())
, m_err_type(err_type)
, m_reason((!reason.empty()) ? reason : ErrorString(reason_type))
, m_reason_type(reason_type)
{	}

dbc::ContainerException::ContainerException(const std::string &what, const std::string &reason)
: std::exception(what.c_str())
, m_err_type(DEFAULT_ERROR)
, m_reason(reason)
, m_reason_type(DEFAULT_ERROR)
{	}

dbc::ContainerException::ContainerException(Error err_type, Error reason_type)
: std::exception(ErrorString(err_type).c_str())
, m_err_type(err_type)
, m_reason(ErrorString(reason_type))
, m_reason_type(reason_type)
{	}

dbc::ContainerException::ContainerException(ErrBase base, ErrIncident incident, ErrBase reason_base, ErrIncident reason_incident)
: std::exception(ErrorString(Error(base, incident)).c_str())
, m_err_type(base, incident)
, m_reason_type(reason_base, reason_incident)
, m_reason(ErrorString(m_reason_type))
{	}

dbc::ContainerException::ContainerException(ErrBase base, ErrIncident incident, Error reason_type)
: std::exception(ErrorString(Error(base, incident)).c_str())
, m_err_type(base, incident)
, m_reason_type(reason_type)
, m_reason(ErrorString(reason_type))
{	}

std::string dbc::ContainerException::Message() const
{
	return std::string(this->what());
}

std::string dbc::ContainerException::Reason() const
{
	return m_reason;
}

std::string dbc::ContainerException::FullMessage() const
{
	std::stringstream stream;
	stream << "Exception: " << what();
	if (m_reason_type != SUCCESS && m_reason_type != ERR_UNDEFINED)
		stream << "; reason: " << m_reason;
	return stream.str();
}

dbc::Error dbc::ContainerException::ErrType() const
{
	return m_err_type;
}

dbc::Error dbc::ContainerException::ReasonType() const
{
	return m_reason_type;
}

bool dbc::ContainerException::Safe(Error err_type)
{
	return (err_type == SUCCESS || err_type == SQL_ROW || err_type == SQL_DONE);
}

bool dbc::ContainerException::Safe() const
{
	if (Safe(m_err_type))
	{
		if (!Safe(m_reason_type))
		{
			return m_reason_type == DEFAULT_ERROR;
		}
		return true;
	}
	return false;
}
