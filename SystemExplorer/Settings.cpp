#include "pch.h"
#include "Settings.h"
#include "IniFile.h"

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

bool Settings::Save(PCWSTR filename) const {
	IniFile file(filename);

	file.WriteBool(L"Options", L"AlwaysOnTop", AlwaysOnTop);
	file.WriteBool(L"Options", L"SingleInstance", SingleInstanceOnly);
	file.WriteInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);
	
	for (int i = 0; i < _countof(Processes.Colors); i++) {
		WriteProcessColor(file, i);
	}

	return true;
}

bool Settings::Load(PCWSTR filename) {
	IniFile file(filename);
	if(!file.IsValid())
		return false;

	AlwaysOnTop = file.ReadBool(L"Options", L"AlwaysOnTop");
	SingleInstanceOnly = file.ReadBool(L"Options", L"SingleInstance");
	Processes.UpdateInterval = file.ReadInt(L"ProcessOptions", L"Interval", Processes.UpdateInterval);

	for (int i = 0; i < _countof(Processes.Colors); i++) {
		ReadProcessColor(file, i);
	}
	return true;
}

void Settings::WriteProcessColor(IniFile& file, int i) const {
	CString text;
	text.Format(L"ProcessColor%d", i);
	auto& info = Processes.Colors[i];
	file.WriteColor(text, L"Color", info.Color);
	file.WriteColor(text, L"TextColor", info.TextColor);
	file.WriteBool(text, L"Enabled", info.Enabled);
	file.WriteString(text, L"Name", info.Name);
}

void Settings::ReadProcessColor(IniFile& file, int i) {
	CString text;
	text.Format(L"ProcessColor%d", i);
	auto& info = Processes.Colors[i];
	info.Color = file.ReadColor(text, L"Color", info.Color);
	info.TextColor = file.ReadColor(text, L"TextColor", info.TextColor);
	info.Enabled = file.ReadBool(text, L"Enabled", info.Enabled);
	info.Name = file.ReadString(text, L"Name", info.Name);
}
