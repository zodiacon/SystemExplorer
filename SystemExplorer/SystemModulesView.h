#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ViewBase.h"
#include "resource.h"
#include "KernelModuleTracker.h"

class CSystemModulesView :
	public CVirtualListView<CSystemModulesView>,
	public CCustomDraw<CSystemModulesView>,
	public CViewBase<CSystemModulesView> {
public:
	DECLARE_WND_CLASS(nullptr);

	CSystemModulesView(IMainFrame* pFrame) : CViewBase(pFrame) {}

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row) const;
	void DoSort(const SortInfo* si);
	bool IsUpdating() const {
		return false;
	}

	void DoRefresh();

	BEGIN_MSG_MAP(CSystemModulesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CViewBase<CSystemModulesView>)
		CHAIN_MSG_MAP(CVirtualListView<CSystemModulesView>)
		CHAIN_MSG_MAP(CCustomDraw<CSystemModulesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	std::vector<std::shared_ptr<WinSys::KernelModuleInfo>> m_Modules;
	WinSys::KernelModuleTracker m_Tracker;
};

