#include "stdafx.h"
#include "EventObjectType.h"

EventObjectType::EventObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString EventObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
