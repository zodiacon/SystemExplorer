#pragma once

#include "ViewBase.h"
#include "VirtualListView.h"
#include <Psapi.h>

class CSysInfoView :
	public CVirtualListView<CSysInfoView>,
	public CViewBase<CSysInfoView>,
	public CCustomDraw<CSysInfoView> {
public:
	CSysInfoView(IMainFrame* frame) : CViewBase(frame) {}

	CString GetColumnText(HWND, int row, int col) const;

	BEGIN_MSG_MAP(CSysInfoView)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CViewBase<CSysInfoView>)
		CHAIN_MSG_MAP(CVirtualListView<CSysInfoView>)
		CHAIN_MSG_MAP(CCustomDraw<CSysInfoView>)
	END_MSG_MAP()

private:
	void Refresh();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	enum class RowType {
		Version, TotalRAM, TotalCPUs, CommitLimit,
		Processes, Threads, Handles,
		COUNT
	};

private:
	std::vector<RowType> m_Items;
	CListViewCtrl m_List;
	WinSys::PerformanceInformation m_SysPerfInfo;
	PERFORMANCE_INFORMATION m_PerfInfo;
};

