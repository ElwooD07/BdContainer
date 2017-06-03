#pragma once
#include <ctime>

namespace dbc
{
	class ElementProperties
	{
	public:
        explicit ElementProperties(time_t date_created = 0, time_t date_modified = 0, const std::string& meta = "");

		time_t DateCreated() const;
		time_t DateModified() const;
        const std::string& Meta() const;

		void SetDateModified(time_t new_date);
        void SetMeta(const std::string& meta);

		bool operator==(const ElementProperties& obj) const;
		bool operator!=(const ElementProperties& obj) const;
		// The tag with length larger than this will be truncated to it
        static const uint32_t s_MaxTagLength = 1024000; //1000K

        void SetCurrentTime();

	private:
		uint64_t m_size;
        time_t m_created;
        time_t m_modified;
        std::string m_meta;
	};

}
