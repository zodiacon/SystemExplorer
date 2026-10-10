#include "pch.h"
#include "WorkerFactoryObjectType.h"
#include "NtDll.h"
#include <ProcessInfo.h>

using namespace WinSys;

WorkerFactoryObjectType::WorkerFactoryObjectType(ProcessManager& pm, int index, PCWSTR name) : _pm(pm), ObjectType(index, name) {
}

CString WorkerFactoryObjectType::GetDetails(HANDLE hObject) {
	NT::WORKER_FACTORY_BASIC_INFORMATION info;
	if(!NT_SUCCESS(NT::NtQueryInformationWorkerFactory(hObject, NT::WorkerFactoryBasicInformation, &info, sizeof(info), nullptr)))
		return L"";

	// the owning process may not be in the snapshot (e.g. it started after it was taken): the name is empty then
	auto processName = _pm.GetProcessNameById(HandleToUlong(info.ProcessId));
	CString text;
	text.Format(L"PID: %u (%s), Min Threads: %u, Max Threads: %u, Stack Commit: %u KB, Stack Reserve: %u KB, Paused: %s, Pending workers: %u",
		HandleToUlong(info.ProcessId), processName.c_str(),
		info.ThreadMinimum, info.ThreadMaximum, (ULONG)(info.StackCommit >> 10), (ULONG)(info.StackReserve >> 10),
		info.Paused ? L"Yes" : L"No", info.PendingWorkerCount);

	return text;
}
