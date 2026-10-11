#include "pch.h"
#include <IconHelper.h>
#include "DialogHelper.h"
#include "resource.h"

void DialogHelper::AdjustOKCancelButtons(CWindow* dlg) {
	CButton ok(dlg->GetDlgItem(IDOK));
	if (ok) {
		CString text;
		ok.GetWindowText(text);
		ok.SetWindowText(L"  " + text);
		// cached icons: a button or dialog doesn't destroy its icon, so loading one per dialog leaked it
		ok.SetIcon(IconHelper::LoadCached(IDI_OK, 16));
	}

	CButton cancel(dlg->GetDlgItem(IDCANCEL));
	if (cancel) {
		cancel.SetWindowText(L"  Cancel");
		cancel.SetIcon(IconHelper::LoadCached(IDI_DELETE, 16));
	}
}

bool DialogHelper::AddIconToButton(CWindow* dlg, WORD id, WORD icon, int size) {
	CButton button(dlg->GetDlgItem(id));
	if (button) {
		button.SetIcon(IconHelper::LoadCached(icon, size));
		CString text;
		button.GetWindowText(text);
		button.SetWindowText(L"  " + text);
	}
	return (bool)button;
}

void DialogHelper::SetDialogIcon(CWindow* dlg, UINT icon) {
	dlg->SetIcon(IconHelper::LoadCached(icon, 16), FALSE);
	dlg->SetIcon(IconHelper::LoadCached(icon, 32), TRUE);
}

void DialogHelper::SetDialogIcon(CWindow* dlg, HICON hIcon) {
	dlg->SetIcon(hIcon, FALSE);
	dlg->SetIcon(hIcon, TRUE);
}
