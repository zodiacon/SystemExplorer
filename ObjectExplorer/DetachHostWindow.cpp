#include "stdafx.h"
#include "DetachHostWindow.h"

void CDetachHostWindow::SetClient(HWND hWnd) {
	m_hWndClient = hWnd;
	::SetParent(hWnd, *this);
	::ShowWindow(hWnd, SW_SHOW);
	UpdateLayout();
}

void CDetachHostWindow::OnFinalMessage(HWND) {
	delete this;
}

LRESULT CDetachHostWindow::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}
