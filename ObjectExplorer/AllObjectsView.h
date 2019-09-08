// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "VirtualListView.h"

class CAllObjectsView : 
	public CWindowImpl<CAllObjectsView, CListViewCtrl>,
	public CMessageFilter,
	public CVirtualListView<CAllObjectsView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CAllObjectsView(ObjectManager& om, CUpdateUIBase* pUpdateUI);

	void Refresh();

	BOOL PreTranslateMessage(MSG* pMsg);
	void DoSort(const SortInfo* si);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CAllObjectsView)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		CHAIN_MSG_MAP_ALT(CVirtualListView<CAllObjectsView>, 1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMessage)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	std::shared_ptr<ObjectInfoEx>& GetItem(int index);
	bool CompareItems(const ObjectInfoEx& o1, const ObjectInfoEx& o2, const SortInfo* si);
	CString GetObjectDetails(ObjectInfoEx* info) const;

private:
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

private:
	static int ColumnCount;
	ObjectManager& m_ObjMgr;
	std::vector<std::shared_ptr<ObjectInfoEx>> m_AllObjects;
	CUpdateUIBase* m_pUpdateUI;
};
