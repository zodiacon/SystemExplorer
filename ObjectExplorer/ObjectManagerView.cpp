#include "stdafx.h"
#include "ObjectManagerView.h"

LRESULT CObjectManagerView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_Splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES);

	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.UpdateSplitterLayout();
	m_Splitter.SetSplitterPosPct(25);

	return 0;
}

LRESULT CObjectManagerView::OnSize(UINT, WPARAM, LPARAM lParam, BOOL&) {
	auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
	if (m_Splitter)
		m_Splitter.MoveWindow(0, 0, x, y);
	return 0;
}
