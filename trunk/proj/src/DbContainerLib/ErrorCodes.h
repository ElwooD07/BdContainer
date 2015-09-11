#pragma once

namespace dbc
{
	enum ErrCommon
	{
		SUCCESS = 0x0000,
		WRONG_PARAMETERS = 0x8000,
		CANT_ALLOC_MEMORY = 0x8100,
		INVALID_PASSWORD = 0x8200,
		OWNER_IS_MISSING = 0x8300,
		ACTION_IS_FORBIDDEN = 0x8400,
		CONTAINER_RESOURCES_NOT_AVAILABLE = 0x8500,
		ERR_INTERNAL = 0xfefe,
		ERR_UNDEFINED = 0xffff
	};

	enum ErrBase // DataBase Container Error
	{
		ERR_SQL = 0x0100, // SQLite requests
		ERR_DB = 0x0200, // Database entirely
		ERR_FS = 0x0300, // File system ("real")
		ERR_DB_FS = 0x0400, // File system in the database ("virtual")
		ERR_DATA = 0x0500, // Data in binary file

		ERR_BASE_COUNT
	};

	enum ErrExtended
	{
		SQL_WRONG_QUERY = 0x0180,
		SQL_DISCONNECTED = 0x0181,
		SQL_STMT_NOT_PREPARED = 0x0182,
		SQL_CANT_PREPARE = 0x0183,
		SQL_CANT_STEP = 0x0184,
		SQL_NO_ACCESS = 0x0185,
		SQL_BUSY = 0x0186,
		SQL_ROW = 0x018a,
		SQL_DONE = 0x018b,

		ERR_DB_NO_CONNECTION = 0x0280,

		ERR_DB_FS_NOT_OPENED = 0x0380,
		ERR_DB_FS_ALREADY_OPENED = 0x0381,

		ERR_DATA_CANT_OPEN_SRC = 0x0580,
		ERR_DATA_CANT_OPEN_DEST = 0x0581,
		ERR_DATA_CANT_ALLOCATE_SPACE = 0x0582,
		ERR_DATA_NOT_INITIALIZED = 0x058a,
	};

	enum ErrIncident
	{
		INCIDENT_NONE = 0x00,
		NOT_FOUND = 0x01,
		ALREADY_EXISTS = 0x02,
		CANT_OPEN = 0x03,
		CANT_CREATE = 0x04,
		CANT_READ = 0x05,
		CANT_WRITE = 0x06,
		CANT_REMOVE = 0x07,
		CANT_EXEC = 0x08,
		NOT_VALID = 0x09,
		IS_EMPTY = 0x0a,
		IS_DAMAGED = 0x0b,
		IS_LOCKED = 0x0c,
		NO_ACCESS = 0x0d,

		INCIDENT_INTERNAL,
		INCIDENT_UNDEFINED,
		INCIDENT_COUNT
	};

	union Error
	{
		Error()
		: code(ERR_UNDEFINED)
		{ }
		Error(unsigned int code)
			: code(code)
		{ }
		Error(ErrBase base)
			: code(base | INCIDENT_UNDEFINED)
		{ }
		Error(ErrBase base, ErrIncident incident)
			: code(base | incident)
		{ }

		ErrBase base;
		ErrIncident incident;
		unsigned int code;

		bool operator==(const Error& err) const;
		bool operator!=(const Error& err) const;
		bool operator==(uint16_t code) const;
		bool operator!=(uint16_t code) const;
	};

	std::string ErrorString(Error err);
	std::string ErrorString(ErrBase base, ErrIncident incident);
}