#pragma once
#include "ProxyProgressObserver.h"
#include "IDefragProgressObserver.h"

namespace dbc
{
	class DefragProxyProgressObserver : public IDefragProgressObserver, public ProxyProgressObserver
	{
	public:
		DefragProxyProgressObserver(IDefragProgressObserver* observer);

		virtual ProgressState OnProgressUpdated(float progress);
		virtual ProgressState OnInfo(const std::string& info);
		virtual ProgressState OnWarning(Error errCode);
		virtual ProgressState OnError(Error errCode);

		virtual ProgressState OnCurrentFileChanged(const std::string& path);
		virtual ProgressState OnCurrentFileDefragmented(float progress);
		virtual ProgressState OnLockedFileSkipped(const std::string& path);

	private:
		IDefragProgressObserver* m_higherObserver;
	};
}