#include "stdafx.h"
#include "Widgets/ElementViewWidget.h"
#include "ModelUtils.h"

namespace
{
	QString GetTypeString(dbc::ElementGuard element)
	{
		assert(element.get() != nullptr);
		dbc::ElementType type = element->Type();
		switch (type)
		{
		case dbc::ElementTypeFile:
			return gui::ElementViewWidget::tr("File");
		case dbc::ElementTypeFolder:
			return gui::ElementViewWidget::tr("Folder");
		case dbc::ElementTypeSymLink:
			return gui::ElementViewWidget::tr("Symbolic link");
		case dbc::ElementTypeDirectLink:
			return gui::ElementViewWidget::tr("Direct link");
		default:
			assert(!"Unknown element type");
			return "";
		}
	}
}

gui::ElementViewWidget::ElementViewWidget(QWidget* parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);
	SetInfoForCurrentElement();
}

void gui::ElementViewWidget::SetElement(dbc::ElementGuard element)
{
	m_element = element;
	SetInfoForCurrentElement();
}

void gui::ElementViewWidget::SetInfoForCurrentElement()
{
	bool elementIsSet = m_element.get() != nullptr;
	m_ui.lblImgElementType->setPixmap(QPixmap()); // TODO: load pixmap according to Type()
	m_ui.lblElementType->setText(elementIsSet ? GetTypeString(m_element) : "");
	m_ui.txtName->setText(elementIsSet ? model::utils::StdString2QString(m_element->Name()) : "");
	dbc::ElementProperties props;
	if (elementIsSet)
	{
		m_element->GetProperties(props);
	}
	m_ui.lblCreated->setText(elementIsSet ? model::utils::Timestamp2QString(props.DateCreated()) : "");
	m_ui.lblModified->setText(elementIsSet ? model::utils::Timestamp2QString(props.DateModified()) : "");
	m_ui.txtTag->setText(elementIsSet ? model::utils::StdString2QString(props.Tag()) : "");
}
