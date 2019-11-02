#pragma once

#include "VirtualListView.h"
#include "IObjectsView.h"
#include "ObjectManager.h"

class CHandlesView :
	public CWindowImpl<CHandlesView, CListViewCtrl>,
	public CVirtualListView<CHandlesView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CHandlesView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type = nullptr, DWORD pid = 0);

	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CHandlesView)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP_ALT(CVirtualListView<CHandlesView>, 1)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

private:
	bool CompareItems(HandleInfo& o1, HandleInfo& o2, const SortInfo* si);
	void Refresh();
	static CString HandleAttributesToString(ULONG attributes);

private:
	ObjectManager m_ObjMgr;
	CString m_HandleType;
	IMainFrame* m_pFrame;
	CUpdateUIBase* m_pUI;
	int m_ColumnCount;
	int m_Pid;
	std::vector<std::shared_ptr<HandleInfo>> m_Handles;
};

