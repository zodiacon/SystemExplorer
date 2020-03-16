#pragma once

struct DriverHelper final {
	static bool LoadDriver();
	static bool InstallDriver();
	static bool IsDriverLoaded();
	static HANDLE OpenHandle(void* pObject, ACCESS_MASK);
	static HANDLE DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK acess, DWORD flags = DUPLICATE_SAME_ACCESS);
	static HANDLE OpenProcess(DWORD pid, ACCESS_MASK access = PROCESS_QUERY_INFORMATION);

private:
	static bool OpenDevice();

	static HANDLE _hDevice;
};

