#pragma once

#include <ProcessManager.h>
#include "ViewBase.h"
#include "VirtualListView.h"
#include <ProcessInfo.h>

class CProcessTreeView : 
	public CViewBase<CProcessTreeView>,
	public CVirtualListView<CProcessTreeView> {
public:
	CProcessTreeView(IMainFrame* frame) : CViewBase(frame) {}

	CString GetColumnText(HWND, int row, int col);

	void OnUpdate();
	void DoRefresh();

	WinSys::ProcessInfo* GetSelectedProcess() const;

	BEGIN_MSG_MAP(CProcessTreeView)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnTreeItemDoubleClick)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeItemChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		COMMAND_RANGE_HANDLER(ID_PROCESS_MEMORYMAP, ID_PROCESS_HEAPS, OnProcessItem)
		COMMAND_ID_HANDLER(ID_PROCESS_ALLOFTHEABOVE, OnProcessItem)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProcessProperties)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CProcessTreeView>)
		CHAIN_MSG_MAP(CViewBase<CProcessTreeView>)
	END_MSG_MAP()

private:
	CString GetDetails(int row);
	CString GetName(int row);
	CString GetValue(int row);
	INT_PTR ShowProcessProperties(uint32_t pid) const;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeItemDoubleClick(int, LPNMHDR hdr, BOOL&);
	LRESULT OnTreeItemChanged(int, LPNMHDR hdr, BOOL&);
	LRESULT OnRightClick(int, LPNMHDR hdr, BOOL&);
	LRESULT OnProcessItem(WORD, WORD id, HWND, BOOL&);
	LRESULT OnProcessProperties(WORD, WORD id, HWND, BOOL&);

	CTreeViewCtrlEx m_Tree;
	CListViewCtrl m_List;
	CSplitterWindow m_Splitter;
	WinSys::ProcessManager m_ProcMgr;
	std::shared_ptr<WinSys::ProcessInfo> m_SelectedProcess;
};
