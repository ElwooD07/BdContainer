#pragma once
#include "ContainerElement.h"
#include "ui_ElementViewWidget.h"

namespace gui
{
	class ElementViewWidget : public QWidget
	{
	public:
		explicit ElementViewWidget(QWidget* parent);

	public slots:
		void SetElement(dbc::ContainerElementGuard element);

	private:
		void SetInfoForCurrentElement();

	private:
		Ui::ElementViewWidget m_ui;
		dbc::ContainerElementGuard m_element;
	};
}