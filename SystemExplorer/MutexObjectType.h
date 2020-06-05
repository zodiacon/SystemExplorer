#pragma once

#include "ObjectType.h"

class MutexObjectType : public ObjectType {
public:
	MutexObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

