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
		virtual QMessageBox::StandardButton ShowQuestion(const QString& message, const QString& title /*= ""*/, QMessageBox::StandardButtons buttons /*= QMessageBox::Yes | QMessageBox::No*/) const;

	public slots:
		virtual void ShowMessage(const QString& message, const QString& title /*= ""*/, QMessageBox::Icon icon /*= QMessageBox::Information*/) const;

	private slots:
		void OnContainerOpenTriggered();
		void OnContainerCreateTriggered();

	private:
		void InitMainControls();
		void InitActions();
		void ContainerOpenOrCreate(bool open);

		QMessageBox::StandardButton ShowMessageDialog(const QString& message, const QString& title, QMessageBox::Icon icon, QMessageBox::StandardButtons buttons) const;

	private:
		Ui::MainWindow m_ui;
		FsTreeWidget* m_fsTreeWidget;
	};
}