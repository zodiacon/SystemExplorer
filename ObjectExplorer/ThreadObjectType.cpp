#include "stdafx.h"
#include "ThreadObjectType.h"

ThreadObjectType::ThreadObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString ThreadObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
