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

std::unique_ptr<Token> Token::Open(DWORD pid, TokenAccessMask access) {
	HANDLE hToken;
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (hProcess) {
		if (::OpenProcessToken(hProcess.get(), static_cast<DWORD>(access), &hToken))
			return std::make_unique<Token>(hToken);
	}
	return nullptr;
}

bool WinSys::Token::IsValid() const {
	return _handle != nullptr;
}
