#pragma once
#include "Element.h"
#include "ui_ElementViewWidget.h"

namespace gui
{
	class ElementViewWidget : public QWidget
	{
	public:
		explicit ElementViewWidget(QWidget* parent);

	public slots:
		void SetElement(dbc::ElementGuard element);

	private:
		void SetInfoForCurrentElement();

	private:
		Ui::ElementViewWidget m_ui;
		dbc::ElementGuard m_element;
	};
}