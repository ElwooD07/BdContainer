#include "stdafx.h"
#include "FsTreeWidget.h"
#include "DbContainerModel.h"
#include "MainWindowView.h"

gui::FsTreeWidget::FsTreeWidget(QWidget* parent, MainWindowView* mainWindow)
	: QWidget(parent)
	, m_mainWindow(mainWindow)
	, m_model(new model::DbContainerModel(this))
{
	m_ui.setupUi(this);
	connect(m_ui.treeFs, &QTreeView::expanded, m_model, &model::DbContainerModel::OnItemExpanded);
	connect(m_model, &model::DbContainerModel::ShowMessage, m_mainWindow, &MainWindowView::OnShowMessage);
	m_ui.treeFs->setModel(m_model);
}

void gui::FsTreeWidget::AddContainer(dbc::ContainerGuard container)
{
	m_model->AddContainer(container);
}

void gui::FsTreeWidget::RenameCurrentItem()
{
	QModelIndex index = m_ui.treeFs->currentIndex();
	if (index.isValid())
	{
		m_ui.treeFs->edit(index);
	}
}
