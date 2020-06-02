#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ProcessInfoEx.h"
#include "resource.h"

class CProcessesView :
	public CFrameWindowImpl<CProcessesView, CWindow, CControlWinTraits>,
	public CVirtualListView<CProcessesView>,
	public CCustomDraw<CProcessesView> {
public:
	DECLARE_WND_CLASS(nullptr)

	CProcessesView(IMainFrame* frame);

	using BaseFrame = CFrameWindowImpl<CProcessesView, CWindow, CControlWinTraits>;

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;
	void DoSort(const SortInfo* si);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CServicesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		//NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		COMMAND_ID_HANDLER(ID_HEADER_HIDECOLUMN, OnHideColumn)
		COMMAND_ID_HANDLER(ID_HEADER_COLUMNS, OnSelectColumns)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_PAUSE, OnPause)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CProcessesView>)
		CHAIN_MSG_MAP(CCustomDraw<CProcessesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHideColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectColumns(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnListRightClick(int, LPNMHDR hdr, BOOL&);
	LRESULT OnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

	ProcessInfoEx& GetProcessInfoEx(WinSys::ProcessInfo* pi) const;
	void GetProcessColors(const ProcessInfoEx& px, COLORREF& bk, COLORREF& text) const;
	static CString CProcessesView::ProcessAttributesToString(ProcessAttributes attributes);
	static void GetCPUColors(int cpu, COLORREF& bk, COLORREF& text);

private:
	enum class ProcessColumn {
		Name, Id, UserName, Session, CPU, CPUTime, Parent, Priority, PriorityClass, Threads, PeakThreads,
		Handles, Attributes, ExePath, CreateTime, CommitSize, PeakCommitSize,
		WorkingSet, PeakWorkingSet, VirtualSize, PeakVirtualSize,
		PagedPool, PeakPagedPool, NonPagedPool, PeakNonPagedPool,
		COUNT
	};

private:
	std::vector<std::shared_ptr<WinSys::ProcessInfo>> m_Processes;
	mutable std::unordered_map<WinSys::ProcessInfo*, ProcessInfoEx> m_ProcessesEx;
	WinSys::ProcessManager m_ProcMgr;
	CImageList m_Images;
	HFONT m_hFont;
	int m_UpdateInterval = 1000, m_OldInterval;
	CListViewCtrl m_List;
	IMainFrame* m_pFrame;
	int m_SelectedHeader;
};

