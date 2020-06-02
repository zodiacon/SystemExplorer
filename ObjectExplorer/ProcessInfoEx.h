#pragma once

enum class ProcessAttributes {
	NotComputed = -1,
	None = 0,
	Protected = 1,
	InJob = 2,
	Service = 4,
	Managed = 8,
	Secure = 0x10,
	Immersive = 0x20,
};
DEFINE_ENUM_FLAG_OPERATORS(ProcessAttributes);

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi);
	ProcessAttributes GetAttributes(const WinSys::ProcessManager& pm) const;
	const std::wstring& GetExecutablePath() const;
	const std::wstring& UserName() const;
	int GetImageIndex(CImageList images) const;

	DWORD64 TargetTime;
	bool IsNew{ false };
	bool IsTerminated{ false };

private:
	std::unique_ptr<WinSys::Process> _process;
	WinSys::ProcessInfo* _pi;
	mutable int _image = -1;
	mutable ProcessAttributes _attributes = ProcessAttributes::NotComputed;
	mutable std::wstring _executablePath;
	mutable std::wstring _username;
};

