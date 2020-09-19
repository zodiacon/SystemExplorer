#include "pch.h"
#include "resource.h"
#include "DriverHelper.h"
#include "SecurityHelper.h"
#include "..\KObjExp\KObjExp.h"

HANDLE DriverHelper::_hDevice;

bool DriverHelper::LoadDriver(bool load) {
	if (_hDevice) {
		::CloseHandle(_hDevice);
		_hDevice = INVALID_HANDLE_VALUE;
	}
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return false;

	wil::unique_schandle hService(::OpenService(hScm.get(), L"KObjExp", SERVICE_ALL_ACCESS));
	if (!hService)
		return false;

	SERVICE_STATUS status;
	bool success = true;
	DWORD targetState;
	::QueryServiceStatus(hService.get(), &status);
	if (load && status.dwCurrentState != (targetState = SERVICE_RUNNING))
		success = ::StartService(hService.get(), 0, nullptr);
	else if (!load && status.dwCurrentState != (targetState = SERVICE_STOPPED))
		success = ::ControlService(hService.get(), SERVICE_CONTROL_STOP, &status);
	else
		return true;

	if (!success)
		return false;

	for (int i = 0; i < 20; i++) {
		::QueryServiceStatus(hService.get(), &status);
		if (status.dwCurrentState == targetState)
			return true;
		::Sleep(200);
	}
	return false;
}

bool DriverHelper::InstallDriver(bool justCopy) {
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
	wil::unique_hfile hFile(::CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_SYSTEM, nullptr));
	if (!hFile)
		return false;

	DWORD bytes = 0;
	::WriteFile(hFile.get(), pBuffer, size, &bytes, nullptr);
	if (bytes != size)
		return false;

	if (justCopy)
		return true;

	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return false;

	wil::unique_schandle hService(::CreateService(hScm.get(), L"KObjExp", nullptr, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path, nullptr, nullptr, nullptr, nullptr, nullptr));
	return hService != nullptr;
}

bool DriverHelper::UpdateDriver() {
	if (!LoadDriver(false))
		return false;
	if (!InstallDriver(true))
		return false;
	if (!LoadDriver())
		return false;

	return true;
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
	HANDLE hTarget = nullptr;
	if (OpenDevice()) {
		DupHandleData data;
		data.AccessMask = access;
		data.Handle = HandleToUlong(hObject);
		data.SourcePid = pid;
		data.Flags = flags;

		DWORD bytes;
		::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_DUP_HANDLE, &data, sizeof(data),
			&hTarget, sizeof(hTarget), &bytes, nullptr);
	}
	if (!hTarget) {
		wil::unique_handle hProcess(OpenProcess(pid, PROCESS_DUP_HANDLE));
		if (hProcess)
			::DuplicateHandle(hObject, hProcess.get(), ::GetCurrentProcess(), &hTarget, access, FALSE, flags);
	}
	return hTarget;
}

HANDLE DriverHelper::OpenProcess(DWORD pid, ACCESS_MASK access) {
	if (OpenDevice()) {
		OpenProcessThreadData data;
		data.AccessMask = access;
		data.Id = pid;
		HANDLE hProcess;
		DWORD bytes;

		return ::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_OPEN_PROCESS, &data, sizeof(data),
			&hProcess, sizeof(hProcess), &bytes, nullptr) ? hProcess : nullptr;
	}
	return ::OpenProcess(access, FALSE, pid);
}

HANDLE DriverHelper::OpenThread(DWORD tid, ACCESS_MASK access) {
	if (OpenDevice()) {
		OpenProcessThreadData data;
		data.AccessMask = access;
		data.Id = tid;
		HANDLE hThread;
		DWORD bytes;

		return ::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_OPEN_PROCESS, &data, sizeof(data),
			&hThread, sizeof(hThread), &bytes, nullptr) ? hThread : nullptr;
	}
	return ::OpenThread(access, FALSE, tid);
}

PVOID DriverHelper::GetObjectAddress(HANDLE hObject) {
	if (!OpenDevice())
		return nullptr;

	PVOID address = nullptr;
	DWORD bytes;
	::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_GET_OBJECT_ADDRESS, &hObject, sizeof(hObject), &address, sizeof(address), &bytes, nullptr);
	return address;
}

USHORT DriverHelper::GetVersion() {
	USHORT version = 0;
	if (!OpenDevice())
		return 0;

	DWORD bytes;
	::DeviceIoControl(_hDevice, IOCTL_KOBJEXP_GET_VERSION, nullptr, 0, &version, sizeof(version), &bytes, nullptr);
	return version;
}

USHORT DriverHelper::GetCurrentVersion() {
	return DRIVER_CURRENT_VERSION;
}

bool DriverHelper::CloseDevice() {
	if (_hDevice) {
		::CloseHandle(_hDevice);
		_hDevice = nullptr;
	}
	return true;
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
		if (_hDevice == INVALID_HANDLE_VALUE) {
			_hDevice = nullptr;
			return false;
		}
	}
	return true;
}
