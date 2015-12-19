#include "stdafx.h"
#include "FsTreeMenuTree.h"
#include "DbContainerModel.h"
#include "ModelUtils.h"
#include "MainWindowView.h"
#include "ContainerException.h"

namespace
{
	bool IsFolder(const QModelIndex& index)
	{
		return index.isValid() && index.data(model::DbContainerModel::ItemTypeRole).value<dbc::ElementType>() == dbc::ElementTypeFolder;
	}
	
	const int s_maxSerachNameIterations = 100;
}

gui::FsTreeMenuTree::FsTreeMenuTree(QWidget* parent, QTreeView* treeView, MainWindowView* mainWindow, model::DbContainerModel* model)
	: QMenu(tr("Tree"), parent)
	, m_treeView(treeView)
	, m_mainWindow(mainWindow)
	, m_model(model)
{
	QList<QAction*> actionsForSelectedFolder;
	actionsForSelectedFolder << addAction(tr("Create file"), this, SLOT(OnCreateFileTriggered()), Qt::Key_Plus | Qt::Key_1);
	actionsForSelectedFolder << addAction(tr("Create folder"), this, SLOT(OnCreateFolderTriggered()), Qt::Key_Plus | Qt::Key_2);
	actionsForSelectedFolder << addAction(tr("Create link..."), this, SLOT(OnCreateLinkTriggered()), Qt::Key_Plus | Qt::Key_3);

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

void gui::FsTreeMenuTree::OnCreateFileTriggered()
{
	CreateChild(tr("New file"), tr("Create file"), dbc::ElementTypeFile);
}

void gui::FsTreeMenuTree::OnCreateFolderTriggered()
{
	CreateChild(tr("New folder"), tr("Create folder"), dbc::ElementTypeFolder);
}

void gui::FsTreeMenuTree::OnCreateLinkTriggered()
{
	CreateChild(tr("New link"), tr("Create link"), dbc::ElementTypeLink);
}

void gui::FsTreeMenuTree::CreateChild(const QString& defaultName, const QString& titleStr, dbc::ElementType type)
{
	try
	{
		if (!CreateChildImpl(defaultName, type))
		{
			m_mainWindow->ShowMessage(tr("Can't create element \"%1\" here").arg(defaultName), titleStr, QMessageBox::Warning);
		}
	}
	catch (const dbc::ContainerException& ex)
	{
		m_mainWindow->ShowMessage(model::utils::StdString2QString(ex.FullMessage()), titleStr, QMessageBox::Warning);
	}
	catch (const std::exception& ex)
	{
		m_mainWindow->ShowMessage(ex.what(), titleStr, QMessageBox::Critical);
	}
}

bool gui::FsTreeMenuTree::CreateChildImpl(const QString& defaultName, dbc::ElementType type)
{
	QModelIndex parent = m_treeView->currentIndex();
	if (model::utils::IsItemEditable(parent))
	{
		m_treeView->expand(parent);
		const QString& itemName = GetUnusedChildName(defaultName, parent);
		if (!itemName.isEmpty())
		{
			QModelIndex createdChild = m_model->AddElement(type, itemName, parent);
			assert(model::utils::IsItemEditable(createdChild));
			m_treeView->edit(createdChild);
			return true;
		}
	}
	return false;
}

QString gui::FsTreeMenuTree::GetUnusedChildName(const QString& defaultName, const QModelIndex& parent)
{
	assert(!defaultName.isEmpty());
	if (!ChildNameIsUsed(defaultName, parent))
	{
		return defaultName;
	}
	for (int n = 2; n < s_maxSerachNameIterations; ++n)
	{
		QString newName = defaultName + " " + QString::number(n);
		if (!ChildNameIsUsed(newName, parent))
		{
			return newName;
		}
	}
	return "";
}

bool gui::FsTreeMenuTree::ChildNameIsUsed(const QString& name, const QModelIndex& parent)
{
	assert(!name.isEmpty());
	int rowCount = m_model->rowCount(parent);
	for (int row = 0; row < rowCount; ++row)
	{
		QModelIndex index = m_model->index(row, 0, parent);
		assert(index.isValid());
		const QString& currentName = index.data(model::DbContainerModel::ItemNameRole).toString();
		if (currentName == name)
		{
			return true;
		}
	}
	return false;
}

