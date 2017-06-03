#include "stdafx.h"
#include "ElementProperties.h"
#include "CommonUtils.h"
#include <sstream>

dbc::ElementProperties::ElementProperties(time_t date_created, time_t date_modified, const std::string& meta)
    : m_created(date_created)
    , m_modified(date_modified)
    , m_meta(meta)
{ }

time_t dbc::ElementProperties::DateCreated() const
{
    return m_created;
}

time_t dbc::ElementProperties::DateModified() const
{
    return m_modified;
}

const std::string& dbc::ElementProperties::Meta() const
{
    return m_meta;
}

void dbc::ElementProperties::SetDateModified(time_t new_date)
{
    m_modified = new_date;
}

void dbc::ElementProperties::SetMeta(const std::string& meta)
{
    std::string::const_iterator end = meta.length() > s_MaxTagLength ? meta.begin() + s_MaxTagLength : meta.end();
    m_meta.assign(meta.cbegin(), end);
}

bool dbc::ElementProperties::operator==(const ElementProperties& obj) const
{
	bool ret = (m_size == obj.m_size);
    ret = ret && (m_created == obj.m_created);
    ret = ret && (m_modified == obj.m_modified);
    ret = ret && (m_meta == obj.m_meta);
	return ret;
}

bool dbc::ElementProperties::operator!=(const ElementProperties& obj) const
{
	return !operator==(obj);
}

void dbc::ElementProperties::SetCurrentTime()
{
    time_t currentTime = ::time(nullptr);
    m_created = currentTime;
    m_modified = currentTime;
}
