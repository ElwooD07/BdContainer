#include "stdafx.h"
#include "Widgets/ChooseContainerDialog.h"
#include "ContainerException.h"

gui::ChooseContainerDialog::ChooseContainerDialog(QWidget* parent, ActionType action)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
	, m_action(action)
	, m_wasShown(false)
{
	m_ui.setupUi(this);
	setFixedSize(500, 75);
	
	setWindowTitle(action == ActionOpen ? tr("Open container") : tr("Create container"));
	m_ui.btnProceed->setText(action == ActionOpen ? tr("Open") : tr("Create"));
	on_txtPath_textChanged();
}

dbc::ContainerGuard gui::ChooseContainerDialog::GetContainer()
{
	return m_container;
}

void gui::ChooseContainerDialog::on_btnBrowse_clicked()
{
	QString path = ChooseFile();
	if (!path.isEmpty())
	{
		m_ui.txtPassword->setFocus(Qt::TabFocusReason);
	}
}

void gui::ChooseContainerDialog::on_btnProceed_clicked()
{
	try
	{
		std::string path = m_ui.txtPath->text().toStdString();
		std::string password = m_ui.txtPassword->text().toStdString();
		if (m_action == ActionOpen)
		{
			m_container = dbc::Connect(path, password);
		}
		else
		{
			m_container = dbc::CreateContainer(path, password);
		}
		accept();
	}
	catch (const dbc::ContainerException& ex)
	{
		QMessageBox::warning(this, tr("Error"), ex.FullMessage().c_str());
	}
}

void gui::ChooseContainerDialog::on_txtPath_textChanged()
{
	m_ui.btnProceed->setEnabled(!m_ui.txtPath->text().isEmpty());
}

void gui::ChooseContainerDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
	if (!m_wasShown)
	{
		QString path = ChooseFile();
		if (path.isEmpty())
		{
			reject();
		}
	}
	m_wasShown = true;
}

QString gui::ChooseContainerDialog::ChooseFile()
{
	QString path;
	if (m_action == ActionOpen)
	{
		path = QFileDialog::getOpenFileName(parentWidget(), windowTitle(), qApp->applicationDirPath(), tr("All files (%1)").arg("*.*", 1));
	}
	else
	{
		path = QFileDialog::getSaveFileName(parentWidget(), windowTitle(), qApp->applicationDirPath(), tr("DataBase files (%1)").arg("*.db", 1));
	}
	m_ui.txtPath->setText(path);
	return path;
}
