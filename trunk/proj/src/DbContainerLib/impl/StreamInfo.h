#pragma once

namespace dbc
{
	struct StreamInfo
	{
		StreamInfo(int64_t id = 0, int64_t fileId = 0, uint64_t order = 0, uint64_t start = 0, uint64_t size = 0, uint64_t used = 0)
			: id(id), fileId(fileId), order(order), start(start), size(size), used(used)
		{ }

		int64_t id;
		int64_t fileId;
		uint64_t order;
		uint64_t start;
		uint64_t size;
		uint64_t used;

		bool IsEmpty() const
		{
			return (id == 0 && fileId == 0 && order == 0 && start == 0 && size == 0 && used == 0);
		}
	};

	typedef std::vector<StreamInfo> StreamsChain_vt;
	typedef std::set<uint64_t> StreamsIds_st;
}