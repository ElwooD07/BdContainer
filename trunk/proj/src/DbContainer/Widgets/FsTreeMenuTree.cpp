#include "stdafx.h"
#include "FsTreeMenuTree.h"
#include "DbContainerModel.h"

namespace
{
	bool IsFolder(const QModelIndex& index)
	{
		return index.isValid() && index.data(model::DbContainerModel::ItemTypeRole).value<dbc::ElementType>() == dbc::ElementTypeFolder;
	}
}

gui::FsTreeMenuTree::FsTreeMenuTree(QWidget* parent, QTreeView* treeView)
	: QMenu(tr("Tree"), parent)
	, m_treeView(treeView)
{
	QList<QAction*> actionsForSelectedFolder;
	actionsForSelectedFolder << addAction(tr("Add file..."), this, SLOT(OnAddFileTriggered()));
	actionsForSelectedFolder << addAction(tr("Add folder..."), this, SLOT(OnAddFolderTriggered()));

	for (QAction* action : actionsForSelectedFolder)
	{
		connect(this, &FsTreeMenuTree::SetEnabledActionsForSelectedFolder, action, &QAction::setEnabled);
	}
	connect(this, &QMenu::aboutToShow, this, &FsTreeMenuTree::OnAboutToShow);
}

void gui::FsTreeMenuTree::OnAboutToShow()
{
	emit SetEnabledActionsForSelectedFolder(IsFolder(m_treeView->currentIndex()));
}

void gui::FsTreeMenuTree::OnAddFileTriggered()
{

}

void gui::FsTreeMenuTree::OnAddFolderTriggered()
{

}

