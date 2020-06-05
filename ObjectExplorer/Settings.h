#pragma once

#include "ProcessColor.h"
#include "StandardColors.h"

class Settings {
public:
	Settings();

	bool AlwaysOnTop{ false };
	struct {
		ProcessColor Colors[(int)ProcessColorIndex::COUNT] {
			ProcessColor(L"New Process", StandardColors::Green, StandardColors::White, true),
			ProcessColor(L"Terminated Processes", StandardColors::Red, StandardColors::Black, true),
			ProcessColor(L"Managed (.NET)", StandardColors::Yellow, StandardColors::Black, true),
			ProcessColor(L"Immersive", StandardColors::Cyan, StandardColors::Black, true),
			ProcessColor(L"Services", StandardColors::Pink, StandardColors::Black, true),
			ProcessColor(L"Protected", StandardColors::Fuchsia, StandardColors::Black, true),
			ProcessColor(L"Secure", StandardColors::Purple, StandardColors::White, true),
			ProcessColor(L"In Job", StandardColors::Brown, StandardColors::White, false),
		};
	} Processes;

	void SetDefaults();
};

