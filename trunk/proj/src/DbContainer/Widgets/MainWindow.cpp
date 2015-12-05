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

void gui::MainWindow::OnContainerInfoClicked()
{
	dbc::ContainerGuard selectedContainer = m_fsTreeWidget->GetSelectedContainer();
	if (selectedContainer.get() != nullptr)
	{
		dbc::ContainerInfo info = selectedContainer->GetInfo();
		QString infoStr = tr("Total elements: %1, Total streams: %2, Used streams: %3, Used space: %4, Free space: %5").arg(
			info->TotalElements(), 1).arg(info->TotalStreams(), 2).arg(info->UsedStreams(), 3).arg(info->UsedSpace(), 4).arg(
			info->FreeSpace(), 5);
		QMessageBox::information(this, tr("Container info"), infoStr);
	}
}

void gui::MainWindow::ShowMessage(const QString& message, const QString& title /*= ""*/, QMessageBox::Icon icon /*= QMessageBox::Information*/) const
{
	ShowMessageDialog(message, title, icon, QMessageBox::NoButton);
}

QMessageBox::StandardButton gui::MainWindow::ShowQuestion(const QString& message, const QString& title /*= ""*/, QMessageBox::StandardButtons buttons /*= QMessageBox::Yes | QMessageBox::No*/) const
{
	assert(buttons != QMessageBox::NoButton);
	return ShowMessageDialog(message, title, QMessageBox::Question, buttons);
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
	connect(m_ui.actionContainerInfo, &QAction::triggered, this, &MainWindow::OnContainerInfoClicked);
	m_ui.actionContainerOpen->setShortcut(Qt::CTRL | Qt::Key_O);
	m_ui.actionContainerCreate->setShortcut(Qt::CTRL | Qt::Key_N);

	m_ui.menuBar->addMenu(m_fsTreeWidget->GetTreeMenu());
	m_ui.menuBar->addMenu(m_fsTreeWidget->GetElementMenu());
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

QMessageBox::StandardButton gui::MainWindow::ShowMessageDialog(const QString& message, const QString& title, QMessageBox::Icon icon, QMessageBox::StandardButtons buttons) const
{
	const QString newTitle = title.isEmpty() ? windowTitle() : title;
	QMessageBox box(icon, newTitle, message, buttons);
	return static_cast<QMessageBox::StandardButton>(box.exec());
}
