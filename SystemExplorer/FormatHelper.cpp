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

PCWSTR FormatHelper::SidNameUseToString(SID_NAME_USE use) {
	switch (use) {
		case SidTypeUser: return L"User";
		case SidTypeGroup: return L"Group";
		case SidTypeDomain: return L"Domain";
		case SidTypeAlias: return L"Alias";
		case SidTypeWellKnownGroup: return L"Well Known Group";
		case SidTypeDeletedAccount: return L"Deleted Account";
		case SidTypeInvalid: return L"Invalid";
		case SidTypeComputer: return L"Computer";
		case SidTypeLabel: return L"Label";
		case SidTypeLogonSession: return L"Logon Session";
	}
	return L"Unknown";
}

CString FormatHelper::SidAttributesToString(WinSys::SidGroupAttributes sidAttributes) {
	CString result;

	static struct {
		DWORD flag;
		PCWSTR text;
	} attributes[] = {
		{ SE_GROUP_ENABLED, L"Enabled" },
		{ SE_GROUP_ENABLED_BY_DEFAULT, L"Default Enabled" },
		{ SE_GROUP_INTEGRITY, L"Integrity" },
		{ SE_GROUP_INTEGRITY_ENABLED, L"Integrity Enabled" },
		{ SE_GROUP_LOGON_ID, L"Logon ID" },
		{ SE_GROUP_MANDATORY, L"Mandatory" },
		{ SE_GROUP_OWNER, L"Owner" },
		{ SE_GROUP_RESOURCE, L"Domain Local" },
		{ SE_GROUP_USE_FOR_DENY_ONLY, L"Deny Only" },
	};

	for (const auto& attr : attributes)
		if ((attr.flag & (DWORD)sidAttributes) == attr.flag)
			(result += attr.text) += ", ";

	if (!result.IsEmpty())
		result = result.Left(result.GetLength() - 2);
	else
		result = "(none)";
	return result;
}

CString FormatHelper::PrivilegeAttributesToString(DWORD pattributes) {
	CString result;
	static struct {
		DWORD flag;
		PCWSTR text;
	} attributes[] = {
		{ SE_PRIVILEGE_ENABLED, L"Enabled" },
		{ SE_PRIVILEGE_ENABLED_BY_DEFAULT, L"Default Enabled" },
		{ SE_PRIVILEGE_REMOVED, L"Removed" },
		{ SE_PRIVILEGE_USED_FOR_ACCESS, L"Used for Access" },
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

PCWSTR FormatHelper::PriorityClassToString(WinSys::ProcessPriorityClass pc) {
	switch (pc) {
		case WinSys::ProcessPriorityClass::Normal: return L"Normal (8)";
		case WinSys::ProcessPriorityClass::AboveNormal: return L"Above Normal (10)";
		case WinSys::ProcessPriorityClass::BelowNormal: return L"Below Normal (6)";
		case WinSys::ProcessPriorityClass::High: return L"High (13)";
		case WinSys::ProcessPriorityClass::Idle: return L"Idle (4)";
		case WinSys::ProcessPriorityClass::Realtime: return L"Realtime (24)";
	}
	return L"";
}
