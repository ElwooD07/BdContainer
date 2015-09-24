#pragma once
#include "IProgressObserver.h"
#include <string>

namespace dbc
{
	class IDefragProgressObserver: public IProgressObserver
	{
	public:
		virtual ~IDefragProgressObserver() { }
		virtual ProgressState OnCurrentFileChanged(const std::string& path) = 0;
		virtual ProgressState OnCurrentFileDefragmented(float progress) = 0;
		virtual ProgressState OnLockedFileSkipped(const std::string& path) = 0;
	};
}