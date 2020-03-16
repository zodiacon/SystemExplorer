#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ObjectManager.h"
#include "ToolBarHelper.h"

class CWindowsView : 
	public CFrameWindowImpl<CWindowsView, CWindow, CControlWinTraits>,
	public CVirtualListView<CWindowsView>,
	public CToolBarHelper<CWindowsView> {
public:
	using BaseFrame = CFrameWindowImpl<CWindowsView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(nullptr)

	CWindowsView(IMainFrame* frame);

	enum { IdRefreshTree = 0x500, IdOnlyWithTitle, IdOnlyVisible };

	BEGIN_MSG_MAP(CWindowsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelectionChanged)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnListGetDispInfo)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(IdRefreshTree, OnRefreshTree)
		COMMAND_ID_HANDLER(IdOnlyVisible, OnRefreshTreeVisible)
		COMMAND_ID_HANDLER(IdOnlyWithTitle, OnRefreshTreeWithTitle)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnFind)
		COMMAND_ID_HANDLER(ID_EDIT_FIND_NEXT, OnFindNext)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CWindowsView>)
		CHAIN_MSG_MAP(CToolBarHelper<CWindowsView>)
	ALT_MSG_MAP(1)
	ALT_MSG_MAP(2)
		MESSAGE_HANDLER(WM_CHAR, OnComboKeyDown)
	END_MSG_MAP()

	void SetDesktopOptions(bool defaultDesktopOnly = true);

	CCommandBarCtrl m_CmdBar;		// unused

	enum class TreeViewOptions {
		None = 0,
		VisibleOnly = 1,
		WithTitleOnly = 2,
	};

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeSelectionChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTreeVisible(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshTreeWithTitle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnComboKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

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
	CToolBarCtrl m_Toolbar;
	HWND m_SelectedHwnd;
	IMainFrame* m_pFrame;
	WinSys::ProcessManager m_ProcMgr;
	CContainedWindowT<CComboBox> m_SearchCombo;
	CContainedWindowT<CEdit> m_SearchEdit;
	ObjectManager m_ObjMgr;
	TreeViewOptions m_TreeViewOptions = TreeViewOptions::VisibleOnly;
	bool m_DefaultDesktopOnly = true;
};

DEFINE_ENUM_FLAG_OPERATORS(CWindowsView::TreeViewOptions);
