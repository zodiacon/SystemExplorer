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
	int DarkMode{ -1 };		// 1 dark, 0 light, -1 (never chosen): as the system is

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
		// the same, for dark mode: muted backgrounds with light text, so highlighted rows don't glare against the
		// dark list and stay distinguishable from each other and from the selection
		HighlightColor DarkColors[(int)ProcessColorIndex::COUNT] {
			HighlightColor(L"New Process", RGB(30, 110, 45), StandardColors::White, true),
			HighlightColor(L"Terminated Processes", RGB(150, 35, 35), StandardColors::White, true),
			HighlightColor(L"Managed (.NET)", RGB(110, 100, 15), StandardColors::White, true),
			HighlightColor(L"Immersive", RGB(0, 100, 115), StandardColors::White, true),
			HighlightColor(L"Services", RGB(125, 60, 90), StandardColors::White, true),
			HighlightColor(L"Protected", RGB(130, 35, 130), StandardColors::White, true),
			HighlightColor(L"Secure", RGB(85, 45, 140), StandardColors::White, true),
			HighlightColor(L"In Job", RGB(115, 75, 40), StandardColors::White, false),
			HighlightColor(L"Wow64", RGB(35, 60, 135), StandardColors::White, false),
		};
		int UpdateInterval{ 1000 };
	} Processes;

	// the process colors for the current mode (dark or light), ProcessColorIndex::COUNT of them
	HighlightColor* GetProcessColors();

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

