#pragma once

#include "VirtualListView.h"
#include "IObjectsView.h"
#include "ObjectManager.h"
#include "resource.h"

class CHandlesView :
	public CWindowImpl<CHandlesView, CListViewCtrl>,
	public CVirtualListView<CHandlesView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CHandlesView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type = nullptr, DWORD pid = 0);

	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;
	static CString HandleAttributesToString(ULONG attributes);

	BEGIN_MSG_MAP(CHandlesView)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP_ALT(CVirtualListView<CHandlesView>, 1)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMessage)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_HANDLES_CLOSEHANDLE, OnCloseHandle)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCloseHandle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	bool CompareItems(HandleInfo& o1, HandleInfo& o2, const SortInfo* si);
	void Refresh();

private:
	ObjectManager m_ObjMgr;
	CString m_HandleType;
	IMainFrame* m_pFrame;
	CUpdateUIBase* m_pUI;
	int m_ColumnCount;
	int m_Pid;
	std::vector<std::shared_ptr<HandleInfo>> m_Handles;
	wil::unique_handle m_hProcess;
};

