#pragma once

#include "ObjectType.h"

class DeviceObjectType : public ObjectType {
public:
	DeviceObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

