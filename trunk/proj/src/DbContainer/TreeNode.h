#pragma once
#include "ContainerAPI.h"

namespace model
{
	class TreeNode;
	typedef QVector<TreeNode*> TreeNodesList;

	class TreeNode
	{
	public:
		TreeNode(const TreeNode* parent, const QString& parentPath, dbc::ContainerElementGuard element);
		~TreeNode();

		void RefreshPath(const QString& newParentPath = "");
		void AddChild(dbc::ContainerElementGuard child);
		void ChangeParent(const TreeNode* newParent);
		TreeNode* GetChild(int row);
		int GetChildrenCount() const;
		const dbc::ContainerElementGuard GetElement() const;
		const TreeNode* GetParent() const;
		int GetRow() const;

		bool operator<(const TreeNode& node) const;
		bool operator==(const TreeNode& node) const;

	public:
		bool wasLoaded;

	private:
		const TreeNode* m_parent;
		QString m_path;
		dbc::ContainerElementGuard m_element;
		bool loaded;

		TreeNodesList m_children;
	};
}
