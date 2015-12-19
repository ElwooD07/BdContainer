#pragma once
#include "ContainerAPI.h"

namespace model
{
	class TreeNode;
	typedef QVector<TreeNode*> TreeNodesList;

	class TreeNode
	{
	public:
		TreeNode(const TreeNode* parent, const QString& parentPath, dbc::ElementGuard element);
		~TreeNode();

		const dbc::ElementGuard GetElement() const;
		const TreeNode* GetParent() const;
		int GetRow() const;

		void RefreshPath(const QString& newParentPath = "");
		void ChangeParent(const TreeNode* newParent);

		TreeNode* GetChild(int row);
		int GetChildrenCount() const;
		TreeNode* AddChild(dbc::ElementGuard child);
		void RemoveChild(int row);

		bool operator<(const TreeNode& node) const;
		bool operator==(const TreeNode& node) const;

	public:
		bool wasLoaded;

	private:
		const TreeNode* m_parent;
		QString m_path;
		dbc::ElementGuard m_element;
		bool loaded;

		TreeNodesList m_children;
	};
}
