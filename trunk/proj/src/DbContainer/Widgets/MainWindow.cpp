#include "stdafx.h"
#include "MainWindow.h"
#include "FsTreeWidget.h"
#include "ElementViewWidget.h"
#include "ChooseContainerDialog.h"
#include "ContainerException.h"
#include "ModelUtils.h"

gui::MainWindow::MainWindow()
	: m_fsTreeWidget(nullptr)
{
	m_ui.setupUi(this);
	InitMainControls();
}

void gui::MainWindow::OnOpenTriggered()
{
	ChooseContainerDialog dialog(this, ChooseContainerDialog::ActionOpen);
	int res = dialog.exec();
	if (res == QDialog::Accepted)
	{
		m_fsTreeWidget->AddContainer(dialog.GetContainer());
	}
}

void gui::MainWindow::InitMainControls()
{
	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	assert(m_fsTreeWidget == nullptr);
	m_fsTreeWidget = new FsTreeWidget(this);
	splitter->addWidget(m_fsTreeWidget);
	ElementViewWidget* elementView = new ElementViewWidget(this);
	splitter->addWidget(elementView);
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter);
	m_ui.centralWidget->setLayout(mainLayout);

	connect(m_ui.actionContainerOpen, &QAction::triggered, this, &MainWindow::OnOpenTriggered);
}
