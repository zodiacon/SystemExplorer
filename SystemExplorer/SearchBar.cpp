#include "pch.h"
#include "SearchBar.h"

CSearchBar::CSearchBar(ISearchBarNotify* notify) : m_pNotify(notify), m_Edit(this, 1) {
}

void CSearchBar::Reset() {
    m_IsSearching = false;
    GetDlgItem(IDC_CANCEL).EnableWindow(FALSE);
    GetDlgItem(IDC_SEARCH).EnableWindow();
}

LRESULT CSearchBar::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    m_Edit.SubclassWindow(GetDlgItem(IDC_TEXT));

    return 0;
}

LRESULT CSearchBar::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
    return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CSearchBar::OnSearch(WORD, WORD, HWND, BOOL&) {
    CString text;
    GetDlgItemText(IDC_TEXT, text);

    if (m_pNotify) {
        m_pNotify->StartSearch(text);
        m_IsSearching = true;
        GetDlgItem(IDC_CANCEL).EnableWindow();
        GetDlgItem(IDC_SEARCH).EnableWindow(FALSE);
    }

    return 0;
}

LRESULT CSearchBar::OnCancel(WORD, WORD, HWND, BOOL&) {
    ATLASSERT(m_IsSearching);

    return 0;
}

LRESULT CSearchBar::OnTextChanged(WORD, WORD, HWND, BOOL&) {
    GetDlgItem(IDC_SEARCH).EnableWindow(GetDlgItem(IDC_TEXT).GetWindowTextLength() > 0);

    return 0;
}

LRESULT CSearchBar::OnEditKeyDown(UINT, WPARAM wp, LPARAM, BOOL& handled) {
    if (wp == VK_RETURN) {
        PostMessage(WM_COMMAND, MAKEWPARAM(IDC_SEARCH, BN_CLICKED));
        return 0;
    }
    handled = FALSE;
    return 0;
}
