#include "pch.h"
#include "TimerObjectType.h"
#include "NtDll.h"

TimerObjectType::TimerObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString TimerObjectType::GetDetails(HANDLE hTimer) {
	CString details;
	NT::TIMER_BASIC_INFORMATION bi;
	if (NT_SUCCESS(NT::NtQueryTimer(hTimer, NT::TimerBasicInformation, &bi, sizeof(bi), nullptr))) {
		if (bi.TimerState != 0)
			details.Format(L"State: %d, Remaining: %lld msec", (int)bi.TimerState, bi.RemainingTime.QuadPart / 10000);
		else
			details.Format(L"State: %d", (int)bi.TimerState);
	}

	return details;
}
