#pragma once

struct HighlightColor {
	HighlightColor(PCWSTR name, COLORREF defaultColor, COLORREF defaultTextColor, bool enabled = true);
	HighlightColor() {}

	CString Name;
	COLORREF DefaultColor;
	COLORREF Color;
	COLORREF DefaultTextColor;
	COLORREF TextColor;
	bool Enabled{ true };
};

