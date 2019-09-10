#include "stdafx.h"
#include "TokenObjectType.h"

TokenObjectType::TokenObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString TokenObjectType::GetDetails(HANDLE hToken) {
	return GetUserName(hToken) + L", " + GetLogonSessionId(hToken) + L", " + GetIntegirtyLevel(hToken);
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
			details = L"User: " + CString(domain) + L"\\" + username;
	}
	return details;
}

CString TokenObjectType::GetLogonSessionId(HANDLE hToken) {
	DWORD id = 0, len;
	CString details;
	if (::GetTokenInformation(hToken, TokenSessionId, &id, sizeof(id), &len)) {
		details.Format(L"Session: %d", id);
	}
	return details;
}

CString TokenObjectType::GetIntegirtyLevel(HANDLE hToken) {
	BYTE buffer[TOKEN_INTEGRITY_LEVEL_MAX_SIZE];
	DWORD len;
	CString details;
	if (::GetTokenInformation(hToken, TokenIntegrityLevel, buffer, sizeof(buffer), &len)) {
		auto label = (TOKEN_MANDATORY_LABEL*)buffer;
		auto sub = ::GetSidSubAuthority(label->Label.Sid, 0);
		switch (*sub) {
			case SECURITY_MANDATORY_SYSTEM_RID: details = L"System"; break;
			case SECURITY_MANDATORY_HIGH_RID: details = L"High"; break;
			case SECURITY_MANDATORY_MEDIUM_RID: details = L"Medium"; break;
			case SECURITY_MANDATORY_MEDIUM_PLUS_RID: details = L"Medium Plus"; break;
			case SECURITY_MANDATORY_LOW_RID: details = L"Low"; break;
			case SECURITY_MANDATORY_UNTRUSTED_RID: details = L"Untrusted"; break;
			default: details = L"Unknown";
		}
	}

	return L"Integrity: " + details;
}
