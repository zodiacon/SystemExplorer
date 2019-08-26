// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "AllObjectsView.h"

BOOL CAllObjectsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CAllObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CAllObjectsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	InsertColumn(0, L"Name");

	return 0;
}
