#pragma once

#include "ObjectType.h"

class ProcessObjectType : public ObjectType {
public:
	ProcessObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

