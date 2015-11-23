#pragma once

namespace model
{
	namespace utils
	{
		QString StdString2QString(const std::string& stdString);
		QString SlashedPath(const QString& path);
		QString SlashedPath(const std::string& path);
	}
}