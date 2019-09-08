#include "stdafx.h"
#include "SymbolicLinkObjectType.h"
#include "NtDll.h"

SymbolicLinkObjectType::SymbolicLinkObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SymbolicLinkObjectType::GetDetails(HANDLE hLink) {
	WCHAR buffer[1024];
	UNICODE_STRING str;
	str.MaximumLength = sizeof(buffer);
	str.Buffer = buffer;
	if (NT_SUCCESS(NT::NtQuerySymbolicLinkObject(hLink, &str, nullptr)))
		return CString(buffer, str.Length / sizeof(WCHAR));
	return L"";
}
