#pragma once

#include "VirtualListView.h"
#include "resource.h"

class CLogonSessionsView :
	public CFrameWindowImpl<CLogonSessionsView, CWindow, CControlWinTraits>,
	public CVirtualListView<CLogonSessionsView> {
public:
	using BaseFrame = CFrameWindowImpl<CLogonSessionsView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(nullptr)

	BEGIN_MSG_MAP(CLogonSessionsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CLogonSessionsView>)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col) const;
	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;
	static PCWSTR LogonTypeToString(WinSys::SecurityLogonType type);

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

private:
	CListViewCtrl m_List;
	std::vector<WinSys::LogonSessionData> m_Sessions;
};
