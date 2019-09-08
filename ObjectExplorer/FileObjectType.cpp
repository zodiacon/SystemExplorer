#include "stdafx.h"
#include "FileObjectType.h"

FileObjectType::FileObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString FileObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
