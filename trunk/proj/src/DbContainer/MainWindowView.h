#pragma once

namespace gui
{
	class MainWindowView: public QMainWindow
	{
		Q_OBJECT;

	public slots :
		virtual void OnShowMessage(const QString& message, const QString& title = "", QMessageBox::Icon icon = QMessageBox::Information) = 0;

	protected:
		MainWindowView(QWidget* parent, Qt::WindowFlags flags)
			:QMainWindow(parent, flags)
		{ }
	};
}
