#pragma once

#include "VirtualListView.h"
#include "ViewBase.h"

struct IMainFrame;

class CMemoryMapView :
	public CFrameWindowImpl<CMemoryMapView, CWindow, CControlWinTraits>,
	public CVirtualListView<CMemoryMapView>,
	public CCustomDraw<CMemoryMapView>,
	public CViewBase<CMemoryMapView> {
public:
	using BaseFrame = CFrameWindowImpl<CMemoryMapView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(nullptr)

	CMemoryMapView(IMainFrame* frame, DWORD pid);

	CString GetColumnText(HWND h, int row, int column) const;
	int GetRowImage(int row) const;
	int GetRowIndent(int row) const;

	void DoSort(const SortInfo* si);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CMemoryMapView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CMemoryMapView>)
		CHAIN_MSG_MAP(CCustomDraw<CMemoryMapView>)
		CHAIN_MSG_MAP(CViewBase<CMemoryMapView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	enum class MemoryUsage {
		PrivateData,
		Heap,
		Image,
		Mapped,
		ThreadStack,
		Unusable,
		Unknown = 99,
	};

	struct ItemDetails {
		CString Details;
		MemoryUsage Usage;
	};

	struct HeapInfo {
		DWORD_PTR Id;
		DWORD_PTR Address;
		DWORD_PTR Size;
		DWORD Flags;
	};

	void Refresh();
	static PCWSTR StateToString(DWORD state);
	static CString ProtectionToString(DWORD state);
	static PCWSTR TypeToString(DWORD type);
	static CString HeapFlagsToString(DWORD flags);
	bool CompareItems(WinSys::MemoryRegionItem& m1, WinSys::MemoryRegionItem& m2, int col, bool asc);
	ItemDetails GetDetails(const WinSys::MemoryRegionItem& mi) const;
	PCWSTR UsageToString(const WinSys::MemoryRegionItem& item) const;
	COLORREF UsageToBackColor(const WinSys::MemoryRegionItem& item) const;
	static CString FormatWithCommas(long long size);

	DWORD m_Pid;
	CListViewCtrl m_List, m_DetailsList;
	CHorSplitterWindow m_Splitter;
	std::vector<std::shared_ptr<WinSys::MemoryRegionItem>> m_Items;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_Threads;
	mutable std::unordered_map<PVOID, ItemDetails> m_Details;
	std::vector<HeapInfo> m_Heaps;
	HANDLE m_hProcess;
	wil::unique_handle m_hReadProcess;
	std::unique_ptr<WinSys::ProcessVMTracker> m_Tracker;
	WinSys::ProcessManager m_ProcMgr;
};

