#pragma once

#include "resource.h"
#include "ProcessColor.h"

class CColorsSelectionDlg : 
	public CDialogImpl<CColorsSelectionDlg>,
	public CCustomDraw<CColorsSelectionDlg> {
public:
	enum { IDD = IDD_COLORS };

	CColorsSelectionDlg(HighlightColor* colors, int count);
	void SetTitle(PCWSTR title) { m_Title = title; }

	const HighlightColor* GetColors() const;

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnPostErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(ID_COLOR_BACKGROUND, OnChangeBackground)
		COMMAND_ID_HANDLER(ID_COLOR_FOREGROUND, OnChangeForeground)
		COMMAND_ID_HANDLER(ID_COLOR_DEFAULT, OnChangeToDefault)
		COMMAND_ID_HANDLER(IDC_DEFAULTS, OnResetColors)
		NOTIFY_CODE_HANDLER(BCN_DROPDOWN, OnChangeDropdown)
		COMMAND_RANGE_HANDLER(IDC_CHANGE, IDC_CHANGE + m_CountColors, OnChangeColor)
		CHAIN_MSG_MAP(CCustomDraw<CColorsSelectionDlg>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	COLORREF SelectColor(COLORREF initial);
	void DoChangeColors(HWND button);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnButtonColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeDropdown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnChangeBackground(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeForeground(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeToDefault(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnResetColors(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	std::vector<HighlightColor> m_Colors;
	int m_CountColors;
	int m_CurrentSelection;
	CString m_Title;
};

