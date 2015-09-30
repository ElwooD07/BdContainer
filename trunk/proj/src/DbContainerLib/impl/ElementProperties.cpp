#include "stdafx.h"
#include "ElementProperties.h"
#include "CommonUtils.h"
#include <sstream>

dbc::ElementProperties::ElementProperties(uint64_t size, time_t date_created, time_t date_modified, const std::string& tag)
{
	m_size = size;
	m_date_created = date_created;
	m_date_modified = date_modified;
	m_tag = tag;
}

dbc::ElementProperties::ElementProperties(const std::string& propsStr)
	: m_size(0)
	, m_date_created(0)
	, m_date_modified(0)
{
	ParseString(propsStr, *this);
}

uint64_t dbc::ElementProperties::Size() const
{
	return m_size;
}

time_t dbc::ElementProperties::DateCreated() const
{
	return m_date_created;
}

time_t dbc::ElementProperties::DateModified() const
{
	return m_date_modified;
}

std::string dbc::ElementProperties::Tag() const
{
	return m_tag;
}

void dbc::ElementProperties::SetSize(uint64_t new_size)
{
	m_size = new_size;
}

void dbc::ElementProperties::SetDateCreated(time_t new_date)
{
	m_date_created = new_date;
	m_date_modified = new_date;
}

void dbc::ElementProperties::SetDateModified(time_t new_date)
{
	m_date_modified = new_date;
}

void dbc::ElementProperties::SetTag(const std::string& tag)
{
	std::string::const_iterator end = tag.length() > TAG_MAX_LEN ? tag.begin() + TAG_MAX_LEN : tag.end();
	m_tag.assign(tag.cbegin(), end);
}

bool dbc::ElementProperties::operator==(const ElementProperties& obj) const
{
	bool ret = (m_size == obj.m_size);
	ret = ret && (m_date_created == obj.m_date_created);
	ret = ret && (m_date_modified == obj.m_date_modified);
	ret = ret && (m_tag == obj.m_tag);
	return ret;
}

bool dbc::ElementProperties::operator!=(const ElementProperties& obj) const
{
	return !operator==(obj);
}

bool dbc::ElementProperties::ParseString(const std::string& props_str, ElementProperties& out_props)
{
	std::vector<std::string> lst;
	dbc::utils::SplitSavingDelim(props_str, '|', lst);
	size_t size = lst.size();
	if (size < 3)
	{
		return false;
	}

	utils::String2Number(lst[0], out_props.m_size);
	utils::String2Number(lst[1], out_props.m_date_created);
	utils::String2Number(lst[2], out_props.m_date_modified);

	std::string tag;
	size_t len = 0;
	for (std::vector<std::string>::const_iterator i = lst.begin() + 3; i != lst.cend(); ++i, len < TAG_MAX_LEN)
	{
		tag.append((*i).begin(), (*i).end());
		len += (*i).length();
	}
	out_props.SetTag(tag);
	return true;
}

void dbc::ElementProperties::MakeString(const ElementProperties& obj, std::string& out_str)
{
	out_str.clear();
	std::stringstream ss;
	ss << obj.m_size << '|' << obj.m_date_created << '|' << obj.m_date_modified << '|';
	ss >> out_str;
	if (!obj.m_tag.empty())
	{
		out_str.append(obj.m_tag.cbegin(), obj.m_tag.cend());
	}
}

void dbc::ElementProperties::SetCurrentTime(ElementProperties& out_props)
{
	time_t cur;
	time(&cur);
	out_props.m_date_created = cur;
	out_props.m_date_modified = cur;
}