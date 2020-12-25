#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "resource.h"
#include "ViewBase.h"
#include "ThreadInfoEx.h"
#include "ProcessManager.h"

class CThreadsView :
	public CVirtualListView<CThreadsView>,
	public CCustomDraw<CThreadsView>,
	public CViewBase<CThreadsView> {
public:
	CThreadsView(IMainFrame* frame, DWORD pid = 0);

	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row) const;
	void DoSort(const SortInfo* si);

	void OnActivate(bool);
	void OnUpdate();
	void OnPauseResume(bool paused);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CThreadsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		COMMAND_ID_HANDLER(ID_HEADER_HIDECOLUMN, OnHideColumn)
		COMMAND_ID_HANDLER(ID_HEADER_COLUMNS, OnSelectColumns)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP(CViewBase<CThreadsView>)
		CHAIN_MSG_MAP(CVirtualListView<CThreadsView>)
		CHAIN_MSG_MAP(CCustomDraw<CThreadsView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHideColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectColumns(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemStateChanged(int, LPNMHDR hdr, BOOL&);
	LRESULT OnListRightClick(int, LPNMHDR hdr, BOOL&);

private:
	void Refresh();
	void UpdateUI();
	static PCWSTR ThreadStateToString(WinSys::ThreadState state);
	static PCWSTR WaitReasonToString(WinSys::WaitReason reason);

	enum class ThreadColumn {
		State, Id, ProcessId, ProcessName, CPU, CPUTime, CreateTime, Priority, BasePriority, Teb,
		WaitReason, StartAddress, Win32StartAddress, StackBase, StackLimit, ContextSwitches,
		KernelTime, UserTime, IoPriority, MemoryPriority, ComFlags, ComApartment, 
		WaitTime, COUNT
	};

	ThreadInfoEx& GetThreadInfoEx(WinSys::ThreadInfo* ti) const;

private:
	CListViewCtrl m_List;
	WinSys::ProcessManager m_ProcMgr;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_Threads;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_NewThreads;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_TermThreads;
	mutable std::unordered_map<WinSys::ThreadInfo*, ThreadInfoEx> m_ThreadsEx;
	HFONT m_hFont;
	DWORD m_Pid;
	int m_SelectedHeader;
};

