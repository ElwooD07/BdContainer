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
		dbc::ContainerGuard GetSelectedContainer();

		QMenu* GetTreeMenu();
		QMenu* GetElementMenu();

	signals:
		void CurrentElementChanged(dbc::ContainerElementGuard element);

	private slots:
		void OnCollapseAllTriggered();
		void OnExpandAllTriggered();
		void OnItemSelected(const QModelIndex& index);

	private:
		void InitMenus();

	private:
		MainWindowView* m_mainWindow;
		Ui::FsTreeWidget m_ui;
		model::DbContainerModel* m_model;

		QMenu* m_menuTree;
		QMenu* m_menuElement;
	};
}
