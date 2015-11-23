#pragma once
#include "ui_FsTreeWidget.h"
#include "ContainerAPI.h"

namespace gui
{
	class FsTreeWidget: public QWidget
	{
		Q_OBJECT;

	public:
		explicit FsTreeWidget(QWidget* parent);
		void AddContainer(dbc::ContainerGuard container);

	private:
		Ui::FsTreeWidget m_ui;
	};

}
