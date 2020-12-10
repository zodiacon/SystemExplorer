#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "ObjectManager.h"
#include "ViewBase.h"
#include "resource.h"

class CHandlesView :
	public CVirtualListView<CHandlesView>,
	public CCustomDraw<CHandlesView>,
	public CViewBase<CHandlesView> {
public:

	DECLARE_WND_CLASS(nullptr)

	CHandlesView(IMainFrame* pFrame, PCWSTR type = nullptr, DWORD pid = 0);
	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;
	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row) const;
	static CString HandleAttributesToString(ULONG attributes);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	bool OnDoubleClickList(int row, int col, POINT& pt) const;

	void OnPauseResume(bool paused);
	void OnUpdate();
	void OnActivate(bool activate);

	BEGIN_MSG_MAP(CHandlesView)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_HANDLES_CLOSEHANDLE, OnCloseHandle)
		COMMAND_ID_HANDLER(ID_EDIT_SECURITY, OnEditSecurity)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLHANDLESFOROBJECT, OnShowAllHandles)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnObjectProperties)
		COMMAND_ID_HANDLER(ID_HANDLES_NAMEDOBJECTSONLY, OnShowNamedObjectsOnly)
		CHAIN_MSG_MAP(CViewBase<CHandlesView>)
		CHAIN_MSG_MAP(CVirtualListView<CHandlesView>)
		CHAIN_MSG_MAP(CCustomDraw<CHandlesView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCloseHandle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowNamedObjectsOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditSecurity(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnObjectProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void ShowObjectProperties(int row) const;
	bool CompareItems(HandleInfo& o1, HandleInfo& o2, const SortInfo* si);
	void Refresh();
	void UpdateUI();
	HWND CreateToolBar();

private:
	struct Change {
		ULONG Handle;
		DWORD64 TargetTime;
		COLORREF Color;
		bool IsNewHandle;
	};

	ObjectManager m_ObjMgr;
	CListViewCtrl m_List;
	WinSys::ProcessManager m_ProcMgr;
	std::unique_ptr<WinSys::ProcessHandlesTracker> m_HandleTracker;
	CString m_HandleType;
	CUpdateUIBase* m_pUI;
	int m_ColumnCount;
	int m_Pid;
	std::vector<std::shared_ptr<HandleInfo>> m_Handles;
	std::unordered_map<HandleInfo*, CString> m_DetailsCache;
	DWORD64 m_TargetUpdateTime = 0;
	std::vector<Change> m_Changes;
	wil::unique_handle m_hProcess;
	bool m_Paused = false;
	bool m_NamedObjectsOnly = false;
};

