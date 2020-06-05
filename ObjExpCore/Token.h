#pragma once

#include "Sid.h"
#include "Processes.h"

namespace WinSys {
	enum class VirtualizationState {
		Unknown,
		NotAllowed,
		Enabled,
		Disabled
	};

	enum class TokenAccessMask : uint32_t {
		Query = TOKEN_QUERY,
		QuerySource = TOKEN_QUERY_SOURCE,
		AssignPrimary = TOKEN_ASSIGN_PRIMARY,
		AdjustPrivileges = TOKEN_ADJUST_PRIVILEGES,
		AdjustDefault = TOKEN_ADJUST_DEFAULT,
		AdjustGroups = TOKEN_ADJUST_GROUPS,
		AdjustSessionId = TOKEN_ADJUST_SESSIONID,
	};

	class Token final {
	public:
		explicit Token(HANDLE hToken);
		Token(DWORD pid, TokenAccessMask access);
		Token(HANDLE hProcess, TokenAccessMask access);
		static std::unique_ptr<Token> Open(DWORD pid, TokenAccessMask access = TokenAccessMask::Query);

		std::pair<std::wstring, Sid> GetUserNameAndSid() const;

		bool IsValid() const;

		bool IsElevated() const;
		VirtualizationState GetVirtualizationState() const;
		IntegrityLevel GetIntegrityLevel() const;

	private:
		wil::unique_handle _handle;
	};
}

