#pragma once
#include "ui_MainWindow.h"
#include "MainWindowView.h"

namespace gui
{
	class FsTreeWidget;

	class MainWindow: public MainWindowView
	{
		Q_OBJECT;

	public:
		MainWindow();

	private slots:
		void OnContainerOpenTriggered();
		void OnContainerCreateTriggered();
		void OnElementRenameTriggered();

	public slots:
		virtual void OnShowMessage(const QString& message, const QString& title /*= ""*/, QMessageBox::Icon icon /*= QMessageBox::Information*/);

	private:
		void InitMainControls();
		void InitActions();
		void ContainerOpenOrCreate(bool open);

	private:
		Ui::MainWindow m_ui;
		FsTreeWidget* m_fsTreeWidget;
	};
}