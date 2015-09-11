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
		virtual uint64_t TotalDataSize();

	private:
		ContainerResources m_resources;
	};
}
