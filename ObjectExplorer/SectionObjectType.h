#pragma once

#include "ObjectType.h"

class SectionObjectType : public ObjectType {
public:
	SectionObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

