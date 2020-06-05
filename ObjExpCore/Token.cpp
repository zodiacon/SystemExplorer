#include "pch.h"
#include "Token.h"

using namespace WinSys;

Token::Token(HANDLE hToken) : _handle(hToken) {
}

Token::Token(DWORD pid, TokenAccessMask access) {
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (hProcess) {
		::OpenProcessToken(hProcess.get(), static_cast<DWORD>(access), _handle.addressof());
	}
}

Token::Token(HANDLE hProcess, TokenAccessMask access) {
	::OpenProcessToken(hProcess, static_cast<DWORD>(access), _handle.addressof());
}

std::unique_ptr<Token> Token::Open(DWORD pid, TokenAccessMask access) {
	HANDLE hToken;
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (hProcess) {
		if (::OpenProcessToken(hProcess.get(), static_cast<DWORD>(access), &hToken))
			return std::make_unique<Token>(hToken);
	}
	return nullptr;
}

bool Token::IsValid() const {
	return _handle != nullptr;
}

bool Token::IsElevated() const {
	ULONG elevated = 0;
	DWORD len;
	::GetTokenInformation(_handle.get(), TokenElevation, &elevated, sizeof(elevated), &len);
	return elevated ? true : false;
}

VirtualizationState Token::GetVirtualizationState() const {
	ULONG virt = 0;
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), TokenVirtualizationAllowed, &virt, sizeof(virt), &len))
		return VirtualizationState::Unknown;

	if (!virt)
		return VirtualizationState::NotAllowed;

	if (::GetTokenInformation(_handle.get(), TokenVirtualizationEnabled, &virt, sizeof(virt), &len))
		return virt ? VirtualizationState::Enabled : VirtualizationState::Disabled;

	return VirtualizationState::Unknown;
}

IntegrityLevel Token::GetIntegrityLevel() const {
	BYTE buffer[TOKEN_INTEGRITY_LEVEL_MAX_SIZE];
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), TokenIntegrityLevel, buffer, sizeof(buffer), &len))
		return IntegrityLevel::Error;

	auto p = (TOKEN_MANDATORY_LABEL*)buffer;
	return (IntegrityLevel)*GetSidSubAuthority(p->Label.Sid, *GetSidSubAuthorityCount(p->Label.Sid) - 1);
}
