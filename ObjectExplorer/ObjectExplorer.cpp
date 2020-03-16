// ObjectExplorer.cpp : main source file for ObjectExplorer.exe
//

#include "stdafx.h"

#include "resource.h"

#include "MainFrm.h"
#include "DriverHelper.h"

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

bool CheckInstall(PCWSTR cmdLine) {
	bool parse = false, success = false;
	if (::wcsstr(cmdLine, L"install")) {
		parse = true;
		success = DriverHelper::LoadDriver();
		if (!success)
			if (DriverHelper::InstallDriver())
				success = DriverHelper::LoadDriver();
		if (!success)
			AtlMessageBox(nullptr, L"Failed to install/load kernel driver", IDS_TITLE, MB_ICONERROR);
	}
	else if (::wcsstr(cmdLine, L"update")) {
		parse = true;
		success = DriverHelper::UpdateDriver();
		if (!success) {
			AtlMessageBox(nullptr, L"Failed to update kernel driver", IDS_TITLE, MB_ICONERROR);
		}
	}
	return parse;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	if (CheckInstall(lpstrCmdLine))
		return 0;

	HRESULT hRes = ::CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES);

	hRes = _Module.Init(nullptr, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

