#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "resource.h"
#include "ViewBase.h"

class CThreadsView :
	public CVirtualListView<CThreadsView>,
	public CCustomDraw<CThreadsView>,
	public CViewBase<CThreadsView> {
public:
	CThreadsView(IMainFrame* frame, DWORD pid = 0);

	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(int row) const;
	void DoSort(const SortInfo* si);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CThreadsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		COMMAND_ID_HANDLER(ID_HEADER_HIDECOLUMN, OnHideColumn)
		COMMAND_ID_HANDLER(ID_HEADER_COLUMNS, OnSelectColumns)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_PAUSE, OnPause)
		//		COMMAND_RANGE_HANDLER(ID_PRIORITYCLASS_IDLE, ID_PRIORITYCLASS_REALTIME, OnPriorityClass)
		CHAIN_MSG_MAP(CViewBase<CThreadsView>)
		CHAIN_MSG_MAP(CVirtualListView<CThreadsView>)
		CHAIN_MSG_MAP(CCustomDraw<CThreadsView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHideColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectColumns(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemStateChanged(int, LPNMHDR hdr, BOOL&);
	LRESULT OnListRightClick(int, LPNMHDR hdr, BOOL&);
	LRESULT OnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void Refresh();
	void UpdateUI();
	static PCWSTR ThreadStateToString(WinSys::ThreadState state);
	static PCWSTR WaitReasonToString(WinSys::WaitReason reason);

	enum class ThreadColumn {
		State, Id, ProcessId, ProcessName, CPU, CPUTime, CreateTime, Priority, BasePriority, Teb,
		WaitReason, StartAddress, Win32StartAddress, StackBase, StackLimit, ContextSwitches,
		KernelTime, UserTime, IoPriority, MemoryPriority, WaitTime,
		COUNT
	};

	struct ThreadInfoEx {
		DWORD64 TargetTime;
		bool IsNew { false };
		bool IsTerminating { false };
		bool IsTerminated { false };
	};

	ThreadInfoEx& GetThreadInfoEx(WinSys::ThreadInfo* ti) const;

private:
	CListViewCtrl m_List;
	WinSys::ProcessManager m_ProcMgr;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_Threads;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_NewThreads;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_TermThreads;
	mutable std::unordered_map<WinSys::ThreadInfo*, ThreadInfoEx> m_ThreadsEx;
	int m_UpdateInterval = 1000, m_OldInterval;
	HFONT m_hFont;
	DWORD m_Pid;
	int m_SelectedHeader;
};

