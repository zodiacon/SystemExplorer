#include "pch.h"
#include "ProcessTreeView.h"
#include "ImageIconCache.h"
#include <ProcessInfo.h>
#include "ProcessInfoEx.h"
#include "ProcessPropertiesDlg.h"
#include "ProcessesView.h"
#include "FormatHelper.h"

CString CProcessTreeView::GetDetails(int row) {
	return CString();
}

CString CProcessTreeView::GetName(int row) {
	static PCWSTR names[] = {
		L"Name", L"ID", L"User Name", L"Session", L"CPU %", L"CPU Time", L"Parent", L"Priority", L"Priority Class",
		L"Threads", L"Peak Threads", L"Handles", L"Attributes", L"Image Path", L"Create Time", L"Commit Size (K)",
		L"Peak Commit (K)", L"Working Set (K)", L"Peak WS (K)", L"Virtual Size (K)", L"Peak Virtual Size (K)",
		L"Paged Pool (K)", L"Peak Paged Pool (K)", L"NP Pool (k)", L"Peak NP Pool (K)", L"Kernel Time", L"User Time",
		L"I/O Priority", L"Memory Priority", L"Command Line", L"Package Full Name", L"Job ID", 
		L"I/O Read Bytes", L"I/O Write Bytes", L"I/O Other Bytes",
		L"I/O Reads", L"I/O Writes", L"I/O Others",
		L"GDI Objects", L"User Objects", L"Peak GDI Objects", L"Peak User Objects", L"Integrity Level",
		L"Elevated?", L"Virtualization", L"Window Title", L"Platform", L"Description", L"Company", L"DPI Awareness",
	};
	if (row >= _countof(names))
		return L"";

	return names[row];
}

CString CProcessTreeView::GetValue(int row) {
	auto pi = GetSelectedProcess();
	if (pi == nullptr)
		return L"";

	ProcessInfoEx px(pi);
	return FormatHelper::GetProcessColumnValue((ProcessColumn)row, m_ProcMgr, pi, px);
}

INT_PTR CProcessTreeView::ShowProcessProperties(uint32_t pid) const {
	ProcessInfoEx px(m_ProcMgr.GetProcessById(pid).get());
	CProcessPropertiesDlg dlg(m_ProcMgr, px);
	dlg.SetModal(true);
	return dlg.DoModal();
}

LRESULT CProcessTreeView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(*this, rcDefault, nullptr, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
	m_Tree.SetImageList(ImageIconCache::Get().GetImageList(), TVSIL_NORMAL);

	m_List.Create(m_Splitter, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);
	m_List.InsertColumn(0, L"Property", LVCFMT_LEFT, 160);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 600);
	//m_List.InsertColumn(2, L"Details", LVCFMT_LEFT, 400);

	m_List.SetItemCount((int)ProcessColumn::COUNT);

	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.SetSplitterPosPct(25);

	DoRefresh();
	m_Tree.SetFocus();

	return 0;
}

LRESULT CProcessTreeView::OnTreeItemDoubleClick(int, LPNMHDR hdr, BOOL&) {
	auto selected = m_Tree.GetSelectedItem();
	if (selected) {
		ShowProcessProperties((uint32_t)selected.GetData());
		return 1;
	}
	return 0;
}

LRESULT CProcessTreeView::OnTreeItemChanged(int, LPNMHDR hdr, BOOL&) {
	m_SelectedProcess = m_ProcMgr.GetProcessById((uint32_t)m_Tree.GetSelectedItem().GetData());
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	auto ui = GetFrame()->GetUpdateUI();
	ui->UIEnable(ID_EDIT_PROPERTIES, TRUE);

	return 0;
}

LRESULT CProcessTreeView::OnRightClick(int, LPNMHDR hdr, BOOL& handled) {
	if (hdr->hwndFrom != m_Tree) {
		handled = FALSE;
		return 0;
	}

	CPoint pt;
	::GetCursorPos(&pt);
	m_Tree.ScreenToClient(&pt);
	auto item = m_Tree.HitTest(pt, nullptr);
	if(item) {
		m_Tree.SelectItem(item);
		CMenu menu;
		menu.LoadMenu(IDR_CONTEXT);
		auto cmd = (UINT)GetFrame()->TrackPopupMenu(menu.GetSubMenu(11), nullptr, nullptr, TPM_RETURNCMD);
		if (cmd)
			PostMessage(WM_COMMAND, cmd);
	}
	return 0;
}

CString CProcessTreeView::GetColumnText(HWND, int row, int col) {
	switch (col) {
		case 0: return GetName(row);
		case 1: return GetValue(row);
		case 2: return GetDetails(row);
	}
	ATLASSERT(false);
	return L"";
}

WinSys::ProcessInfo* CProcessTreeView::GetSelectedProcess() const {
	return m_SelectedProcess.get();
}

void CProcessTreeView::OnUpdate() {
	m_ProcMgr.EnumProcesses();
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

void CProcessTreeView::DoRefresh() {
	m_Tree.LockWindowUpdate(TRUE);

	m_Tree.DeleteAllItems();
	int currentIndent = -1;
	CTreeItem node = TVI_ROOT;
	CString text;
	auto& icons = ImageIconCache::Get();

	for (auto& [p, indent] : m_ProcMgr.BuildProcessTree()) {
		text.Format(L"%s (%d)", p->GetImageName().c_str(), p->Id);
		ProcessInfoEx px(p.get());
		int icon = icons.GetIcon(px.GetExecutablePath());

		if(indent > currentIndent) {
			ATLASSERT(indent == currentIndent + 1);
			node = m_Tree.InsertItem(text, icon, icon, indent == 0 ? TVI_ROOT : node.m_hTreeItem, TVI_LAST);
			node.SetData(p->Id);
			currentIndent = indent;
		}
		else if(indent < currentIndent) {
			while (indent < currentIndent) {
				node = node.GetParent();
				currentIndent--;
			}
			node = m_Tree.InsertItem(text, icon, icon, indent == 0 ? TVI_ROOT : node.GetParent().m_hTreeItem, TVI_LAST);
			node.SetData(p->Id);
		}
		else {
			auto parent = node.GetParent() == (HTREEITEM)nullptr ? TVI_ROOT : node.GetParent().m_hTreeItem;
			ATLASSERT(parent);
			node = m_Tree.InsertItem(text, icon, icon, parent, TVI_LAST);
			node.SetData(p->Id);
		}
	}
	m_Tree.SelectItem(m_Tree.GetRootItem());
	m_Tree.LockWindowUpdate(FALSE);
}

LRESULT CProcessTreeView::OnProcessItem(WORD, WORD id, HWND, BOOL&) {
	return GetFrame()->SendFrameMessage(WM_COMMAND, id, reinterpret_cast<LPARAM>(GetSelectedProcess()));
}

LRESULT CProcessTreeView::OnProcessProperties(WORD, WORD id, HWND, BOOL&) {
	ShowProcessProperties(GetSelectedProcess()->Id);
	return 0;
}

