#pragma once

struct ProcessHelper final {
	static CString GetFullProcessName(DWORD pid);
	static CString GetProcessName(DWORD pid);
};

