#include "stdafx.h"
#include "Thread.h"
#include "Handles.h"
#include "SystemInformation.h"
#include "Processes.h"
#include "subprocesstag.h"

using namespace WinSys;

Thread::Thread(HANDLE handle, ThreadAccessMask access) : _handle(handle), _access(access) {}

int Thread::GetMemoryPriority() const {
	int priority = -1;
	ULONG len;
	::NtQueryInformationThread(_handle, ThreadPagePriority, &priority, sizeof(priority), &len);
	return priority;
}

IoPriority Thread::GetIoPriority() const {
	IoPriority priority = IoPriority::Unknown;
	ULONG len;
	::NtQueryInformationThread(_handle, ThreadIoPriority, &priority, sizeof(priority), &len);
	return priority;
}

size_t WinSys::Thread::GetSubProcessTag() const {
	THREAD_BASIC_INFORMATION tbi;
	auto status = ::NtQueryInformationThread(_handle, ThreadBasicInformation, &tbi, sizeof(tbi), nullptr);
	if (!NT_SUCCESS(status))
		return 0;

	if (tbi.TebBaseAddress == 0)
		return 0;

	bool is64bit = SystemInformation::GetBasicSystemInfo().MaximumAppAddress > (void*)(1LL << 32);
	auto pid = ::GetProcessIdOfThread(_handle);
	auto process = Process::OpenById(pid, ProcessAccessMask::QueryLimitedInformation | ProcessAccessMask::VmRead);
	if (!process)
		return 0;

	size_t tag = 0;
	if (!is64bit || (process->IsWow64Process() && is64bit)) {
		auto teb = (TEB32*)tbi.TebBaseAddress;
		::ReadProcessMemory(process->GetHandle(), (BYTE*)teb + offsetof(TEB32, SubProcessTag), &tag, sizeof(ULONG), nullptr);
	}
	else {
		auto teb = (TEB*)tbi.TebBaseAddress;
		::ReadProcessMemory(process->GetHandle(), (BYTE*)teb + offsetof(TEB, SubProcessTag), &tag, sizeof(tag), nullptr);
	}
	return tag;
}

std::wstring Thread::GetServiceNameByTag(uint32_t pid) {
	auto QueryTagInformation = (PQUERY_TAG_INFORMATION)::GetProcAddress(::GetModuleHandle(L"advapi32"), "I_QueryTagInformation");
	if (QueryTagInformation == nullptr)
		return L"";
	auto tag = GetSubProcessTag();
	if (tag == 0)
		return L"";
	TAG_INFO_NAME_FROM_TAG info = { 0 };
	info.InParams.dwPid = pid;
	info.InParams.dwTag = static_cast<uint32_t>(tag);
	auto err = QueryTagInformation(nullptr, eTagInfoLevelNameFromTag, &info);
	if (err)
		return L"";
	return info.OutParams.pszName;
}

CpuNumber Thread::GetIdealProcessor() const {
	PROCESSOR_NUMBER cpu;
	ULONG len;
	CpuNumber number;
	if (NT_SUCCESS(::NtQueryInformationThread(_handle, ThreadIdealProcessorEx, &cpu, sizeof(cpu), &len))) {
		number.Group = cpu.Group;
		number.Number = cpu.Number;
	}
	else {
		number.Number = -1;
		number.Group = -1;
	}
	return number;
}


std::unique_ptr<Thread> Thread::OpenById(uint32_t tid, ThreadAccessMask accessMask) {
	HANDLE hThread = ::OpenThread(static_cast<ACCESS_MASK>(accessMask), FALSE, tid);
	if (!hThread)
		return nullptr;

	return std::make_unique<Thread>(hThread, accessMask);
}


