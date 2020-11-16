#pragma once

#include "Interfaces.h"
#include "resource.h"

template<typename T, typename TBase = CFrameWindowImpl<T, CWindow, CControlWinTraits>>
class CViewBase abstract : 
	public TBase,
	public CAutoUpdateUI<T>,
	public CIdleHandler {
public:
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {
		ATLASSERT(frame);
	}

	int GetUpdateInterval() const {
		return m_UpdateInterval;
	}

	bool IsPaused() const {
		return m_Paused;
	}

protected:
	BEGIN_MSG_MAP(CViewBase)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(ID_VIEW_PAUSE, OnPauseResume)
		MESSAGE_HANDLER(OM_ACTIVATE_PAGE, OnActivate)
		COMMAND_RANGE_HANDLER(ID_UPDATEINTERVAL_1SECOND, ID_UPDATEINTERVAL_10SECONDS, OnUpdateInterval)
		MESSAGE_HANDLER(OM_NEW_FRAME, OnNewFrame)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	LRESULT OnActivate(UINT /*uMsg*/, WPARAM activate, LPARAM, BOOL&) {
		auto pT = static_cast<T*>(this);
		if (activate) {
			auto ui = GetFrame()->GetUpdateUI();
			ui->UISetRadioMenuItem(m_CurrentUpdateId, ID_UPDATEINTERVAL_1SECOND, ID_UPDATEINTERVAL_10SECONDS);
			ui->UISetCheck(ID_VIEW_PAUSE, m_Paused);
			pT->SetTimer(1, GetUpdateInterval(), nullptr);
			//pT->UpdateUI();
		}
		else
			pT->KillTimer(1);
		pT->OnActivate(activate);
		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM id, LPARAM lParam, BOOL& bHandled) {
		if (id != 1) {
			bHandled = FALSE;
			return 0;
		}
		static_cast<T*>(this)->OnUpdate();
		return 0;
	}

	LRESULT OnPauseResume(WORD, WORD, HWND, BOOL&) {
		Pause(!m_Paused);
		static_cast<T*>(this)->OnPauseResume(m_Paused);
		return 0;
	}

	void Pause(bool pause) {
		m_Paused = pause;
		auto pT = static_cast<T*>(this);
		if (m_Paused)
			pT->KillTimer(1);
		else
			pT->SetTimer(1, GetUpdateInterval(), nullptr);
	}

	LRESULT OnUpdateInterval(WORD, WORD id, HWND, BOOL&) {
		int intervals[] = { 1000, 2000, 5000, 10000 };
		int index = id - ID_UPDATEINTERVAL_1SECOND;
		ATLASSERT(index >= 0 && index < _countof(intervals));

		m_UpdateInterval = intervals[index];
		auto pT = static_cast<T*>(this);
		if (!m_Paused) {
			pT->SetTimer(1, m_UpdateInterval, nullptr);
		}
		pT->OnUpdateIntervalChanged(m_UpdateInterval);
		GetFrame()->GetUpdateUI()->UISetRadioMenuItem(m_CurrentUpdateId = id, ID_UPDATEINTERVAL_1SECOND, ID_UPDATEINTERVAL_10SECONDS);
		return 0;
	}

	void OnPauseResume(bool paused) {}
	void OnUpdateIntervalChanged(int interval) {}
	void OnUpdate() {}
	void OnActivate(bool) {}

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
	int m_CurrentUpdateId = ID_UPDATEINTERVAL_1SECOND;
	int m_UpdateInterval{ 1000 };
	bool m_Paused{ false };
};
