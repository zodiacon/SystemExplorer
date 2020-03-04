#pragma once

#include "ObjectManager.h"
#include "Interfaces.h"

class CObjectSummaryView :
	public CWindowImpl<CObjectSummaryView, CListViewCtrl>,
	public CCustomDraw<CObjectSummaryView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CObjectSummaryView(IMainFrame* pFrame, CUpdateUIBase& updateUI) : m_pFrame(pFrame), m_UIUpdate(updateUI) {}

	BOOL PreTranslateMessage(MSG* pMsg);

	static const int ColumnCount = 10;

	bool TogglePause();
	void SetInterval(int interval);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	void OnViewActivated();
	void UpdateUI();

	BEGIN_MSG_MAP(CObjectSummaryView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivatePage)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ODFINDITEM, OnFindItem)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnSelectionChanged)
		COMMAND_ID_HANDLER(ID_VIEW_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnExport)
		COMMAND_ID_HANDLER(ID_TYPE_ALLHANDLES, OnShowAllHandles)
		COMMAND_ID_HANDLER(ID_TYPE_ALLOBJECTS, OnShowAllObjects)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CObjectSummaryView>, 1)
		DEFAULT_REFLECTION_HANDLER()
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMessage)
		ALT_MSG_MAP(1)
	END_MSG_MAP()

private:
	LRESULT OnActivatePage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnFindItem(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectionChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllObjects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	std::shared_ptr<ObjectTypeInfo> GetItem(int index) const;
	static PCWSTR PoolTypeToString(PoolType type);
	bool CompareItems(const std::shared_ptr<ObjectTypeInfo>& item1, const std::shared_ptr<ObjectTypeInfo>& item2) const;
	void DoSort();
	int MapChangeToColumn(ObjectManager::ChangeType type) const;

private:
	ObjectManager m_ObjectManager;
	std::vector<std::shared_ptr<ObjectTypeInfo>> m_Items;
	CUpdateUIBase& m_UIUpdate;
	IMainFrame* m_pFrame;
	int m_Interval = 1000;
	int m_SortColumn = -1;
	bool m_SortAscending;
	bool m_Paused = false;
};

