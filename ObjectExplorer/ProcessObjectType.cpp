#include "stdafx.h"
#include "ProcessObjectType.h"
#include "ProcessHelper.h"
#include "ObjectManager.h"

ProcessObjectType::ProcessObjectType(const ObjectManager& om, int index, PCWSTR name) : 
	ObjectType(index, name), _om(om) {
}

CString ProcessObjectType::GetDetails(HANDLE hProcess) {
	CString details;
	auto pid = ::GetProcessId(hProcess);
	auto name = ProcessHelper::GetProcessName(hProcess);
	if (name.IsEmpty())
		name = _om.GetProcessNameById(pid);
	FILETIME create, exit{}, dummy;
	if (::GetProcessTimes(hProcess, &create, &exit, &dummy, &dummy)) {
		details.Format(L"PID: %d (%s) Created: %s Exited: %s", pid, name,
			CTime(create).Format(L"%D %X"),
			exit.dwHighDateTime + exit.dwLowDateTime == 0 ? L"(running)" : CTime(exit).Format(L"%D %X"));
	}
	else {
		details.Format(L"PID: %d (%s)", pid, name);
	}
	return details;
}
