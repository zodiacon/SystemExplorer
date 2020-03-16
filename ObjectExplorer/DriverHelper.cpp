#include "stdafx.h"
#include "resource.h"
#include "DriverHelper.h"
#include "SecurityHelper.h"
#include "..\KObjExp\KObjExp.h"

HANDLE DriverHelper::_hDevice;

bool DriverHelper::LoadDriver() {
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return false;

	wil::unique_schandle hService(::OpenService(hScm.get(), L"KObjExp", SERVICE_ALL_ACCESS));
	if (!hService)
		return false;

	return ::StartService(hService.get(), 0, nullptr) ? true : false;
}

bool DriverHelper::InstallDriver() {
	if (!SecurityHelper::IsRunningElevated())
		return false;

	// locate the driver binary resource, extract to temp folder and install

	auto hRes = ::FindResource(nullptr, MAKEINTRESOURCE(IDR_DRIVER), L"BIN");
	if (!hRes)
		return false;

	auto hGlobal = ::LoadResource(nullptr, hRes);
	if (!hGlobal)
		return false;

	auto size = ::SizeofResource(nullptr, hRes);
	void* pBuffer = ::LockResource(hGlobal);

	WCHAR path[MAX_PATH];
	::GetSystemDirectory(path, MAX_PATH);
	::wcscat_s(path, L"\\Drivers\\KObjExp.sys");
	wil::unique_hfile hFile(::CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_SYSTEM, nullptr));
	if (!hFile)
		return false;

	DWORD bytes = 0;
	::WriteFile(hFile.get(), pBuffer, size, &bytes, nullptr);
	if (bytes != size)
		return false;

	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return false;

	wil::unique_schandle hService(::CreateService(hScm.get(), L"KObjExp", nullptr, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path, nullptr, nullptr, nullptr, nullptr, nullptr));
	return hService != nullptr;
}

HANDLE DriverHelper::OpenHandle(void* pObject, ACCESS_MASK access) {
	if (!OpenDevice())
		return nullptr;

	OpenObjectData data;
	data.Access = access;
	data.Address = pObject;

	DWORD bytes;
	HANDLE hObject;
	return ::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_OPEN_OBJECT, &data, sizeof(data), 
		&hObject, sizeof(hObject), &bytes, nullptr)
		? hObject : nullptr;
}

HANDLE DriverHelper::DupHandle(HANDLE hObject, ULONG pid, ACCESS_MASK access, DWORD flags) {
	if (!OpenDevice())
		return nullptr;

	DupHandleData data;
	data.AccessMask = access;
	data.Handle = HandleToUlong(hObject);
	data.SourcePid = pid;
	data.Flags = flags;

	DWORD bytes;
	HANDLE hTarget = nullptr;
	return ::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_DUP_HANDLE, &data, sizeof(data),
		&hTarget, sizeof(hTarget), &bytes, nullptr)
		? hTarget : nullptr;
}

HANDLE DriverHelper::OpenProcess(DWORD pid, ACCESS_MASK access) {
	if (!OpenDevice())
		return nullptr;

	OpenProcessData data;
	data.AccessMask = access;
	data.ProcessId = pid;
	HANDLE hProcess;
	DWORD bytes;

	return ::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_OPEN_PROCESS, &data, sizeof(data),
		&hProcess, sizeof(hProcess), &bytes, nullptr) ? hProcess : nullptr;
}

bool DriverHelper::IsDriverLoaded() {
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE));
	if (!hScm)
		return false;

	wil::unique_schandle hService(::OpenService(hScm.get(), L"KObjExp", SERVICE_QUERY_STATUS));
	if (!hService)
		return false;

	SERVICE_STATUS status;
	if (!::QueryServiceStatus(hService.get(), &status))
		return false;

	return status.dwCurrentState == SERVICE_RUNNING;
}

bool DriverHelper::OpenDevice() {
	if (!_hDevice) {
		_hDevice = ::CreateFile(L"\\\\.\\KObjExp", GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
			OPEN_EXISTING, 0, nullptr);
		if (_hDevice == INVALID_HANDLE_VALUE)
			return nullptr;
	}
	return _hDevice != INVALID_HANDLE_VALUE;
}
