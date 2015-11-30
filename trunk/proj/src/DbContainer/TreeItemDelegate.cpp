#include "stdafx.h"
#include "TreeItemDelegate.h"
#include "DbContainerModel.h"

namespace
{
	const char* s_oldValueProp = "txt_old_value_prop";
}

gui::TreeItemDelegate::TreeItemDelegate(QObject* parent /*= 0*/)
	: QStyledItemDelegate(parent)
{ }

QWidget * gui::TreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return new QLineEdit(parent);
}

void gui::TreeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QLineEdit* txt = dynamic_cast<QLineEdit*>(editor);
	assert(txt != nullptr);
	const QString& text = index.data(model::DbContainerModel::ItemNameRole).toString();
	txt->setText(text);
	txt->setProperty(s_oldValueProp, text);
}

void gui::TreeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QLineEdit* txt = dynamic_cast<QLineEdit*>(editor);
	assert(txt != nullptr);
	const QString& newText = txt->text();
	const QString& oldText = txt->property(s_oldValueProp).toString();
	assert(!oldText.isEmpty());
	if (!newText.isEmpty() && newText != oldText)
	{
		model->setData(index, newText, model::DbContainerModel::ItemNameRole);
	}
}

void gui::TreeItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}
