#pragma once
#include "IDefragProgressObserver.h"
#include "IContainnerResources.h"
#include "StreamInfo.h"

namespace dbc
{
	class ContainerDefragmenter
	{
	public:
		ContainerDefragmenter(ContainerResources resources, IDataStorage* storage);

		typedef std::set<uint64_t> FilesIds_st;
		// returns value from 0 to 1, where 1 means not fragmented container is and 1 means totally fragmented
		float CalculateFragmentationLevel(IProgressObserver* observer = nullptr);
		float CalculateFragmentationLevel(const FilesIds_st& files, IProgressObserver* observer = nullptr);
		static DataFragmentationLevel InterpretFragmentationLevelValue(float fragmentation);

		void Defragment(DataFragmentationLevel targetQuality = DataFragmentationLevelMin, IDefragProgressObserver* observer = nullptr);
		void Defragment(const FilesIds_st& files, DataFragmentationLevel targetQuality = DataFragmentationLevelMin, IDefragProgressObserver* observer = nullptr);

	private:
		void CollectAllFileIds(FilesIds_st& files);
		float CalculateSingleFileFragmentation(uint64_t fileId);

	private:
		ContainerResources m_resources;
		IDataStorage* m_storage;

		typedef std::map<uint64_t, float> m_fragmentation;
	};
}