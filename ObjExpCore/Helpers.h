#pragma once

namespace WinSys {
	struct Helpers final abstract {
		static std::wstring GetDosNameFromNtName(PCWSTR name);
	};
}
