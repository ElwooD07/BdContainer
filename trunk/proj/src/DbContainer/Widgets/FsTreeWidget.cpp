#include "stdafx.h"
#include "FsTreeWidget.h"
#include "FsTreeMenuTree.h"
#include "FsTreeMenuElement.h"
#include "DbContainerModel.h"
#include "MainWindowView.h"
#include "TreeItemDelegate.h"

gui::FsTreeWidget::FsTreeWidget(QWidget* parent, MainWindowView* mainWindow)
	: QWidget(parent)
	, m_mainWindow(mainWindow)
	, m_model(new model::DbContainerModel(this))
	, m_menuTree(nullptr)
	, m_menuElement(nullptr)
{
	m_ui.setupUi(this);
	connect(m_ui.treeFs, &QTreeView::expanded, m_model, &model::DbContainerModel::OnItemExpanded);
	connect(m_model, &model::DbContainerModel::ShowMessage, m_mainWindow, &MainWindowView::ShowMessage);
	m_ui.treeFs->setEditTriggers(QAbstractItemView::DoubleClicked);
	m_ui.treeFs->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui.treeFs->setModel(m_model);
	m_ui.treeFs->setItemDelegate(new TreeItemDelegate(this));

	connect(m_ui.treeFs, &QTreeView::clicked, this, &gui::FsTreeWidget::OnItemSelected);

	InitMenus();
}

void gui::FsTreeWidget::AddContainer(dbc::ContainerGuard container)
{
	m_model->AddContainer(container);
}

dbc::ContainerGuard gui::FsTreeWidget::GetSelectedContainer()
{
	return m_model->GetContainerByIndex(m_ui.treeFs->currentIndex());
}

QMenu* gui::FsTreeWidget::GetTreeMenu()
{
	assert(m_menuTree != nullptr);
	return m_menuTree;
}

QMenu* gui::FsTreeWidget::GetElementMenu()
{
	assert(m_menuElement != nullptr);
	return m_menuElement;
}

void gui::FsTreeWidget::OnCollapseAllTriggered()
{
	m_ui.treeFs->collapseAll();
}

void gui::FsTreeWidget::OnExpandAllTriggered()
{
	m_ui.treeFs->expandAll();
}

void gui::FsTreeWidget::OnItemSelected(const QModelIndex& index)
{
	dbc::ContainerElementGuard element = m_model->GetElementByIndex(index);
	emit CurrentElementChanged(element);
}

void gui::FsTreeWidget::InitMenus()
{
	assert(m_menuTree == nullptr && m_menuElement == nullptr);

	m_menuTree = new FsTreeMenuTree(this, m_ui.treeFs, m_mainWindow, m_model);
	m_menuElement = new FsTreeMenuElement(this, m_ui.treeFs, m_mainWindow, m_model);

	m_menuTree->addSeparator();
	m_menuTree->addAction(tr("Collapse All"), this, SLOT(OnCollapseAllTriggered()));
	m_menuTree->addAction(tr("Expand All"), this, SLOT(OnExpandAllTriggered()));
}
