#include "stdafx.h"
#include "DbContainerModel.h"
#include "ModelUtils.h"
#include "ContainerException.h"

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

#define DBC_MODEL_TRY(actionStr) const QString titleStr(actionStr); \
	try {
#define DBC_MODEL_CATCH } \
	catch (const dbc::ContainerException& ex) \
	{ \
		emit ShowMessage(model::utils::StdString2QString(ex.FullMessage()), titleStr, QMessageBox::Warning); \
	} \
	catch(const std::exception& ex) \
	{ \
		emit ShowMessage(model::utils::StdString2QString(ex.what()), titleStr, QMessageBox::Critical); \
	}
}

model::DbContainerModel::DbContainerModel(QObject* parent /*= nullptr*/)
	: QAbstractItemModel(parent)
{ }

model::DbContainerModel::~DbContainerModel()
{
	qDeleteAll(m_rootNodes);
}

void model::DbContainerModel::AddContainer(dbc::ContainerGuard container)
{
	m_containers.push_back(container);
	int containerIndex = m_containers.size() - 1;
	beginInsertRows(QModelIndex(), containerIndex, containerIndex);
	dbc::ContainerFolderGuard root = container->GetRoot();
	m_rootNodes.push_back(new TreeNode(nullptr, utils::StdString2QString(root->Path()), root));
	endInsertRows();
}

QVariant model::DbContainerModel::data(const QModelIndex& index, int role) const
{
	TreeNode* node = Index2Node(index);
	if (node != nullptr)
	{
		if (role == Qt::DisplayRole)
		{
			return GetDisplayData(node, index.row());
		}
	}
	return QVariant();
}

bool model::DbContainerModel::hasChildren(const QModelIndex& parent /*= QModelIndex()*/) const
{
	TreeNode* node = Index2Node(parent);
	if (node == nullptr) // For the root
	{
		return !m_rootNodes.isEmpty();
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
		return m_rootNodes.empty() ? QModelIndex() : createIndex(row, column, m_rootNodes[row]);
	}
	TreeNode* parentNode = ParentIndex2Node(parent, row);
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
	TreeNode* parentNode = const_cast<TreeNode*>(node->GetParent());
	// It is one of the root nodes, set its row as the index of the roots vector
	int rootIndex = m_rootNodes.indexOf(parentNode);
	return createIndex(rootIndex >= 0 ? rootIndex : node->GetRow(), 0, parentNode);
}

int model::DbContainerModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	TreeNode* node = Index2Node(parent);
	// Check for root
	return (node == nullptr) ? m_rootNodes.size() : node->GetChildrenCount();
}

int model::DbContainerModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return 1;
}

bool model::DbContainerModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
	TreeNode* node = Index2Node(index);
	if (node != nullptr)
	{
		switch (role)
		{
		case RoleName:
			SetNodeName(index, value.toString());
			break;
		default:
			return QAbstractItemModel::setData(index, value, role);
		}
		return true;
	}
	return false;
}

void model::DbContainerModel::OnItemExpanded(const QModelIndex& index)
{
	LoadChildren(index);
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

QVariant model::DbContainerModel::GetDisplayData(model::TreeNode* node, int row) const
{
	if (node->GetElement()->Type() == dbc::ElementTypeFolder && node->GetElement()->AsFolder()->IsRoot())
	{
		return utils::StdString2QString(m_containers[row]->GetPath());
	}
	else
	{
		return utils::StdString2QString(node->GetElement()->Name());
	}
}

void model::DbContainerModel::SetNodeName(const QModelIndex& index, const QString& name)
{
	DBC_MODEL_TRY(tr("Set element name"));
	TreeNode* node = Index2Node(index);
	assert(node != nullptr);
	node->GetElement()->Rename(utils::QString2StdString(name));
	node->RefreshPath();
	emit dataChanged(index, index);
	DBC_MODEL_CATCH;
}

model::TreeNode* model::DbContainerModel::ParentIndex2Node(const QModelIndex& parent, int row) const
{
	model::TreeNode* parentNode = Index2Node(parent);
	if (parentNode == nullptr)
	{
		parentNode = m_rootNodes[row];
	}
	return parentNode;
}
