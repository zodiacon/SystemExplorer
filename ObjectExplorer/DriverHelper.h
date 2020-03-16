#pragma once

struct DriverHelper final {
	static const USHORT CurrentVersion = 0x100;

	static bool LoadDriver(bool load = true);
	static bool InstallDriver();
	static bool UpdateDriver();
	static bool IsDriverLoaded();
	static HANDLE OpenHandle(void* pObject, ACCESS_MASK);
	static HANDLE DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK acess, DWORD flags = DUPLICATE_SAME_ACCESS);
	static HANDLE OpenProcess(DWORD pid, ACCESS_MASK access = PROCESS_QUERY_INFORMATION);
	static PVOID GetObjectAddress(HANDLE hObject);
	static USHORT GetVersion();

private:
	static bool OpenDevice();

	static HANDLE _hDevice;
};

