#pragma once

struct SecurityHelper abstract final {
	static bool IsRunningElevated();
	static HICON GetShieldIcon();
	static bool RunElevated(PCWSTR param, bool ui);
	static bool EnablePrivilege(PCWSTR privName, bool enable);
	static CString GetSidFromUser(PCWSTR name);
};

