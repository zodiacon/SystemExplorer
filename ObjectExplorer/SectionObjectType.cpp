#include "stdafx.h"
#include "SectionObjectType.h"


SectionObjectType::SectionObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SectionObjectType::GetDetails(HANDLE hObject) {
	return CString();
}
