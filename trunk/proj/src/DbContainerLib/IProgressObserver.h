#pragma once
#include <string>
#include "ErrorCodes.h"

namespace dbc
{
	enum ProgressState
	{
		None = 0,
		Continue,
		Pause,
		Stop,
		Cancel
	};

	class IProgressObserver
	{
	public:
		virtual ~IProgressObserver() { }
		virtual ProgressState OnProgressUpdated(float progress) = 0; // from 0 to 1
		virtual ProgressState OnWarning(Error errCode) = 0;
		virtual ProgressState OnError(Error errCode) = 0;
	};
}