#include "stdafx.h"
#include "ThreadObjectType.h"
#include "ObjectManager.h"

ThreadObjectType::ThreadObjectType(const ObjectManager& om, int index, PCWSTR name) : 
	ObjectType(index, name), _om(om) {
}

CString ThreadObjectType::GetDetails(HANDLE hThread) {
	CString details;
	auto tid = ::GetThreadId(hThread);
	if (tid == 0)
		return details;

	auto pid = ::GetProcessIdOfThread(hThread);
	FILETIME created{}, exited{}, kernel{}, user{};
	ATLVERIFY(::GetThreadTimes(hThread, &created, &exited, &kernel, &user));
	details.Format(L"TID: %d, PID: %d (%s) Created: %s, Exited: %s, CPU Time: %s",
		tid, pid, _om.GetProcessNameById(pid),
		CTime(created).Format(L"%D %X"),
		exited.dwHighDateTime + exited.dwLowDateTime == 0 ? L"(running)" : CTime(exited).Format(L"%D %X"),
		CTimeSpan((*(int64_t*)& kernel + *(int64_t*)& user) / 10000000).Format(L"%D:%H:%M:%S"));

	return details;

}
