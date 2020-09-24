#include "pch.h"
#include "ProcessHelper.h"
#include "JobPropertiesDlg.h"
#include "ProcessPropertiesDlg.h"
#include "ObjectManager.h"
#include "TokenPropertiesDlg.h"
#include "ProcessPropertiesDlg.h"
#include "ProcessInfoEx.h"

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
