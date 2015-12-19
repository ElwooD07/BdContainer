#include "stdafx.h"
#include "TreeNode.h"
#include "ModelUtils.h"

model::TreeNode::TreeNode(const TreeNode* parent, const QString& parentPath, dbc::ElementGuard element)
	: m_parent(parent)
	, m_path(utils::SlashedPath(parentPath) + utils::StdString2QString(element->Name()))
	, m_element(element)
	, wasLoaded(false)
{ }

model::TreeNode::~TreeNode()
{
	qDeleteAll(m_children);
}

void model::TreeNode::RefreshPath(const QString& newParentPath /*= ""*/)
{
	if (newParentPath.isEmpty())
	{
		m_path = utils::StdString2QString(m_element->Path());
	}
	else
	{
		m_path = utils::SlashedPath(newParentPath) + utils::StdString2QString(m_element->Name());
	}
	for (TreeNode* child : m_children)
	{
		child->RefreshPath(m_path);
	}
}

model::TreeNode* model::TreeNode::AddChild(dbc::ElementGuard child)
{
	assert(child.get() != nullptr);
	m_children.push_back(new TreeNode(this, m_path, child));
	return m_children.back();
}

void model::TreeNode::RemoveChild(int row)
{
	m_children[row]->GetElement()->Remove();
	delete m_children[row];
	m_children.erase(m_children.begin() + row);
}

void model::TreeNode::ChangeParent(const TreeNode* newParent)
{
	assert(newParent != nullptr);
	m_parent = newParent;
}

model::TreeNode* model::TreeNode::GetChild(int row)
{
	if (row < 0 || row >= m_children.size())
	{
		return nullptr;
	}
	return m_children[row];
}

int model::TreeNode::GetChildrenCount() const
{
	return m_children.size();
}

const dbc::ElementGuard model::TreeNode::GetElement() const
{
	return m_element;
}

const model::TreeNode* model::TreeNode::GetParent() const
{
	return m_parent;
}

int model::TreeNode::GetRow() const
{
	if (m_parent != nullptr)
	{
		return m_parent->m_children.indexOf(const_cast<TreeNode*>(this));
	}
	return 0;
}

bool model::TreeNode::operator<(const TreeNode& node) const
{
	return m_path < node.m_path;
}

bool model::TreeNode::operator==(const TreeNode& node) const
{
	return m_path == node.m_path;
}
