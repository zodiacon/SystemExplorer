#pragma once

#include "VirtualListView.h"

class CObjectManagerView : 
	public CWindowImpl<CObjectManagerView> {
public:
	DECLARE_WND_CLASS(nullptr)

	BEGIN_MSG_MAP(CObjectManagerView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CTreeViewCtrlEx m_Tree;
	CListViewCtrl m_List;
	CSplitterWindow m_Splitter;
};

