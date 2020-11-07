#include "pch.h"
#include "DeviceObjectType.h"

DeviceObjectType::DeviceObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString DeviceObjectType::GetDetails(HANDLE hObject) {
    return L"";
}
