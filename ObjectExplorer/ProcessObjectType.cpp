#include "stdafx.h"
#include "ProcessObjectType.h"
#include "ProcessHelper.h"

ProcessObjectType::ProcessObjectType(const WinSys::ProcessManager& pm, int index, PCWSTR name) :
	ObjectType(index, name), _pm(pm) {
}

CString ProcessObjectType::GetDetails(HANDLE hProcess) {
	CString details;
	auto pid = ::GetProcessId(hProcess);
	auto name = ProcessHelper::GetProcessName(hProcess);
	if (name.IsEmpty()) {
		auto info = _pm.GetProcessById(pid);
		if (info)
			name = info->GetImageName().c_str();
	}
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
