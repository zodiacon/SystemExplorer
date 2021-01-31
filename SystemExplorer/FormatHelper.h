#pragma once

#include <Token.h>
#include <ProcessInfo.h>
#include <ProcessManager.h>
#include "ProcessInfoEx.h"

enum class ProcessColumn {
	Name, Id, UserName, Session, CPU, CPUTime, Parent, Priority, PriorityClass, Threads, PeakThreads,
	Handles, Attributes, ExePath, CreateTime, CommitSize, PeakCommitSize,
	WorkingSet, PeakWorkingSet, VirtualSize, PeakVirtualSize,
	PagedPool, PeakPagedPool, NonPagedPool, PeakNonPagedPool,
	KernelTime, UserTime,
	IoPriority, MemoryPriority, CommandLine, PackageFullName, JobId,
	IoReadBytes, IoWriteBytes, IoOtherBytes, IoReads, IoWrites, IoOther,
	GDIObjects, UserObjects, PeakGdiObjects, PeakUserObjects, Integrity, Elevated, Virtualized,
	WindowTitle, Platform, Description, Company, DpiAwareness,
	COUNT
};

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
	static CString ComFlagsToString(WinSys::ComFlags flags);
	static PCWSTR ComApartmentToString(WinSys::ComFlags flags);
	static CString GetProcessColumnValue(ProcessColumn col, const WinSys::ProcessManager& pm, WinSys::ProcessInfo* pi, ProcessInfoEx& px);
	static CString ProcessAttributesToString(ProcessAttributes attributes);
	static PCWSTR DpiAwarenessToString(DpiAwareness da);
};
