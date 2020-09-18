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

	enum class SidGroupAttributes : uint32_t {
		Enabled = SE_GROUP_ENABLED,
		DefaultEnabled = SE_GROUP_ENABLED_BY_DEFAULT,
		Integrity = SE_GROUP_INTEGRITY,
		IntegrityEnabled = SE_GROUP_INTEGRITY_ENABLED,
		LogonId = SE_GROUP_LOGON_ID,
		Mandatory = SE_GROUP_MANDATORY,
		Owner = SE_GROUP_OWNER,
		DomainLocal = SE_GROUP_RESOURCE,
		DenyOnly = SE_GROUP_USE_FOR_DENY_ONLY
	};
	DEFINE_ENUM_FLAG_OPERATORS(WinSys::SidGroupAttributes);

	struct TokenGroup {
		std::wstring Name;
		std::wstring Sid;
		SID_NAME_USE Use;
		SidGroupAttributes Attributes;
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
		DWORD GetSessionId() const;
		TOKEN_STATISTICS GetStats() const;
		std::vector<TokenGroup> EnumGroups() const;

	private:
		wil::unique_handle _handle;
	};
}

