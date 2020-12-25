// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectManager.h"
#include "Interfaces.h"
#include "ToolBarHelper.h"
#include "Settings.h"
#include "NotifyIcon.h"
#include <unordered_set>

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CAutoUpdateUI<CMainFrame>,
	public CToolBarHelper<CMainFrame>,
	public CNotifyIcon<CMainFrame>,
	public IMainFrame,
	public CMessageFilter, 
	public CIdleHandler {
public:
	DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

	CMainFrame();
	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnIdle() override;
	void OnFinalMessage(HWND) override;
	void SaveSettings(PCWSTR filename = nullptr);
	void LoadSettings(PCWSTR filename = nullptr);
	void OnTrayIconSelected();

	CCommandBarCtrl m_CmdBar;

	// Inherited via IMainFrame
	BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT*, UINT) override;
	HIMAGELIST GetImageList() override;
	int GetIconIndexByType(PCWSTR type) const override;
	void ShowAllHandles(PCWSTR type) override;
	void ShowAllObjects(PCWSTR type) override;
	CUpdateUIBase* GetUpdateUI() override;
	CFont& GetMonoFont() override;
	LRESULT SendFrameMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void CloseView(HWND hWnd) override;

	HWND CreateAndAddThreadsView(const CString& name, DWORD pid);
	HWND CreateAndAddModulesView(const CString& name, DWORD pid);
	HWND CreateAndAddMemoryMapView(const CString& name, DWORD pid);
	HWND CreateAndAddHandlesView(const CString& name, DWORD pid);

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE, OnWindowClose)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLOBJECTTYPES, OnShowAllTypes)
		COMMAND_ID_HANDLER(ID_OBJECTS_ALLOBJECTS, OnViewAllObjects)
		COMMAND_ID_HANDLER(ID_HANDLES_PIPES, OnShowAllPipes)
		COMMAND_ID_HANDLER(ID_OBJECTS_MAILSLOTS, OnShowAllMailslots)
		COMMAND_ID_HANDLER(ID_HANDLES_ALLHANDLES, OnShowAllHandles)
		COMMAND_ID_HANDLER(ID_HANDLES_SHOWHANDLEINPROCESS, OnShowHandlesInProcess)
		COMMAND_ID_HANDLER(ID_SYSTEM_SERVICES, OnViewSystemServices)
		COMMAND_ID_HANDLER(ID_SYSTEM_DEVICES, OnViewSystemDevices)
		COMMAND_ID_HANDLER(ID_SYSTEM_PROCESSES, OnViewSystemProcesses)
		COMMAND_ID_HANDLER(ID_SYSTEM_THREADS, OnViewSystemThreads)
		COMMAND_ID_HANDLER(ID_SYSTEM_LOGONSESSIONS, OnViewLogonSessions)
		COMMAND_ID_HANDLER(ID_SYSTEM_INFORMATION, OnViewSystemInformation)
		COMMAND_ID_HANDLER(ID_SYSTEM_KERNELMODULES, OnViewKernelModules)
		COMMAND_ID_HANDLER(ID_SYSTEM_COM, OnViewCom)
		COMMAND_ID_HANDLER(ID_SYSTEM_DRIVERS, OnViewDrivers)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
		COMMAND_ID_HANDLER(ID_SYSTEM_INSTALLSERVICE, OnInstallService)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSEALLBUTTHIS, OnCloseAllButThis)
		COMMAND_ID_HANDLER(ID_OPTIONS_ALWAYSONTOP, OnAlwaysOnTop)
		COMMAND_ID_HANDLER(ID_OBJECTS_OBJECTMANAGER, OnShowObjectManager)
		COMMAND_ID_HANDLER(ID_DESKTOPS_ALLWINDOWS, OnShowAllWindows)
		COMMAND_ID_HANDLER(ID_FILE_RUNASADMINISTRATOR, OnRunAsAdmin)
		COMMAND_ID_HANDLER(ID_TAB_NEWWINDOW, OnNewWindow)
		COMMAND_ID_HANDLER(ID_TAB_DETACH, OnDetachTab)

		MESSAGE_HANDLER(CFindReplaceDialog::GetFindReplaceMsg(), OnFindReplaceMessage)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnEditFind)
		COMMAND_ID_HANDLER(ID_EDIT_FIND_NEXT, OnEditFindNext)

		COMMAND_ID_HANDLER(ID_PROCESS_MEMORYMAP, OnProcessMemoryMap)
		COMMAND_ID_HANDLER(ID_PROCESS_MODULES, OnProcessModules)
		COMMAND_ID_HANDLER(ID_PROCESS_THREADS, OnProcessThreads)
		COMMAND_ID_HANDLER(ID_PROCESS_HEAPS, OnProcessHeaps)
		COMMAND_ID_HANDLER(ID_PROCESS_HANDLES, OnShowHandlesInProcess)
		COMMAND_ID_HANDLER(ID_PROCESS_ALLOFTHEABOVE, OnProcessAll)
		COMMAND_ID_HANDLER(ID_SYSTEM_SEARCH, OnSystemSearch)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)

		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_EXITALL, OnFileExitAll)
		COMMAND_ID_HANDLER(ID_PROCESS_COLORS, OnProcessColors)
		COMMAND_ID_HANDLER(ID_OPTIONS_REPLACETASKMANAGER, OnReplaceTaskManager)
		COMMAND_ID_HANDLER(ID_OPTIONS_SINGLEINSTANCEONLY, OnSingleInstance)
		COMMAND_ID_HANDLER(ID_OPTIONS_MINIMIZETOTRAY, OnMinimizeToTray)

		COMMAND_ID_HANDLER(ID_GUI_ALLWINDOWSINDEFAULTDESKTOP, OnShowAllWindowsDefaultDesktop)
		COMMAND_RANGE_HANDLER(ID_WINDOW_TABFIRST, ID_WINDOW_TABLAST, OnWindowActivate)
		COMMAND_RANGE_HANDLER(ID_SHOWOBJECTSOFTYPE_PROCESS, ID_SHOWOBJECTSOFTYPE_PROCESS + 17, OnShowObjectOfType)
		COMMAND_RANGE_HANDLER(ID_SHOWHANDLESOFTYPE_PROCESS, ID_SHOWHANDLESOFTYPE_PROCESS + 17, OnShowHandlesOfType)
		NOTIFY_CODE_HANDLER(TBVN_PAGEACTIVATED, OnTabActivated)
		NOTIFY_CODE_HANDLER(TBVN_CONTEXTMENU, OnTabContextMenu)
		NOTIFY_HANDLER(ATL_IDW_TOOLBAR, NM_RCLICK, OnBandRightClick)
		CHAIN_MSG_MAP(CAutoUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		CHAIN_MSG_MAP(CToolBarHelper<CMainFrame>)
		CHAIN_MSG_MAP(CNotifyIcon<CMainFrame>)
		COMMAND_RANGE_HANDLER(32768, 65535, OnForward)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnProcessMemoryMap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessThreads(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessModules(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessHeaps(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInstallService(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTabActivated(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnTabContextMenu(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileExitAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSystemServices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSystemDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewAllObjects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllHandles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMsg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCloseAllButThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowActivate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowObjectOfType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHandlesOfType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllTypes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHandlesInProcess(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnForward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAlwaysOnTop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowObjectManager(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllWindowsDefaultDesktop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRunAsAdmin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBandRightClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllPipes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllMailslots(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDetachTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewLogonSessions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSystemProcesses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSystemThreads(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewCom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSystemSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReplaceTaskManager(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSingleInstance(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMinimizeToTray(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSystemInformation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewDrivers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindReplaceMessage(UINT /*uMsg*/, WPARAM id, LPARAM lParam, BOOL& handled);
	LRESULT OnEditFind(WORD, WORD, HWND, BOOL&);
	LRESULT OnEditFindNext(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewKernelModules(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CString GetDefaultSettingsFile();
	void ToggleAlwaysOnTop(UINT id);
	void CloseAllBut(int page);
	void InitCommandBar();
	void InitToolBar(CToolBarCtrl& tb);
	bool DetachTab(int index);
	LRESULT SendMessageToAllFrames(bool excludeCurrent, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT ShowNotImplemented();

private:
	CTabView m_view;
	CMultiPaneStatusBarCtrl m_StatusBar;
	inline static ObjectManager m_ObjMgr;
	int m_CurrentPage = -1;
	inline static CImageListManaged m_TabImages;
	inline static std::unordered_map<std::wstring, int> m_IconMap;
	inline static CFont m_MonoFont;

	enum class IconType {
		Objects, Types, Handles, ObjectManager, Windows, Services,
		Devices, Memory, Login, Modules, Processes, COM, Threads,
		Search, SystemInfo, Kernel,
		COUNT
	};
	CFindReplaceDialog* m_pFindDlg{ nullptr };
	inline static int m_Icons[(int)IconType::COUNT];
	inline static int s_FrameCount;
	inline static std::unordered_set<CMainFrame*> s_Frames;
	inline static CString m_FindText;
	inline static DWORD m_FindFlags{ 0 };
};
