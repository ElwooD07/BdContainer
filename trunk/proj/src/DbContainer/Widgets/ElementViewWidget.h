#pragma once
#include "ui_ElementViewWidget.h"

namespace gui
{
	class ElementViewWidget: public QWidget
	{
		Q_OBJECT;

	public:
		explicit ElementViewWidget(QWidget* parent);

	private:
		Ui::ElementViewWidget m_ui;
	};
}
