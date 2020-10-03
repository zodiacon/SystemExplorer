#pragma once

template<typename T>
class CNotifyIcon {
public:
	CNotifyIcon() {
		m_MessageId = ::RegisterWindowMessage(L"SystemExplorerTrayIconMessage");
	}

	BEGIN_MSG_MAP(CNotifyIcon)
		MESSAGE_HANDLER(m_MessageId, OnTrayMessage)
	END_MSG_MAP()

	LRESULT OnTrayMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		auto evt = LOWORD(lParam);
		auto pT = static_cast<T*>(this);
		switch (evt) {
			case WM_CONTEXTMENU:
				pT->OnTrayIconContextMenu();
				break;

			case WM_LBUTTONDBLCLK:
				pT->OnTrayIconDoubleClick();
				break;

			case NIN_SELECT:
				pT->OnTrayIconSelected();
				break;
		}
		return 0;
	}

	void OnTrayIconContextMenu() {}
	void OnTrayIconDoubleClick() {}
	void OnTrayIconSelected() {}

	bool AddTrayIcon(HICON hIcon, PCWSTR tip) {
		if (m_IconAdded)
			return true;

		NOTIFYICONDATA data = { sizeof(data) };
		auto pT = static_cast<T*>(this);
		data.hWnd = pT->m_hWnd;
		data.uID = pT->GetIconId();
		data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		data.hIcon = hIcon;
		data.uCallbackMessage = m_MessageId;
		::wcscpy_s(data.szTip, tip);
		m_IconAdded =  ::Shell_NotifyIcon(NIM_ADD, &data);
		if (m_IconAdded) {
			data.uVersion = NOTIFYICON_VERSION_4;
			::Shell_NotifyIconW(NIM_SETVERSION, &data);
		}
		return m_IconAdded;
	}

	bool RemoveTrayIcon() {
		NOTIFYICONDATA data = { sizeof(data) };
		auto pT = static_cast<T*>(this);
		data.uID = pT->GetIconId();
		data.hWnd = pT->m_hWnd;
		m_IconAdded = !::Shell_NotifyIcon(NIM_DELETE, &data);
		return !m_IconAdded;
	}

	UINT GetIconId() {
		auto pT = static_cast<T*>(this);
		return HandleToULong(pT->m_hWnd) & 0xffff;
	}

	bool m_IconAdded{ false };
	UINT m_MessageId;
};
