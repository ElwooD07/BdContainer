#include "stdafx.h"
#include "ProxyProgressObserver.h"

namespace
{
	inline float AdjustRange(float& range)
	{
		if (range < 0.0)
		{
			return 0.0;
		}
		else if (range > 1.0)
		{
			return 1.0;
		}
		else
		{
			return range;
		}
	}
}

dbc::ProxyProgressObserver::ProxyProgressObserver(IProgressObserver* higherObserver)
	: m_higherObserver(higherObserver)
	, m_rangeLower(0.0)
	, m_rangeUpper(1.0)
{ }

void dbc::ProxyProgressObserver::SetRange(float lower, float upper)
{
	m_rangeLower = AdjustRange(lower);
	m_rangeUpper = AdjustRange(upper);
	if (m_rangeLower > m_rangeUpper)
	{
		m_rangeLower = m_rangeUpper; // Progress will stuck in lower range
	}
}

dbc::ProgressState dbc::ProxyProgressObserver::OnProgressUpdated(float progress)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnProgressUpdated(m_rangeLower + (m_rangeUpper - m_rangeLower) * progress);
	}
	return Continue;
}

dbc::ProgressState dbc::ProxyProgressObserver::OnInfo(const std::string& info)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnInfo(info);
	}
	return dbc::Continue;
}

dbc::ProgressState dbc::ProxyProgressObserver::OnWarning(Error errCode)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnWarning(errCode);
	}
	return Stop;
}

dbc::ProgressState dbc::ProxyProgressObserver::OnError(Error errCode)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnError(errCode);
	}
	return Stop;
}
