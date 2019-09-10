#pragma once

#include "ObjectType.h"

class TokenObjectType :	public ObjectType {
public:
	TokenObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

private:
	static CString GetUserName(HANDLE hToken);
	static CString GetLogonSessionId(HANDLE hToken);
	static CString GetIntegirtyLevel(HANDLE hToken);
};

