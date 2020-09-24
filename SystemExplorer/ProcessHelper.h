#pragma once

struct ProcessHelper final {
	static CString GetFullProcessName(DWORD pid);
	static CString GetProcessName(DWORD pid);
	static CString GetProcessName(HANDLE hProcess);

	static int OpenObjectDialog(const WinSys::ProcessManager& pm, HANDLE hObject, const CString& type);
};

