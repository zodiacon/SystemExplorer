#include "pch.h"
#include "ColorsSelectionDlg.h"
#include "DialogHelper.h"
#include "Settings.h"

static WCHAR iniFileFilter[] = L"Ini files (*.ini)\0*.ini\0All Files\0*.*\0";

CColorsSelectionDlg::CColorsSelectionDlg(HighlightColor* colors, int count) : m_Colors(colors, colors + count), m_CountColors(count) {
    ATLASSERT(colors);
}

const HighlightColor* CColorsSelectionDlg::GetColors() const {
    return m_Colors.data();
}

DWORD CColorsSelectionDlg::OnPrePaint(int, LPNMCUSTOMDRAW) {
    return CDRF_NOTIFYITEMDRAW;
}

DWORD CColorsSelectionDlg::OnPreErase(int, LPNMCUSTOMDRAW cd) {
    auto id = cd->hdr.idFrom;
    if (id >= IDC_ENABLED && id < IDC_ENABLED + m_CountColors) {
        auto i = id - IDC_ENABLED;
        CDCHandle dc(cd->hdc);
        CRect rc(cd->rc);
        rc.InflateRect(-20, 10, -10, 10);
        dc.FillSolidRect(&rc, m_Colors[i].Color);
        dc.SetTextColor(m_Colors[i].TextColor);
        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(m_Colors[i].Name, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }

    return CDRF_SKIPPOSTPAINT;
}

DWORD CColorsSelectionDlg::OnPostErase(int, LPNMCUSTOMDRAW cd) {
    ::SetTextColor(cd->hdc, RGB(255, 255, 255));

    return CDRF_SKIPPOSTPAINT;
}

COLORREF CColorsSelectionDlg::SelectColor(COLORREF initial) {
    CColorDialog dlg(initial, CC_FULLOPEN, *this);
    if (dlg.DoModal() == IDOK)
        initial = dlg.GetColor();

    return initial;
}

LRESULT CColorsSelectionDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    DialogHelper::AdjustOKCancelButtons(this);
    DialogHelper::SetDialogIcon(this, IDI_COLORWHEEL);
    SetWindowText(m_Title);

    CButton cbMain(GetDlgItem(IDC_ENABLED));
    CRect rc;
    cbMain.GetWindowRect(&rc);
    ScreenToClient(&rc);
    auto cbStyle = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_CLIPSIBLINGS | BS_CENTER | BS_VCENTER;
    cbMain.DestroyWindow();

    CButton changeButton(GetDlgItem(IDC_CHANGE));
    CRect rcChange;
    changeButton.GetWindowRect(&rcChange);
    ScreenToClient(&rcChange);
    changeButton.DestroyWindow();

    for (int i = 0; i < m_CountColors; i++) {
        auto color = m_Colors.begin() + i;
        CButton cb;
        cb.Create(*this, &rc, L"", cbStyle, 0, IDC_ENABLED + i);
        cb.SetFont(GetFont());
        cb.SetCheck(color->Enabled ? BST_CHECKED : BST_UNCHECKED);
        rc.OffsetRect(0, rc.Height() + 12);

        CButton split;
        split.Create(*this, rcChange, L"Change", WS_VISIBLE | WS_CHILD | BS_SPLITBUTTON | WS_CLIPSIBLINGS, 0, IDC_CHANGE + i);
        split.SetFont(GetFont());
        rcChange.OffsetRect(0, rc.Height() + 12);
    }
    return 0;
}

LRESULT CColorsSelectionDlg::OnButtonColor(UINT, WPARAM, LPARAM, BOOL&) {
    return LRESULT(::GetSysColorBrush(BLACK_BRUSH));
}

LRESULT CColorsSelectionDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
    if (wID == IDOK) {
        for (int i = 0; i < m_CountColors; i++)
            m_Colors[i].Enabled = IsDlgButtonChecked(IDC_ENABLED + i);
    }
    EndDialog(wID);
    return 0;
}

LRESULT CColorsSelectionDlg::OnChangeColor(WORD, WORD wID, HWND hWnd, BOOL&) {
    DoChangeColors(hWnd);

    return 0;
}

LRESULT CColorsSelectionDlg::OnChangeDropdown(int, LPNMHDR hdr, BOOL&) {
    DoChangeColors(hdr->hwndFrom);

    return 0;
}

void CColorsSelectionDlg::DoChangeColors(HWND button) {
    CRect rc;
    ::GetWindowRect(button, &rc);

    CMenu menu;
    menu.LoadMenu(IDR_SPLIT);
    m_CurrentSelection = CButton(button).GetDlgCtrlID() - IDC_CHANGE;
    TrackPopupMenu(menu.GetSubMenu(0), 0, rc.left, rc.bottom, 0, *this, nullptr);
}

LRESULT CColorsSelectionDlg::OnChangeBackground(WORD, WORD wID, HWND, BOOL&) {
    int index = m_CurrentSelection;
    ATLASSERT(index >= 0 && index < m_CountColors);
    m_Colors[index].Color = SelectColor(m_Colors[index].Color);
    GetDlgItem(IDC_ENABLED + index).RedrawWindow();

    return 0;
}

LRESULT CColorsSelectionDlg::OnChangeForeground(WORD, WORD wID, HWND, BOOL&) {
    int index = m_CurrentSelection;
    ATLASSERT(index >= 0 && index < m_CountColors);
    m_Colors[index].TextColor = SelectColor(m_Colors[index].TextColor);
    GetDlgItem(IDC_ENABLED + index).RedrawWindow();

    return 0;
}

LRESULT CColorsSelectionDlg::OnChangeToDefault(WORD, WORD wID, HWND, BOOL&) {
    int index = m_CurrentSelection;
    ATLASSERT(index >= 0 && index < m_CountColors);
    m_Colors[index].Color = m_Colors[index].DefaultColor;
    m_Colors[index].TextColor = m_Colors[index].DefaultTextColor;
    GetDlgItem(IDC_ENABLED + index).RedrawWindow();

    return 0;
}

LRESULT CColorsSelectionDlg::OnResetColors(WORD, WORD wID, HWND, BOOL&) {
    for (int index = 0; index < m_CountColors; index++) {
        m_Colors[index].Color = m_Colors[index].DefaultColor;
        m_Colors[index].TextColor = m_Colors[index].DefaultTextColor;
        GetDlgItem(IDC_ENABLED + index).RedrawWindow();
    }
    return 0;
}

LRESULT CColorsSelectionDlg::OnSave(WORD, WORD wID, HWND, BOOL&) {
    CSimpleFileDialog dlg(FALSE, L"ini", nullptr, OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT, iniFileFilter, *this);
    if (dlg.DoModal() == IDOK) {
        Settings::SaveColors(dlg.m_szFileName, L"ProcessColor", m_Colors.data(), m_CountColors);
    }
    return 0;
}

LRESULT CColorsSelectionDlg::OnLoad(WORD, WORD wID, HWND, BOOL&) {
    CSimpleFileDialog dlg(TRUE, L"ini", nullptr, OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST, iniFileFilter, *this);
    if (dlg.DoModal() == IDOK) {
        if (Settings::LoadColors(dlg.m_szFileName, L"ProcessColor", m_Colors.data(), m_CountColors))
            RedrawWindow();
    }
    return 0;
}
