#pragma once

#include "ObjectType.h"

class EventObjectType :	public ObjectType {
public:
	EventObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

