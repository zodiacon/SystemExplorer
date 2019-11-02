#pragma once

#include "resource.h"
#include "ObjectManager.h"

class CProcessSelectDlg : public CDialogImpl<CProcessSelectDlg> {
public:
	enum { IDD = IDD_PROCSELECT };

	int GetSelectedProcess() const;

	BEGIN_MSG_MAP(CProcessSelectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	void InitProcessList();
	static int CALLBACK CompareItems(LPARAM i1, LPARAM i2, LPARAM col);
	int CompareItems(LPARAM i1, LPARAM i2);

private:
	int m_SortColumn = -1;
	int m_SelectedPid = -1;
	bool m_SortAscending = true;
	CListViewCtrl m_List;
	ObjectManager m_ObjMgr;
};

