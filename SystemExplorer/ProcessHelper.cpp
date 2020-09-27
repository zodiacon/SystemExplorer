#include "pch.h"
#include "ProcessHelper.h"
#include "JobPropertiesDlg.h"
#include "ProcessPropertiesDlg.h"
#include "ObjectManager.h"
#include "TokenPropertiesDlg.h"
#include "ProcessPropertiesDlg.h"
#include "ProcessInfoEx.h"

const CString ifeoKey(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\");

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

bool ProcessHelper::IsReplacingTaskManager() {
	CRegKey key;
	if (ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, ifeoKey + L"taskmgr.exe", KEY_READ))
		return false;
	WCHAR value[MAX_PATH];
	ULONG count = _countof(value);
	if (ERROR_SUCCESS != key.QueryStringValue(L"Debugger", value, &count))
		return false;
	return ::_wcsicmp(GetProcessImageName(), value) == 0;
}

bool ProcessHelper::ReplaceTaskManager(bool revert) {
	CRegKey key;
	DWORD disp;
	if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, ifeoKey + L"taskmgr.exe", nullptr, 0, KEY_READ | KEY_WRITE | DELETE, nullptr, &disp))
		return false;

	ATLASSERT(!revert || (revert && disp == REG_OPENED_EXISTING_KEY));

	if (revert)
		return key.DeleteValue(L"Debugger") == ERROR_SUCCESS;

	return key.SetStringValue(L"Debugger", GetProcessImageName(), REG_SZ) == ERROR_SUCCESS;
}

CString ProcessHelper::GetProcessImageName() {
	static CString path;
	if (path.IsEmpty()) {
		::GetModuleFileName(nullptr, path.GetBufferSetLength(MAX_PATH), MAX_PATH);
		path.FreeExtra();
	}
	return path;
}

bool ProcessHelper::IsInstanceRunning() {
	static HANDLE hMutex = nullptr;
	if (!hMutex) {
		hMutex = ::CreateMutex(nullptr, FALSE, L"SystemExplorerInstanceMutex");
		if (::GetLastError() == ERROR_ALREADY_EXISTS)
			return true;
	}
	return false;
}

bool ProcessHelper::SetExistingInstanceFocus(PCWSTR name) {
	auto hWnd = ::FindWindow(name, nullptr);
	if (!hWnd)
		return false;

	::SetForegroundWindow(hWnd);
	return true;
}

int ProcessHelper::OpenObjectDialog(const WinSys::ProcessManager& pm, HANDLE hObject, const CString& type) {
	if (type == L"Job") {
		auto name = ObjectManager::GetObjectName(hObject, ObjectManager::GetType(type)->TypeIndex);
		CJobPropertiesDlg dlg(pm, hObject, name);
		dlg.DoModal();
	}
	else if (type == L"Token") {
		CTokenPropertiesDlg dlg(hObject);
		dlg.DoModal();
	}
	else if (type == L"Process") {
		auto pi = pm.GetProcessById(::GetProcessId(hObject));
		if (pi == nullptr)
			return OpenObjectDialog(pm, nullptr, L"");
		ProcessInfoEx px(pi.get());
		CProcessPropertiesDlg dlg(pm, px);
		dlg.SetModal(true);
		dlg.DoModal();
	}
	else {
		AtlMessageBox(nullptr, L"Object properties not available", IDS_TITLE, MB_ICONINFORMATION);
	}
	return 0;
}
