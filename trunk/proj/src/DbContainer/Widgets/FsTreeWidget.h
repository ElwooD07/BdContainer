#pragma once
#include "ui_FsTreeWidget.h"
#include "ContainerAPI.h"

namespace model
{
	class DbContainerModel;
}

namespace gui
{
	class MainWindowView;
	class FsTreeWidget: public QWidget
	{
		Q_OBJECT;

	public:
		explicit FsTreeWidget(QWidget* parent, MainWindowView* mainWindow);
		void AddContainer(dbc::ContainerGuard container);

		void RenameCurrentItem();

	private:
		MainWindowView* m_mainWindow;
		Ui::FsTreeWidget m_ui;
		model::DbContainerModel* m_model;
	};

}
