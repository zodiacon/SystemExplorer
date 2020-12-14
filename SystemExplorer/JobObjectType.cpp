#include "pch.h"
#include "JobObjectType.h"

JobObjectType::JobObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString JobObjectType::GetDetails(HANDLE hJob) {
	CString details;
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info;
	SILOOBJECT_BASIC_INFORMATION silo;
	CString ssilo;
	if (::QueryInformationJobObject(hJob, JobObjectSiloBasicInformation, &silo, sizeof(silo), nullptr))
		ssilo.Format(L"(%s Silo) ", silo.IsInServerSilo ? L"Server" : L"App");

	if (::QueryInformationJobObject(hJob, JobObjectBasicAccountingInformation, &info, sizeof(info), nullptr)) {
		details.Format(L"%sProcesses: %d, Total Processes: %d, CPU Time: %s", (PCWSTR)ssilo,
			info.ActiveProcesses, info.TotalProcesses, 
			(PCWSTR)CTimeSpan((info.TotalKernelTime.QuadPart + info.TotalUserTime.QuadPart) / 10000000).Format(L"%D:%H:%M:%S"));
	}
	return details;
}
