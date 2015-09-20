#include "stdafx.h"
#include "ShittyProgressObserver.h"

dbc::ProgressState ShittyProgressObserver::OnProgressUpdated(float progress)
{
	throw std::runtime_error("Unexpected error");
}

dbc::ProgressState ShittyProgressObserver::OnWarning(dbc::Error errCode)
{
	return dbc::Stop;
}

dbc::ProgressState ShittyProgressObserver::OnError(dbc::Error errCode)
{
	return dbc::Stop;
}