#include "pch.h"
#include "EventObjectType.h"
#include "NtDll.h"

EventObjectType::EventObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString EventObjectType::GetDetails(HANDLE hEvent) {
	NT::EVENT_BASIC_INFORMATION info;
	CString details;
	if (NT_SUCCESS(NT::NtQueryEvent(hEvent, NT::EventBasicInformation, &info, sizeof(info), nullptr))) {
		details.Format(L"Type: %s, Signaled: %s", info.EventType == NT::SynchronizationEvent ? L"Synchronization" : L"Notification",
			info.EventState ? L"True" : L"False");
	}

	return details;
}
