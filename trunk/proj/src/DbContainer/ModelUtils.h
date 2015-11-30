#pragma once

namespace model
{
	namespace utils
	{
		QString StdString2QString(const std::string& stdString);
		std::string QString2StdString(const QString& str);
		QString SlashedPath(const QString& path);
		QString SlashedPath(const std::string& path);
		bool IsItemEditable(const QModelIndex& index);
	}
}