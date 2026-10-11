// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <IconHelper.h>
#include "ImageHelper.h"
#include "resource.h"
#include <Psapi.h>
#include "aboutdlg.h"
#include "ObjectsView.h"
#include "MainFrm.h"
#include "ObjectsSummaryView.h"
#include "DriverHelper.h"
#include "HandlesView.h"
#include "ProcessHelper.h"
#include "ProcessSelectDlg.h"
#include "ObjectManagerView.h"
#include "SecurityHelper.h"
#include "WindowsView.h"
#include "ServicesView.h"
#include "DeviceManagerView.h"
#include "PipesMailslotsDlg.h"
#include "DetachHostWindow.h"
#include "MemoryMapView.h"
#include "LogonSessionsView.h"
#include "ModulesView.h"
#include "ProcessesView.h"
#include "ThreadsView.h"
#include "COMView.h"
#include "SearchView.h"
#include "ColorsSelectionDlg.h"
#include "SysInfoView.h"
#include "InstallServiceDlg.h"
#include "SystemModulesView.h"
#include "ProcessTreeView.h"
#include <ProcessInfo.h>
#include <Helpers.h>
#include <WTLHelper.h>

const UINT WINDOW_MENU_POSITION = 9;

CMainFrame::CMainFrame() {
	s_FrameCount++;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (m_pFindDlg && m_pFindDlg->IsDialogMessage(pMsg))
		return TRUE;

	if (m_view.PreTranslateMessage(pMsg))
		return TRUE;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return FALSE;
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	//UIUpdateStatusBar();

	return FALSE;
}

void CMainFrame::OnFinalMessage(HWND) {
	delete this;
}

void CMainFrame::SaveSettings(PCWSTR filename) {
	CString path;
	if (filename == nullptr) {
		path = GetDefaultSettingsFile();
		filename = path;
	}
	auto& settings = Settings::Get();
	settings.AlwaysOnTop = (GetExStyle() & WS_EX_TOPMOST) > 0;
	settings.Save(filename);
}

void CMainFrame::LoadSettings(PCWSTR filename) {
	CString path;
	if (filename == nullptr) {
		path = GetDefaultSettingsFile();
		filename = path;
	}
	Settings::Get().Load(filename);
}

CString CMainFrame::GetDefaultSettingsFile() {
	WCHAR path[MAX_PATH];
	::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path);
	::StringCchCat(path, _countof(path), L"\\SystemExplorer.ini");
	return path;
}

void CMainFrame::OnTrayIconSelected() {
	ShowWindow(SW_RESTORE);
	RemoveTrayIcon();
}

LRESULT CMainFrame::OnProcessMemoryMap(WORD, WORD, HWND, BOOL&) {
	auto process = (WinSys::ProcessInfo*)GetCurrentMessage()->lParam;
	DWORD pid = 0;
	CString name;
	if (process == nullptr) {
		CProcessSelectDlg dlg;
		if (dlg.DoModal() == IDOK) {
			pid = dlg.GetSelectedProcess(name);
		}
	}
	else {
		pid = process->Id;
		name = process->GetImageName().c_str();
	}
	if (pid) {
		if (!CreateAndAddMemoryMapView(name, pid)) {
			AtlMessageBox(*this, L"Process not accessible", IDS_TITLE, MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnProcessThreads(WORD, WORD, HWND, BOOL&) {
	auto process = (WinSys::ProcessInfo*)GetCurrentMessage()->lParam;
	DWORD pid = 0;
	CString name;
	if (process == nullptr) {
		CProcessSelectDlg dlg;
		if (dlg.DoModal() == IDOK) {
			pid = dlg.GetSelectedProcess(name);
		}
	}
	else {
		pid = process->Id;
		name = process->GetImageName().c_str();
	}

	if (pid) {
		CreateAndAddThreadsView(name, pid);
	}
	return 0;
}

LRESULT CMainFrame::OnProcessModules(WORD, WORD, HWND, BOOL&) {
	auto process = (WinSys::ProcessInfo*)GetCurrentMessage()->lParam;
	DWORD pid = 0;
	CString name;
	if (process == nullptr) {
		CProcessSelectDlg dlg;
		if (dlg.DoModal() == IDOK) {
			pid = dlg.GetSelectedProcess(name);
		}
	}
	else {
		pid = process->Id;
		name = process->GetImageName().c_str();
	}

	if (pid) {
		if (!CreateAndAddModulesView(name, pid)) {
			AtlMessageBox(*this, L"Process not accessible", IDS_TITLE, MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnProcessHeaps(WORD, WORD, HWND, BOOL&) {
	return ShowNotImplemented();
}

LRESULT CMainFrame::OnNewWindow(WORD, WORD, HWND, BOOL&) {
	auto frame = new CMainFrame;
	frame->CreateEx();
	frame->ShowWindow(SW_SHOW);

	return 0;
}

LRESULT CMainFrame::OnProcessColors(WORD, WORD, HWND, BOOL&) {
	// edits the colors of the current mode; each mode has its own
	auto colors = Settings::Get().GetProcessColors();
	const int count = (int)ProcessColorIndex::COUNT;
	CColorsSelectionDlg dlg(colors, count);
	dlg.SetTitle(WTLHelper::IsDarkMode() ? L"Process Colors (Dark Mode)" : L"Process Colors (Light Mode)");
	if (dlg.DoModal() == IDOK) {
		std::copy_n(dlg.GetColors(), count, colors);
		for (auto frame : s_Frames)
			frame->RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	return 0;
}

LRESULT CMainFrame::OnInstallService(WORD, WORD, HWND, BOOL&) {
	CInstallServiceDlg dlg;
	if (dlg.DoModal() == IDOK) {
		auto svc = WinSys::ServiceManager::Install(dlg.GetInstallParams());
		if (svc == nullptr)
			AtlMessageBox(*this, WinSys::Helpers::GetErrorText(::GetLastError()).c_str(), IDS_TITLE, MB_ICONERROR);
		else
			AtlMessageBox(*this, L"Installation successful", IDS_TITLE, MB_ICONINFORMATION);
	}
	return 0;
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
	if (hWnd)
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
	auto cmd = (UINT)TrackPopupMenu(menu.GetSubMenu(1), m_hWnd, &pt, TPM_RETURNCMD);
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
	if (s_FrameCount == 1) {
		LoadSettings();
		if (Settings::Get().SingleInstanceOnly && ProcessHelper::IsInstanceRunning()) {
			WCHAR className[64]{};
			::GetClassName(m_hWnd, className, _countof(className));
			if(ProcessHelper::SetExistingInstanceFocus(className))
				return -1;
		}
	}

	CMenuHandle hMenu = GetMenu();
	if (SecurityHelper::IsRunningElevated()) {
		hMenu.GetSubMenu(0).DeleteMenu(ID_FILE_RUNASADMINISTRATOR, MF_BYCOMMAND);
		CString text;
		GetWindowText(text);
		SetWindowText(text + L" (Administrator)");
	}

	s_Frames.insert(this);

	UIAddMenu(hMenu);
	UIAddMenu(IDR_CONTEXT);

	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
	tb.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	InitToolBar(tb);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);

	CReBarCtrl(m_hWndToolBar).LockBands(TRUE);

	CreateSimpleStatusBar();
	m_StatusBar.SubclassWindow(m_hWndStatusBar);
	int parts[] = { 100, 200, 300, 430, 560, 700, 830, 960, 1100 };
	m_StatusBar.SetParts(_countof(parts), parts);

	m_view.m_bDestroyImageList = false;
	// a close button shows on the tab under the mouse; clicking it removes the page (as Close does)
	m_view.m_bTabCloseButton = true;

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	UIAddToolBar(hWndToolBar);
	UIAddStatusBar(m_hWndStatusBar, _countof(parts));

	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_OPTIONS_DARKMODE, WTLHelper::IsDarkMode());
	UIEnable(ID_OBJECTS_ALLHANDLESFOROBJECT, FALSE);
	UIEnable(ID_HANDLES_CLOSEHANDLE, FALSE);
	UIEnable(ID_EDIT_FIND, FALSE);
	UIEnable(ID_EDIT_FIND_NEXT, FALSE);
	UISetCheck(ID_OPTIONS_REPLACETASKMANAGER, ProcessHelper::IsReplacingTaskManager());

	// register object for message filtering and idle updates
	auto pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));
	m_view.SetTitleBarWindow(m_hWnd);
	
	const auto& settings = Settings::Get();

	if (settings.AlwaysOnTop)
		ToggleAlwaysOnTop(ID_OPTIONS_ALWAYSONTOP);
	if (settings.SingleInstanceOnly)
		UISetCheck(ID_OPTIONS_SINGLEINSTANCEONLY, TRUE);
	if (settings.MinimizeToTray)
		UISetCheck(ID_OPTIONS_MINIMIZETOTRAY, TRUE);

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
		{ IDI_DEVICE,		L"Device",			{ 0, 0 } },
		{ IDI_FILE,			L"File",			{ ID_SHOWOBJECTSOFTYPE_FILE			, ID_SHOWHANDLESOFTYPE_FILE		} },
		{ IDI_SYMLINK,		L"SymbolicLink",	{ ID_SHOWOBJECTSOFTYPE_SYMBOLICLINK , ID_SHOWHANDLESOFTYPE_SYMBOLICLINK	} },
		{ IDI_SECTION,		L"Section",			{ ID_SHOWOBJECTSOFTYPE_SECTION		, ID_SHOWHANDLESOFTYPE_SECTION	} },
		{ IDI_DIRECTORY,	L"Directory",		{ ID_SHOWOBJECTSOFTYPE_DIRECTORY	, ID_SHOWHANDLESOFTYPE_DIRECTORY		} },
		{ IDI_TIMER,		L"Timer",			{ ID_SHOWOBJECTSOFTYPE_TIMER		, ID_SHOWHANDLESOFTYPE_TIMER		} },
		{ IDI_TOKEN,		L"Token",			{ ID_SHOWOBJECTSOFTYPE_TOKEN		, ID_SHOWHANDLESOFTYPE_TOKEN	} },
		{ IDI_CAR,			L"Driver",			{ 0, 0 } },
		{ IDI_ATOM,			L"PowerRequest",	{ ID_SHOWOBJECTSOFTYPE_POWERREQUEST, ID_SHOWHANDLESOFTYPE_POWERREQUEST	} },
		{ IDI_FACTORY,		L"TpWorkerFactory",	{ ID_SHOWOBJECTSOFTYPE_WORKERFACTORY, ID_SHOWHANDLESOFTYPE_WORKERFACTORY	} },
	};

	if (!m_TabImages) {
		m_TabImages.Create(16, 16, ILC_COLOR32 | ILC_HIGHQUALITYSCALE, 16, 8);
		int index = 0;
		for (auto& icon : icons) {
			auto hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
				MAKEINTRESOURCE(icon.icon), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION | LR_COLOR | LR_LOADTRANSPARENT);
			ImageHelper::AddIcon(m_TabImages, hIcon);
			m_IconMap.insert({ icon.name, index++ });
		}

		UINT ids[] = {
			IDI_OBJECTS, IDI_TYPES, IDI_HANDLES, IDI_PACKAGE,
			IDI_WINDOWS, IDI_SERVICES, IDI_DEVICE, IDI_DRAM,
			IDI_LOGIN, IDI_DLL, IDI_PROCESSES, IDI_COMPONENT, IDI_THREAD,
			IDI_FIND, IDI_SYSINFO, IDI_ATOM, IDI_TREE, IDI_CAR,
		};

		for (int i = 0; i < _countof(ids); i++)
			m_Icons[i] = ImageHelper::AddIcon(m_TabImages, AtlLoadIconImage(ids[i], 0, 16, 16));

		m_MonoFont.CreatePointFont(95, L"Consolas");

		index = 0;
		for (auto& icon : icons) {
			HICON hIcon = nullptr;
			for (auto& id : icon.id) {
				if (id == 0)
					continue;
				if (!hIcon)
					hIcon = m_TabImages.GetIcon(index);
				s_TypeMenuItems.push_back(MenuItemData{ (int)id, 0, hIcon });
			}
			index++;
		}
	}

	InitMenu(hMenu);
	m_view.SetImageList(m_TabImages);

	if (s_FrameCount == 1) {
		if (!DriverHelper::IsDriverLoaded()) {
			if (!SecurityHelper::IsRunningElevated()) {
				if (AtlMessageBox(nullptr, L"Kernel Driver not loaded. Some functionality will not be available. Install?",
					IDS_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES) {
					if (!SecurityHelper::RunElevated(L"install", false)) {
						AtlMessageBox(*this, L"Error running driver installer", IDS_TITLE, MB_ICONERROR);
					}
				}
			}
			else {
				auto loaded = DriverHelper::LoadDriver();
				if (!loaded) {
					loaded = DriverHelper::InstallDriver();
					if (loaded)
						loaded = DriverHelper::LoadDriver();
				}
				if(!loaded) {
					//AtlMessageBox(m_hWnd, L"Failed to install driver. Some functionality will not be available.", IDS_TITLE, MB_ICONERROR);
				}
			}
		}
		if (DriverHelper::IsDriverLoaded()) {
			if (DriverHelper::GetVersion() < DriverHelper::GetCurrentVersion()) {
				auto response = AtlMessageBox(nullptr, L"A newer driver is available with new functionality. Update?",
					IDS_TITLE, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
				if (response == IDYES) {
					if (SecurityHelper::IsRunningElevated()) {
						if (!DriverHelper::UpdateDriver())
							AtlMessageBox(nullptr, L"Failed to update driver", IDS_TITLE, MB_ICONERROR);
					}
					else {
						DriverHelper::CloseDevice();
						SecurityHelper::RunElevated(L"update", false);
					}
				}
			}
		}

		::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		//PostMessage(WM_COMMAND, ID_OBJECTS_ALLOBJECTTYPES);
		PostMessage(WM_COMMAND, ID_SYSTEM_PROCESSES);
	}
	SetTimer(1, 1000, nullptr);

	return 0;
}

#define ROUND_MEM(x) ((x + (1 << 17)) >> 18)

LRESULT CMainFrame::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		static ObjectAndHandleStats stats;
		static PERFORMANCE_INFORMATION pi = { sizeof(pi) };
		CString text;
		if (::GetPerformanceInfo(&pi, sizeof(pi))) {
			text.Format(L"Processes: %u", pi.ProcessCount);
			m_StatusBar.SetText(1, text);
			text.Format(L"Threads: %u", pi.ThreadCount);
			m_StatusBar.SetText(2, text);
			text.Format(L"Commit: %u / %u GB", ROUND_MEM(pi.CommitTotal), ROUND_MEM(pi.CommitLimit));
			m_StatusBar.SetText(3, text);
			text.Format(L"RAM Avail: %u / %u GB", ROUND_MEM(pi.PhysicalAvailable), ROUND_MEM(pi.PhysicalTotal));
			m_StatusBar.SetText(4, text);
			text.Format(L"Kernel Paged: %llu MB", pi.KernelPaged >> 8);
			m_StatusBar.SetText(5, text);
			text.Format(L"Kernel NP: %llu MB", pi.KernelNonpaged >> 8);
			m_StatusBar.SetText(6, text);
		}
		if (ObjectManager::GetStats(stats)) {
			text.Format(L"Handles: %lld", stats.TotalHandles);
			m_StatusBar.SetText(7, text);
			text.Format(L"Objects: %lld", stats.TotalObjects);
			m_StatusBar.SetText(8, text);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnSysCommand(UINT, WPARAM wp, LPARAM, BOOL& bHandled) {
	if ((wp & 0xfff0) == SC_MINIMIZE && Settings::Get().MinimizeToTray) {
		// cached: a new icon was loaded (and never destroyed) on every minimize
		AddTrayIcon(IconHelper::LoadCached(IDR_MAINFRAME, 16), L"System Explorer");
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
		return 0;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = --s_FrameCount > 0;
	s_Frames.erase(this);

	if (s_FrameCount == 0) {
		SaveSettings();
	}

	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);

	return 0;
}

LRESULT CMainFrame::OnFileExitAll(WORD, WORD, HWND, BOOL&) {
	for (auto frame : s_Frames)
		frame->PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewSystemServices(WORD, WORD, HWND, BOOL&) {
	auto pView = new CServicesView(this, true);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Services", m_Icons[(int)IconType::Services], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnViewSystemDrivers(WORD, WORD, HWND, BOOL&) {
	auto pView = new CServicesView(this, false);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Drivers", m_Icons[(int)IconType::Drivers], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnViewSystemDevices(WORD, WORD, HWND, BOOL&) {
	auto pView = new CDeviceManagerView(this);
	auto hWnd = pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	if (!hWnd)
		return 0;	// the view has freed itself (OnFinalMessage)
	m_view.AddPage(hWnd, L"Devices", m_Icons[(int)IconType::Devices], pView);

	return 0;
}

LRESULT CMainFrame::OnViewAllObjects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto pView = new CObjectsView(this);
	if (pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"All Objects", m_Icons[(int)IconType::Objects], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnShowAllHandles(WORD, WORD, HWND, BOOL&) {
	auto pView = new CHandlesView(this);
	if (pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"All Handles", m_Icons[(int)IconType::Handles], (IView*)pView);

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
	// the toolbar's band, found by its child (its ID isn't ATL_IDW_BAND_FIRST + 1, which assumed a menu band before it)
	CReBarCtrl rebar = m_hWndToolBar;
	int index = -1;
	for (int i = 0; i < (int)rebar.GetBandCount() && index < 0; i++) {
		REBARBANDINFO rbbi = { sizeof(rbbi), RBBIM_CHILD };
		if (rebar.GetBandInfo(i, &rbbi) && rbbi.hwndChild && CWindow(rbbi.hwndChild).GetDlgCtrlID() == ATL_IDW_TOOLBAR)
			index = i;
	}
	if (index < 0)
		return 0;

	// shown if it's hidden and the other way around, by this frame's band (a static flag was shared by all frames)
	REBARBANDINFO rbbi = { sizeof(rbbi), RBBIM_STYLE };
	rebar.GetBandInfo(index, &rbbi);
	BOOL visible = (rbbi.fStyle & RBBS_HIDDEN) != 0;
	rebar.ShowBand(index, visible);
	UISetCheck(ID_VIEW_TOOLBAR, visible);
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

LRESULT CMainFrame::OnShowObjectOfType(WORD, WORD id, HWND, BOOL&) {
	CString type;
	CMenuHandle(GetMenu()).GetMenuStringW(id, type, 0);
	type.Replace(L"&", L"");

	ShowAllObjects(type);

	return 0;
}

LRESULT CMainFrame::OnShowHandlesOfType(WORD, WORD id, HWND, BOOL&) {
	CString type;
	CMenuHandle(GetMenu()).GetMenuStringW(id, type, 0);
	type.Replace(L"&", L"");

	ShowAllHandles(type);

	return 0;
}

LRESULT CMainFrame::OnShowAllTypes(WORD, WORD, HWND, BOOL&) {
	auto tab = new CObjectSummaryView(this);
	if (tab->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(tab->m_hWnd, L"Object Types", m_Icons[(int)IconType::Types], (IView*)tab);
	return 0;
}

LRESULT CMainFrame::OnShowHandlesInProcess(WORD, WORD, HWND, BOOL&) {
	auto process = (WinSys::ProcessInfo*)GetCurrentMessage()->lParam;
	DWORD pid = 0;
	CString name;
	if (process == nullptr) {
		CProcessSelectDlg dlg;
		if (dlg.DoModal() == IDOK) {
			pid = dlg.GetSelectedProcess(name);
		}
	}
	else {
		pid = process->Id;
		name = process->GetImageName().c_str();
	}
	if (pid) {
		CreateAndAddHandlesView(name, pid);
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
	ToggleAlwaysOnTop(id);
	return 0;
}

LRESULT CMainFrame::OnShowObjectManager(WORD, WORD, HWND, BOOL&) {
	auto view = new CObjectManagerView(this);
	if (view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(view->m_hWnd, L"Object Manager", m_Icons[(int)IconType::ObjectManager], (IView*)view);
	return 0;
}

LRESULT CMainFrame::OnShowAllWindows(WORD, WORD, HWND, BOOL&) {
	auto view = new CWindowsView(this);
	view->SetDesktopOptions(false);
	if (view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(view->m_hWnd, L"All Windows", m_Icons[(int)IconType::Windows], (IView*)view);
	return 0;
}

LRESULT CMainFrame::OnShowAllWindowsDefaultDesktop(WORD, WORD, HWND, BOOL&) {
	auto view = new CWindowsView(this);
	if (view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(view->m_hWnd, L"Windows (Default Desktop)", m_Icons[(int)IconType::Windows], (IView*)view);
	return 0;
}

LRESULT CMainFrame::OnRunAsAdmin(WORD, WORD, HWND, BOOL&) {
	if (SecurityHelper::RunElevated(nullptr, true)) {
		SendMessage(WM_CLOSE);
	}

	return 0;
}

LRESULT CMainFrame::OnBandRightClick(int, LPNMHDR, BOOL&) {
	auto count = m_view.GetPageCount();
	if (count == 0)
		return 0;

	return 0;
}

LRESULT CMainFrame::OnShowAllPipes(WORD, WORD, HWND, BOOL&) {
	CPipesMailslotsDlg dlg(CPipesMailslotsDlg::Type::Pipes);
	dlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnShowAllMailslots(WORD, WORD, HWND, BOOL&) {
	CPipesMailslotsDlg dlg(CPipesMailslotsDlg::Type::Mailslots);
	dlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnDetachTab(WORD, WORD, HWND, BOOL&) {
	auto index = m_view.GetActivePage();
	if (index < 0)
		return 0;

	DetachTab(index);

	return 0;
}

LRESULT CMainFrame::OnViewLogonSessions(WORD, WORD, HWND, BOOL&) {
	auto pView = new CLogonSessionsView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Logon Sessions", m_Icons[(int)IconType::Login], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnViewSystemProcesses(WORD, WORD, HWND, BOOL&) {
	auto pView = new CProcessesView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Processes", m_Icons[(int)IconType::Processes], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnViewSystemThreads(WORD, WORD, HWND, BOOL&) {
	auto pView = new CThreadsView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Threads", m_Icons[(int)IconType::Threads], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnProcessAll(WORD, WORD, HWND, BOOL&) {
	auto process = (WinSys::ProcessInfo*)GetCurrentMessage()->lParam;
	DWORD pid = 0;
	CString name;
	if (process == nullptr) {
		CProcessSelectDlg dlg;
		if (dlg.DoModal() == IDOK) {
			pid = dlg.GetSelectedProcess(name);
		}
	}
	else {
		pid = process->Id;
		name = process->GetImageName().c_str();
	}
	if (pid) {
		// create a new frame
		auto frame = new CMainFrame;
		frame->CreateEx();
		frame->CreateAndAddThreadsView(name, pid);
		frame->CreateAndAddModulesView(name, pid);
		frame->CreateAndAddHandlesView(name, pid);
		frame->CreateAndAddMemoryMapView(name, pid);
		frame->ShowWindow(SW_SHOWDEFAULT);
	}
	return 0;
}

LRESULT CMainFrame::OnViewCom(WORD, WORD, HWND, BOOL&) {
	auto pView = new CComView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"COM", m_Icons[(int)IconType::COM], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnSystemSearch(WORD, WORD, HWND, BOOL&) {
	auto pView = new CSearchView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Search", m_Icons[(int)IconType::Search], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::OnReplaceTaskManager(WORD, WORD, HWND, BOOL&) {
	bool replacing = ProcessHelper::IsReplacingTaskManager();
	if (!ProcessHelper::ReplaceTaskManager(replacing)) {
		AtlMessageBox(*this, L"Failed to replace Task Manager", IDS_TITLE, MB_ICONWARNING);
	}
	else {
		UISetCheck(ID_OPTIONS_REPLACETASKMANAGER, !replacing);
	}
	return 0;
}

LRESULT CMainFrame::OnSingleInstance(WORD, WORD, HWND, BOOL&) {
	auto& s = Settings::Get();
	s.SingleInstanceOnly = !s.SingleInstanceOnly;	// it was assigned to itself, so it never changed
	UISetCheck(ID_OPTIONS_SINGLEINSTANCEONLY, s.SingleInstanceOnly);

	return 0;
}

LRESULT CMainFrame::OnMinimizeToTray(WORD, WORD, HWND, BOOL&) {
	auto& s = Settings::Get();
	s.MinimizeToTray = !s.MinimizeToTray;
	UISetCheck(ID_OPTIONS_MINIMIZETOTRAY, s.MinimizeToTray);

	return 0;
}

LRESULT CMainFrame::OnToggleDarkMode(WORD, WORD, HWND, BOOL&) {
	WTLHelper::SwitchToMode(WTLHelper::IsDarkMode() ? DarkModeKind::Classic : DarkModeKind::Dark, m_hWnd);
	Settings::Get().DarkMode = WTLHelper::IsDarkMode() ? 1 : 0;
	for (auto frame : s_Frames) {
		if (frame != this) {
			// SwitchToMode re-themes only the window it's given; the mode itself is global
			DarkMode::setDarkTitleBarEx(*frame, true);
			DarkMode::setChildCtrlsTheme(*frame);
			frame->SendMessageToDescendants(WTLHelper::ThemeChangedMessage, 0, static_cast<LPARAM>(WTLHelper::DarkModeType()));
			frame->RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_FRAME);
		}
		frame->OnModeSwitched();
	}
	return 0;
}

void CMainFrame::OnModeSwitched() {
	// the menu icons are bitmaps drawn on the menu's background
	InitMenu(GetMenu());
	DrawMenuBar();
	UISetCheck(ID_OPTIONS_DARKMODE, WTLHelper::IsDarkMode());
}

LRESULT CMainFrame::OnViewSystemInformation(WORD, WORD, HWND, BOOL&) {
	auto pView = new CSysInfoView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"System Information", m_Icons[(int)IconType::SystemInfo], (IView*)pView);
	return 0;
}

LRESULT CMainFrame::OnViewDrivers(WORD, WORD, HWND, BOOL&) {
	return ShowNotImplemented();
}

void CMainFrame::ToggleAlwaysOnTop(UINT id) {
	bool onTop = GetExStyle() & WS_EX_TOPMOST;
	SetWindowPos(onTop ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	UISetCheck(id, !onTop);
}

void CMainFrame::CloseAllBut(int tab) {
	while (m_view.GetPageCount() > tab + 1)
		m_view.RemovePage(m_view.GetPageCount() - 1);
	while (m_view.GetPageCount() > 1)
		m_view.RemovePage(0);
}

BOOL CMainFrame::TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt, UINT flags) {
	POINT pos;
	if (pt)
		pos = *pt;
	else
		::GetCursorPos(&pos);
	InitMenu(hMenu);
	return ::TrackPopupMenu(hMenu, flags, pos.x, pos.y, 0, m_hWnd, nullptr);
}

HIMAGELIST CMainFrame::GetImageList() {
	return m_TabImages;
}

int CMainFrame::GetIconIndexByType(PCWSTR type) const {
	auto it = m_IconMap.find(type);
	return it == m_IconMap.end() ? 0 : it->second;
}

void CMainFrame::InitMenu(HMENU hMenu) {
	static HICON hShieldIcon = SecurityHelper::GetShieldIcon();
	MenuItemData cmds[] = {
		{ ID_PROCESS_COLORS, IDI_COLORWHEEL },
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_OPTIONS_ALWAYSONTOP, IDI_PIN },
		{ ID_VIEW_PAUSE, IDI_PAUSE },
		{ ID_FILE_SAVE, IDI_FILE },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ ID_OBJECTS_ALLOBJECTS, IDI_OBJECTS },
		{ ID_OBJECTS_ALLOBJECTTYPES, IDI_TYPES },
		{ ID_HANDLES_ALLHANDLES, IDI_HANDLES },
		{ ID_HANDLES_SHOWHANDLEINPROCESS, IDI_HANDLES },
		{ ID_PROCESS_HANDLES, IDI_HANDLES },
		{ ID_HANDLES_CLOSEHANDLE, IDI_DELETE },
		{ ID_OBJECTS_OBJECTMANAGER, IDI_PACKAGE },
		{ ID_GUI_ALLWINDOWSINDEFAULTDESKTOP, IDI_WINDOWS },
		{ ID_GUI_GDIOBJECTSINPROCESS, IDI_BRUSH },
		{ ID_EDIT_FIND, IDI_FIND },
		{ ID_SYSTEM_SEARCH, IDI_FIND },
		{ ID_EDIT_FIND_NEXT, IDI_FIND_NEXT },
		{ ID_WINDOW_CLOSE, IDI_WINDOW_CLOSE },
		{ ID_SYSTEM_SERVICES, IDI_SERVICES },
		{ ID_SYSTEM_DRIVERS, IDI_CAR},
		{ ID_SYSTEM_INFORMATION, IDI_SYSINFO },
		{ ID_FILE_RUNASADMINISTRATOR, 0, hShieldIcon },
		{ ID_SERVICE_START, IDI_PLAY },
		{ ID_SERVICE_STOP, IDI_STOP },
		{ ID_SERVICE_PAUSE, IDI_PAUSE },
		{ ID_SERVICE_CONTINUE, IDI_RESUME },
		{ ID_EDIT_SECURITY, IDI_SHIELD2 },
		{ ID_SYSTEM_DEVICES, IDI_DEVICE },
		{ ID_HANDLES_PIPES, IDI_PIPE },
		{ ID_OBJECTS_MAILSLOTS, IDI_MAILBOX },
		{ ID_TAB_NEWWINDOW, IDI_WINDOW_NEW },
		{ ID_PROCESS_MEMORYMAP, IDI_DRAM },
		{ ID_SYSTEM_PROCESSES, IDI_PROCESSES },
		{ ID_SYSTEM_KERNELMODULES, IDI_ATOM },
		{ ID_SYSTEM_THREADS, IDI_THREAD },
		{ ID_SYSTEM_LOGONSESSIONS, IDI_LOGIN },
		{ ID_HEADER_HIDECOLUMN, IDI_HIDECOLUMN },
		{ ID_HEADER_COLUMNS, IDI_EDITCOLUMNS },
		{ ID_PROCESS_MODULES, IDI_DLL },
		{ ID_PROCESS_THREADS, IDI_THREAD },
		{ ID_PROCESS_KILL, IDI_DELETE },
		{ ID_PROCESS_HEAPS, IDI_HEAP },
		{ ID_SYSTEM_COM, IDI_COMPONENT },
		{ ID_SYSTEM_PROCESSTREE, IDI_TREE },
		{ ID_EDIT_PROPERTIES, IDI_PROPERTIES },
	};
	WTLHelper::InitMenu(hMenu, cmds, _countof(cmds));
	if (!s_TypeMenuItems.empty())
		WTLHelper::InitMenu(hMenu, s_TypeMenuItems.data(), (int)s_TypeMenuItems.size());
}

#define ID_OBJECTS_OFTYPE 200
#define ID_HANDLES_OFTYPE 201

void CMainFrame::InitToolBar(CToolBarCtrl& tb) {
	CImageList tbImages;
	tbImages.Create(24, 24, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);
	m_ToolBarImages.Attach(tbImages);	// destroyed with the frame (a frame per window, each with its own toolbar)

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
		PCWSTR text = nullptr;
	} buttons[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ 0 },
		{ ID_VIEW_PAUSE, IDI_PAUSE },
		{ 0 },
		{ ID_SYSTEM_SEARCH, IDI_FIND },
		{ 0 },
		{ ID_OBJECTS_ALLOBJECTTYPES, IDI_TYPES },
		{ ID_OBJECTS_ALLOBJECTS, IDI_OBJECTS },
		{ ID_HANDLES_ALLHANDLES, IDI_HANDLES },
		{ ID_OBJECTS_OBJECTMANAGER, IDI_PACKAGE },
		{ 0 },
		{ ID_SYSTEM_PROCESSES, IDI_PROCESSES },
		{ ID_SYSTEM_THREADS, IDI_THREAD },
		{ ID_SYSTEM_SERVICES, IDI_SERVICES },
		{ 0 },
		{ ID_SYSTEM_DEVICES, IDI_DEVICE },
		{ 0 },
		{ ID_OBJECTS_OFTYPE, IDI_OBJECTS, BTNS_SHOWTEXT, L"Objects" },
		{ 0 },
		{ ID_HANDLES_OFTYPE, IDI_HANDLES, BTNS_SHOWTEXT, L"Handles" },
		{ 0 },
		{ ID_GUI_ALLWINDOWSINDEFAULTDESKTOP, IDI_WINDOWS },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = ImageHelper::AddIcon(tbImages, AtlLoadIconImage(b.image, 0, 24, 24));
			tb.AddButton(b.id, b.style, TBSTATE_ENABLED, image, b.text, 0);
		}
	}
	AddToolBarDropDownMenu(tb, ID_HANDLES_OFTYPE, IDR_CONTEXT, 4);
	AddToolBarDropDownMenu(tb, ID_OBJECTS_OFTYPE, IDR_CONTEXT, 5);
	//AddToolBarDropDownMenu(tb, 100, IDR_CONTEXT, 10);
}

bool CMainFrame::DetachTab(int page) {
	auto frame = new CMainFrame;
	auto hFrame = frame->CreateEx();
	if (!hFrame) {
		AtlMessageBox(*this, L"Failed to create new frame", IDS_TITLE, MB_ICONERROR);
		return false;
	}

	auto hWnd = m_view.GetPageHWND(page);
	ATLASSERT(hWnd);
	CString title(m_view.GetPageTitle(page));
	int icon = m_view.GetPageImage(page);
	auto param = m_view.GetPageData(page);
	HICON hIcon = m_TabImages.GetIcon(m_view.GetPageImage(page));

	m_view.m_bDestroyPageOnRemove = false;
	m_view.RemovePage(page);
	m_view.m_bDestroyPageOnRemove = true;

	::SendMessage(hWnd, OM_NEW_FRAME, 0, reinterpret_cast<LPARAM>(static_cast<IMainFrame*>(frame)));
	frame->m_view.AddPage(hWnd, title, icon, param);
	::SetParent(hWnd, frame->m_view);
	frame->ShowWindow(SW_SHOW);
	ATLASSERT(::IsWindow(hWnd));

	return true;
}

LRESULT CMainFrame::ShowNotImplemented() {
	AtlMessageBox(*this, L"Not implemented yet :)", IDS_TITLE, MB_ICONINFORMATION);
	return 0;
}

void CMainFrame::ShowAllHandles(PCWSTR type) {
	auto pView = new CHandlesView(this, type);
	if (pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, CString(type) + L" Handles", GetIconIndexByType(type), (IView*)pView);

}

void CMainFrame::ShowAllObjects(PCWSTR type) {
	auto tab = new CObjectsView(this, type);
	if (tab->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(tab->m_hWnd, CString(type) + L" Objects", GetIconIndexByType(type), (IView*)tab);
}

CUpdateUIBase* CMainFrame::GetUpdateUI() {
	return this;
}

CFont& CMainFrame::GetMonoFont() {
	return m_MonoFont;
}

LRESULT CMainFrame::SendFrameMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	return SendMessage(msg, wParam, lParam);
}

void CMainFrame::CloseView(HWND hWnd) {
	auto count = m_view.GetPageCount();
	for (int i = 0; i < count; i++) {
		if (m_view.GetPageHWND(i) == hWnd) {
			m_view.RemovePage(i);
			break;
		}
	}
}

HWND CMainFrame::CreateAndAddThreadsView(const CString& name, DWORD pid) {
	auto view = new CThreadsView(this, pid);
	auto hWnd = view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	if (!hWnd)
		return nullptr;		// the view has deleted itself (OnFinalMessage)
	CString title;
	title.Format(L"Threads (%s: %u)", (PCWSTR)name, pid);
	m_view.AddPage(hWnd, title, m_Icons[(int)IconType::Threads], view);

	return hWnd;
}

HWND CMainFrame::CreateAndAddModulesView(const CString& name, DWORD pid) {
	auto hProcess = DriverHelper::OpenProcess(pid, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE);
	auto view = hProcess ? new CModulesView(hProcess, this) : new CModulesView(pid, this);
	auto hWnd = view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	// if Create failed, the view has freed itself (OnFinalMessage)
	if (hWnd) {
		CString title;
		title.Format(L"Modules (%s: %u)", (PCWSTR)name, pid);
		m_view.AddPage(*view, title, m_Icons[(int)IconType::Modules], (IView*)view);
	}
	return hWnd;
}

HWND CMainFrame::CreateAndAddMemoryMapView(const CString& name, DWORD pid) {
	auto view = new CMemoryMapView(this, pid);
	auto hWnd = view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	// if Create failed, the view has freed itself (OnFinalMessage)
	if (hWnd) {
		CString title;
		title.Format(L"Memory (%s: %u)", (PCWSTR)name, pid);
		m_view.AddPage(*view, title, m_Icons[(int)IconType::Memory], (IView*)view);
	}
	return hWnd;
}

HWND CMainFrame::CreateAndAddHandlesView(const CString& name, DWORD pid) {
	auto pView = new CHandlesView(this, nullptr, pid);
	auto hWnd = pView->Create(m_view, rcDefault, nullptr, ListViewDefaultStyle, 0);
	if (!hWnd)
		return nullptr;	// the view has deleted itself (OnFinalMessage)
	CString title;
	title.Format(L"Handles (%s: %d)", (PCWSTR)name, pid);
	m_view.AddPage(hWnd, title, m_Icons[(int)IconType::Handles], pView);
	return hWnd;
}

LRESULT CMainFrame::OnFindReplaceMessage(UINT /*uMsg*/, WPARAM id, LPARAM lParam, BOOL& handled) {
	// the dialog closing must be seen even when no tab is open, or m_pFindDlg is left pointing to a deleted dialog
	auto fr = reinterpret_cast<FINDREPLACE*>(lParam);
	if (fr->Flags & FR_DIALOGTERM) {
		m_pFindDlg = nullptr;
		return 0;
	}

	auto page = m_view.GetActivePage();
	if (page < 0)
		return 0;
	m_FindText = fr->lpstrFindWhat;
	m_FindFlags = fr->Flags;

	auto view = (IView*)m_view.GetPageData(m_view.GetActivePage());
	if (view)
		view->DoFind(m_FindText, fr->Flags);

	return 0;
}

LRESULT CMainFrame::OnEditFind(WORD, WORD, HWND, BOOL&) {
	auto page = m_view.GetActivePage();
	if (page < 0)
		return 0;

	auto view = (IView*)m_view.GetPageData(m_view.GetActivePage());
	if (view && view->IsFindSupported()) {
		if (!m_pFindDlg) {
			m_pFindDlg = new CFindReplaceDialog;
			m_pFindDlg->Create(TRUE, m_FindText, nullptr, FR_DOWN, m_hWnd);
		}
		if (!m_pFindDlg->IsWindowVisible()) {
			m_pFindDlg->ShowWindow(SW_SHOW);
		}
		m_pFindDlg->BringWindowToTop();
		m_pFindDlg->SetFocus();
	}
	return 0;
}

LRESULT CMainFrame::OnEditFindNext(WORD, WORD, HWND, BOOL&) {
	auto page = m_view.GetActivePage();
	if (page < 0)
		return 0;

	auto view = (IView*)m_view.GetPageData(m_view.GetActivePage());
	if (view && view->IsFindSupported()) {
		view->DoFind(m_FindText, m_FindFlags);
	}
	return 0;
}

LRESULT CMainFrame::OnViewKernelModules(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// Windows hides kernel module addresses from non-elevated callers, so the view would be empty
	if (!SecurityHelper::IsRunningElevated()) {
		AtlMessageBox(*this, L"Kernel modules can only be listed when System Explorer runs elevated.\n"
			L"Use File > Run as Administrator.", IDS_TITLE, MB_ICONINFORMATION);
		return 0;
	}

	auto pView = new CSystemModulesView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Kernel Modules", m_Icons[(int)IconType::Kernel], (IView*)pView);

	return 0;
}

LRESULT CMainFrame::SendMessageToAllFrames(bool excludeCurrent, UINT msg, WPARAM wParam, LPARAM lParam) {
	return 0;
}

LRESULT CMainFrame::OnViewProcessTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto pView = new CProcessTreeView(this);
	if (pView->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0))	// else the view has deleted itself (OnFinalMessage)
		m_view.AddPage(pView->m_hWnd, L"Process Tree", m_Icons[(int)IconType::Tree], (IView*)pView);

	return 0;
}

