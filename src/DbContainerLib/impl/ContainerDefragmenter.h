#pragma once
#include "IDefragProgressObserver.h"
#include "IContainnerResources.h"
#include "DataUsagePreferences.h"

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
		typedef std::map<uint64_t, float> FilesFragmentation_mp;
		void CreateFragmentationLevelsMap(const FilesIds_st& filesIds, FilesFragmentation_mp& fragmentation, IProgressObserver* observer);
		float CalculateSingleFileFragmentation(uint64_t fileId);
		float CalculateAverageFragmentation(const FilesFragmentation_mp& fragmentation);

		void DefragImpl(FilesFragmentation_mp& fragmentationInfo, DataFragmentationLevel targetQuality, IDefragProgressObserver* observer);

	private:
		ContainerResources m_resources;
		IDataStorage* m_storage;
	};
}