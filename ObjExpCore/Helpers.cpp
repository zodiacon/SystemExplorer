#include "pch.h"
#include "Helpers.h"

using namespace WinSys;

std::wstring Helpers::GetDosNameFromNtName(PCWSTR name) {
	// the drives' device names, e.g. C: -> \Device\HarddiskVolume3 (initialized once, thread safely)
	static const auto deviceNames = [] {
		std::vector<std::pair<std::wstring, std::wstring>> names;
		auto drives = ::GetLogicalDrives();
		int drive = 0;
		while (drives) {
			if (drives & 1) {
				// drive exists
				WCHAR driveName[] = L"X:";
				driveName[0] = (WCHAR)(drive + 'A');
				WCHAR path[MAX_PATH];
				if (::QueryDosDevice(driveName, path, MAX_PATH)) {
					names.push_back({ path, driveName });
				}
			}
			drive++;
			drives >>= 1;
		}
		return names;
	}();

	for (auto& [ntName, dosName] : deviceNames) {
		// the device name must be the whole first part of the path: \Device\HarddiskVolume1 is also the start of
		// \Device\HarddiskVolume10\..., which gave such paths the wrong drive letter
		if (::_wcsnicmp(name, ntName.c_str(), ntName.size()) != 0)
			continue;
		auto rest = name + ntName.size();
		if (*rest == L'\\' || *rest == 0)
			return dosName + rest;
	}
	return L"";
}

std::wstring WinSys::Helpers::GetErrorText(DWORD error) {
	PWSTR buffer;
	std::wstring text;
	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, error, 0, (LPTSTR)&buffer, 0, nullptr) > 0) {
		text = buffer;
		::LocalFree(buffer);
	}
	return text;
}
