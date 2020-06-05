#pragma once

#define ATL_SIMPLE_TOOLBAR_PANE_STYLE_EX	(ATL_SIMPLE_TOOLBAR_PANE_STYLE|TBSTYLE_LIST)

#include <atlcoll.h>

template <class T>
class CToolBarHelper {
private:
	/// Wrapper class for the Win32 TBBUTTONINFO structure.
	class CTBButtonInfo : public TBBUTTONINFO {
	public:
		/// Constructor
		CTBButtonInfo(DWORD dwInitialMask = 0) {
			memset(this, 0, sizeof(TBBUTTONINFO));
			cbSize = sizeof(TBBUTTONINFO);
			dwMask = dwInitialMask;
		}
	};

	/// Wrapper class for the Win32 TBBUTTON structure.
	class CTBButton : public TBBUTTON {
	public:
		/// Constructor
		CTBButton() {
			memset(this, 0, sizeof(TBBUTTON));
		}
	};
private:
	CFont m_fontCombo;			///< Font to use for comboboxes
	CAtlMap<UINT, std::pair<UINT, UINT>> m_mapMenu;	///< Map of command ID -> menu ID and index
public:
	/// Message map
	BEGIN_MSG_MAP(CToolBarHelper<T>)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelChangeToolBarCombo)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnToolbarDropDown)
	END_MSG_MAP()

	/// Modify a toolbar button to have a drop-down button
	void AddToolBarDropDownMenu(HWND hWndToolBar, UINT nButtonID, UINT nMenuID, UINT index = 0) {
		ATLASSERT(hWndToolBar);
		ATLASSERT(nButtonID > 0);
		// Use built-in WTL toolbar wrapper class
		CToolBarCtrl toolbar(hWndToolBar);
		// Add the necessary style bit (TBSTYLE_EX_DRAWDDARROWS) if
		// not already present
		if ((toolbar.GetExtendedStyle() & TBSTYLE_EX_DRAWDDARROWS) != TBSTYLE_EX_DRAWDDARROWS)
			toolbar.SetExtendedStyle(toolbar.GetExtendedStyle() | TBSTYLE_EX_DRAWDDARROWS);
		// Get existing button style
		CTBButtonInfo tbi(TBIF_STYLE);
		if (toolbar.GetButtonInfo(nButtonID, &tbi) != -1) {
			// Modify the button
			tbi.fsStyle |= TBSTYLE_DROPDOWN;
			toolbar.SetButtonInfo(nButtonID, &tbi);
			// We need to remember that this menu ID is associated with the button ID
			// so use a basic map for this.
			m_mapMenu.SetAt(nButtonID, std::make_pair(nMenuID, index));
		}
	}

	LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		// Get the toolbar data
		auto ptb = reinterpret_cast<NMTOOLBAR*>(pnmh);
		// See if the button ID has an asscociated menu ID
		auto pair = m_mapMenu.Lookup(ptb->iItem);
		auto nMenuID = pair->m_value.first;
		if (nMenuID) {
			// Get the toolbar control
			CToolBarCtrl toolbar(pnmh->hwndFrom);
			// Get the button rect
			CRect rect;
			toolbar.GetItemRect(toolbar.CommandToIndex(ptb->iItem), &rect);
			// Create a point
			CPoint pt(rect.left, rect.bottom);
			// Map the points
			toolbar.MapWindowPoints(HWND_DESKTOP, &pt, 1);
			// Load the menu
			CMenu menu;
			if (menu.LoadMenu(nMenuID)) {
				CMenuHandle menuPopup = menu.GetSubMenu(pair->m_value.second);
				ATLASSERT(menuPopup != NULL);
				T* pT = static_cast<T*>(this);
				// Allow the menu items to be initialised (for example,
				// new items could be added here for example)
				pT->PrepareToolBarMenu(nMenuID, menuPopup);
				// Display the menu				
				// Using command bar TrackPopupMenu method means menu icons are displayed
				pT->m_CmdBar.TrackPopupMenu(menuPopup, TPM_RIGHTBUTTON | TPM_VERTICAL, pt.x, pt.y);
			}
		}
		return 0;
	}

	/// Override this Allow the menu items to be enabled/checked/etc.
	void PrepareToolBarMenu(UINT /*nMenuID*/, HMENU /*hMenu*/) {
	}

	/// Add text to a toolbar button
	void AddToolbarButtonText(HWND hWndToolBar, UINT nID, LPCTSTR lpsz) {
		// Use built-in WTL toolbar wrapper class
		CToolBarCtrl toolbar(hWndToolBar);
		// Set extended style
		if ((toolbar.GetExtendedStyle() & TBSTYLE_EX_MIXEDBUTTONS) != TBSTYLE_EX_MIXEDBUTTONS)
			toolbar.SetExtendedStyle(toolbar.GetExtendedStyle() | TBSTYLE_EX_MIXEDBUTTONS);
		// Get the button index
		int nIndex = toolbar.CommandToIndex(nID);
		CTBButton tb;
		toolbar.GetButton(nIndex, &tb);
		int nStringID = toolbar.AddStrings(lpsz);
		// Alter the button style
		tb.iString = nStringID;
		tb.fsStyle |= TBSTYLE_AUTOSIZE | BTNS_SHOWTEXT;
		// Delete and re-insert the button
		toolbar.DeleteButton(nIndex);
		toolbar.InsertButton(nIndex, &tb);
	}

	/// Add resource string to a toolbar button
	void AddToolbarButtonText(HWND hWndToolBar, UINT nID, UINT nStringID) {
		CString str;
		if (str.LoadString(nStringID))
			AddToolbarButtonText(hWndToolBar, nID, str);
	}

	/// Add text to a toolbar button (using tooltip text)
	void AddToolbarButtonText(HWND hWndToolBar, UINT nID) {
		TCHAR sz[256];
		if (AtlLoadString(nID, sz, 256) > 0) {
			// Add the text following the '\n'
			TCHAR* psz = _tcsrchr(sz, '\n');
			if (psz != NULL) {
				// Skip to first character of the tooltip
				psz++;
				// The tooltip text may include the accelerator, i.e.
				//	Open (Ctrl+O)
				// So look for an open brace
				TCHAR* pBrace = _tcschr(psz, '(');
				if (pBrace != NULL)
					*(pBrace - 1) = '\0';
				AddToolbarButtonText(hWndToolBar, nID, psz);
			}
		}
	}

	/// Create a combobox on a toolbar
	HWND CreateToolbarComboBox(HWND hWndToolBar, UINT nID, UINT nWidth = 16, UINT nHeight = 16, DWORD dwComboStyle = CBS_DROPDOWNLIST) {
		T* pT = static_cast<T*>(this);
		// Use built-in WTL toolbar wrapper class
		CToolBarCtrl toolbar(hWndToolBar);
		// Get the size of the combobox font
		CreateComboFont();
		CSize sizeFont = GetComboFontSize();
		// Compute the width and height
		UINT cx = (nWidth + 8) * sizeFont.cx;
		UINT cy = nHeight * sizeFont.cy;
		// Set the button width
		CTBButtonInfo tbi(TBIF_SIZE | TBIF_STATE | TBIF_STYLE);
		// Make sure the underlying button is disabled
		tbi.fsState = 0;
		// BTNS_SHOWTEXT will allow the button size to be altered
		tbi.fsStyle = BTNS_SHOWTEXT;
		auto textWidth = WORD(32);	// extra is for the icon
		tbi.cx = static_cast<WORD>(cx + textWidth);
		toolbar.SetButtonInfo(nID, &tbi);
		// Get the index of the toolbar button
		int nIndex = toolbar.CommandToIndex(nID);
		// Get the button rect
		CRect rc;
		toolbar.GetItemRect(nIndex, rc);
		rc.bottom = cy;
		rc.left += textWidth;
		rc.top += 4; rc.bottom -= 4;
		// Create the combobox
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | dwComboStyle;
		CComboBox combo;
		combo.Create(pT->m_hWnd, rc, NULL, dwStyle, 0, nID);
		combo.SetFont(m_fontCombo);
		combo.SetParent(toolbar);
		// The combobox might not be centered vertically, and we won't know the
		// height until it has been created. Get the size now and see if it
		// needs to be moved.
		CRect rectToolBar;
		CRect rectCombo;
		toolbar.GetClientRect(&rectToolBar);
		combo.GetWindowRect(rectCombo);
		// Get the different between the heights of the toolbar and
		// the combobox
		int nDiff = rectToolBar.Height() - rectCombo.Height();
		// If there is a difference, then move the combobox
		if (nDiff > 1) {
			toolbar.ScreenToClient(&rectCombo);
			combo.MoveWindow(rectCombo.left, rc.top + (nDiff / 2), rectCombo.Width(), rectCombo.Height());
		}
		return combo;
	}

	/// Create the font to use for comboboxes
	void CreateComboFont() {
		if (m_fontCombo == NULL) {
			NONCLIENTMETRICS ncm;
			ncm.cbSize = sizeof(NONCLIENTMETRICS);
			::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
			// Create menu font
			m_fontCombo.CreateFontIndirect(&ncm.lfMenuFont);
			ATLASSERT(m_fontCombo != NULL);
		}
	}

	/// Get the size of the default GUI font
	CSize GetComboFontSize() {
		ATLASSERT(m_fontCombo);
		// We need a temporary DC
		const T* pT = static_cast<const T*>(this);
		CClientDC dc(pT->m_hWnd);
		// Select in the menu font
		CFontHandle fontOld = dc.SelectFont(m_fontCombo);
		// Get the font size
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		// Done with the font
		dc.SelectFont(fontOld);
		// Return the width and height
		return CSize(tm.tmAveCharWidth, tm.tmHeight + tm.tmExternalLeading);
	}

	LRESULT OnSelChangeToolBarCombo(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		if (hWndCtl == nullptr) {
			bHandled = FALSE;
			return 0;
		}
		T* pT = static_cast<T*>(this);
		// Get the newly selected item index
		CComboBox combo(hWndCtl);
		int nSel = combo.GetCurSel();
		// Get the item text
		CString strItemText;
		combo.GetLBText(nSel, strItemText);
		// Get the item data
		auto dwItemData = combo.GetItemData(nSel);
		// Call special function to handle the selection change
		pT->OnToolBarCombo(combo, wID, nSel, strItemText, dwItemData);
		// Set focus to the main window
		pT->SetFocus();
		return TRUE;
	}

	// override
	void OnToolBarCombo(CComboBox& combo, UINT wID, int nSel, const CString& itemText, DWORD_PTR itemData) {
	}
};
