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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	if (ParseCommandLine(lpstrCmdLine))
		return 0;
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

bool ParseCommandLine(PCWSTR cmdLine) {
	if (::_wcsicmp(cmdLine, L"enablentflag") == 0) {
		CRegKey key;
		auto error = key.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Session Manager", 
			KEY_WRITE | KEY_READ);
		if (error == ERROR_SUCCESS) {
			DWORD value = 0;
			key.QueryDWORDValue(L"GlobalFlag", value);
			value |= 0x4000;
			key.SetDWORDValue(L"GlobalFlag", value);
			::MessageBox(nullptr, L"Registry modified. Please restart Windows for the change to take effect.",
				L"Object Explorer", MB_ICONINFORMATION);
		}
		else {
			CString text;
			text.Format(L"Failed to open registry key (Error: %d). Are you running elevated?", error);
			::MessageBox(nullptr, text, L"Object Explorer", MB_ICONERROR);
		}
		return true;
	}
	return false;
}
