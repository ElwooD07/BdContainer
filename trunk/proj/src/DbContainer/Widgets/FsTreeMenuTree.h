#pragma once

namespace gui
{
	class FsTreeMenuTree : public QMenu
	{
		Q_OBJECT;

	public:
		FsTreeMenuTree(QWidget* parent, QTreeView* treeView);

	signals:
		void SetEnabledActionsForSelectedFolder(bool enabled);

	private slots:
		void OnAboutToShow();

		void OnAddFileTriggered();
		void OnAddFolderTriggered();

	private:
		QTreeView* m_treeView;
	};
}