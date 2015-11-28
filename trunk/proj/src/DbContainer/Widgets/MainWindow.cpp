#include "stdafx.h"
#include "MainWindow.h"
#include "FsTreeWidget.h"
#include "ElementViewWidget.h"
#include "ChooseContainerDialog.h"
#include "ContainerException.h"
#include "ModelUtils.h"

gui::MainWindow::MainWindow()
	: MainWindowView(nullptr, 0)
	, m_fsTreeWidget(nullptr)
{
	m_ui.setupUi(this);
	InitMainControls();
	InitActions();
}

void gui::MainWindow::OnContainerOpenTriggered()
{
	ContainerOpenOrCreate(true);
}

void gui::MainWindow::OnContainerCreateTriggered()
{
	ContainerOpenOrCreate(false);
}

void gui::MainWindow::OnElementRenameTriggered()
{
	m_fsTreeWidget->RenameCurrentItem();
}

void gui::MainWindow::OnShowMessage(const QString& message, const QString& title /*= ""*/, QMessageBox::Icon icon /*= QMessageBox::Information*/)
{
	const QString newTitle = title.isEmpty() ? windowTitle() : title;
	QMessageBox box(icon, newTitle, message);
	box.exec();
}

void gui::MainWindow::InitMainControls()
{
	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	assert(m_fsTreeWidget == nullptr);
	m_fsTreeWidget = new FsTreeWidget(this, this);
	splitter->addWidget(m_fsTreeWidget);
	ElementViewWidget* elementView = new ElementViewWidget(this);
	splitter->addWidget(elementView);
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter);
	m_ui.centralWidget->setLayout(mainLayout);
}

void gui::MainWindow::InitActions()
{
	connect(m_ui.actionContainerOpen, &QAction::triggered, this, &MainWindow::OnContainerOpenTriggered);
	connect(m_ui.actionContainerCreate, &QAction::triggered, this, &MainWindow::OnContainerCreateTriggered);
	connect(m_ui.actionElementRename, &QAction::triggered, this, &MainWindow::OnElementRenameTriggered);
}

void gui::MainWindow::ContainerOpenOrCreate(bool open)
{
	ChooseContainerDialog::ActionType action = open ? ChooseContainerDialog::ActionOpen : ChooseContainerDialog::ActionCreate;
	ChooseContainerDialog dialog(this, action);
	int res = dialog.exec();
	if (res == QDialog::Accepted)
	{
		m_fsTreeWidget->AddContainer(dialog.GetContainer());
	}
}
