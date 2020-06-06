#include "stdafx.h"
#include "Settings.h"

Settings::Settings() {
}

void Settings::SetDefaults() {
	AlwaysOnTop = false;
}

void Settings::GetCPUColors(int cpu, COLORREF& bk, COLORREF& text) {
	if (cpu > 90) {
		bk = StandardColors::Black;
		text = StandardColors::White;
	}
	else if (cpu > 80) {
		bk = StandardColors::DarkRed;
		text = StandardColors::White;
	}
	else if (cpu > 70) {
		bk = StandardColors::DarkOrange;
		text = StandardColors::Black;
	}
	else if (cpu > 60) {
		bk = StandardColors::Orange;
		text = StandardColors::Black;
	}
	else if (cpu > 50) {
		bk = StandardColors::Yellow;
		text = StandardColors::Black;
	}
	else if (cpu > 40) {
		bk = StandardColors::DarkBlue;
		text = StandardColors::White;
	}
	else if (cpu > 30) {
		bk = StandardColors::Blue;
		text = StandardColors::White;
	}
	else if (cpu > 20) {
		bk = StandardColors::LightBlue;
		text = StandardColors::Black;
	}
	else if (cpu > 15) {
		bk = StandardColors::Cyan;
		text = StandardColors::Black;
	}
	else if (cpu > 10) {
		bk = StandardColors::LightCyan;
		text = StandardColors::Black;
	}
	else if (cpu > 5) {
		bk = StandardColors::LimeGreen;
		text = StandardColors::Black;
	}
}
