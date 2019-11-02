#include "stdafx.h"
#include "WinStationObjectType.h"

WinStationObjectType::WinStationObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString WinStationObjectType::GetDetails(HANDLE hObject) {
	CString details;
	::EnumDesktops((HWINSTA)hObject, [](auto name, auto d) {
		*((CString*)d) += CString(", ") + name;
		return TRUE;
		}, reinterpret_cast<LPARAM>(&details));

	if (!details.IsEmpty()) {
		details = L"Desktops: " + details.Mid(2);
	}
	return details;
}
