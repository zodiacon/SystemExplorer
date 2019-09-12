// ObjectExplorer.cpp : main source file for ObjectExplorer.exe
//

#include "stdafx.h"

#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

CAppModule _Module;

bool ParseCommandLine(PCWSTR cmdLine);

int Run(LPTSTR /*lpstrCmdLine*/ = nullptr, int nCmdShow = SW_SHOWDEFAULT) {

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if (wndMain.CreateEx() == nullptr) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

#include "NtDll.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	//auto h = ::CreateWaitableTimer(nullptr, FALSE, nullptr);
	//LARGE_INTEGER time;

	//int64_t ft;
	//::GetSystemTimeAsFileTime((FILETIME*)&ft);
	//time.QuadPart = ft + 10000000; //-10000000;
	//::SetWaitableTimer(h, &time, 0, nullptr, nullptr, FALSE);
	//NT::TIMER_BASIC_INFORMATION bi;
	//for (int i = 0; i < 20; i++) {
	//	::GetSystemTimeAsFileTime((FILETIME*)&ft);
	//	NT::NtQueryTimer(h, NT::TimerBasicInformation, &bi, sizeof(bi), nullptr);
	//	ATLTRACE(L"%lld\n", -bi.RemainingTime.QuadPart + *(int64_t*)&ft);
	//	::Sleep(100);
	//}

	HRESULT hRes = ::CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(nullptr, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

