#include "pch.h"
#include "FormatHelper.h"

CString FormatHelper::TimeSpanToString(int64_t ts) {
	auto str = CTimeSpan(ts / 10000000).Format(L"%D.%H:%M:%S");

	str.Format(L"%s.%03d", str, (ts / 10000) % 1000);
	return str;
}

CString FormatHelper::FormatWithCommas(long long size) {
	CString result;
	result.Format(L"%lld", size);
	int i = 3;
	while (result.GetLength() - i > 0) {
		result = result.Left(result.GetLength() - i) + L"," + result.Right(i);
		i += 4;
	}
	return result;
}

CString FormatHelper::TimeToString(int64_t time, bool includeMS) {
	auto str = CTime(*(FILETIME*)&time).Format(L"%x %X");
	if (includeMS) {
		str.Format(L"%s.%03d", str, (time / 10000) % 1000);
	}
	return str;
}

CString FormatHelper::PrivilegeAttributesToString(DWORD pattributes) {
	CString result;
	static struct {
		DWORD flag;
		PCSTR text;
	} attributes[] = {
		{ SE_PRIVILEGE_ENABLED, "Enabled" },
		{ SE_PRIVILEGE_ENABLED_BY_DEFAULT, "Default Enabled" },
	};

	for (const auto& attr : attributes)
		if ((attr.flag & pattributes) == attr.flag)
			(result += attr.text) += ", ";

	if (!result.IsEmpty())
		result = result.Left(result.GetLength() - 2);
	else
		result = "Disabled";
	return result;
}

PCWSTR FormatHelper::VirtualizationStateToString(WinSys::VirtualizationState state) {
	switch (state) {
		case WinSys::VirtualizationState::Disabled: return L"Disabled";
		case WinSys::VirtualizationState::Enabled: return L"Enabled";
		case WinSys::VirtualizationState::NotAllowed: return L"Not Allowed";
	}
	return L"Unknown";
}

PCWSTR FormatHelper::IntegrityToString(WinSys::IntegrityLevel level) {
	switch (level) {
		case WinSys::IntegrityLevel::High: return L"High";
		case WinSys::IntegrityLevel::Medium: return L"Medium";
		case WinSys::IntegrityLevel::MediumPlus: return L"Medium+";
		case WinSys::IntegrityLevel::Low: return L"Low";
		case WinSys::IntegrityLevel::System: return L"System";
		case WinSys::IntegrityLevel::Untrusted: return L"Untrusted";
	}
	return L"Unknown";
}
