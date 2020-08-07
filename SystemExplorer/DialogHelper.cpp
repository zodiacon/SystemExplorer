#include "pch.h"
#include "DialogHelper.h"
#include "resource.h"

void DialogHelper::AdjustOKCancelButtons(CWindow* dlg) {
	CButton ok(dlg->GetDlgItem(IDOK));
	if (ok) {
		ok.SetWindowText(L"  OK");
		ok.SetIcon(AtlLoadIconImage(IDI_OK, 0, 16, 16));
	}

	CButton cancel(dlg->GetDlgItem(IDCANCEL));
	if (cancel) {
		cancel.SetWindowText(L"  Cancel");
		cancel.SetIcon(AtlLoadIconImage(IDI_DELETE, 0, 16, 16));
	}
}

bool DialogHelper::AddIconToButton(CWindow* dlg, WORD id, WORD icon) {
	CButton button(dlg->GetDlgItem(id));
	if (button) {
		button.SetIcon(AtlLoadIconImage(icon, 0, 16, 16));
		CString text;
		button.GetWindowText(text);
		button.SetWindowText(L"  " + text);
	}
	return (bool)button;
}
