// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "IObjectsView.h"

#define WINDOW_MENU_POSITION	6

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public IMainFrame,
	public CMessageFilter, 
	public CIdleHandler {
public:
	DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	template<typename View = CWindow>
	View* GetActiveView() {
		auto n = m_view.GetActivePage();
		return n < 0 ? nullptr : static_cast<View*>(m_view.GetPageData(n));
	};

	// Inherited via IMainFrame
	BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd) override;
	HIMAGELIST GetImageList() override;
	int GetIconIndexByType(PCWSTR type) const override;

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLOBJECTS, OnViewAllObjects)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnForwardMsg)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnForwardMsg)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE, OnWindowClose)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLOBJECTTYPES, OnShowAllTypes)
		COMMAND_ID_HANDLER(ID_HANDLES_ALLHANDLES, OnShowAllHandles)
		COMMAND_ID_HANDLER(ID_HANDLES_SHOWHANDLEINPROCESS, OnShowHandlesInProcess)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
		COMMAND_RANGE_HANDLER(ID_SHOWOBJECTSOFTYPE_PROCESS, ID_SHOWOBJECTSOFTYPE_PROCESS + 17, OnShowObjectOfType)
		COMMAND_RANGE_HANDLER(ID_SHOWHANDLESOFTYPE_PROCESS, ID_SHOWHANDLESOFTYPE_PROCESS + 17, OnShowHandlesOfType)
		COMMAND_RANGE_HANDLER(ID_WINDOW_TABFIRST, ID_WINDOW_TABLAST, OnWindowActivate)
		NOTIFY_CODE_HANDLER(TBVN_PAGEACTIVATED, OnTabActivated)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnTabActivated(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewAllObjects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMsg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowActivate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowObjectOfType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHandlesOfType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllTypes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHandlesInProcess(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CTabView m_view;
	CCommandBarCtrl m_CmdBar;
	ObjectManager m_ObjMgr;
	CImageListManaged m_TabImages;
	std::unordered_map<std::wstring, int> m_IconMap;
	int m_CurrentPage = -1;
	int m_ObjectsIcon, m_TypesIcon;
};
