#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ProcessInfoEx.h"
#include "resource.h"
#include "ViewBase.h"

class CProcessesView :
	public CVirtualListView<CProcessesView>,
	public CCustomDraw<CProcessesView>,
	public CViewBase<CProcessesView> {
public:
	CProcessesView(IMainFrame* frame);

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row) const;
	void DoSort(const SortInfo* si);
	bool OnDoubleClickList(int row, int col, POINT& pt);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CProcessesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		COMMAND_ID_HANDLER(ID_HEADER_HIDECOLUMN, OnHideColumn)
		COMMAND_ID_HANDLER(ID_HEADER_COLUMNS, OnSelectColumns)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_PAUSE, OnPause)
		COMMAND_RANGE_HANDLER(ID_PROCESS_MEMORYMAP, ID_PROCESS_HEAPS, OnProcessItem)
		COMMAND_ID_HANDLER(ID_HANDLES_SHOWHANDLEINPROCESS, OnProcessItem)
		COMMAND_ID_HANDLER(ID_PROCESS_KILL, OnProcessKill)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_PROCESS_COLORS, OnProcessColors)
		COMMAND_ID_HANDLER(ID_PROCESS_GOTOFILELOCATION, OnProcessGoToFileLocation)
		COMMAND_RANGE_HANDLER(ID_PRIORITYCLASS_IDLE, ID_PRIORITYCLASS_REALTIME, OnPriorityClass)
		CHAIN_MSG_MAP(CViewBase<CProcessesView>)
		CHAIN_MSG_MAP(CVirtualListView<CProcessesView>)
		CHAIN_MSG_MAP(CCustomDraw<CProcessesView>)
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
	LRESULT OnProcessKill(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessGoToFileLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPriorityClass(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();
	void UpdateUI();
	void ShowProperties(int row);
	void TogglePause();
	ProcessInfoEx& GetProcessInfoEx(WinSys::ProcessInfo* pi) const;
	void GetProcessColors(const ProcessInfoEx& px, COLORREF& bk, COLORREF& text) const;
	static CString CProcessesView::ProcessAttributesToString(ProcessAttributes attributes);
	static CString IoPriorityToString(WinSys::IoPriority io);
	static PCWSTR IntegrityLevelToString(WinSys::IntegrityLevel level);
	static PCWSTR VirtualizationStateToString(WinSys::VirtualizationState state);

private:
	enum class ProcessColumn {
		Name, Id, UserName, Session, CPU, CPUTime, Parent, Priority, PriorityClass, Threads, PeakThreads,
		Handles, Attributes, ExePath, CreateTime, CommitSize, PeakCommitSize,
		WorkingSet, PeakWorkingSet, VirtualSize, PeakVirtualSize,
		PagedPool, PeakPagedPool, NonPagedPool, PeakNonPagedPool,
		KernelTime, UserTime,
		IoPriority, MemoryPriority, CommandLine, PackageFullName, JobId,
		IoReadBytes, IoWriteBytes, IoOtherBytes, IoReads, IoWrites, IoOther,
		GDIObjects, UserObjects, PeakGdiObjects, PeakUserObjects, Integrity, Elevated, Virtualized, 
		WindowTitle, Platform, Description,
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
	int m_SelectedHeader;
};

