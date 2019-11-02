#pragma once
#include "ObjectType.h"

class WinStationObjectType : public ObjectType {
public:
	WinStationObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	CString GetDetails(HANDLE hObject) override;
};

