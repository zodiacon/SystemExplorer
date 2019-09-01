#pragma once

struct DriverHelper final {
	static bool LoadDriver();
	static bool InstallDriver();
	static HANDLE OpenHandle(void* pObject, ACCESS_MASK);
	static HANDLE DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK acess);
	static bool IsDriverLoaded();

private:
	static bool OpenDevice();

	static HANDLE _hDevice;
};

