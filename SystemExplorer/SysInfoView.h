#pragma once

#include "ViewBase.h"
#include "VirtualListView.h"
#include <Psapi.h>

class CSysInfoView :
	public CVirtualListView<CSysInfoView>,
	public CViewBase<CSysInfoView>,
	public CCustomDraw<CSysInfoView> {
public:
	CSysInfoView(IMainFrame* frame);

	CString GetColumnText(HWND, int row, int col) const;

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CSysInfoView)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		CHAIN_MSG_MAP(CViewBase<CSysInfoView>)
		CHAIN_MSG_MAP(CVirtualListView<CSysInfoView>)
		CHAIN_MSG_MAP(CCustomDraw<CSysInfoView>)
	END_MSG_MAP()

private:
	void Refresh();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	enum class RowType {
		Version, TotalRAM, TotalCPUs, CommitLimit, PeakCommit,
		Processes, Threads, Handles,
		CommitSize, AvailableVM, PagedPoolSize, NonPagedPoolSize, ResidentPagedPoolSize,
		IdleTime, SystemCalls, PageFaultCount, PageReadCount, DemandZeroCount, PageReadIoCount,
		CopyOnWriteCount, TransitionCount, FreeSystemPtes, AvailabePagePool, CacheTransitionCount, CacheReadCount,
		IoReadTransferCount, IoWriteTransferCount, IoOtherTransferCount,
		IoReadOperationCount, IoWriteOperationCount, IoOtherOperationCount, BootTime,
		SystemDriverPages, SystemCodePages, ResidentSystemCodePages,
		COUNT
	};

	enum class ItemDataFlags {
		None = 0,
		Const = 1,
		Bits32 = 2,
		Time = 4,
		TimeSpan = 8,
		MemoryInPages = 16,
		MemorySize = 32,
		Special = 64
	};

	struct ItemData {
		ItemData(RowType type, PCWSTR name, uint32_t* pCurrent, uint32_t* pPrevious, ItemDataFlags flags = ItemDataFlags::None);
		ItemData(RowType type, PCWSTR name, DWORD* pCurrent, DWORD* pPrevious, ItemDataFlags flags = ItemDataFlags::None);
		ItemData(RowType type, PCWSTR name, uint64_t* pCurrent, uint64_t* pPrevious, ItemDataFlags flags = ItemDataFlags::None);
		ItemData(RowType type, PCWSTR name, int64_t* pCurrent, int64_t* pPrevious, ItemDataFlags flags = ItemDataFlags::None);
		ItemData(RowType type, PCWSTR name, ItemDataFlags flags = ItemDataFlags::None);

		RowType Type;
		union {
			uint32_t* Value32;
			uint64_t* Value64;
		} Current;
		union {
			uint32_t* Value32;
			uint64_t* Value64;
		} Previous;
		CString Name;
		ItemDataFlags Flags;
	};

private:
	std::vector<ItemData> m_Items;
	CListViewCtrl m_List;
	WinSys::BasicSystemInfo m_BasicSysInfo;
	WinSys::PerformanceInformation m_SysPerfInfo{};
	PERFORMANCE_INFORMATION m_PerfInfo{};
	WinSys::PerformanceInformation m_OldSysPerfInfo{};
	PERFORMANCE_INFORMATION m_OldPerfInfo{};
};

DEFINE_ENUM_FLAG_OPERATORS(CSysInfoView::ItemDataFlags);
