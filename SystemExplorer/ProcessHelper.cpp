#include "stdafx.h"
#include "ProcessHelper.h"

CString ProcessHelper::GetFullProcessName(DWORD pid) {
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
	if (hProcess) {
		WCHAR path[MAX_PATH];
		DWORD size = MAX_PATH;
		if (::QueryFullProcessImageName(hProcess.get(), 0, path, &size))
			return path;
	}
	return L"";
}

CString ProcessHelper::GetProcessName(DWORD pid) {
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
	if (hProcess) {
		return GetProcessName(hProcess.get());
	}
	return L"";
}

CString ProcessHelper::GetProcessName(HANDLE hProcess) {
	WCHAR path[MAX_PATH];
	DWORD size = MAX_PATH;
	if (::QueryFullProcessImageName(hProcess, 0, path, &size))
		return ::wcsrchr(path, L'\\') + 1;
	return L"";
}
