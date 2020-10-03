#pragma once

#include "ProcessColor.h"
#include "StandardColors.h"

class IniFile;

enum class ProcessColorIndex {
	NewObjects,
	DeletedObjects,
	Manageed,
	Immersive,
	Services,
	Protected,
	Secure,
	InJob,
	Wow64,
	COUNT
};

class Settings {
public:
	static Settings& Get();
	bool AlwaysOnTop{ false };
	bool SingleInstanceOnly{ false };
	bool MinimizeToTray{ false };

	struct {
		HighlightColor Colors[(int)ProcessColorIndex::COUNT] {
			HighlightColor(L"New Process", StandardColors::LimeGreen, StandardColors::Black, true),
			HighlightColor(L"Terminated Processes", StandardColors::Red, StandardColors::Black, true),
			HighlightColor(L"Managed (.NET)", StandardColors::Yellow, StandardColors::Black, true),
			HighlightColor(L"Immersive", StandardColors::Cyan, StandardColors::Black, true),
			HighlightColor(L"Services", StandardColors::Pink, StandardColors::Black, true),
			HighlightColor(L"Protected", StandardColors::Fuchsia, StandardColors::Black, true),
			HighlightColor(L"Secure", StandardColors::Purple, StandardColors::White, true),
			HighlightColor(L"In Job", StandardColors::Brown, StandardColors::White, false),
			HighlightColor(L"Wow64", StandardColors::DarkBlue, StandardColors::White, false),
		};
		int UpdateInterval{ 1000 };
	} Processes;

	struct {
		HighlightColor Increase{ L"Increase Objects/Handles", StandardColors::LightGreen, StandardColors::Black };
		HighlightColor Decrease{ L"Decrease Objects/Handles", StandardColors::Red, StandardColors::White };
		int UpdateInterval{ 10000 };		
	} ObjectTypes;

	void SetDefaults();

	void GetCPUColors(int cpu, COLORREF& bk, COLORREF& text);

	bool Save(PCWSTR filename) const;
	bool Load(PCWSTR filename);

	static bool SaveColors(PCWSTR path, PCWSTR prefix, const HighlightColor* colors, int count);
	static bool LoadColors(PCWSTR path, PCWSTR prefix, HighlightColor* colors, int count);

private:
	Settings(Settings const&) = delete;
	Settings(Settings&&) = delete;
	Settings& operator=(Settings const&) = delete;
	Settings& operator=(Settings&&) = delete;
	Settings();
};

