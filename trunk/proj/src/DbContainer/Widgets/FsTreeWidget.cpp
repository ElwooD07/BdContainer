#include "stdafx.h"
#include "FsTreeWidget.h"
#include "DbContainerModel.h"

gui::FsTreeWidget::FsTreeWidget(QWidget* parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);
}

void gui::FsTreeWidget::AddContainer(dbc::ContainerGuard container)
{
	model::DbContainerModel* model = new model::DbContainerModel(container, this);
	connect(m_ui.treeFs, &QTreeView::expanded, model, &model::DbContainerModel::OnItemExpanded);
	m_ui.treeFs->setModel(model);
}
