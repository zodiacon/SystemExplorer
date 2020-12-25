#pragma once

#include <ProcessManager.h>

struct ProcessHelper final {
	static CString GetFullProcessName(DWORD pid);
	static CString GetProcessName(DWORD pid);
	static CString GetProcessName(HANDLE hProcess);
	static bool IsReplacingTaskManager();
	static bool ReplaceTaskManager(bool revert = false);
	static CString GetProcessImageName();
	static bool IsInstanceRunning();
	static bool SetExistingInstanceFocus(PCWSTR className);

	static int OpenObjectDialog(const WinSys::ProcessManager& pm, HANDLE hObject, const CString& type);
};

