#pragma once

#include "ObjectType.h"

class WinSys::ProcessManager;

class WorkerFactoryObjectType :	public ObjectType {
public:
	WorkerFactoryObjectType(WinSys::ProcessManager& pm, int index, PCWSTR name);

	// Inherited via ObjectType
	CString GetDetails(HANDLE hObject) override;

private:
	WinSys::ProcessManager& _pm;
};

