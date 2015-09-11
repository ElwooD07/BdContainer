#include "stdafx.h"
#include "DataUsagePreferences.h"

namespace
{
	unsigned short NormalizeClusterSizeLevel(unsigned short level)
	{
		if (level > dbc::DataUsagePreferences::CLUSTER_SIZE_MAX)
		{
			return dbc::DataUsagePreferences::CLUSTER_SIZE_MAX;
		}
		return level;
	}
}

dbc::DataUsagePreferences::DataUsagePreferences(
	unsigned short clusterSizeLevel,
	DataFragmentationLevel fragmentationLevel,
	bool transactionalWrite)
	: m_clusterSizeLevel(NormalizeClusterSizeLevel(clusterSizeLevel))
	, m_clusterSize(GetRealClusterSize(m_clusterSizeLevel))
	, m_fragmentationLevel(fragmentationLevel)
	, m_transactionalWrite(transactionalWrite)
{ }

unsigned short dbc::DataUsagePreferences::ClusterSizeLevel() const
{
	return m_clusterSizeLevel;
}

unsigned int dbc::DataUsagePreferences::ClusterSize() const
{
	return m_clusterSize;
}

dbc::DataFragmentationLevel dbc::DataUsagePreferences::FragmentationLevel() const
{
	return m_fragmentationLevel;
}

bool dbc::DataUsagePreferences::TransactionalWrite() const
{
	return m_transactionalWrite;
}

unsigned int dbc::DataUsagePreferences::GetRealClusterSize(unsigned short level)
{
	level = NormalizeClusterSizeLevel(level);
	return static_cast<unsigned int>(std::pow(static_cast<float>(2), static_cast<int>(9 + level))); // min is 512, max - 64k
}