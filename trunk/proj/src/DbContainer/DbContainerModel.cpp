#include "stdafx.h"
#include "DbContainerModel.h"
#include "ModelUtils.h"

namespace
{
	model::TreeNode* Index2Node(const QModelIndex& index)
	{
		if (!index.isValid())
		{
			return nullptr;
		}
		return static_cast<model::TreeNode*>(index.internalPointer());
	}
}

model::DbContainerModel::DbContainerModel(dbc::ContainerGuard container, QObject* parent /*= nullptr*/)
	: m_container(container)
{
	LoadRoot();
}

QVariant model::DbContainerModel::data(const QModelIndex& index, int role) const
{
	TreeNode* node = Index2Node(index);
	if (node != nullptr)
	{
		if (role == Qt::DisplayRole)
		{
			return utils::StdString2QString(node->GetElement()->Name());
		}
	}
	return QVariant();
}

bool model::DbContainerModel::hasChildren(const QModelIndex &parent/*= QModelIndex()*/) const
{
	TreeNode* node = Index2Node(parent);
	if (node == nullptr) // For the root
	{
		return 1;
	}
	bool showExpand = node->GetElement()->Type() == dbc::ElementTypeFolder;
	if (node->wasLoaded && node->GetChildrenCount() == 0)
	{
		showExpand = false;
	}
	return showExpand;
}

Qt::ItemFlags model::DbContainerModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index);
}

QVariant model::DbContainerModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	return QVariant();
}

QModelIndex model::DbContainerModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
	if (!parent.isValid()) // For the root
	{
		return createIndex(row, column, m_rootNode.get());
	}
	TreeNode* parentNode = ParentIndex2Node(parent);
	assert(parentNode != nullptr);
	TreeNode* childNode = parentNode->GetChild(row);
	return createIndex(row, 0, childNode);
}

QModelIndex model::DbContainerModel::parent(const QModelIndex& index) const
{
	TreeNode* node = Index2Node(index);
	if (node == nullptr || node->GetParent() == nullptr)
	{
		return QModelIndex();
	}
	return createIndex(node->GetRow(), 0, const_cast<TreeNode*>(node->GetParent()));
}

int model::DbContainerModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	TreeNode* node = Index2Node(parent);
	if (node == nullptr)  // For root
	{
		return 1;
	}
	else
	{
		return node->GetChildrenCount();
	}
}

int model::DbContainerModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return 1;
}

void model::DbContainerModel::OnItemExpanded(const QModelIndex& index)
{
	LoadChildren(index);
}

void model::DbContainerModel::LoadRoot()
{
	assert(m_rootNode.get() == nullptr);
	dbc::ContainerFolderGuard root = m_container->GetRoot();

	beginInsertRows(QModelIndex(), 0, 0);
	m_rootNode.reset(new TreeNode(nullptr, utils::StdString2QString(root->Path()), root));
	endInsertRows();

	LoadChildren(index(0, 0));
}

void model::DbContainerModel::LoadChildren(const QModelIndex& parent)
{
	TreeNode* node = Index2Node(parent);
	assert(node != nullptr);
	dbc::ContainerElementGuard element = node->GetElement();
	if (!node->wasLoaded && element->Type() == dbc::ElementTypeFolder)
	{
		dbc::ContainerFolder* folder = element->AsFolder();
		assert(folder != nullptr);
		dbc::DbcElementsIterator iterator = folder->EnumFsEntries();
		if (iterator->HasNext())
		{
			beginInsertRows(parent, 0, iterator->Count() - 1);
			int row = 0;
			for (; iterator->HasNext(); ++row)
			{
				node->AddChild(iterator->Next());
			}
			endInsertRows();
		}
	}
	node->wasLoaded = true;
	emit dataChanged(parent, parent);
}

model::TreeNode* model::DbContainerModel::ParentIndex2Node(const QModelIndex& parent) const
{
	model::TreeNode* parentNode = Index2Node(parent);
	if (parentNode == nullptr)
	{
		parentNode = m_rootNode.get();
	}
	return parentNode;
}
