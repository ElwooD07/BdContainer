#include "stdafx.h"
#include "FsTreeMenuElement.h"
#include "MainWindowView.h"
#include "ModelUtils.h"

gui::FsTreeMenuElement::FsTreeMenuElement(QWidget* parent, QTreeView* treeView, MainWindowView* mainWindow, QAbstractItemModel* model)
	: QMenu(tr("Element"), parent)
	, m_treeView(treeView)
	, m_mainWindow(mainWindow)
	, m_model(model)
{
	QList<QAction*> actionsForEditableElement;
	actionsForEditableElement << addAction(tr("Rename"), this, SLOT(OnRenameTriggered()), Qt::Key_F2);
	actionsForEditableElement << addAction(tr("Add content..."), this, SLOT(OnAddContentTriggered()));
	actionsForEditableElement << addAction(tr("Export..."), this, SLOT(OnExportTriggered()));
	actionsForEditableElement << addAction(tr("Delete"), this, SLOT(OnDeleteTriggered()), Qt::Key_Delete);
	addSeparator();
	actionsForEditableElement << addAction(tr("Info..."), this, SLOT(OnInfoTriggered()));

	for (QAction* action : actionsForEditableElement)
	{
		connect(this, &FsTreeMenuElement::SetEnabledActionForEditableElement, action, &QAction::setEnabled);
	}
	connect(this, &QMenu::aboutToShow, this, &FsTreeMenuElement::OnAboutToShow);
}

void gui::FsTreeMenuElement::OnAboutToShow()
{
	emit SetEnabledActionForEditableElement(model::utils::IsItemEditable(m_treeView->currentIndex()));
}

void gui::FsTreeMenuElement::OnRenameTriggered()
{
	QModelIndex index = m_treeView->currentIndex();
	if (index.isValid())
	{
		m_treeView->edit(index);
	}
}

void gui::FsTreeMenuElement::OnAddContentTriggered()
{

}

void gui::FsTreeMenuElement::OnExportTriggered()
{

}

void gui::FsTreeMenuElement::OnDeleteTriggered()
{
	QModelIndex index = m_treeView->currentIndex();
	if (model::utils::IsItemEditable(index))
	{
		const QString& itemName = index.data(Qt::DisplayRole).toString();
		if (m_mainWindow->ShowQuestion(tr("Do you really want to delete item %1?").arg(itemName, 1),
			tr("Delete item")) == QMessageBox::Yes)
		{
			m_model->removeRow(index.row(), index.parent());
		}
	}
}

void gui::FsTreeMenuElement::OnInfoTriggered()
{

}

