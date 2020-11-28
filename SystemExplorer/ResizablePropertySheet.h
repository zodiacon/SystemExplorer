#pragma once

template<typename T>
class CResizablePropertySheetImpl : public CPropertySheetImpl<T> {
public:
	CResizablePropertySheetImpl(ATL::_U_STRINGorID title = (LPCTSTR)nullptr, UINT uStartPage = 0, HWND hWndParent = nullptr) :
		CPropertySheetImpl<T>(title, uStartPage, hWndParent) {
		this->m_psh.pfnCallback = DoCallback;
	}

	static int CALLBACK DoCallback(HWND h, UINT msg, LPARAM lParam) {
		typedef struct {
			WORD      dlgVer;
			WORD      signature;
			DWORD     helpID;
			DWORD     exStyle;
			DWORD     style;
			WORD      cDlgItems;
			short     x;
			short     y;
			short     cx;
			short     cy;
		} DLGTEMPLATEEX;

		if (msg == PSCB_PRECREATE) {
			if (lParam) {
				auto pDlgTemplateEx = (DLGTEMPLATEEX*)lParam;
				if (pDlgTemplateEx->signature == 0xFFFF) {
					pDlgTemplateEx->style |= WS_THICKFRAME;
				}
				else {
					auto pDlgTemplate = (DLGTEMPLATE*)lParam;
					pDlgTemplate->style |= WS_THICKFRAME;
				}
			}
		}
		return T::PropSheetCallback(h, msg, lParam);
	}
};


class CResizablePropertySheet : public CResizablePropertySheetImpl<CResizablePropertySheet> {
public:
	CResizablePropertySheet(ATL::_U_STRINGorID title = (LPCTSTR)nullptr, UINT uStartPage = 0, HWND hWndParent = nullptr) :
		CResizablePropertySheetImpl(title, uStartPage, hWndParent) {
	}
};
