#pragma once
#include "ContainerAPI.h"

class ShittyProgressObserver : public dbc::IProgressObserver
{
public:
	virtual dbc::ProgressState OnProgressUpdated(float progress);
	virtual dbc::ProgressState OnWarning(dbc::Error errCode);
	virtual dbc::ProgressState OnError(dbc::Error errCode);
};