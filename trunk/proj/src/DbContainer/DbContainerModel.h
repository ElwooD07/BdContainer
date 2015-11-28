#pragma once
#include "ContainerAPI.h"
#include "TreeNode.h"

namespace model
{
	class DbContainerModel: public QAbstractItemModel
	{
		Q_OBJECT

	public:
		explicit DbContainerModel(QObject* parent = nullptr);
		~DbContainerModel();

		void AddContainer(dbc::ContainerGuard container);

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
		void LoadChildren(const QModelIndex& parent);

		QVariant GetDisplayData(model::TreeNode* node, int row) const;

		model::TreeNode* ParentIndex2Node(const QModelIndex& parent, int row) const;

	private:
		QVector<dbc::ContainerGuard> m_containers;
		QVector<TreeNode*> m_rootNodes;
	};
}

