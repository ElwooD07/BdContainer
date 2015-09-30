#pragma once
#include <ctime>

namespace dbc
{
	class ElementProperties
	{
	public:
		ElementProperties(uint64_t size = 0, time_t date_created = 0, time_t date_modified = 0, const std::string& tag = "");
		explicit ElementProperties(const std::string& props_str);

		uint64_t Size() const;
		time_t DateCreated() const;
		time_t DateModified() const;
		std::string Tag() const;

		void SetSize(uint64_t new_size);
		void SetDateCreated(time_t new_date);
		void SetDateModified(time_t new_date);
		void SetTag(const std::string& tag);

		bool operator==(const ElementProperties& obj) const;
		bool operator!=(const ElementProperties& obj) const;
		// The tag with length larger than this will be truncated to it
		static const short int TAG_MAX_LEN = 200;

		static bool ParseString(const std::string& props_str, ElementProperties& out_props);
		static void MakeString(const ElementProperties& obj, std::string& out_wstr);
		static void SetCurrentTime(ElementProperties& out_props);

	private:
		uint64_t m_size;
		time_t m_date_created;
		time_t m_date_modified;
		std::string m_tag;
	};

}
