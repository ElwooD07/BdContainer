#include "stdafx.h"
#include "FsTreeWidget.h"
#include "DbContainerModel.h"

gui::FsTreeWidget::FsTreeWidget(QWidget* parent)
	: QWidget(parent)
	, m_model(new model::DbContainerModel(this))
{
	m_ui.setupUi(this);
	connect(m_ui.treeFs, &QTreeView::expanded, m_model, &model::DbContainerModel::OnItemExpanded);
	m_ui.treeFs->setModel(m_model);
}

void gui::FsTreeWidget::AddContainer(dbc::ContainerGuard container)
{
	m_model->AddContainer(container);
}
