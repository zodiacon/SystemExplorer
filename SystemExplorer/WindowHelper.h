#pragma once

struct WindowHelper abstract final {
	static CString WindowStyleToString(HWND hWnd);
	static CString WindowExtendedStyleToString(HWND hWnd);
	static CString WindowRectToString(HWND hWnd);
};

