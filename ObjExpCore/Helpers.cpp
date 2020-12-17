#include "pch.h"
#include "Helpers.h"

using namespace WinSys;

std::wstring Helpers::GetDosNameFromNtName(PCWSTR name) {
	static std::vector<std::pair<std::wstring, std::wstring>> deviceNames;
	static bool first = true;
	if (first) {
		auto drives = ::GetLogicalDrives();
		int drive = 0;
		while (drives) {
			if (drives & 1) {
				// drive exists
				WCHAR driveName[] = L"X:";
				driveName[0] = (WCHAR)(drive + 'A');
				WCHAR path[MAX_PATH];
				if (::QueryDosDevice(driveName, path, MAX_PATH)) {
					deviceNames.push_back({ path, driveName });
				}
			}
			drive++;
			drives >>= 1;
		}
		first = false;
	}

	for (auto& [ntName, dosName] : deviceNames) {
		if (::_wcsnicmp(name, ntName.c_str(), ntName.size()) == 0)
			return dosName + (name + ntName.size());
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
