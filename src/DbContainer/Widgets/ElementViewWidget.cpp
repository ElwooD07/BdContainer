#include "stdafx.h"
#include "Widgets/ElementViewWidget.h"
#include "ModelUtils.h"
#include "File.h"
#include "Folder.h"
#include "SymLink.h"
#include "DirectLink.h"

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
	RefreshCommonInfo();
	RefreshSpecificInfo();
}

void gui::ElementViewWidget::SetElement(dbc::ElementGuard element)
{
	m_element = element;
	RefreshCommonInfo();
	RefreshSpecificInfo();
}

void gui::ElementViewWidget::on_btnRefreshSpecificInfo_clicked()
{
	QString res = "Will be implemented later";
	switch (m_element->Type())
	{
	case dbc::ElementTypeFile:
		res = QString::number(m_element->AsFile()->Size());
		break;
	case dbc::ElementTypeFolder:
		break;
	case dbc::ElementTypeSymLink:
		res = model::utils::StdString2QString(m_element->AsSymLink()->TargetPath());
		break;
	case dbc::ElementTypeDirectLink:
		if (m_element->AsDirectLink()->Target().get() != nullptr)
		{
			res = model::utils::StdString2QString(m_element->AsDirectLink()->Target()->Path());
		}
		else
		{
			res = tr("Not exists");
		}
		break;
	default:
		assert(!"Unknown element type");
	}
	m_ui.lblSpecificInfo->setText(res);
}

void gui::ElementViewWidget::RefreshCommonInfo()
{
	bool elementIsSet = m_element.get() != nullptr;
	m_ui.lblImgElementType->setPixmap(QPixmap()); // TODO: load pixmap according to Type()
	m_ui.lblElementType->setText(elementIsSet ? GetTypeString(m_element) : "");
	m_ui.txtName->setText(elementIsSet ? model::utils::StdString2QString(m_element->Name()) : "");
	dbc::ElementProperties props;
	if (elementIsSet)
	{
		props = m_element->GetProperties();
	}
	m_ui.lblCreated->setText(elementIsSet ? model::utils::Timestamp2QString(props.DateCreated()) : "");
	m_ui.lblModified->setText(elementIsSet ? model::utils::Timestamp2QString(props.DateModified()) : "");
	m_ui.txtTag->setText(elementIsSet ? model::utils::StdString2QString(props.Tag()) : "");
}

void gui::ElementViewWidget::RefreshSpecificInfo()
{
	bool elementIsSet = m_element.get() != nullptr;
	m_ui.lblSpecificInfoCaption->setVisible(elementIsSet);
	m_ui.lblSpecificInfo->setVisible(elementIsSet);
	m_ui.btnRefreshSpecificInfo->setVisible(elementIsSet);
	if (elementIsSet)
	{
		switch (m_element->Type())
		{
		case dbc::ElementTypeFile:
		case dbc::ElementTypeFolder:
			m_ui.lblSpecificInfoCaption->setText(tr("Size:"));
			m_ui.lblSpecificInfo->setText("?");
			break;
		case dbc::ElementTypeSymLink:
			m_ui.lblSpecificInfoCaption->setText(tr("Target:"));
			m_ui.lblSpecificInfo->setText(model::utils::StdString2QString(m_element->AsSymLink()->TargetPath()));
			break;
		case dbc::ElementTypeDirectLink:
			m_ui.lblSpecificInfoCaption->setText(tr("Target:"));
			m_ui.lblSpecificInfo->setText("?");
			break;
		default:
			assert(!"Unknown element type");
		}
	}
}
