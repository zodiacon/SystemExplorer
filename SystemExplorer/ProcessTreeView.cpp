#include "pch.h"
#include "ProcessTreeView.h"
#include "ImageIconCache.h"
#include <ProcessInfo.h>
#include "ProcessInfoEx.h"
#include "ProcessPropertiesDlg.h"

LRESULT CProcessTreeView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Tree.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);

	m_Tree.SetImageList(ImageIconCache::Get().GetImageList(), TVSIL_NORMAL);

	DoRefresh();

	return 0;
}

LRESULT CProcessTreeView::OnTreeItemDoubleClick(int, LPNMHDR hdr, BOOL&) {
	auto selected = m_Tree.GetSelectedItem();
	if (selected) {
		ProcessInfoEx px(m_ProcMgr.GetProcessById((uint32_t)selected.GetData()).get());
		CProcessPropertiesDlg dlg(m_ProcMgr, px);
		dlg.SetModal(true);
		dlg.DoModal();
		return 1;
	}
	return 0;
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

	m_Tree.LockWindowUpdate(FALSE);
}
