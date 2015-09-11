#include "stdafx.h"
#include "ErrorCodes.h"

using namespace dbc;

namespace
{
	static const char s_undefinedErrorStr[]("Undefined container error");
	static const char s_undefinedIncidentStr[](": undefined error");

	const char* CommonErrorString(unsigned int code)
	{
		switch (code)
		{
		case SUCCESS:
			return "Operation successfull";
		case WRONG_PARAMETERS:
			return "Wrong parameters";
		case CANT_ALLOC_MEMORY:
			return "Heap memory allocation failed";
		case INVALID_PASSWORD:
			return "Invalid password";
		case OWNER_IS_MISSING:
			return "Owner of this object is missing or invalid";
		case ACTION_IS_FORBIDDEN:
			return "This action is forbidden";
		case CONTAINER_RESOURCES_NOT_AVAILABLE:
			return "Resources which were allocated by the Container have been deallocated with Container object and are no longer available";
		case ERR_INTERNAL:
			return "Internal error";
		case ERR_UNDEFINED:
			return s_undefinedErrorStr;
		default:
			return nullptr;
		}
	}

	const char* BaseErrorString(unsigned int base)
	{
		switch (base)
		{
		case ERR_SQL:
			return "Container database connection ";
		case ERR_DB:
			return "Container database ";
		case ERR_FS:
			return "File system object ";
		case ERR_DB_FS:
			return "Container file system object ";
		case ERR_DATA:
			return "Container file data ";
		default:
			return nullptr;
		}
	}

	const char* IncidentString(unsigned int incident)
	{
		switch (incident)
		{
		case INCIDENT_NONE:
			 return "is OK";
		case NOT_FOUND:
			return "not found";
		case ALREADY_EXISTS:
			return "is already exists";
		case CANT_OPEN:
			return "- unable to open";
		case CANT_CREATE:
			return "- unable to create";
		case CANT_READ:
			return "- unable to read";
		case CANT_WRITE:
			return "- unable to write";
		case CANT_REMOVE:
			return "- unable to remove";
		case CANT_EXEC:
			return " - unable to execute";
		case NOT_VALID:
			return "is not valid";
		case IS_EMPTY:
			return "is empty";
		case IS_DAMAGED:
			return "is damaged";
		case NO_ACCESS:
			return "- no access";
		case INCIDENT_INTERNAL:
			return ": internal error";
		case INCIDENT_UNDEFINED:
			return s_undefinedIncidentStr;
		default:
			return nullptr;
		}
	}

	const char* ExtendedErrorString(unsigned int code)
	{
		switch (code)
		{
		case SQL_DISCONNECTED:
			return "is broken";
		case SQL_CANT_STEP:
			return "is unable to get the content";
		case SQL_NO_ACCESS:
			return "can't access to some records";
		case SQL_BUSY:
			return "is busy";
		case SQL_ROW:
			return "message: the content received";
		case SQL_DONE:
			return "message: the content receiving done";
		case ERR_DB_NO_CONNECTION:
			return "has no connection";
		case ERR_DB_FS_NOT_OPENED:
			return "is not opened";
		case ERR_DB_FS_ALREADY_OPENED:
			return "is already opened";
		case ERR_DATA_CANT_OPEN_SRC:
			return "can not be processed: unable to open source stream";
		case ERR_DATA_CANT_OPEN_DEST:
			return "can not be processed: unable to open destination stream";
		case ERR_DATA_CANT_ALLOCATE_SPACE:
			return "storage can't provide enough space for the new data";
		case ERR_DATA_NOT_INITIALIZED:
			return "was not initialized with the container database";
		case SQL_WRONG_QUERY:
		case SQL_STMT_NOT_PREPARED:
		case SQL_CANT_PREPARE:
			return ": Unexpected internal error";
		default:
			return nullptr;
		}
	}
}

bool dbc::Error::operator==(const Error& err) const
{
	return this->code == err.code;
}

bool dbc::Error::operator!=(const Error& err) const
{
	return !operator==(err);
}

bool dbc::Error::operator==(uint16_t code) const
{
	return this->code == code;
}

bool dbc::Error::operator!=(uint16_t code) const
{
	return !operator==(code);
}

std::string dbc::ErrorString(Error err)
{
	const char* errCmn = CommonErrorString(err.code);
	if (errCmn != nullptr)
	{
		return errCmn;
	}

	unsigned int base = err.base & 0xff00;
	if (base < ERR_BASE_COUNT)
	{
		const char* errBase = BaseErrorString(base);
		if (errBase == nullptr)
		{
			return s_undefinedErrorStr;
		}

		unsigned int incident = err.incident & 0x00ff;
		const char* errIncident = (incident < INCIDENT_COUNT) ? IncidentString(incident) : ExtendedErrorString(err.code);
		if (errIncident == nullptr)
		{
			errIncident = s_undefinedIncidentStr;
		}
		return std::string(errBase) + errIncident;
	}
	else
	{
		return s_undefinedErrorStr;
	}
}

std::string dbc::ErrorString(ErrBase base, ErrIncident incident)
{
	return ErrorString(Error(base, incident));
}
