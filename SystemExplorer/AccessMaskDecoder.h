#pragma once

class AccessMaskDecoder abstract final {
public:
	static CString DecodeAccessMask(PCWSTR typeName, ACCESS_MASK access);

private:
	struct AccessMaskPair {
		DWORD AccessMask;
		PCWSTR Decoded;
		bool All{ false };
	};

	static std::unordered_map<std::wstring, std::vector<AccessMaskPair>> Tables;
};

