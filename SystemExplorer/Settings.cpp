#include "pch.h"
#include "Settings.h"
#include "IniFile.h"

Settings::Settings() {
}

Settings& Settings::Get() {
	static Settings settings;
	return settings;
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

bool Settings::Save(PCWSTR filename) const {
	IniFile file(filename);

	file.WriteBool(L"Options", L"AlwaysOnTop", AlwaysOnTop);
	file.WriteBool(L"Options", L"SingleInstance", SingleInstanceOnly);
	file.WriteBool(L"Options", L"MinimizeToTray", MinimizeToTray);
	file.WriteInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);
	
	return SaveColors(filename, L"ProcessColors", Processes.Colors, _countof(Processes.Colors));
}

bool Settings::Load(PCWSTR filename) {
	IniFile file(filename);
	if(!file.IsValid())
		return false;

	AlwaysOnTop = file.ReadBool(L"Options", L"AlwaysOnTop");
	SingleInstanceOnly = file.ReadBool(L"Options", L"SingleInstance");
	MinimizeToTray = file.ReadBool(L"Options", L"MinimizeToTray");
	Processes.UpdateInterval = file.ReadInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);

	return LoadColors(filename, L"ProcessColor", Processes.Colors, _countof(Processes.Colors));
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

