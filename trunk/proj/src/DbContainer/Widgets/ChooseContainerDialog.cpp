#include "stdafx.h"
#include "Widgets/ChooseContainerDialog.h"
#include "ContainerException.h"

gui::ChooseContainerDialog::ChooseContainerDialog(QWidget* parent, ActionType action)
	: QDialog(parent)
	, m_action(action)
{
	m_ui.setupUi(this);
	m_ui.btnProceed->setText(action == ActionOpen ? tr("Open") : tr("Create"));
	on_txtPath_textChanged();
}

dbc::ContainerGuard gui::ChooseContainerDialog::GetContainer()
{
	return m_container;
}

QString gui::ChooseContainerDialog::GetChosenPath()
{
	return m_ui.txtPath->text();
}

void gui::ChooseContainerDialog::on_btnBrowse_clicked()
{
	QString path;
	if (m_action == ActionOpen)
	{
		path = QFileDialog::getOpenFileName(this, tr("Open container"), qApp->applicationDirPath(), tr("All files (%1)").arg("*.*", 1));
	}
	else
	{
		path = QFileDialog::getSaveFileName(this, tr("Create container"), qApp->applicationDirPath(), tr("DataBase files (%1)").arg("*.db", 1));
	}
	if (!path.isEmpty())
	{
		m_ui.txtPath->setText(path);
	}
}

void gui::ChooseContainerDialog::on_btnProceed_clicked()
{
	try
	{
		if (m_action == ActionOpen)
		{
			m_container = dbc::Connect(m_ui.txtPath->text().toStdString(), m_ui.txtPassword->text().toStdString());
		}
		else
		{
			m_container = dbc::CreateContainer(m_ui.txtPath->text().toStdString(), m_ui.txtPassword->text().toStdString());
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
