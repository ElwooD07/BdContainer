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

	InitMenus();
}

void gui::FsTreeWidget::AddContainer(dbc::ContainerGuard container)
{
	m_model->AddContainer(container);
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

void gui::FsTreeWidget::OnMenuTreeAboutToShow()
{
	emit SetEnabledActionsForSelectedItem(m_ui.treeFs->currentIndex().isValid());
}

void gui::FsTreeWidget::OnCollapseTriggered()
{
	m_ui.treeFs->collapse(m_ui.treeFs->currentIndex());
}

void gui::FsTreeWidget::OnCollapseAllTriggered()
{
	m_ui.treeFs->collapseAll();
}

void gui::FsTreeWidget::OnExpandTriggered()
{
	m_ui.treeFs->expand(m_ui.treeFs->currentIndex());
}

void gui::FsTreeWidget::OnExpandAllTriggered()
{
	m_ui.treeFs->expandAll();
}

void gui::FsTreeWidget::InitMenus()
{
	assert(m_menuTree == nullptr && m_menuElement == nullptr);

	m_menuTree = new FsTreeMenuTree(this, m_ui.treeFs);
	m_menuElement = new FsTreeMenuElement(this, m_ui.treeFs, m_mainWindow, m_model);

	m_menuTree->addSeparator();
	QList<QAction*> actionsForSelectedItem;
	actionsForSelectedItem << m_menuTree->addAction(tr("Collapse"), this, SLOT(OnCollapseTriggered()));
	m_menuTree->addAction(tr("Collapse All"), this, SLOT(OnCollapseAllTriggered()));
	actionsForSelectedItem << m_menuTree->addAction(tr("Expand"), this, SLOT(OnExpandTriggered()));
	m_menuTree->addAction(tr("Expand All"), this, SLOT(OnExpandAllTriggered()));

	for (QAction* action : actionsForSelectedItem)
	{
		connect(this, &FsTreeWidget::SetEnabledActionsForSelectedItem, action, &QAction::setEnabled);
	}
}
