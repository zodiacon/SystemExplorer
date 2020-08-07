#include "pch.h"
#include "ProcessColor.h"

HighlightColor::HighlightColor(PCWSTR name, COLORREF defaultColor, COLORREF defaultTextColor, bool enabled) 
	: Name(name), DefaultColor(defaultColor), Color(defaultColor), 
	DefaultTextColor(defaultTextColor), TextColor(defaultTextColor), Enabled(enabled) {
}
