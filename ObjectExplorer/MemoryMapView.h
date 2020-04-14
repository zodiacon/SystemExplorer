#pragma once

#include "VirtualListView.h"

class CMemoryMapView :
	public CFrameWindowImpl<CMemoryMapView, CWindow, CControlWinTraits>,
	public CVirtualListView<CMemoryMapView>,
	public CCustomDraw<CMemoryMapView> {
public:
	using BaseFrame = CFrameWindowImpl<CMemoryMapView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS(nullptr)

	CMemoryMapView(DWORD pid);

	CString GetColumnText(HWND h, int row, int column) const;
	int GetRowImage(int row) const;
	int GetRowIndent(int row) const;

	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CMemoryMapView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CMemoryMapView>)
		CHAIN_MSG_MAP(CCustomDraw<CMemoryMapView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	struct MemoryItem : MEMORY_BASIC_INFORMATION {
		mutable CString Details;
		mutable bool AttemptDetails : 1 = false;
	};

	void Refresh();
	static PCWSTR StateToString(DWORD state);
	static CString ProtectionToString(DWORD state);
	static PCWSTR TypeToString(DWORD type);
	bool CompareItems(MemoryItem& m1, MemoryItem& m2, int col, bool asc);
	const CString& GetDetails(const MemoryItem& mi) const;

private:
	DWORD m_Pid;
	CListViewCtrl m_List, m_DetailsList;
	CHorSplitterWindow m_Splitter;
	std::vector<MemoryItem> m_Items;
	wil::unique_handle m_hProcess;
	BOOL m_Is32Bit;
};

