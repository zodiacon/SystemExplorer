// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "ObjectsView.h"
#include "MainFrm.h"
#include "ObjectsSummaryView.h"
#include "DriverHelper.h"
#include "HandlesView.h"
#include "ProcessSelectDlg.h"

const DWORD ListViewDefaultStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (m_view.PreTranslateMessage(pMsg))
		return TRUE;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return FALSE;
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnTabActivated(int, LPNMHDR hdr, BOOL&) {
	auto page = static_cast<int>(hdr->idFrom);
	HWND hWnd = nullptr;
	if (page >= 0) {
		hWnd = m_view.GetPageHWND(page);
		ATLASSERT(::IsWindow(hWnd));
		if (!m_view.IsWindow())
			return 0;
	}
	if (m_CurrentPage >= 0 && m_CurrentPage < m_view.GetPageCount())
		::SendMessage(m_view.GetPageHWND(m_CurrentPage), OM_ACTIVATE_PAGE, 0, 0);
	if(hWnd)
		::SendMessage(hWnd, OM_ACTIVATE_PAGE, 1, 0);
	m_CurrentPage = page;

	return 0;
}

LRESULT CMainFrame::OnTabContextMenu(int, LPNMHDR hdr, BOOL&) {
	CMenu menu;
	menu.LoadMenuW(IDR_CONTEXT);
	auto tab = static_cast<int>(hdr->idFrom);
	POINT pt;
	::GetCursorPos(&pt);
	auto cmd = (UINT)m_CmdBar.TrackPopupMenu(menu.GetSubMenu(1), TPM_RETURNCMD, pt.x, pt.y);
	switch (cmd) {
		case ID_WINDOW_CLOSE:
			m_view.RemovePage(tab);
			return 0;

		case ID_WINDOW_CLOSEALLBUTTHIS:
			CloseAllBut(tab);
			return 0;

	}
	return SendMessage(WM_COMMAND, cmd);
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu());
	SetMenu(nullptr);
	m_CmdBar.m_bAlphaImages = true;
	InitCommandBar();

	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, ATL_IDW_TOOLBAR);
	InitToolBar(tb);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);
	CReBarCtrl(m_hWndToolBar).LockBands(TRUE);

	CreateSimpleStatusBar();

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UIEnable(ID_OBJECTS_ALLHANDLESFOROBJECT, FALSE);
	UIEnable(ID_HANDLES_CLOSEHANDLE, FALSE);

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
		UINT id[2];
	} icons[] = {
		{ IDI_GENERIC,		L"" },
		{ IDI_PROCESS,		L"Process",			{ ID_SHOWOBJECTSOFTYPE_PROCESS		, ID_SHOWHANDLESOFTYPE_PROCESS		} },
		{ IDI_THREAD,		L"Thread",			{ ID_SHOWOBJECTSOFTYPE_THREAD		, ID_SHOWHANDLESOFTYPE_THREAD			} },
		{ IDI_JOB,			L"Job",				{ ID_SHOWOBJECTSOFTYPE_JOB			, ID_SHOWHANDLESOFTYPE_JOB			} },
		{ IDI_MUTEX,		L"Mutant",			{ ID_SHOWOBJECTSOFTYPE_MUTEX		, ID_SHOWHANDLESOFTYPE_MUTEX			} },
		{ IDI_EVENT,		L"Event",			{ ID_SHOWOBJECTSOFTYPE_EVENT		, ID_SHOWHANDLESOFTYPE_EVENT			} },
		{ IDI_SEMAPHORE,	L"Semaphore",		{ ID_SHOWOBJECTSOFTYPE_SEMAPHORE	, ID_SHOWHANDLESOFTYPE_SEMAPHORE		} },
		{ IDI_DESKTOP,		L"Desktop",			{ ID_SHOWOBJECTSOFTYPE_DESKTOP		, ID_SHOWHANDLESOFTYPE_DESKTOP			} },
		{ IDI_WINSTATION,	L"WindowStation",	{ ID_SHOWOBJECTSOFTYPE_WINDOWSTATION, ID_SHOWHANDLESOFTYPE_WINDOWSTATION } },
		{ IDI_PORT,			L"ALPC Port",		{ ID_SHOWOBJECTSOFTYPE_ALPCPORT		, ID_SHOWHANDLESOFTYPE_ALPCPORT	} },
		{ IDI_KEY,			L"Key",				{ ID_SHOWOBJECTSOFTYPE_KEY			, ID_SHOWHANDLESOFTYPE_KEY		} },
		{ IDI_DEVICE,		L"Device",			{ ID_SHOWOBJECTSOFTYPE_DEVICE		} },
		{ IDI_FILE,			L"File",			{ ID_SHOWOBJECTSOFTYPE_FILE			, ID_SHOWHANDLESOFTYPE_FILE		} },
		{ IDI_SYMLINK,		L"SymbolicLink",	{ ID_SHOWOBJECTSOFTYPE_SYMBOLICLINK , ID_SHOWHANDLESOFTYPE_SYMBOLICLINK	} },
		{ IDI_SECTION,		L"Section",			{ ID_SHOWOBJECTSOFTYPE_SECTION		, ID_SHOWHANDLESOFTYPE_SECTION	} },
		{ IDI_DIRECTORY,	L"Directory",		{ ID_SHOWOBJECTSOFTYPE_DIRECTORY	, ID_SHOWHANDLESOFTYPE_DIRECTORY		} },
		{ IDI_TIMER,		L"Timer",			{ ID_SHOWOBJECTSOFTYPE_TIMER		, ID_SHOWHANDLESOFTYPE_TIMER		} },
		{ IDI_TOKEN,		L"Token",			{ ID_SHOWOBJECTSOFTYPE_TOKEN		, ID_SHOWHANDLESOFTYPE_TOKEN	} },
	};

	m_TabImages.Create(16, 16, ILC_COLOR32 | ILC_HIGHQUALITYSCALE, 16, 8);
	int index = 0;
	for (auto& icon : icons) {
		auto hIcon = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(),
			MAKEINTRESOURCE(icon.icon), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION | LR_COLOR | LR_LOADTRANSPARENT);
		m_TabImages.AddIcon(hIcon);
		m_IconMap.insert({ icon.name, index++ });
		for(auto& id : icon.id)
			m_CmdBar.AddIcon(hIcon, id);
	}

	m_ObjectsIcon = m_TabImages.AddIcon(AtlLoadIcon(IDI_OBJECTS));
	m_TypesIcon = m_TabImages.AddIcon(AtlLoadIcon(IDI_TYPES));
	m_HandlesIcon = m_TabImages.AddIcon(AtlLoadIcon(IDI_HANDLES));

	m_view.SetImageList(m_TabImages);

	if (!DriverHelper::IsDriverLoaded()) {
		if (MessageBox(L"Kernel Driver not loaded. Most functionality will not be available. Install?", L"Object Explorer", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			WCHAR path[MAX_PATH];
			::GetModuleFileName(nullptr, path, _countof(path));
			SHELLEXECUTEINFO shi = { sizeof(shi) };
			shi.lpFile = path;
			shi.lpVerb = L"runas";
			shi.lpParameters = L"install";
			shi.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE | SEE_MASK_NOASYNC;
			if (!::ShellExecuteEx(&shi)) {
				MessageBox(L"Error running driver installer", L"Object Explorer", MB_ICONERROR);
			}
		}
	}

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
	pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
	m_view.AddPage(pView->m_hWnd, L"All Objects", m_ObjectsIcon, pView);

	return 0;
}

LRESULT CMainFrame::OnShowAllHandles(WORD, WORD, HWND, BOOL&) {
	auto pView = new CHandlesView(this, this);
	pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
	m_view.AddPage(pView->m_hWnd, L"All Handles", m_HandlesIcon, pView);

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

LRESULT CMainFrame::OnCloseAllButThis(WORD, WORD, HWND, BOOL&) {
	CloseAllBut(m_view.GetActivePage());
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

	ShowAllObjects(type);

	return 0;
}

LRESULT CMainFrame::OnShowHandlesOfType(WORD, WORD id, HWND, BOOL&) {
	CString type;
	m_CmdBar.GetMenu().GetMenuStringW(id, type, 0);
	type.Replace(L"&", L"");

	ShowAllHandles(type);

	return 0;
}

LRESULT CMainFrame::OnShowAllTypes(WORD, WORD, HWND, BOOL &) {
	auto tab = new CObjectSummaryView(this, *this);
	tab->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);

	m_view.AddPage(tab->m_hWnd, L"Types", m_TypesIcon, tab);
	return 0;
}

LRESULT CMainFrame::OnShowHandlesInProcess(WORD, WORD, HWND, BOOL&) {
	CProcessSelectDlg dlg;
	if (dlg.DoModal() == IDOK) {
		CString name;
		auto pid = dlg.GetSelectedProcess(name);
		auto pView = new CHandlesView(this, this, nullptr, pid);
		pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
		CString title;
		title.Format(L"Handles (%s: %d)", name, pid);
		m_view.AddPage(pView->m_hWnd, title, 1, pView);
	}

	return 0;
}

LRESULT CMainFrame::OnForward(WORD, WORD, HWND, BOOL& bHandled) {
	auto msg = GetCurrentMessage();
	auto hPage = m_view.GetPageHWND(m_view.GetActivePage());
	if (hPage)
		::SendMessage(hPage, msg->message, msg->wParam, msg->lParam);
	return 0;
}

LRESULT CMainFrame::OnAlwaysOnTop(WORD, WORD id, HWND, BOOL&) {
	bool onTop = GetExStyle() & WS_EX_TOPMOST;
	SetWindowPos(onTop ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	UISetCheck(id, !onTop);

	return 0;
}

void CMainFrame::CloseAllBut(int tab) {
	while (m_view.GetPageCount() > tab + 1)
		m_view.RemovePage(m_view.GetPageCount() - 1);
	while (m_view.GetPageCount() > 1)
		m_view.RemovePage(0);
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

void CMainFrame::InitCommandBar() {
	struct {
		UINT id, icon;
	} cmds[] = {
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_VIEW_PAUSE, IDI_PAUSE },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ ID_OBJECTS_ALLOBJECTS, IDI_OBJECTS },
		{ ID_OBJECTS_ALLOBJECTTYPES, IDI_TYPES },
		{ ID_HANDLES_ALLHANDLES, IDI_HANDLES },
		{ ID_HANDLES_SHOWHANDLEINPROCESS, IDI_PROCESS_VIEW },
		{ ID_HANDLES_CLOSEHANDLE, IDI_DELETE },
		{ ID_APP_ABOUT, IDI_ABOUT },
		{ ID_OBJECTS_OBJECTMANAGER, IDI_PACKAGE },

	};
	for (auto& cmd : cmds)
		m_CmdBar.AddIcon(AtlLoadIcon(cmd.icon), cmd.id);
}

void CMainFrame::InitToolBar(CToolBarCtrl& tb) {
	CImageList tbImages;
	tbImages.Create(24, 24, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
	} buttons[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ 0 },
		{ ID_VIEW_PAUSE, IDI_PAUSE },
		{ 0 },
		{ ID_OBJECTS_ALLOBJECTTYPES, IDI_TYPES },
		{ ID_OBJECTS_ALLOBJECTS, IDI_OBJECTS },
		{ ID_HANDLES_ALLHANDLES, IDI_HANDLES },
		{ ID_OBJECTS_OBJECTMANAGER, IDI_PACKAGE },
		{ 0 },
		{ ID_HANDLES_SHOWHANDLEINPROCESS, IDI_PROCESS_VIEW },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = tbImages.AddIcon(AtlLoadIconImage(b.image, 0, 24, 24));
			tb.AddButton(b.id, b.style, TBSTATE_ENABLED, image, nullptr, 0);
		}
	}
}

void CMainFrame::ShowAllHandles(PCWSTR type) {
	auto pView = new CHandlesView(this, this, type);
	pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
	m_view.AddPage(pView->m_hWnd, CString(type) + L" Handles", GetIconIndexByType(type), pView);

}

void CMainFrame::ShowAllObjects(PCWSTR type) {
	auto tab = new CObjectsView(this, this, type);
	tab->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
	m_view.AddPage(tab->m_hWnd, CString(type) + L" Objects", GetIconIndexByType(type), tab);
}
