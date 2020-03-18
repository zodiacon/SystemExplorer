#include "stdafx.h"
#include "SecurityHelper.h"


bool SecurityHelper::IsRunningElevated() {
	wil::unique_handle hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, hToken.addressof()))
		return false;

	TOKEN_ELEVATION te;
	DWORD len;
	if (::GetTokenInformation(hToken.get(), TokenElevation, &te, sizeof(te), &len))
		return te.TokenIsElevated ? true : false;
	return false;
}

HICON SecurityHelper::GetShieldIcon() {
	SHSTOCKICONINFO ssii = { sizeof(ssii) };
	if (FAILED(::SHGetStockIconInfo(SIID_SHIELD, SHGSI_SMALLICON | SHGSI_ICON, &ssii)))
		return nullptr;

	return ssii.hIcon;
}

bool SecurityHelper::RunElevated(PCWSTR param, bool ui) {
	WCHAR path[MAX_PATH];
	::GetModuleFileName(nullptr, path, _countof(path));
	SHELLEXECUTEINFO shi = { sizeof(shi) };
	shi.lpFile = path;
	shi.nShow = SW_SHOWDEFAULT;
	shi.lpVerb = L"runas";
	shi.lpParameters = param;
	shi.fMask = (ui ? 0 : (SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE)) | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
	auto ok =::ShellExecuteEx(&shi);
	if (!ok)
		return false;

	DWORD rc = WAIT_OBJECT_0;
	if (!ui) {
		rc = ::WaitForSingleObject(shi.hProcess, 5000);
	}
	::CloseHandle(shi.hProcess);
	return rc == WAIT_OBJECT_0;
}
