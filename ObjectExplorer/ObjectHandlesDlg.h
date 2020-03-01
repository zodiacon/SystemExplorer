#pragma once

#include "resource.h"
#include "VirtualListView.h"

struct ObjectInfo;
struct HandleInfo;
class ObjectManager;

class CObjectHandlesDlg : 
	public CDialogImpl<CObjectHandlesDlg>,
	public CVirtualListView<CObjectHandlesDlg> {
public:
	CObjectHandlesDlg(ObjectInfo* info, ObjectManager& om);
	enum { IDD = IDD_HANDLES };

	BEGIN_MSG_MAP(CObjectHandlesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(CVirtualListView<CObjectHandlesDlg>)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGetDispInfo(int, LPNMHDR hdr, BOOL&);

private:
	CListViewCtrl m_List;
	ObjectInfo* m_pObject;
	std::vector<std::shared_ptr<HandleInfo>> m_Handles;
	ObjectManager& m_ObjMgr;
};

