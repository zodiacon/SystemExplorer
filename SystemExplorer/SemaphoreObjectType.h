#pragma once

#include "ObjectType.h"

class SemaphoreObjectType :	public ObjectType {
public:
	SemaphoreObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

