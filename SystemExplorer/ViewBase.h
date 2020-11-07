#pragma once

#include "Interfaces.h"

template<typename T, typename TBase = CFrameWindowImpl<T, CWindow, CControlWinTraits>>
class CViewBase abstract : 
	public TBase,
	public CAutoUpdateUI<T>,
	public CIdleHandler {
public:
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {
		ATLASSERT(frame);
	}

protected:
	BEGIN_MSG_MAP(CViewBase)
		MESSAGE_HANDLER(OM_NEW_FRAME, OnNewFrame)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	BOOL OnIdle() override {
		CAutoUpdateUI<T>::UIUpdateToolBar();
		return FALSE;
	}

	IMainFrame* GetFrame() const {
		return m_pFrame;
	}

	LRESULT OnNewFrame(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		m_pFrame = reinterpret_cast<IMainFrame*>(lParam);
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		auto pT = static_cast<T*>(this);

		bHandled = FALSE;
		if(pT->m_hWndToolBar)
			_Module.GetMessageLoop()->RemoveIdleHandler(this);
		return 0;
	}

	struct ToolBarButtonInfo {
		UINT id;
		int image;
		BYTE style = BTNS_BUTTON;
		PCWSTR text = nullptr;
	};

	HWND CreateAndInitToolBar(const ToolBarButtonInfo* buttons, int count) {
		auto pT = static_cast<T*>(this);

		CToolBarCtrl tb;
		auto hWndToolBar = tb.Create(pT->m_hWnd, pT->rcDefault, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
		tb.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

		CImageList tbImages;
		tbImages.Create(24, 24, ILC_COLOR32, 4, 4);
		tb.SetImageList(tbImages);

		for (int i = 0; i < count; i++) {
			auto& b = buttons[i];
			if (b.id == 0)
				tb.AddSeparator(0);
			else {
				int image = b.image == 0 ? I_IMAGENONE : tbImages.AddIcon(AtlLoadIconImage(b.image, 0, 24, 24));
				tb.AddButton(b.id, b.style | (b.text ? BTNS_SHOWTEXT : 0), TBSTATE_ENABLED, image, b.text, 0);
			}
		}
		pT->CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		pT->AddSimpleReBarBand(tb);

		pT->UIAddToolBar(hWndToolBar);
		_Module.GetMessageLoop()->AddIdleHandler(this);

		return hWndToolBar;
	}

private:
	IMainFrame* m_pFrame;
};
