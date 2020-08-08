#pragma once

#include "resource.h"

struct ISearchBarNotify {
	virtual void StartSearch(PCWSTR text) {}
	virtual bool CancelSearch() { return false; }
};

class CSearchBar : public CDialogImpl<CSearchBar> {
public:
	CSearchBar(ISearchBarNotify* notify = nullptr);

	void Reset();

	enum { IDD = IDD_SEARCHBAR };

	BEGIN_MSG_MAP(CSearchBar)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		COMMAND_CODE_HANDLER(EN_CHANGE, OnTextChanged)
		COMMAND_ID_HANDLER(IDC_SEARCH, OnSearch)
		COMMAND_ID_HANDLER(IDC_CANCEL, OnCancel)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnEditKeyDown)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSearch(WORD /*wNotifyCode*/, WORD, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTextChanged(WORD /*wNotifyCode*/, WORD, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CContainedWindowT<CEdit> m_Edit;
	ISearchBarNotify* m_pNotify;
	bool m_IsSearching{ false };
};

