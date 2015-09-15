#pragma once
#include "IProgressObserver.h"

namespace dbc
{
	class StreamProxyProgressObserver : public IProgressObserver
	{
	public:
		StreamProxyProgressObserver(IProgressObserver* higherObserver);
		void SetRange(float lower, float upper);

		virtual ProgressState OnProgressUpdated(float progress);
		virtual ProgressState OnWarning(Error errCode);
		virtual ProgressState OnError(Error errCode);

	private:
		IProgressObserver* m_higherObserver;
		float m_rangeLower;
		float m_rangeUpper;
	};
}