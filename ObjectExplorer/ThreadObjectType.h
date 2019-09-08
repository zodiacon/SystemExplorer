#pragma once

#include "ObjectType.h"

class ObjectManager;

class ThreadObjectType : public ObjectType {
public:
	ThreadObjectType(const ObjectManager& om, int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

private:
	const ObjectManager& _om;
};

