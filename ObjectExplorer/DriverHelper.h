#pragma once

struct DriverHelper final {
	static bool LoadDriver();
	static bool InstallDriver();
	static HANDLE OpenHandle(void* pObject, ACCESS_MASK);
	static HANDLE DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK acess, DWORD flags = DUPLICATE_SAME_ACCESS);
	static bool IsDriverLoaded();

private:
	static bool OpenDevice();

	static HANDLE _hDevice;
};

