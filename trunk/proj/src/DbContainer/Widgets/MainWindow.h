#pragma once
#include "ui_MainWindow.h"

namespace gui
{
	class FsTreeWidget;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT;

	public:
		MainWindow();

	private slots:
		void OnOpenTriggered();

	private:
		void InitMainControls();

	private:
		Ui::MainWindow m_ui;
		FsTreeWidget* m_fsTreeWidget;
	};
}