#pragma once

#include "ObjectType.h"

class SymbolicLinkObjectType : public ObjectType {
public:
	SymbolicLinkObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

