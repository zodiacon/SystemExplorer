#pragma once

#include "ObjectType.h"

class ThreadObjectType : public ObjectType {
public:
	ThreadObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

