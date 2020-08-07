#include "pch.h"
#include "SearchView.h"

CSearchView::CSearchView(IMainFrame* frame) : CViewBase(frame) {
}

CString CSearchView::GetColumnText(HWND, int row, int col) const {
	return CString();
}

int CSearchView::GetRowImage(int row) const {
	return 0;
}

void CSearchView::DoSort(const SortInfo*) {
}

LRESULT CSearchView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	return LRESULT();
}
