#pragma once

#include "ObjectType.h"

class TokenObjectType :	public ObjectType {
public:
	TokenObjectType(int index, PCWSTR name);

	// Inherited via ObjectType
	virtual CString GetDetails(HANDLE hObject) override;

	static CString GetUserName(HANDLE hToken);
	static CString GetLogonSessionId(HANDLE hToken, const CString& prefix);
	static CString GetIntegirtyLevel(HANDLE hToken, const CString& prefix);
};

