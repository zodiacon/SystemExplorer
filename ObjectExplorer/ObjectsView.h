// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "VirtualListView.h"

struct IObjectsView;
struct IMainFrame;

class CObjectsView : 
	public CWindowImpl<CObjectsView, CListViewCtrl>,
	public CMessageFilter,
	public CVirtualListView<CObjectsView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CObjectsView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type = nullptr);

	void Refresh();

	BOOL PreTranslateMessage(MSG* pMsg);
	void DoSort(const SortInfo* si);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CObjectsView)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CREATE, OnDestroy)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		CHAIN_MSG_MAP_ALT(CVirtualListView<CObjectsView>, 1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		//MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMessage)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	std::shared_ptr<ObjectInfoEx>& GetItem(int index);
	bool CompareItems(const ObjectInfoEx& o1, const ObjectInfoEx& o2, const SortInfo* si);
	CString GetObjectDetails(ObjectInfoEx* info) const;
	CString GetProcessHandleInfo(const HandleInfo& hi) const;

private:
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

private:
	IMainFrame* m_pFrame;
	std::unordered_map<std::wstring, int> _iconMap;
	static int ColumnCount;
	CImageListManaged m_Images;
	std::vector<std::shared_ptr<ObjectInfoEx>> m_Objects;
	CUpdateUIBase* m_pUpdateUI;
	IObjectsView* m_pView;
	ObjectManager m_ObjMgr;
	CString m_Typename;
};
