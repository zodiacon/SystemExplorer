#include "pch.h"
#include "ProcessInfoEx.h"
#include "DriverHelper.h"
#include <ProcessInfo.h>
#include <Helpers.h>
#include <shellscalingapi.h>

#pragma comment(lib, "Version.lib")

ProcessInfoEx::ProcessInfoEx(WinSys::ProcessInfo* pi) : _pi(pi) {
	auto hProcess = DriverHelper::OpenProcess(_pi->Id, PROCESS_QUERY_INFORMATION);
	if (!hProcess)
		hProcess = DriverHelper::OpenProcess(_pi->Id, PROCESS_QUERY_LIMITED_INFORMATION);
	if (hProcess)
		_process.reset(new WinSys::Process(hProcess));
}

WinSys::ProcessInfo* ProcessInfoEx::GetProcessInfo() const {
	return _pi;
}

ProcessAttributes ProcessInfoEx::GetAttributes(const WinSys::ProcessManager& pm) const {
	if (_attributes == ProcessAttributes::NotComputed) {
		_attributes = ProcessAttributes::None;
		if (_process) {
			if (_process->IsManaged())
				_attributes |= ProcessAttributes::Managed;
			if (_process->IsProtected())
				_attributes |= ProcessAttributes::Protected;
			if (_process->IsImmersive())
				_attributes |= ProcessAttributes::Immersive;
			if (_process->IsSecure())
				_attributes |= ProcessAttributes::Secure;
			if (_process->IsInJob())
				_attributes |= ProcessAttributes::InJob;
			auto parent = pm.GetProcessById(_pi->ParentId);
			if (parent && ::_wcsicmp(parent->GetImageName().c_str(), L"services.exe") == 0)
				_attributes |= ProcessAttributes::Service;
			if (_process->IsWow64Process())
				_attributes |= ProcessAttributes::Wow64;
		}
	}
	return _attributes;
}

const std::wstring& ProcessInfoEx::GetExecutablePath() const {
	if (_executablePath.empty() && _pi->Id != 0) {
		const auto& path = _pi->GetNativeImagePath();
		if (path[0] == L'\\')
			_executablePath = WinSys::Helpers::GetDosNameFromNtName(path.c_str());
		if (_executablePath.empty() && path.substr(0, 8) == L"\\Device\\")
			_executablePath = L"\\??\\" + path.substr(8);
		if (_executablePath.empty())
			_executablePath = path;
	}
	return _executablePath;
}

const std::wstring& ProcessInfoEx::UserName() const {
	if (_username.empty()) {
		if (_pi->Id <= 4)
			_username = L"NT AUTHORITY\\SYSTEM";
		else {
			if (_process)
				_username = _process->GetUserName();
			if (_username.empty())
				_username = L"<access denied>";
		}
	}
	return _username;
}

int ProcessInfoEx::GetImageIndex(CImageList images) const {
	if (_image < 0) {
		_image = 0;
		HICON hIcon = nullptr;
		::ExtractIconEx(GetExecutablePath().c_str(), 0, nullptr, &hIcon, 1);
		if (hIcon)
			_image = images.AddIcon(hIcon);
	}
	return _image;
}

WinSys::IoPriority ProcessInfoEx::GetIoPriority() const {
	return _process ? _process->GetIoPriority() : WinSys::IoPriority::Unknown;
}

int ProcessInfoEx::GetMemoryPriority() const {
	return _process ? _process->GetMemoryPriority() : -1;
}

WinSys::ProcessPriorityClass ProcessInfoEx::GetPriorityClass() {
	return _process ? _process->GetPriorityClass() : WinSys::ProcessPriorityClass::Unknown;
}

const std::wstring& ProcessInfoEx::GetCommandLine() const {
	if (_commandLine.empty() && _pi->Id > 4) {
		if (_process)
			_commandLine = _process->GetCommandLine();
	}
	return _commandLine;
}

bool ProcessInfoEx::IsElevated() const {
	if (!_elevatedChecked) {
		_elevatedChecked = true;
		if (_pi->Id > 4 && _process != nullptr) {
			WinSys::Token token(_process->GetHandle(), WinSys::TokenAccessMask::Query);
			if (token.IsValid())
				_elevated = token.IsElevated();
		}
	}
	return _elevated;
}

uint32_t ProcessInfoEx::GetGdiObjects() const {
	return _process ? _process->GetGdiObjectCount() : 0;
}

uint32_t ProcessInfoEx::GetUserObjects() const {
	return _process ? _process->GetUserObjectCount() : 0;
}

uint32_t ProcessInfoEx::GetPeakGdiObjects() const {
	return _process ? _process->GetPeakGdiObjectCount() : 0;
}

uint32_t ProcessInfoEx::GetPeakUserObjects() const {
	return _process ? _process->GetPeakUserObjectCount() : 0;
}

WinSys::IntegrityLevel ProcessInfoEx::GetIntegrityLevel() const {
	if (_process) {
		WinSys::Token token(_process->GetHandle(), WinSys::TokenAccessMask::Query);
		if (token.IsValid())
			return token.GetIntegrityLevel();
	}
	return WinSys::IntegrityLevel::Error;
}

WinSys::VirtualizationState ProcessInfoEx::GetVirtualizationState() const {
	if (_process) {
		WinSys::Token token(_process->GetHandle(), WinSys::TokenAccessMask::Query);
		if (token.IsValid())
			return token.GetVirtualizationState();
	}
	return WinSys::VirtualizationState::Unknown;
}

CString ProcessInfoEx::GetWindowTitle() const {
	if (_process == nullptr)
		return L"";
	CString text;
	if (!_hWnd) {
		if (_firstThreadId == 0) {
			auto hThread = _process->GetNextThread();
			if (hThread) {
				::EnumThreadWindows(::GetThreadId(hThread), [](auto hWnd, auto param) {
					if (::IsWindowVisible(hWnd)) {
						*(HWND*)param = hWnd;
						return FALSE;
					}
					return TRUE;
					}, (LPARAM)&_hWnd);
				::CloseHandle(hThread);
			}
		}
	}
	if (_hWnd) {
		::GetWindowText(_hWnd, text.GetBufferSetLength(128), 128);
	}
	return text;
}

std::wstring ProcessInfoEx::GetCurrentDirectory() const {
	auto hProcess = DriverHelper::OpenProcess(_pi->Id, PROCESS_VM_READ | PROCESS_QUERY_INFORMATION);
	if (!hProcess)
		return L"";

	auto dir = WinSys::Process::GetCurrentDirectory(hProcess);
	if (hProcess)
		::CloseHandle(hProcess);
	return dir;
}

const CString& ProcessInfoEx::GetDescription() const {
	if (!_descChecked) {
		_description = GetVersionObject(L"FileDescription");
		_descChecked = true;
	}
	return _description;
}

const CString& ProcessInfoEx::GetCompanyName() const {
	if (!_companyChecked) {
		_company = GetVersionObject(L"CompanyName");
		_companyChecked = true;
	}
	return _company;
}

DpiAwareness ProcessInfoEx::GetDpiAwareness() const {
	static const auto pGetProcessDpiAware = (decltype(::GetProcessDpiAwareness)*)::GetProcAddress(::GetModuleHandle(L"shcore"), "GetProcessDpiAwareness");

	if (!_process || pGetProcessDpiAware == nullptr)
		return DpiAwareness::None;

	DpiAwareness da = DpiAwareness::None;
	pGetProcessDpiAware(_process->GetHandle(), reinterpret_cast<PROCESS_DPI_AWARENESS*>(&da));
	return da;
}

CString ProcessInfoEx::GetVersionObject(const CString& name) const {
	BYTE buffer[1 << 12];
	CString result;
	const auto& exe = GetExecutablePath();
	if (::GetFileVersionInfo(exe.c_str(), 0, sizeof(buffer), buffer)) {
		WORD* langAndCodePage;
		UINT len;
		if (::VerQueryValue(buffer, L"\\VarFileInfo\\Translation", (void**)&langAndCodePage, &len)) {
			CString text;
			text.Format(L"\\StringFileInfo\\%04x%04x\\" + name, langAndCodePage[0], langAndCodePage[1]);
			WCHAR* desc;
			if (::VerQueryValue(buffer, text, (void**)&desc, &len))
				result = desc;
		}
	}
	return result;
}

int ProcessInfoEx::GetBitness() const {
	if (_bitness == 0) {
		static SYSTEM_INFO si = { 0 };
		if (si.dwNumberOfProcessors == 0)
			::GetNativeSystemInfo(&si);
		if (_process == nullptr) {
			_bitness = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM ? 32 : 64;
		}
		else {
			if (_process->IsWow64Process())
				_bitness = 32;
			else
				_bitness = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM ? 32 : 64;
		}
	}
	return _bitness;
}
