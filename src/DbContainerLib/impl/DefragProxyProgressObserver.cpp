#include "stdafx.h"
#include "DefragProxyProgressObserver.h"

dbc::DefragProxyProgressObserver::DefragProxyProgressObserver(IDefragProgressObserver* observer)
	: ProxyProgressObserver(observer)
	, m_higherObserver(observer)
{ }

dbc::ProgressState dbc::DefragProxyProgressObserver::OnProgressUpdated(float progress)
{
	return ProxyProgressObserver::OnProgressUpdated(progress);
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnInfo(const std::string& info)
{
	return ProxyProgressObserver::OnInfo(info);
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnWarning(Error errCode)
{
	return ProxyProgressObserver::OnWarning(errCode);
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnError(Error errCode)
{
	return ProxyProgressObserver::OnError(errCode);
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnCurrentFileChanged(const std::string& path)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnCurrentFileChanged(path);
	}
	return dbc::Continue;
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnCurrentFileDefragmented(float progress)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnCurrentFileDefragmented(progress);
	}
	return dbc::Continue;
}

dbc::ProgressState dbc::DefragProxyProgressObserver::OnLockedFileSkipped(const std::string& path)
{
	if (m_higherObserver != nullptr)
	{
		return m_higherObserver->OnLockedFileSkipped(path);
	}
	return dbc::Continue;
}