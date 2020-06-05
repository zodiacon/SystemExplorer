#pragma once

#include "ObjectType.h"

class FileObjectType : public ObjectType {
public:
	FileObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;
};

