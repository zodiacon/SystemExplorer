#include "stdafx.h"
#include "SemaphoreObjectType.h"



SemaphoreObjectType::SemaphoreObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SemaphoreObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
