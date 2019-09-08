#pragma once

#include "ObjectType.h"

class KeyObjectType : public ObjectType {
public:
	KeyObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

};

