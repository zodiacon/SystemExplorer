#pragma once

#include "VirtualListView.h"
#include "ViewBase.h"
#include "Interfaces.h"
#include "ObjectSearcher.h"

class CSearchView :
	public CVirtualListView<CSearchView>,
	public CViewBase<CSearchView> {
public:
	CSearchView(IMainFrame* frame);

	BEGIN_MSG_MAP(CSearchView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CViewBase<CSearchView>)
		CHAIN_MSG_MAP(CVirtualListView<CSearchView>)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;

	void DoSort(const SortInfo*);

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	std::unique_ptr<ObjectSearcher> m_Searcher;
	std::vector<SearchResultItem> m_Items;
};

