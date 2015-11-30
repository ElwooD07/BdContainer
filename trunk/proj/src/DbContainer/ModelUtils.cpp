#include "stdafx.h"
#include "ModelUtils.h"
#include "Types.h"

QString model::utils::StdString2QString(const std::string& stdString)
{
	return std::move(QString::fromUtf8(stdString.c_str(), stdString.size()));
}

std::string model::utils::QString2StdString(const QString& str)
{
	return std::move(std::string(str.toUtf8().constData()));
}

QString model::utils::SlashedPath(const QString& path)
{
	QString res(path);
	if (path.length() > 0 && path[path.length() - 1] != dbc::PATH_SEPARATOR)
	{
		res += dbc::PATH_SEPARATOR;
	}
	return std::move(res);
}

QString model::utils::SlashedPath(const std::string& path)
{
	return SlashedPath(StdString2QString(path));
}

bool model::utils::IsItemEditable(const QModelIndex& index)
{
	return (index.isValid() && (index.flags() & Qt::ItemIsEditable) == Qt::ItemIsEditable);
}
