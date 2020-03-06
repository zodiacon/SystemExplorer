// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "VirtualListView.h"
#include "Interfaces.h"

struct IObjectsView;
struct IMainFrame;

class CObjectsView : 
	public CWindowImpl<CObjectsView, CListViewCtrl>,
	public CMessageFilter,
	public CCustomDraw<CObjectsView>,
	public CVirtualListView<CObjectsView> {
public:
	DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	CObjectsView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type = nullptr);

	void Refresh();

	BOOL PreTranslateMessage(MSG* pMsg);
	void DoSort(const SortInfo* si);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CObjectsView)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivatePage)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMessage)
		CHAIN_MSG_MAP_ALT(CVirtualListView<CObjectsView>, 1)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLHANDLESFOROBJECT, OnShowAllHandles)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	std::shared_ptr<ObjectInfo>& GetItem(int index);
	bool CompareItems(const ObjectInfo& o1, const ObjectInfo& o2, const SortInfo* si);
	CString GetObjectDetails(ObjectInfo* info) const;
	CString GetProcessHandleInfo(const HandleInfo& hi) const;

private:
	LRESULT OnActivatePage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	IMainFrame* m_pFrame;
	static int ColumnCount;
	CImageListManaged m_Images;
	std::vector<std::shared_ptr<ObjectInfo>> m_Objects;
	CUpdateUIBase* m_pUpdateUI;
	IObjectsView* m_pView{ nullptr };
	ObjectManager m_ObjMgr;
	CString m_Typename;
};
