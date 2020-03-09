#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ObjectManager.h"

class CWindowsView : 
	public CWindowImpl<CWindowsView>,
	public CVirtualListView<CWindowsView> {
public:
	DECLARE_WND_CLASS(nullptr)

	CWindowsView(IMainFrame* frame);

	enum { IdRefreshTree = 0x500, IdOnlyWithTitle, IdOnlyVisible };

	BEGIN_MSG_MAP(CWindowsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelectionChanged)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnListGetDispInfo)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(IdRefreshTree, OnRefreshTree)
		COMMAND_ID_HANDLER(IdOnlyVisible, OnRefreshTreeVisible)
		COMMAND_ID_HANDLER(IdOnlyWithTitle, OnRefreshTreeWithTitle)
		CHAIN_MSG_MAP(CVirtualListView<CWindowsView>)
	END_MSG_MAP()

	void SetDesktopOptions(bool defaultDesktopOnly = true);

	enum class TreeViewOptions {
		None = 0,
		VisibleOnly = 1,
		WithTitleOnly = 2,
	};

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeSelectionChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTreeVisible(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTreeWithTitle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void InitTree();
	void UpdateList(bool newNode);
	void InitTreeToolbar(CToolBarCtrl& tb);
	CTreeItem InsertWindow(HWND hWnd, HTREEITEM hParent);
	CString GetPropertyValue(HWND hWnd, int index) const;
	CString GetPropertyDetails(HWND hWnd, int index) const;

private:
	CTreeViewCtrlEx m_Tree;
	CListViewCtrl m_List;
	CSplitterWindow m_Splitter;
	CHorSplitterWindow m_ToolbarSplitter;
	CToolBarCtrl m_Toolbar;
	HWND m_SelectedHwnd;
	IMainFrame* m_pFrame;
	ObjectManager m_ObjMgr;
	TreeViewOptions m_TreeViewOptions = TreeViewOptions::VisibleOnly;
	bool m_DefaultDesktopOnly = true;
};

DEFINE_ENUM_FLAG_OPERATORS(CWindowsView::TreeViewOptions);
