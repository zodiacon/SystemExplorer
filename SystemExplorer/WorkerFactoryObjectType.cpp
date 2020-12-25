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

	CString text;
	text.Format(L"PID: %u (%s), Min Threads: %u, Max Threads: %u, Stack Commit: %u KB, Stack Reserve: %u KB, Paused: %s, Pending workers: %u",
		HandleToUlong(info.ProcessId), _pm.GetProcessById(HandleToUlong(info.ProcessId))->GetImageName().c_str(),
		info.ThreadMinimum, info.ThreadMaximum, info.StackCommit >> 10, info.StackReserve >> 10,
		info.Paused ? L"Yes" : L"No", info.PendingWorkerCount);

	return text;
}
