#pragma once
#include "IContainerInfo.h"
#include "IContainnerResources.h"

namespace dbc
{
	class Container;
	enum ElementType;

	class ContainerInfoImpl: public IContainerInfo
	{
	public:
		explicit ContainerInfoImpl(ContainerResources resources);

		virtual bool IsEmpty();
		virtual uint64_t TotalElements();
		virtual uint64_t TotalElements(ElementType type);
		virtual uint64_t UsedSpace();
		virtual uint64_t FreeSpace();
		virtual uint64_t TotalStreams();
		virtual uint64_t UsedStreams();

	private:
		ContainerResources m_resources;
	};
}
