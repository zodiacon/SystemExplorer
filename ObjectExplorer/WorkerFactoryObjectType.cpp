#include "stdafx.h"
#include "WorkerFactoryObjectType.h"
#include "NtDll.h"

WorkerFactoryObjectType::WorkerFactoryObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString WorkerFactoryObjectType::GetDetails(HANDLE hObject) {
	NT::WORKER_FACTORY_BASIC_INFORMATION info;
	if(!NT_SUCCESS(NT::NtQueryInformationWorkerFactory(hObject, NT::WorkerFactoryBasicInformation, &info, sizeof(info), nullptr)))
		return L"";

	CString text;
	text.Format(L"PID: %u, Min Threads: %u, Max Threads: %u, Stack Commit: %u KB, Stack Reserve: %u KB, Paused: %s, Pending workers: %u",
		info.ProcessId, info.ThreadMinimum, info.ThreadMaximum, info.StackCommit >> 10, info.StackReserve >> 10,
		info.Paused ? L"Yes" : L"No", info.PendingWorkerCount);

	return text;
}
