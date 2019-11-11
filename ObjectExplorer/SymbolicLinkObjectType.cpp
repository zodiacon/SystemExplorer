#include "stdafx.h"
#include "SymbolicLinkObjectType.h"
#include "NtDll.h"

SymbolicLinkObjectType::SymbolicLinkObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SymbolicLinkObjectType::GetDetails(HANDLE hLink) {
	WCHAR buffer[1024];
	CString details;
	NT::OBJECT_BASIC_INFORMATION bi{};
	NT::NtQueryObject(hLink, NT::ObjectBasicInformation, &bi, sizeof(bi), nullptr);
	UNICODE_STRING str;
	str.MaximumLength = sizeof(buffer);
	str.Buffer = buffer;
	if (NT_SUCCESS(NT::NtQuerySymbolicLinkObject(hLink, &str, nullptr)))
		details = L"Target: " + CString(buffer, str.Length / sizeof(WCHAR));
	if (bi.CreationTime.QuadPart > 0)
		details += L", Created: " + CTime(*(FILETIME*)&bi.CreationTime.QuadPart).Format(L"%D %X");

	return details;
}
