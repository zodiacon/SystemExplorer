// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "VirtualListView.h"
#include "Interfaces.h"
#include "ToolBarHelper.h"

struct IMainFrame;

class CObjectsView : 
	public CFrameWindowImpl<CObjectsView, CWindow, CControlWinTraits>,
	public CMessageFilter,
	public CCustomDraw<CObjectsView>,
	public CVirtualListView<CObjectsView> {
public:
	using BaseClass = CFrameWindowImpl<CObjectsView, CWindow, CControlWinTraits>;

	DECLARE_WND_CLASS_EX(nullptr, CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, NULL);

	CObjectsView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type = nullptr);

	void Refresh();

	BOOL PreTranslateMessage(MSG* pMsg);
	void DoSort(const SortInfo* si);
	bool IsSortable(int col) const;

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CObjectsView)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivatePage)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnContextMenu)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_HANDLES_NAMEDOBJECTSONLY, OnShowNamedObjectsOnly)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLHANDLESFOROBJECT, OnShowAllHandles)
		COMMAND_ID_HANDLER(ID_EDIT_SECURITY, OnEditSecurity)
		CHAIN_MSG_MAP(BaseClass)
		CHAIN_MSG_MAP(CVirtualListView<CObjectsView>)
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
	HWND CreateToolBar();

private:
	LRESULT OnActivatePage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowNamedObjectsOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditSecurity(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CListViewCtrl m_List;
	IMainFrame* m_pFrame;
	static int ColumnCount;
	CImageListManaged m_Images;
	std::vector<std::shared_ptr<ObjectInfo>> m_Objects;
	CUpdateUIBase* m_pUpdateUI;
	ObjectManager m_ObjMgr;
	WinSys::ProcessManager m_ProcMgr;
	CString m_Typename;
	bool m_NamedObjectsOnly = false;
};
