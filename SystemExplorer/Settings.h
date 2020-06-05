#pragma once

#include "ProcessColor.h"
#include "StandardColors.h"

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

class Settings {
public:
	Settings();

	bool AlwaysOnTop{ false };
	struct {
		HighlightColor Colors[(int)ProcessColorIndex::COUNT] {
			HighlightColor(L"New Process", StandardColors::Green, StandardColors::White, true),
			HighlightColor(L"Terminated Processes", StandardColors::Red, StandardColors::Black, true),
			HighlightColor(L"Managed (.NET)", StandardColors::Yellow, StandardColors::Black, true),
			HighlightColor(L"Immersive", StandardColors::Cyan, StandardColors::Black, true),
			HighlightColor(L"Services", StandardColors::Pink, StandardColors::Black, true),
			HighlightColor(L"Protected", StandardColors::Fuchsia, StandardColors::Black, true),
			HighlightColor(L"Secure", StandardColors::Purple, StandardColors::White, true),
			HighlightColor(L"In Job", StandardColors::Brown, StandardColors::White, false),
		};
		int UpdateInterval{ 1000 };
	} Processes;

	struct {
		HighlightColor Increase{ L"Increase Objects/Handles", StandardColors::LightGreen, StandardColors::Black };
		HighlightColor Decrease{ L"Decrease Objects/Handles", StandardColors::Red, StandardColors::White };
		int UpdateInterval{ 10000 };		
	} ObjectTypes;

	void SetDefaults();
};

