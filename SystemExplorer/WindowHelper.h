#pragma once

struct WindowHelper abstract final {
	static CString WindowStyleToString(HWND hWnd);
	static CString WindowExtendedStyleToString(HWND hWnd);
	static CString WindowRectToString(HWND hWnd);
	// owned: set to true if the icon is the caller's to destroy (extracted from the executable), false if it's the
	// window's own icon (which must not be destroyed)
	static HICON GetWindowOrProcessIcon(HWND hWnd, bool* owned = nullptr);
};

