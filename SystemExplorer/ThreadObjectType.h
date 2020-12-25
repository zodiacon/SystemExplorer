#pragma once

#include "ObjectType.h"
#include <ProcessManager.h>

class ThreadObjectType : public ObjectType {
public:
	ThreadObjectType(const WinSys::ProcessManager& pm, int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

private:
	const WinSys::ProcessManager& _pm;
};

