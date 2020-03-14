#include "stdafx.h"
#include "ThreadObjectType.h"

ThreadObjectType::ThreadObjectType(const WinSys::ProcessManager& pm, int index, PCWSTR name) :
	ObjectType(index, name), _pm(pm) {
}

CString ThreadObjectType::GetDetails(HANDLE hThread) {
	CString details;
	auto tid = ::GetThreadId(hThread);
	if (tid == 0)
		return details;

	auto pid = ::GetProcessIdOfThread(hThread);
	FILETIME created{}, exited{}, kernel{}, user{};
	ATLVERIFY(::GetThreadTimes(hThread, &created, &exited, &kernel, &user));
	CString name;
	auto info = _pm.GetProcessById(pid);
	if (info)
		name = info->GetImageName().c_str();
	details.Format(L"TID: %d, PID: %d (%s) Created: %s, Exited: %s, CPU Time: %s",
		tid, pid, name,	CTime(created).Format(L"%D %X"),
		exited.dwHighDateTime + exited.dwLowDateTime == 0 ? L"(running)" : CTime(exited).Format(L"%D %X"),
		CTimeSpan((*(int64_t*)& kernel + *(int64_t*)& user) / 10000000).Format(L"%D:%H:%M:%S"));

	return details;
}
