#pragma once

#include "Interfaces.h"

template<typename T>
class CViewBase abstract {
public:
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {
		ATLASSERT(frame);
	}

protected:
	BEGIN_MSG_MAP(CViewBase)
		MESSAGE_HANDLER(OM_NEW_FRAME, OnNewFrame)
	END_MSG_MAP()

	IMainFrame* GetFrame() const {
		return m_pFrame;
	}

	LRESULT OnNewFrame(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		m_pFrame = reinterpret_cast<IMainFrame*>(lParam);
		return 0;
	}

private:
	IMainFrame* m_pFrame;
};
