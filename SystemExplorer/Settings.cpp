#include "pch.h"
#include "Settings.h"
#include "IniFile.h"
#include <WTLHelper.h>

Settings::Settings() {
}

Settings& Settings::Get() {
	static Settings settings;
	return settings;
}

void Settings::SetDefaults() {
	AlwaysOnTop = false;
}

HighlightColor* Settings::GetProcessColors() {
	return WTLHelper::IsDarkMode() ? Processes.DarkColors : Processes.Colors;
}

void Settings::GetCPUColors(int cpu, COLORREF& bk, COLORREF& text) {
	// The brighter stops in this gradient (orange/yellow/light blue/cyan) are
	// too glaring against a dark background, so they get muted, darker
	// equivalents in dark mode; DarkRed/DarkBlue/Black are already dark enough
	// to leave alone.
	auto dark = WTLHelper::IsDarkMode();
	if (cpu > 90) {
		bk = StandardColors::Black;
		text = StandardColors::White;
	}
	else if (cpu > 80) {
		bk = StandardColors::DarkRed;
		text = StandardColors::White;
	}
	else if (cpu > 70) {
		bk = dark ? RGB(140, 80, 0) : StandardColors::DarkOrange;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 60) {
		bk = dark ? RGB(150, 95, 0) : StandardColors::Orange;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 50) {
		bk = dark ? RGB(120, 120, 0) : StandardColors::Yellow;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 40) {
		bk = StandardColors::DarkBlue;
		text = StandardColors::White;
	}
	else if (cpu > 30) {
		bk = dark ? RGB(0, 0, 140) : StandardColors::Blue;
		text = StandardColors::White;
	}
	else if (cpu > 20) {
		bk = dark ? RGB(50, 80, 100) : StandardColors::LightBlue;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 15) {
		bk = dark ? RGB(0, 110, 110) : StandardColors::Cyan;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 10) {
		bk = dark ? RGB(60, 110, 110) : StandardColors::LightCyan;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
	else if (cpu > 5) {
		bk = dark ? RGB(20, 90, 20) : StandardColors::LimeGreen;
		text = dark ? StandardColors::White : StandardColors::Black;
	}
}

bool Settings::Save(PCWSTR filename) const {
	IniFile file(filename);

	file.WriteBool(L"Options", L"AlwaysOnTop", AlwaysOnTop);
	file.WriteBool(L"Options", L"SingleInstance", SingleInstanceOnly);
	file.WriteBool(L"Options", L"MinimizeToTray", MinimizeToTray);
	file.WriteInt(L"Options", L"DarkMode", DarkMode);
	file.WriteInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);
	
	return SaveColors(filename, L"ProcessColors", Processes.Colors, _countof(Processes.Colors))
		&& SaveColors(filename, L"ProcessColorsDark", Processes.DarkColors, _countof(Processes.DarkColors));
}

bool Settings::Load(PCWSTR filename) {
	IniFile file(filename);
	if(!file.IsValid())
		return false;

	AlwaysOnTop = file.ReadBool(L"Options", L"AlwaysOnTop");
	SingleInstanceOnly = file.ReadBool(L"Options", L"SingleInstance");
	MinimizeToTray = file.ReadBool(L"Options", L"MinimizeToTray");
	DarkMode = file.ReadInt(L"Options", L"DarkMode", -1);
	Processes.UpdateInterval = file.ReadInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);

	// a settings file from before there were dark colors has none; the dark defaults stay then
	LoadColors(filename, L"ProcessColorsDark", Processes.DarkColors, _countof(Processes.DarkColors));
	return LoadColors(filename, L"ProcessColors", Processes.Colors, _countof(Processes.Colors));
}

bool Settings::SaveColors(PCWSTR path, PCWSTR prefix, const HighlightColor* colors, int count) {
	IniFile file(path);
	CString text;
	for (int i = 0; i < count; i++) {
		text.Format(L"%s%d", prefix, i);
		auto& info = colors[i];
		if (!file.WriteColor(text, L"Color", info.Color))
			return false;
		file.WriteColor(text, L"TextColor", info.TextColor);
		file.WriteBool(text, L"Enabled", info.Enabled);
		file.WriteString(text, L"Name", info.Name);
	}
	return true;
}

bool Settings::LoadColors(PCWSTR path, PCWSTR prefix, HighlightColor* colors, int count) {
	IniFile file(path);
	if (!file.IsValid())
		return false;
	
	CString text;
	for (int i = 0; i < count; i++) {
		text.Format(L"%s%d", prefix, i);
		auto& info = colors[i];
		info.Color = file.ReadColor(text, L"Color", info.Color);
		info.TextColor = file.ReadColor(text, L"TextColor", info.TextColor);
		info.Enabled = file.ReadBool(text, L"Enabled", info.Enabled);
		info.Name = file.ReadString(text, L"Name", info.Name);
	}
	return true;
}

