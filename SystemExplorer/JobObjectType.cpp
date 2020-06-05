#include "stdafx.h"
#include "JobObjectType.h"

JobObjectType::JobObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString JobObjectType::GetDetails(HANDLE hJob) {
	CString details;
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info;
	if (::QueryInformationJobObject(hJob, JobObjectBasicAccountingInformation, &info, sizeof(info), nullptr)) {
		details.Format(L"Processes: %d, Total Processes: %d, CPU Time: %s", 
			info.ActiveProcesses, info.TotalProcesses, 
			(PCWSTR)CTimeSpan((info.TotalKernelTime.QuadPart + info.TotalUserTime.QuadPart) / 10000000).Format(L"%D:%H:%M:%S"));
	}
	return details;
}
