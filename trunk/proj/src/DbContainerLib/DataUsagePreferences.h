#pragma once

namespace dbc
{
	enum DataFragmentationLevel // NOTE: This level is inversely proportional to the useful place in Data container
	{
		DataFragmentationLevelMin = 0,
		DataFragmentationLevelNormal,
		DataFragmentationLevelLarge
	};

	class DataUsagePreferences
	{
	public:
		static const unsigned short CLUSTER_SIZE_MIN = 0;
		static const unsigned short CLUSTER_SIZE_DEF = 3;
		static const unsigned short CLUSTER_SIZE_MAX = 7;

	public:
		DataUsagePreferences(
			unsigned short clusterSizeLevel = CLUSTER_SIZE_DEF,
			DataFragmentationLevel fragmentationLevel = DataFragmentationLevelNormal,
			bool transactionalWrite = true);

		unsigned short ClusterSizeLevel() const;
		unsigned int ClusterSize() const; // in bytes
		DataFragmentationLevel FragmentationLevel() const;
		bool TransactionalWrite() const;

		static unsigned int GetRealClusterSize(unsigned short level);

	private:
		unsigned short m_clusterSizeLevel;
		unsigned int m_clusterSize;
		DataFragmentationLevel m_fragmentationLevel;
		bool m_transactionalWrite;
	};
}