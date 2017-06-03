#pragma once
#include "Element.h"
#include "ui_ElementViewWidget.h"

namespace gui
{
	class ElementViewWidget : public QWidget
	{
		Q_OBJECT;

	public:
		explicit ElementViewWidget(QWidget* parent);

	public slots:
		void SetElement(dbc::ElementGuard element);

	private slots:
		void on_btnRefreshSpecificInfo_clicked();

	private:
		void RefreshCommonInfo();
		void RefreshSpecificInfo();

	private:
		Ui::ElementViewWidget m_ui;
		dbc::ElementGuard m_element;
	};
}