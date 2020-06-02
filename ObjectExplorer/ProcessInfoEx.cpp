#include "stdafx.h"
#include "ProcessInfoEx.h"
#include "DriverHelper.h"

ProcessInfoEx::ProcessInfoEx(WinSys::ProcessInfo* pi) : _pi(pi) {
	auto hProcess = DriverHelper::OpenProcess(_pi->Id, PROCESS_QUERY_LIMITED_INFORMATION);
	if (hProcess)
		_process.reset(new WinSys::Process(hProcess));
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
		}
	}
	return _attributes;
}

const std::wstring& ProcessInfoEx::GetExecutablePath() const {
	if (_executablePath.empty() && _pi->Id != 0) {
		const auto& path = _pi->GetNativeImagePath();
		if (path[0] == L'\\') {
			_executablePath = WinSys::Helpers::GetDosNameFromNtName(path.c_str());
		}
		else {
			_executablePath = path;
		}
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
