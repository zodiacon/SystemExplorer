#pragma once

#include "VirtualListView.h"
#include "ViewBase.h"
#include "Interfaces.h"
#include "ObjectSearcher.h"
#include "SearchBar.h"

class CSearchView :
	public ISearchBarNotify,
	public CVirtualListView<CSearchView>,
	public CViewBase<CSearchView> {
public:
	CSearchView(IMainFrame* frame);

	BEGIN_MSG_MAP(CSearchView)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(ObjectSearcher::SearchDoneMessage, OnSearchDone)
		CHAIN_MSG_MAP(CViewBase<CSearchView>)
		CHAIN_MSG_MAP(CVirtualListView<CSearchView>)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row) const;
	void DoSort(const SortInfo*);
	void UpdateResults();
	bool IsUpdating() const;

	void StartSearch(PCWSTR text) override;
	bool CancelSearch() override;

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSearchDone(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	std::unique_ptr<ObjectSearcher> m_Searcher;
	std::vector<SearchResultItem> m_Items;
	int m_FirstIcon;
	CListViewCtrl m_List;
	CSearchBar m_SearchBar;
};

