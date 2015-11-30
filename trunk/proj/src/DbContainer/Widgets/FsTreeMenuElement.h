#pragma once

namespace gui
{
	class MainWindowView;

	class FsTreeMenuElement : public QMenu
	{
		Q_OBJECT;

	public:
		FsTreeMenuElement(QWidget* parent, QTreeView* treeView, MainWindowView* mainWindow, QAbstractItemModel* model);

	signals:
		void SetEnabledActionForEditableElement(bool enabled);

	private slots:
		void OnAboutToShow();

		void OnRenameTriggered();
		void OnAddContentTriggered();
		void OnExportTriggered();
		void OnDeleteTriggered();
		void OnInfoTriggered();

	private:
		QTreeView* m_treeView;
		MainWindowView* m_mainWindow;
		QAbstractItemModel* m_model;
	};
}