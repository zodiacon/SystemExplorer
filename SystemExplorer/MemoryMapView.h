#pragma once

#include <ProcessVMTracker.h>
#include <ThreadInfo.h>
#include <ProcessManager.h>
#include "ViewBase.h"
#include <TreeListView.h>

struct IMainFrame;

class CMemoryMapView :
	public CViewBase<CMemoryMapView> {
public:
	DECLARE_WND_CLASS(nullptr)

	CMemoryMapView(IMainFrame* frame, DWORD pid);

	bool IsUpdating() const;

	BEGIN_MSG_MAP(CMemoryMapView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		MESSAGE_HANDLER(WTLHelper::ThemeChangedMessage, OnDarkModeChanged)
		CHAIN_MSG_MAP(CViewBase<CMemoryMapView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDarkModeChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
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
	HTREEITEM AddItem(HTREEITEM hParent, const WinSys::MemoryRegionItem& item);
	void ApplyItemColors(HTREEITEM hItem, const WinSys::MemoryRegionItem& item);
	static PCWSTR StateToString(DWORD state);
	static CString ProtectionToString(DWORD state);
	static PCWSTR TypeToString(DWORD type);
	static CString HeapFlagsToString(DWORD flags);
	ItemDetails GetDetails(const WinSys::MemoryRegionItem& mi) const;
	PCWSTR UsageToString(const WinSys::MemoryRegionItem& item) const;
	COLORREF UsageToBackColor(const WinSys::MemoryRegionItem& item) const;
	static CString FormatWithCommas(long long size);

	DWORD m_Pid;
	CTreeListViewCtrl m_Tree;
	std::vector<std::shared_ptr<WinSys::MemoryRegionItem>> m_Items;
	std::vector<HTREEITEM> m_ItemHandles;	// parallel to m_Items, filled by Refresh()
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_Threads;
	mutable std::unordered_map<PVOID, ItemDetails> m_Details;
	std::vector<HeapInfo> m_Heaps;
	HANDLE m_hProcess;
	wil::unique_handle m_hReadProcess;
	std::unique_ptr<WinSys::ProcessVMTracker> m_Tracker;
	WinSys::ProcessManager m_ProcMgr;
};
