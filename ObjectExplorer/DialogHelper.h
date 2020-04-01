#pragma once

class DialogHelper abstract {
public:
	static void AdjustOKCancelButtons(CWindow* dlg);
	static bool AddIconToButton(CWindow* dlg, WORD id, WORD icon);
};

