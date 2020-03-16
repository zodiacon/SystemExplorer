#pragma once

#include "Interfaces.h"
#include "resource.h"
#include "VirtualListView.h"

struct ServiceInfoEx {
	ServiceInfoEx(PCWSTR name);
	const WinSys::ServiceConfiguration* GetConfiguration() const;

private:
	mutable std::unique_ptr<WinSys::ServiceConfiguration> _config;
	std::wstring _name;
};

class CServicesView :
	public CFrameWindowImpl<CServicesView, CWindow, CControlWinTraits>,
	public CVirtualListView<CServicesView>,
	public CAutoUpdateUI<CServicesView>,
	public CIdleHandler {
public:
	DECLARE_WND_CLASS(nullptr)

	using BaseFrame = CFrameWindowImpl<CServicesView, CWindow, CControlWinTraits>;

	CServicesView(IMainFrame* pFrame);

	BOOL OnIdle() override;
	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;

	BEGIN_MSG_MAP(CServicesView)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemStateChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnListRightClick)
		COMMAND_ID_HANDLER(ID_SERVICE_START, OnServiceStart)
		COMMAND_ID_HANDLER(ID_SERVICE_STOP, OnServiceStop)
		COMMAND_ID_HANDLER(ID_SERVICE_PAUSE, OnServicePause)
		COMMAND_ID_HANDLER(ID_SERVICE_CONTINUE, OnServiceContinue)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CAutoUpdateUI<CServicesView>)
		CHAIN_MSG_MAP(CVirtualListView<CServicesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListRightClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnItemStateChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnServiceStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnServiceStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnServicePause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnServiceContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	bool CompareItems(const WinSys::ServiceInfo& s1, const WinSys::ServiceInfo& s2, int col, bool asc);

	int ServiceStatusToImage(WinSys::ServiceState state);
	static PCWSTR ServiceStateToString(WinSys::ServiceState state);
	static CString ServiceStartTypeToString(const WinSys::ServiceConfiguration&);
	ServiceInfoEx& GetServiceInfoEx(const std::wstring& name);

	void InitToolBar(CToolBarCtrl& tb);
	void Refresh();
	void UpdateUI(CUpdateUIBase*);

private:
	std::vector<WinSys::ServiceInfo> m_Services;
	std::unordered_map<std::wstring, ServiceInfoEx> m_ServicesEx;

	IMainFrame* m_pFrame;
	CToolBarCtrl m_ToolBar;
	CListViewCtrl m_List;
	WinSys::ProcessManager m_ProcMgr;
	bool m_RunningElevated;
};

