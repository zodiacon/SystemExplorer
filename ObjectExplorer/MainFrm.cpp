// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "ObjectsView.h"
#include "MainFrm.h"
#include "ObjectViewByType.h"
#include "ObjectsSummaryView.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnTabActivated(int, LPNMHDR hdr, BOOL&) {
	auto page = static_cast<int>(hdr->idFrom);
	auto hWnd = m_view.GetPageHWND(page);
	ATLASSERT(::IsWindow(hWnd));
	if (m_CurrentPage >= 0)
		::SendMessage(m_view.GetPageHWND(m_CurrentPage), OM_ACTIVATE_PAGE, 0, 0);
	::SendMessage(hWnd, OM_ACTIVATE_PAGE, 1, 0);
	m_CurrentPage = page;

	return 0;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu());
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	SetMenu(nullptr);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);

	CreateSimpleStatusBar();

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	CReBarCtrl(m_hWndToolBar).LockBands(TRUE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = m_CmdBar.GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	// icons
	struct {
		UINT icon;
		PCWSTR name;
	} icons[] = {
		{ IDI_GENERIC,		L"" },
		{ IDI_PROCESS,		L"Process" },
		{ IDI_THREAD,		L"Thread" },
		{ IDI_JOB,			L"Job" },
		{ IDI_MUTEX,		L"Mutant" },
		{ IDI_EVENT,		L"Event" },
		{ IDI_SEMAPHORE,	L"Semaphore" },
		{ IDI_DESKTOP,		L"Desktop" },
		{ IDI_WINSTATION,	L"WindowStation" },
		{ IDI_PORT,			L"ALPC Port" },
		{ IDI_KEY,			L"Key" },
		{ IDI_DEVICE,		L"Device" },
		{ IDI_FILE,			L"File" },
		{ IDI_SYMLINK,		L"SymbolicLink" },
		{ IDI_SECTION,		L"Section" },
		{ IDI_DIRECTORY,	L"Directory" },
		{ IDI_TIMER,		L"Timer" },
		{ IDI_TOKEN,		L"Token" },
	};

	m_TabImages.Create(16, 16, ILC_COLOR32 | ILC_HIGHQUALITYSCALE, 16, 8);
	int index = 0;
	for (auto& icon : icons) {
		m_TabImages.AddIcon(AtlLoadIcon(icon.icon));
		m_IconMap.insert({ icon.name, index++ });
	}

	m_ObjectsIcon = m_TabImages.AddIcon(AtlLoadIcon(IDI_OBJECTS));
	m_TypesIcon = m_TabImages.AddIcon(AtlLoadIcon(IDI_TYPES));

	m_view.SetImageList(m_TabImages);

	PostMessage(WM_COMMAND, ID_OBJECTS_ALLOBJECTTYPES);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewAllObjects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto pView = new CObjectsView(this, this);
	pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL, 0);
	m_view.AddPage(pView->m_hWnd, L"All Objects", m_ObjectsIcon, pView);

	return 0;
}

LRESULT CMainFrame::OnForwardMsg(WORD, WORD, HWND, BOOL& handled) {
	auto h = m_view.GetPageHWND(m_view.GetActivePage());
	if (!h) return 0;
	auto msg = GetCurrentMessage();
	::SendMessage(h, msg->message, msg->wParam, msg->lParam);

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nActivePage = m_view.GetActivePage();
	if (nActivePage != -1) {
		m_view.RemovePage(nActivePage);
	}
	else
		::MessageBeep((UINT)-1);

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_view.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_view.SetActivePage(nPage);

	return 0;
}

LRESULT CMainFrame::OnShowObjectOfType(WORD, WORD id, HWND, BOOL &) {
	CString type;
	m_CmdBar.GetMenu().GetMenuStringW(id, type, 0);
	type.Replace(L"&", L"");

	auto tab = new CObjectsView(this, this, type);
	tab->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL, 0);

	m_view.AddPage(tab->m_hWnd, type, GetIconIndexByType(type), tab);

	return LRESULT();
}

LRESULT CMainFrame::OnShowAllTypes(WORD, WORD, HWND, BOOL &) {
	auto tab = new CObjectSummaryView(this, *this);
	tab->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL, 0);

	m_view.AddPage(tab->m_hWnd, L"Types", m_TypesIcon, tab);
	return 0;
}

BOOL CMainFrame::TrackPopupMenu(HMENU hMenu, HWND hWnd) {
	POINT pt;
	::GetCursorPos(&pt);
	return m_CmdBar.TrackPopupMenu(hMenu, 0, pt.x, pt.y);
}

HIMAGELIST CMainFrame::GetImageList() {
	return m_TabImages;
}

int CMainFrame::GetIconIndexByType(PCWSTR type) const {
	auto it = m_IconMap.find(type);
	return it == m_IconMap.end() ? 0 : it->second;
}
