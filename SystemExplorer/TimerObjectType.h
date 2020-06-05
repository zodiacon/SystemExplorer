#pragma once

#include "ObjectType.h"

class TimerObjectType :	public ObjectType {
public:
	TimerObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

