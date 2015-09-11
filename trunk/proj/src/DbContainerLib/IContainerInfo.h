#pragma once
#include "IContainerElement.h"
#include <memory>

namespace dbc
{
	class IContainerInfo
	{
	public:
		virtual ~IContainerInfo() { }

		virtual bool IsEmpty() = 0;
		virtual uint64_t TotalElements() = 0;
		virtual uint64_t TotalElements(ElementType type) = 0;
		virtual uint64_t TotalDataSize() = 0;
	};

	typedef std::shared_ptr<IContainerInfo> ContainerInfo;
}