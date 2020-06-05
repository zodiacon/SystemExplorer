#pragma once

#include "ObjectType.h"

class JobObjectType : public ObjectType {
public:
	JobObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

