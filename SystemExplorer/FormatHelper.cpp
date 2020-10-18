#include "pch.h"
#include "FormatHelper.h"

using namespace WinSys;

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

PCWSTR FormatHelper::VirtualizationStateToString(VirtualizationState state) {
	switch (state) {
		case VirtualizationState::Disabled: return L"Disabled";
		case VirtualizationState::Enabled: return L"Enabled";
		case VirtualizationState::NotAllowed: return L"Not Allowed";
	}
	return L"Unknown";
}

PCWSTR FormatHelper::IntegrityToString(IntegrityLevel level) {
	switch (level) {
		case IntegrityLevel::High: return L"High";
		case IntegrityLevel::Medium: return L"Medium";
		case IntegrityLevel::MediumPlus: return L"Medium+";
		case IntegrityLevel::Low: return L"Low";
		case IntegrityLevel::System: return L"System";
		case IntegrityLevel::Untrusted: return L"Untrusted";
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

CString FormatHelper::JobCpuRateControlFlagsToString(DWORD flags) {
	CString result;
	struct {
		DWORD flag;
		PCWSTR text;
	} data[] = {
		//{ JOB_OBJECT_CPU_RATE_CONTROL_ENABLE, L"Enabled" },
		{ JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP, L"Hard Cap" },
		{ JOB_OBJECT_CPU_RATE_CONTROL_MIN_MAX_RATE, L"Min Max Rate" },
		{ JOB_OBJECT_CPU_RATE_CONTROL_NOTIFY, L"Notify" },
		{ JOB_OBJECT_CPU_RATE_CONTROL_WEIGHT_BASED, L"Weight Based" },
	};
	for (const auto& attr : data)
		if ((attr.flag & flags) == attr.flag)
			(result += attr.text) += ", ";

	if (!result.IsEmpty())
		result = result.Left(result.GetLength() - 2);
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

PCWSTR FormatHelper::IoPriorityToString(WinSys::IoPriority io) {
	switch (io) {
		case IoPriority::Critical: return L"Critical";
		case IoPriority::High: return L"High";
		case IoPriority::Low: return L"Low";
		case IoPriority::Normal: return L"Normal";
		case IoPriority::VeryLow: return L"Very Low";
	}
	return L"";
}

CString FormatHelper::ComFlagsToString(ComFlags flags) {
	static const struct {
		ComFlags Flag;
		PCWSTR Name;
	} sflags[] = {
		{ ComFlags::LocalTid, L"Local TID" },
		{ ComFlags::UuidInitialized, L"GUID Initialized" },
		{ ComFlags::InThreadDetach, L"In Thread Detach" },
		{ ComFlags::ChannelInitialized, L"Channel Initialized" },
		{ ComFlags::WowThread, L"Wow Thread" },
		{ ComFlags::ThreadUninitializing, L"Thread Uninitializing" },
		{ ComFlags::DisableOle1DDE, L"Disable Ole1 DDE" },
		{ ComFlags::STA, L"STA" },
		{ ComFlags::MTA, L"MTA" },
		{ ComFlags::Impersonating, L"Impersonating" },
		{ ComFlags::DisableEventLogger, L"Disable Event Logger" },
		{ ComFlags::InNeutralApartment, L"In NA" },
		{ ComFlags::DispatchThread, L"Disptach Thread" },
		{ ComFlags::HostThread, L"Host Thread" },
		{ ComFlags::AllowCoInit, L"Allow CoInit" },
		{ ComFlags::PendingUninit, L"Pending Uninit" },
		{ ComFlags::FirstMTAInit, L"First MTA Init" },
		{ ComFlags::FirstNTAInit, L"First TNA Init" },
		{ ComFlags::ApartmentInitializing, L"Apt Initializing" },
		{ ComFlags::UIMessageInModalLoop, L"UI Msg in Modal Loop" },
		{ ComFlags::MarshallingErrorObject, L"Marshaling Error Object" },
		{ ComFlags::WinRTInitialize, L"WinRT Init" },
		{ ComFlags::ASTA, L"ASTA" },
		{ ComFlags::InShutdownCallbacks, L"In Shutdown Callbacks" },
		{ ComFlags::PointerInputBlocked, L"Pointer Input Blocked" },
		{ ComFlags::InActivationFilter, L"In Activation Filter" },
		{ ComFlags::ASTAtoASTAExempQuirk, L"ASTA to STA Exep Quirk" },
		{ ComFlags::ASTAtoASTAExempProxy, L"ASTA to STA Exep Proxy" },
		{ ComFlags::ASTAtoASTAExempIndoubt, L"ASTA to STA Exep In Doubt" },
		{ ComFlags::DetectedUserInit, L"Detect User Init" },
		{ ComFlags::BridgeSTA, L"Bridge STA" },
		{ ComFlags::MainInitializing, L"Main Initializing" },
	};

	CString result;
	for (const auto& attr : sflags)
		if ((attr.Flag & flags) == attr.Flag)
			(result += attr.Name) += ", ";

	if (!result.IsEmpty())
		result = result.Left(result.GetLength() - 2);
	return result;
}

PCWSTR FormatHelper::ComApartmentToString(ComFlags flags) {
	if (flags == ComFlags::Error)
		return L"";
	if ((flags & ComFlags::ASTA) == ComFlags::ASTA)
		return L"ASTA";
	if ((flags & ComFlags::STA) == ComFlags::STA)
		return L"STA";
	if ((flags & ComFlags::MTA) == ComFlags::MTA)
		return L"MTA";

	return L"";
}
