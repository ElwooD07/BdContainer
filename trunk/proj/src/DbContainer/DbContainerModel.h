#pragma once
#include "ContainerAPI.h"
#include "TreeNode.h"

namespace model
{
	class DbContainerModel: public QAbstractItemModel
	{
		Q_OBJECT

	public:
		explicit DbContainerModel(dbc::ContainerGuard container, QObject* parent = nullptr);

		virtual QVariant data(const QModelIndex& index, int role) const;
		virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
		virtual QModelIndex parent(const QModelIndex& index) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

	public slots:
		void OnItemExpanded(const QModelIndex& index);

	private:
		void LoadRoot();
		void LoadChildren(const QModelIndex& parent);

		model::TreeNode* ParentIndex2Node(const QModelIndex& parent) const;

	private:
		dbc::ContainerGuard m_container;
		std::auto_ptr<TreeNode> m_rootNode;
	};
}

