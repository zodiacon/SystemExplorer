#pragma once

struct DriverHelper final {
	static bool LoadDriver(bool load = true);
	static bool InstallDriver(bool justCopy = false);
	static bool UpdateDriver();
	static bool IsDriverLoaded();
	static HANDLE OpenHandle(void* pObject, ACCESS_MASK);
	static HANDLE DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK acess, DWORD flags = DUPLICATE_SAME_ACCESS);
	static HANDLE OpenProcess(DWORD pid, ACCESS_MASK access = PROCESS_QUERY_INFORMATION);
	static PVOID GetObjectAddress(HANDLE hObject);
	static USHORT GetVersion();
	static USHORT GetCurrentVersion();
	static bool CloseDevice();
	static HANDLE OpenThread(DWORD tid, ACCESS_MASK access = THREAD_QUERY_INFORMATION);

private:
	static bool OpenDevice();

	static HANDLE _hDevice;
};

