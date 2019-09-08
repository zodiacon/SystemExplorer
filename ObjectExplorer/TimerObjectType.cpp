#include "stdafx.h"
#include "TimerObjectType.h"

TimerObjectType::TimerObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString TimerObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
