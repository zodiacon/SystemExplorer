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
