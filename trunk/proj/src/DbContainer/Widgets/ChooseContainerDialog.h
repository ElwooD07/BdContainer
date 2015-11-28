#pragma once
#include "ui_ChooseContainerDialog.h"
#include "ContainerAPI.h"

namespace gui
{
	class ChooseContainerDialog : public QDialog
	{
		Q_OBJECT;

	public:
		enum ActionType
		{
			ActionOpen,
			ActionCreate
		};

		ChooseContainerDialog(QWidget* parent, ActionType action);
		dbc::ContainerGuard GetContainer();

	private slots:
		void on_btnBrowse_clicked();
		void on_btnProceed_clicked();
		void on_txtPath_textChanged();

	private:
		Ui::ChooseContainerDialog m_ui;
		ActionType m_action;

		dbc::ContainerGuard m_container;
	};
}