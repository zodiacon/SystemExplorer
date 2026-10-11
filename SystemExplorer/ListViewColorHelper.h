#pragma once

#include <WTLHelper.h>

//
// Default row text color for list/tree view custom draw. Returns CLR_INVALID
// (control/theme default) outside dark mode, so light-mode appearance is
// unchanged; in dark mode the control's own default text is unreliable for
// LVS_OWNERDATA views, so an explicit light color is used instead.
//
inline COLORREF GetDefaultTextColor() {
	return WTLHelper::IsDarkMode() ? RGB(255, 255, 255) : CLR_INVALID;
}

//
// Picks black or white text for good contrast against an arbitrary highlight
// background color, based on perceived luminance.
//
//
// A highlight background that works in dark and light mode alike: the window's background tinted toward a color
// (percent: how much of the color; more for short-lived highlights that should stand out). The mode's usual text
// color (COLOR_WINDOWTEXT) reads well on it, and the selection still stands out.
//
inline COLORREF GetTintedBackground(COLORREF color, int darkPercent, int lightPercent) {
	auto back = ::GetSysColor(COLOR_WINDOW);
	int percent = WTLHelper::IsDarkMode() ? darkPercent : lightPercent;
	auto blend = [percent](int c, int b) { return (c * percent + b * (100 - percent)) / 100; };
	return RGB(blend(GetRValue(color), GetRValue(back)), blend(GetGValue(color), GetGValue(back)), blend(GetBValue(color), GetBValue(back)));
}

inline COLORREF GetContrastingTextColor(COLORREF bk) {
	auto luminance = (GetRValue(bk) * 299 + GetGValue(bk) * 587 + GetBValue(bk) * 114) / 1000;
	return luminance > 128 ? RGB(0, 0, 0) : RGB(255, 255, 255);
}
