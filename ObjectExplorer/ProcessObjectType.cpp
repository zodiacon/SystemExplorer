#include "stdafx.h"
#include "ProcessObjectType.h"


ProcessObjectType::ProcessObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString ProcessObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
