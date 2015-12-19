#pragma once
#include "Types.h"

namespace model
{
	class DbContainerModel;
}

namespace gui
{
	class MainWindowView;

	class FsTreeMenuTree : public QMenu
	{
		Q_OBJECT;

	public:
		FsTreeMenuTree(QWidget* parent, QTreeView* treeView, MainWindowView* mainWindow, model::DbContainerModel* model);

	signals:
		void SetEnabledActionsForSelectedFolder(bool enabled);

	private slots:
		void OnAboutToShow();

		void OnCreateFileTriggered();
		void OnCreateFolderTriggered();
		void OnCreateLinkTriggered();

	private:
		void CreateChild(const QString& defaultName, const QString& titleStr, dbc::ElementType type);
		bool CreateChildImpl(const QString& defaultName, dbc::ElementType type);
		QString GetUnusedChildName(const QString& defaultName, const QModelIndex& parent);
		bool ChildNameIsUsed(const QString& name, const QModelIndex& parent);

	private:
		QTreeView* m_treeView;
		MainWindowView* m_mainWindow;
		model::DbContainerModel* m_model;
	};
}