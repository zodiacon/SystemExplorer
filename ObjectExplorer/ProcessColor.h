#pragma once

enum class ProcessColorIndex {
	NewObjects,
	DeletedObjects,
	Manageed,
	Immersive,
	Services,
	Protected,
	Secure,
	InJob,
	COUNT
};

struct ProcessColor {
	ProcessColor(PCWSTR name, COLORREF defaultColor, COLORREF defaultTextColor, bool enabled = true);
	ProcessColor() {}

	CString Name;
	COLORREF DefaultColor;
	COLORREF Color;
	COLORREF DefaultTextColor;
	COLORREF TextColor;
	bool Enabled{ true };
};

