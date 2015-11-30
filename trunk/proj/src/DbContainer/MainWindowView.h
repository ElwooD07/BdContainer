#pragma once

namespace gui
{
	class MainWindowView: public QMainWindow
	{
		Q_OBJECT;

	public:
		virtual QMessageBox::StandardButton ShowQuestion(const QString& message, const QString& title = "", QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No) const = 0;

	public slots:
		virtual void ShowMessage(const QString& message, const QString& title = "", QMessageBox::Icon icon = QMessageBox::Information) const = 0;

	protected:
		MainWindowView(QWidget* parent, Qt::WindowFlags flags)
			:QMainWindow(parent, flags)
		{ }
	};
}
