#pragma once
#include "IProgressObserver.h"

namespace dbc
{
	class ProxyProgressObserver : public IProgressObserver
	{
	public:
		ProxyProgressObserver(IProgressObserver* higherObserver);
		void SetRange(float lower, float upper);

		virtual ProgressState OnProgressUpdated(float progress);
		virtual ProgressState OnInfo(const std::string& info);
		virtual ProgressState OnWarning(Error errCode);
		virtual ProgressState OnError(Error errCode);

	private:
		IProgressObserver* m_higherObserver;
		float m_rangeLower;
		float m_rangeUpper;
	};
}