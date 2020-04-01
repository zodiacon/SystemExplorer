#pragma once

#include "resource.h"
#include "ObjectManager.h"
#include "VirtualListView.h"
#include "ProcessManager.h"

class CPipesMailslotsDlg : 
	public CDialogImpl<CPipesMailslotsDlg>,
	public CDialogResize<CPipesMailslotsDlg>,
	public CVirtualListView<CPipesMailslotsDlg> {
public:
	enum class Type { Pipes, Mailslots };

	CPipesMailslotsDlg(Type type);

	CString GetColumnText(int row, int col);
	int GetRowImage(int row);

	enum { IDD = IDD_PIPES_MAILSLOTS };

	BEGIN_DLGRESIZE_MAP(CPipesMailslotsDlg)
		BEGIN_DLGRESIZE_GROUP()
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_GROUP()
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CPipesMailslotsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(CVirtualListView<CPipesMailslotsDlg>)
		CHAIN_MSG_MAP(CDialogResize<CPipesMailslotsDlg>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void EnumObjects();

private:
	std::vector<std::shared_ptr<ObjectInfo>> m_Objects;
	WinSys::ProcessManager m_ProcMgr;
	CListViewCtrl m_List;
	Type m_Type;
	CString m_Prefix;
};

