#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"

class CModulesView : 
	public CFrameWindowImpl<CModulesView, CWindow, CControlWinTraits>,
	public CVirtualListView<CModulesView>,
	public CCustomDraw<CModulesView> {
public:
	CModulesView(DWORD pid, IMainFrame* frame);
	CModulesView(HANDLE hProcess, IMainFrame* frame);

	DECLARE_WND_CLASS(nullptr)

	using BaseFrame = CFrameWindowImpl<CModulesView, CWindow, CControlWinTraits>;

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;

	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CServicesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		//NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		//NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		//COMMAND_ID_HANDLER(ID_HEADER_HIDECOLUMN, OnHideColumn)
		//COMMAND_ID_HANDLER(ID_HEADER_COLUMNS, OnSelectColumns)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CModulesView>)
		CHAIN_MSG_MAP(CCustomDraw<CModulesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

	struct ModuleInfoEx {
		ModuleInfoEx(std::shared_ptr<WinSys::ModuleInfo> mi);
		DWORD64 TargetTime;
		bool IsNew : 1 { false};
		bool IsUnloaded : 1 {false};
	};

private:
	CListViewCtrl m_List;
	IMainFrame* m_pFrame;
	std::vector<std::shared_ptr<WinSys::ModuleInfo>> m_Modules;
	std::unordered_map<std::wstring, ModuleInfoEx> m_ModulesEx;
	WinSys::ProcessModuleTracker m_Tracker;
};

