#include "stdafx.h"
#include "TokenObjectType.h"

TokenObjectType::TokenObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString TokenObjectType::GetDetails(HANDLE hToken) {
	return GetUserName(hToken);
}

CString TokenObjectType::GetUserName(HANDLE hToken) {
	CString details;
	BYTE buffer[256];
	DWORD len;
	if (::GetTokenInformation(hToken, TokenUser, buffer, sizeof(buffer), &len)) {
		auto user = (TOKEN_USER*)buffer;
		WCHAR username[128], domain[128];
		DWORD usernameLen = _countof(username), domainLen = _countof(domain);
		SID_NAME_USE use;
		if (::LookupAccountSid(nullptr, user->User.Sid, username, &usernameLen,
			domain, &domainLen, &use))
			details = CString(domain) + L"\\" + username;
	}
	return details;
}
