#pragma once

class CDetachHostWindow : public CFrameWindowImpl<CDetachHostWindow> {
public:
	DECLARE_FRAME_WND_CLASS(nullptr, 0)

	BEGIN_MSG_MAP(CDetachHostWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CFrameWindowImpl<CDetachHostWindow>)
	END_MSG_MAP()

	void SetClient(HWND hWnd);
	void OnFinalMessage(HWND) override;

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};

