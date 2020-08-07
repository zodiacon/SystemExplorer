#include "pch.h"
#include "SemaphoreObjectType.h"
#include "NtDll.h"


SemaphoreObjectType::SemaphoreObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SemaphoreObjectType::GetDetails(HANDLE hSemaphore) {
	CString details;
	NT::SEMAPHORE_BASIC_INFORMATION info;
	if (NT_SUCCESS(NT::NtQuerySemaphore(hSemaphore, NT::SemaphoreBasicInformation, &info, sizeof(info), nullptr))) {
		details.Format(L"Maximum: %d (0x%X), Current: %d (0x%X)", 
			info.MaximumCount, info.MaximumCount, info.CurrentCount, info.CurrentCount);
	}
	return details;
}
