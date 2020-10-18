#pragma once

struct FormatHelper {
	static CString TimeSpanToString(int64_t ts);
	static CString FormatWithCommas(long long size);
	static CString TimeToString(int64_t time, bool includeMS = true);
	static CString PrivilegeAttributesToString(DWORD pattributes);
	static PCWSTR VirtualizationStateToString(WinSys::VirtualizationState state);
	static PCWSTR IntegrityToString(WinSys::IntegrityLevel level);
	static PCWSTR SidNameUseToString(SID_NAME_USE use);
	static CString SidAttributesToString(WinSys::SidGroupAttributes attributes);
	static PCWSTR PriorityClassToString(WinSys::ProcessPriorityClass pc);
	static CString JobCpuRateControlFlagsToString(DWORD flags);
	static PCWSTR IoPriorityToString(WinSys::IoPriority io);

};
