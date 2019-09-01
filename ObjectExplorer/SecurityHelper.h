#pragma once

struct SecurityHelper final {
	static bool IsRunningElevated();
	static HICON GetShieldIcon();
};

