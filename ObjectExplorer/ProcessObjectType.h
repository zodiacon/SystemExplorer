#pragma once

#include "ObjectType.h"

class ObjectManager;

class ProcessObjectType : public ObjectType {
public:
	ProcessObjectType(const ObjectManager& om, int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

private:
	const ObjectManager& _om;
};

