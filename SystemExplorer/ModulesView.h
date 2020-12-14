#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ViewBase.h"

class CModulesView : 
	public CVirtualListView<CModulesView>,
	public CCustomDraw<CModulesView>,
	public CViewBase<CModulesView> {
public:
	CModulesView(DWORD pid, IMainFrame* frame);
	CModulesView(HANDLE hProcess, IMainFrame* frame);

	DECLARE_WND_CLASS(nullptr)

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row) const;
	void DoSort(const SortInfo* si);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	bool OnRightClickList(int row, int col, POINT& pt);

	void OnActivate(bool activate);

	BEGIN_MSG_MAP(CServicesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_PROCESS_GOTOFILELOCATION, OnGoToFileLocation)
		CHAIN_MSG_MAP(CVirtualListView<CModulesView>)
		CHAIN_MSG_MAP(CCustomDraw<CModulesView>)
		CHAIN_MSG_MAP(CViewBase<CModulesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGoToFileLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

	struct ModuleInfoEx {
		DWORD64 TargetTime = 0;
		bool IsNew { false };
		bool IsUnloaded { false };
	};

	ModuleInfoEx& GetModuleEx(WinSys::ModuleInfo* mi);
	static CString CharacteristicsToString(WinSys::DllCharacteristics c);

private:
	CListViewCtrl m_List;
	std::vector<std::shared_ptr<WinSys::ModuleInfo>> m_Modules;
	std::unordered_map<WinSys::ModuleInfo*, ModuleInfoEx> m_ModulesEx;
	WinSys::ProcessModuleTracker m_Tracker;
};

