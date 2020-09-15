#include "pch.h"
#include "ProcessPropertiesDlg.h"
#include "DialogHelper.h"
#include "SecurityHelper.h"
#include "FormatHelper.h"

void CProcessPropertiesDlg::OnFinalMessage(HWND) {
    delete this;
}

void CProcessPropertiesDlg::InitProcess() {
    auto pi = m_px.GetProcessInfo();
    CString text;
    text.Format(L"%s (%u) Properties", pi->GetImageName().c_str(), pi->Id);
    SetWindowText(text);
    
    HICON hIcon = nullptr, hIconBig;
    ::ExtractIconEx(m_px.GetExecutablePath().c_str(), 0, &hIconBig, &hIcon, 1);
    if (hIcon == nullptr) {
        hIcon = AtlLoadSysIcon(IDI_APPLICATION);
        hIconBig = hIcon;
    }
    DialogHelper::SetDialogIcon(this, hIcon);
    ((CStatic)GetDlgItem(IDC_APPICON)).SetIcon(hIconBig);

    SetDlgItemText(IDC_NAME, pi->GetImageName().c_str());
    SetDlgItemInt(IDC_PID, pi->Id);
    auto& path = m_px.GetExecutablePath();
    bool imagePath = false;
    if (path[1] == L':') {
        imagePath = true;
        SetDlgItemText(IDC_PATH, path.c_str());
    }
    SetDlgItemText(IDC_COMMANDLINE, m_px.GetCommandLine().c_str());
    text.Format(L"%d bit", m_px.GetBitness());
    SetDlgItemText(IDC_PLATFORM, text);
    SetDlgItemText(IDC_USERNAME, m_px.UserName().c_str());
    SetDlgItemText(IDC_CREATED, FormatHelper::TimeToString(pi->CreateTime));
    if (pi->ParentId > 0) {
        auto parent = m_pm.GetProcessById(pi->ParentId);
        if (parent && parent->CreateTime < pi->CreateTime) {
            text.Format(L" %s (%u)", parent->GetImageName().c_str(), parent->Id);
        }
        else {
            text.Format(L" <non-existent> (%u)", pi->ParentId);
        }
        SetDlgItemText(IDC_PARENT, text);
    }

    GetDlgItem(IDC_EXPLORE).EnableWindow(imagePath);
    auto dir = m_px.GetCurrentDirectory();
    if (dir.empty())
        GetDlgItem(IDC_EXPLORE_DIR).EnableWindow(FALSE);
    else
        SetDlgItemText(IDC_CURDIR, dir.c_str());
}

LRESULT CProcessPropertiesDlg::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
    return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CProcessPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    InitDynamicLayout();

    DialogHelper::AddIconToButton(this, IDC_TOKEN, IDI_TOKEN);
    DialogHelper::AddIconToButton(this, IDC_JOB, IDI_JOB);
    DialogHelper::AdjustOKCancelButtons(this);
    DialogHelper::AddIconToButton(this, IDC_COPY, IDI_COPY);

    InitProcess();

    return 0;
}

LRESULT CProcessPropertiesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
    DestroyWindow();
    return 0;
}

LRESULT CProcessPropertiesDlg::OnExplore(WORD, WORD wID, HWND, BOOL&) {
    if ((INT_PTR)::ShellExecute(nullptr, L"open", L"explorer",
        (L"/select,\"" + m_px.GetExecutablePath() + L"\"").c_str(),
        nullptr, SW_SHOWDEFAULT) < 32)
        AtlMessageBox(*this, L"Failed to locate executable", IDS_TITLE, MB_ICONERROR);

    return 0;
}

LRESULT CProcessPropertiesDlg::OnExploreDirectory(WORD, WORD wID, HWND, BOOL&) {
    if ((INT_PTR)::ShellExecute(nullptr, L"explore", m_px.GetCurrentDirectory().c_str(),
        nullptr, nullptr, SW_SHOWDEFAULT) < 32)
        AtlMessageBox(*this, L"Failed to locate directory", IDS_TITLE, MB_ICONERROR);

    return 0;
}

LRESULT CProcessPropertiesDlg::OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
    auto pMMI = (PMINMAXINFO)lParam;
    pMMI->ptMaxTrackSize.y = m_ptMinTrackSize.y;
    pMMI->ptMaxTrackSize.x = m_ptMinTrackSize.x * 2;
    handled = FALSE;

    return 0;
}
