#pragma once

#include "ObjectType.h"
#include <ProcessManager.h>

class ObjectManager;

class ProcessObjectType : public ObjectType {
public:
	ProcessObjectType(const WinSys::ProcessManager& pm, int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

private:
	const WinSys::ProcessManager& _pm;
};

