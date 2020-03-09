#pragma once

#include "ObjectType.h"

class WorkerFactoryObjectType :	public ObjectType {
public:
	WorkerFactoryObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	CString GetDetails(HANDLE hObject) override;
};

