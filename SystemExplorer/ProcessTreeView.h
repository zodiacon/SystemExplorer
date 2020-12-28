#pragma once

#include <ProcessManager.h>
#include "ViewBase.h"

class CProcessTreeView : public CViewBase<CProcessTreeView> {
public:
	CProcessTreeView(IMainFrame* frame) : CViewBase(frame) {}

	BEGIN_MSG_MAP(CProcessTreeView)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnTreeItemDoubleClick)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CViewBase<CProcessTreeView>)
	END_MSG_MAP()

	void DoRefresh();

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeItemDoubleClick(int, LPNMHDR hdr, BOOL&);

	CTreeViewCtrlEx m_Tree;
	WinSys::ProcessManager m_ProcMgr;
};
