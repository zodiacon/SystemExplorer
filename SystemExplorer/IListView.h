#pragma once

#define LVM_QUERYINTERFACE (LVM_FIRST + 189)

const IID IID_IListViewFooterCallback = {0x88EB9442, 0x913B, 0x4AB4, 
              {0xA7, 0x41, 0xDD, 0x99, 0xDC, 0xB7, 0x55, 0x8B}};

struct IListViewFooterCallback : IUnknown {
    /// \brief Notifies the client that a footer item has been clicked
    ///
    /// This method is called by the list view control to notify
    /// the client application that the user has
    /// clicked a footer item.
    ///
    /// \param[in] itemIndex The zero-based index
    ///     of the footer item that has been clicked.
    /// \param[in] lParam The application-defined integer
    ///    value that is associated with the clicked item.
    /// \param[out] pRemoveFooter If set to \c TRUE, the list view
    ///    control will remove the footer area.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE OnButtonClicked(int itemIndex, 
                    LPARAM lParam, PINT pRemoveFooter) = 0;
    /// \brief Notifies the client that a footer item has been removed
    ///
    /// This method is called by the list view control to notify
    /// the client application that it has removed a
    /// footer item.
    ///
    /// \param[in] itemIndex The zero-based index of the footer item 
    /// that has been removed.
    /// \param[in] lParam The application-defined integer
    ///   value that is associated with the removed item.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE OnDestroyButton(int itemIndex, LPARAM lParam) = 0;
};

const IID IID_IListViewFooter = {0xF0034DA8, 0x8A22, 0x4151, 
          {0x8F, 0x16, 0x2E, 0xBA, 0x76, 0x56, 0x5B, 0xCC}};

struct IListViewFooter : IUnknown
{
public:
    /// \brief Retrieves whether the footer area is currently displayed
    ///
    /// Retrieves whether the list view control's footer area is currently displayed.
    ///
    /// \param[out] pVisible \c TRUE if the footer area is visible; otherwise \c FALSE.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE IsVisible(PINT pVisible) = 0;
    /// \brief Retrieves the caret footer item
    ///
    /// Retrieves the list view control's focused footer item.
    ///
    /// \param[out] pItemIndex Receives the zero-based index
    ///      of the footer item that has the keyboard focus.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE GetFooterFocus(PINT pItemIndex) = 0;
    /// \brief Sets the caret footer item
    ///
    /// Sets the list view control's focused footer item.
    ///
    /// \param[in] itemIndex The zero-based index
    ///        of the footer item to which to set the keyboard focus.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE SetFooterFocus(int itemIndex) = 0;
    /// \brief Sets the footer area's caption
    ///
    /// Sets the title text of the list view control's footer area.
    ///
    /// \param[in] pText The text to display in the footer area's title.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE SetIntroText(LPCWSTR pText) = 0;
    /// \brief Makes the footer area visible
    ///
    /// Makes the list view control's footer area visible
    /// and registers the callback object that is notified
    /// about item clicks and item deletions.
    ///
    /// \param[in] pCallbackObject The \c IListViewFooterCallback
    ///   implementation of the callback object to register.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE Show(IListViewFooterCallback* pCallbackObject) = 0;
    /// \brief Removes all footer items
    ///
    /// Removes all footer items from the list view control's footer area.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE RemoveAllButtons(void) = 0;
    /// \brief Inserts a footer item
    ///
    /// Inserts a new footer item with the specified properties
    /// at the specified position into the list view
    /// control.
    ///
    /// \param[in] insertAt The zero-based index at which to insert the new footer item.
    /// \param[in] pText The new footer item's text.
    /// \param[in] pUnknown ???
    /// \param[in] iconIndex The zero-based index of the new footer item's icon.
    /// \param[in] lParam The integer data that will be associated 
    /// with the new footer item.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE InsertButton(int insertAt, 
            LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam) = 0;
    /// \brief Retrieves a footer item's associated data
    ///
    /// Retrieves the integer data associated with the specified footer item.
    ///
    /// \param[in] itemIndex The zero-based index
    ///       of the footer for which to retrieve the associated data.
    /// \param[out] pLParam Receives the associated data.
    ///
    /// \return An \c HRESULT error code.
    virtual HRESULT STDMETHODCALLTYPE GetButtonLParam(int itemIndex, LONG* pLParam) = 0;
};

struct __declspec(uuid("{44C09D56-8D3B-419D-A462-7B956B105B47}")) IOwnerDataCallback : IUnknown {
	/// \brief <em>TODO</em>
	///
	/// TODO
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE GetItemPosition(int itemIndex, LPPOINT pPosition) = 0;
	/// \brief <em>TODO</em>
	///
	/// TODO
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE SetItemPosition(int itemIndex, POINT position) = 0;
	/// \brief <em>Will be called to retrieve an item's zero-based control-wide index</em>
	///
	/// This method is called by the listview control to retrieve an item's zero-based control-wide index.
	/// The item is identified by a zero-based group index, which identifies the listview group in which
	/// the item is displayed, and a zero-based group-wide item index, which identifies the item within its
	/// group.
	///
	/// \param[in] groupIndex The zero-based index of the listview group containing the item.
	/// \param[in] groupWideItemIndex The item's zero-based group-wide index within the listview group
	///            specified by \c groupIndex.
	/// \param[out] pTotalItemIndex Receives the item's zero-based control-wide index.
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE GetItemInGroup(int groupIndex, int groupWideItemIndex, PINT pTotalItemIndex) = 0;
	/// \brief <em>Will be called to retrieve the group containing a specific occurrence of an item</em>
	///
	/// This method is called by the listview control to retrieve the listview group in which the specified
	/// occurrence of the specified item is displayed.
	///
	/// \param[in] itemIndex The item's zero-based (control-wide) index.
	/// \param[in] occurrenceIndex The zero-based index of the item's copy for which the group membership is
	///            retrieved.
	/// \param[out] pGroupIndex Receives the zero-based index of the listview group that shall contain the
	///             specified copy of the specified item.
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE GetItemGroup(int itemIndex, int occurenceIndex, PINT pGroupIndex) = 0;
	/// \brief <em>Will be called to determine how often an item occurs in the listview control</em>
	///
	/// This method is called by the listview control to determine how often the specified item occurs in the
	/// listview control.
	///
	/// \param[in] itemIndex The item's zero-based (control-wide) index.
	/// \param[out] pOccurrencesCount Receives the number of occurrences of the item in the listview control.
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE GetItemGroupCount(int itemIndex, PINT pOccurenceCount) = 0;
	/// \brief <em>Will be called to prepare the client app that the data for a certain range of items will be required very soon</em>
	///
	/// This method is similar to the \c LVN_ODCACHEHINT notification. It tells the client application that
	/// it should preload the details for a certain range of items because the listview control is about to
	/// request these details. The difference to \c LVN_ODCACHEHINT is that this method identifies the items
	/// by their zero-based group-wide index and the zero-based index of the listview group containing the
	/// item.
	///
	/// \param[in] firstItem The first item to cache.
	/// \param[in] lastItem The last item to cache.
	///
	/// \return An \c HRESULT error code.
	virtual HRESULT STDMETHODCALLTYPE OnCacheHint(LVITEMINDEX firstItem, LVITEMINDEX lastItem) = 0;
};

struct __declspec(uuid("{E5B16AF2-3990-4681-A609-1F060CD14269}")) IListView : IOleWindow {
	virtual HRESULT __stdcall GetImageList(int imageList, HIMAGELIST* pHImageList) const = 0;
	virtual HRESULT __stdcall SetImageList(int imageList, HIMAGELIST hNewImageList, HIMAGELIST* pHOldImageList) = 0;
	virtual HRESULT __stdcall GetBackgroundColor(COLORREF* pColor) const = 0;
	virtual HRESULT __stdcall SetBackgroundColor(COLORREF color) = 0;
	virtual HRESULT __stdcall GetTextColor(COLORREF* pColor) const = 0;
	virtual HRESULT __stdcall SetTextColor(COLORREF color) = 0;
	virtual HRESULT __stdcall GetTextBackgroundColor(COLORREF* pColor) const = 0;
	virtual HRESULT __stdcall SetTextBackgroundColor(COLORREF color) = 0;
	virtual HRESULT __stdcall GetHotLightColor(COLORREF* pColor) = 0;
	virtual HRESULT __stdcall SetHotLightColor(COLORREF color) = 0;
	virtual HRESULT __stdcall GetItemCount(PINT pItemCount) = 0;
	virtual HRESULT __stdcall SetItemCount(int itemCount, DWORD flags) = 0;
	virtual HRESULT __stdcall GetItem(LVITEMW* pItem) = 0;
	virtual HRESULT __stdcall SetItem(LVITEMW* const pItem) = 0;
	virtual HRESULT __stdcall GetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG* pState) = 0;
	virtual HRESULT __stdcall SetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG state) = 0;
	virtual HRESULT __stdcall GetItemText(int itemIndex, int subItemIndex, LPWSTR pBuffer, int bufferSize) = 0;
	virtual HRESULT __stdcall SetItemText(int itemIndex, int subItemIndex, LPCWSTR pText) = 0;
	virtual HRESULT __stdcall GetBackgroundImage(LVBKIMAGEW* pBkImage) = 0;
	virtual HRESULT __stdcall SetBackgroundImage(LVBKIMAGEW* const pBkImage) = 0;
	virtual HRESULT __stdcall GetFocusedColumn(PINT pColumnIndex) = 0;
	// parameters may be in wrong order
	virtual HRESULT __stdcall SetSelectionFlags(ULONG mask, ULONG flags) const = 0;
	virtual HRESULT __stdcall GetSelectedColumn(PINT pColumnIndex) = 0;
	virtual HRESULT __stdcall SetSelectedColumn(int columnIndex) = 0;
	virtual HRESULT __stdcall GetView(DWORD* pView) const = 0;
	virtual HRESULT __stdcall SetView(DWORD view) = 0;
	virtual HRESULT __stdcall InsertItem(LVITEMW* const pItem, PINT pItemIndex) = 0;
	virtual HRESULT __stdcall DeleteItem(int itemIndex) = 0;
	virtual HRESULT __stdcall DeleteAllItems(void) = 0;
	virtual HRESULT __stdcall UpdateItem(int itemIndex) = 0;
	virtual HRESULT __stdcall GetItemRect(LVITEMINDEX itemIndex, int rectangleType, LPRECT pRectangle) = 0;
	virtual HRESULT __stdcall GetSubItemRect(LVITEMINDEX itemIndex, int subItemIndex, int rectangleType, LPRECT pRectangle) = 0;
	virtual HRESULT __stdcall HitTestSubItem(LVHITTESTINFO* pHitTestData) = 0;
	virtual HRESULT __stdcall GetIncrSearchString(PWSTR pBuffer, int bufferSize, PINT pCopiedChars) = 0;
	// pHorizontalSpacing and pVerticalSpacing may be in wrong order
	virtual HRESULT __stdcall GetItemSpacing(BOOL smallIconView, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
	// parameters may be in wrong order
	virtual HRESULT __stdcall SetIconSpacing(int horizontalSpacing, int verticalSpacing, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
	virtual HRESULT __stdcall GetNextItem(LVITEMINDEX itemIndex, ULONG flags, LVITEMINDEX* pNextItemIndex) const = 0;
	virtual HRESULT __stdcall FindItem(LVITEMINDEX startItemIndex, LVFINDINFOW const* pFindInfo, LVITEMINDEX* pFoundItemIndex) const = 0;
	virtual HRESULT __stdcall GetSelectionMark(LVITEMINDEX* pSelectionMark) const = 0;
	virtual HRESULT __stdcall SetSelectionMark(LVITEMINDEX newSelectionMark, LVITEMINDEX* pOldSelectionMark) = 0;
	virtual HRESULT __stdcall GetItemPosition(LVITEMINDEX itemIndex, POINT* pPosition) const = 0;
	virtual HRESULT __stdcall SetItemPosition(int itemIndex, POINT const* pPosition) = 0;
	// parameters may be in wrong order
	virtual HRESULT __stdcall ScrollView(int horizontalScrollDistance, int verticalScrollDistance) const = 0;
	virtual HRESULT __stdcall EnsureItemVisible(LVITEMINDEX itemIndex, BOOL partialOk) const = 0;
	virtual HRESULT __stdcall EnsureSubItemVisible(LVITEMINDEX itemIndex, int subItemIndex) const = 0;
	virtual HRESULT __stdcall EditSubItem(LVITEMINDEX itemIndex, int subItemIndex) const = 0;
	virtual HRESULT __stdcall RedrawItems(int firstItemIndex, int lastItemIndex) const = 0;
	virtual HRESULT __stdcall ArrangeItems(int mode) const = 0;
	virtual HRESULT __stdcall RecomputeItems(BOOL force) const = 0;
	virtual HRESULT __stdcall GetEditControl(HWND* pHWndEdit) const = 0;
	// TODO: verify that 'initialEditText' really is used to specify the initial text
	virtual HRESULT __stdcall EditLabel(LVITEMINDEX itemIndex, LPCWSTR initialEditText, HWND* phWndEdit) const = 0;
	virtual HRESULT __stdcall EditGroupLabel(int groupIndex) const = 0;
	virtual HRESULT __stdcall CancelEditLabel(void) = 0;
	virtual HRESULT __stdcall GetEditItem(LVITEMINDEX* itemIndex, PINT subItemIndex) = 0;
	virtual HRESULT __stdcall HitTest(LVHITTESTINFO* pHitTestData) = 0;
	virtual HRESULT __stdcall GetStringWidth(PCWSTR pString, PINT pWidth) = 0;
	virtual HRESULT __stdcall GetColumn(int columnIndex, LVCOLUMNW* pColumn) = 0;
	virtual HRESULT __stdcall SetColumn(int columnIndex, LVCOLUMNW* const pColumn) = 0;
	virtual HRESULT __stdcall GetColumnOrderArray(int numberOfColumns, PINT pColumns) = 0;
	virtual HRESULT __stdcall SetColumnOrderArray(int numberOfColumns, int const* pColumns) = 0;
	virtual HRESULT __stdcall GetHeaderControl(HWND* pHWndHeader) = 0;
	virtual HRESULT __stdcall InsertColumn(int insertAt, LVCOLUMNW* const pColumn, PINT pColumnIndex) = 0;
	virtual HRESULT __stdcall DeleteColumn(int columnIndex) = 0;
	virtual HRESULT __stdcall CreateDragImage(int itemIndex, POINT const* pUpperLeft, HIMAGELIST* pHImageList) = 0;
	virtual HRESULT __stdcall GetViewRect(RECT* pRectangle) const = 0;
	virtual HRESULT __stdcall GetClientRect(BOOL unknown, RECT* pClientRectangle) const = 0;
	virtual HRESULT __stdcall GetColumnWidth(int columnIndex, PINT pWidth) = 0;
	virtual HRESULT __stdcall SetColumnWidth(int columnIndex, int width) = 0;
	virtual HRESULT __stdcall GetCallbackMask(ULONG* pMask) = 0;
	virtual HRESULT __stdcall SetCallbackMask(ULONG mask) = 0;
	virtual HRESULT __stdcall GetTopIndex(PINT pTopIndex) const = 0;
	virtual HRESULT __stdcall GetCountPerPage(PINT pCountPerPage) const = 0;
	virtual HRESULT __stdcall GetOrigin(POINT* pOrigin) const = 0;
	virtual HRESULT __stdcall GetSelectedCount(PINT pSelectedCount) const = 0;
	// 'unknown' might specify whether to pass items' data or indexes
	virtual HRESULT __stdcall SortItems(BOOL unknown, LPARAM lParam, PFNLVCOMPARE pComparisonFunction) = 0;
	virtual HRESULT __stdcall GetExtendedStyle(DWORD* pStyle) const = 0;
	// parameters may be in wrong order
	virtual HRESULT __stdcall SetExtendedStyle(DWORD mask, DWORD style, DWORD* pOldStyle) = 0;
	virtual HRESULT __stdcall GetHoverTime(UINT* pTime) const = 0;
	virtual HRESULT __stdcall SetHoverTime(UINT time, UINT* pOldSetting) = 0;
	virtual HRESULT __stdcall GetToolTip(HWND* pHWndToolTip) const = 0;
	virtual HRESULT __stdcall SetToolTip(HWND hWndToolTip, HWND* pHWndOldToolTip) = 0;
	virtual HRESULT __stdcall GetHotItem(LVITEMINDEX* pHotItem) const = 0;
	virtual HRESULT __stdcall SetHotItem(LVITEMINDEX newHotItem, LVITEMINDEX* pOldHotItem) = 0;
	virtual HRESULT __stdcall GetHotCursor(HCURSOR* pHCursor) = 0;
	virtual HRESULT __stdcall SetHotCursor(HCURSOR hCursor, HCURSOR* pHOldCursor) = 0;
	// parameters may be in wrong order
	virtual HRESULT __stdcall ApproximateViewRect(int itemCount, PINT pWidth, PINT pHeight) = 0;
	virtual HRESULT __stdcall SetRangeObject(int unknown, LPVOID/*ILVRange**/ pObject) = 0;
	virtual HRESULT __stdcall GetWorkAreas(int numberOfWorkAreas, RECT* pWorkAreas) = 0;
	virtual HRESULT __stdcall SetWorkAreas(int numberOfWorkAreas, RECT const* pWorkAreas) = 0;
	virtual HRESULT __stdcall GetWorkAreaCount(PINT pNumberOfWorkAreas) = 0;
	virtual HRESULT __stdcall ResetEmptyText(void) = 0;
	virtual HRESULT __stdcall EnableGroupView(BOOL enable) = 0;
	virtual HRESULT __stdcall IsGroupViewEnabled(BOOL* pIsEnabled) = 0;
	virtual HRESULT __stdcall SortGroups(PFNLVGROUPCOMPARE pComparisonFunction, PVOID lParam) = 0;
	virtual HRESULT __stdcall GetGroupInfo(int unknown1, int unknown2, LVGROUP* pGroup) const = 0;
	virtual HRESULT __stdcall SetGroupInfo(int unknown, int groupID, LVGROUP* const pGroup) = 0;
	virtual HRESULT __stdcall GetGroupRect(BOOL unknown, int groupID, int rectangleType, RECT* pRectangle) const = 0;
	virtual HRESULT __stdcall GetGroupState(int groupID, ULONG mask, ULONG* pState) = 0;
	virtual HRESULT __stdcall HasGroup(int groupID, BOOL* pHasGroup) const = 0;
	virtual HRESULT __stdcall InsertGroup(int insertAt, LVGROUP* const pGroup, PINT pGroupID) = 0;
	virtual HRESULT __stdcall RemoveGroup(int groupID) = 0;
	virtual HRESULT __stdcall InsertGroupSorted(LVINSERTGROUPSORTED const* pGroup, PINT pGroupID) = 0;
	virtual HRESULT __stdcall GetGroupMetrics(LVGROUPMETRICS* pMetrics) const = 0;
	virtual HRESULT __stdcall SetGroupMetrics(LVGROUPMETRICS* const pMetrics) = 0;
	virtual HRESULT __stdcall RemoveAllGroups(void) = 0;
	virtual HRESULT __stdcall GetFocusedGroup(PINT pGroupID) const = 0;
	virtual HRESULT __stdcall GetGroupCount(PINT pCount) const = 0;
	virtual HRESULT __stdcall SetOwnerDataCallback(IOwnerDataCallback* pCallback) = 0;
	virtual HRESULT __stdcall GetTileViewInfo(LVTILEVIEWINFO* pInfo) const = 0;
	virtual HRESULT __stdcall SetTileViewInfo(LVTILEVIEWINFO* const pInfo) = 0;
	virtual HRESULT __stdcall GetTileInfo(LVTILEINFO* pTileInfo) const = 0;
	virtual HRESULT __stdcall SetTileInfo(LVTILEINFO* const pTileInfo) = 0;
	virtual HRESULT __stdcall GetInsertMark(LVINSERTMARK* pInsertMarkDetails) const = 0;
	virtual HRESULT __stdcall SetInsertMark(LVINSERTMARK const* pInsertMarkDetails) = 0;
	virtual HRESULT __stdcall GetInsertMarkRect(LPRECT pInsertMarkRectangle) const = 0;
	virtual HRESULT __stdcall GetInsertMarkColor(COLORREF* pColor) const = 0;
	virtual HRESULT __stdcall SetInsertMarkColor(COLORREF color, COLORREF* pOldColor) = 0;
	virtual HRESULT __stdcall HitTestInsertMark(POINT const* pPoint, LVINSERTMARK* pInsertMarkDetails) const = 0;
	virtual HRESULT __stdcall SetInfoTip(LVSETINFOTIP* const pInfoTip) = 0;
	virtual HRESULT __stdcall GetOutlineColor(COLORREF* pColor) const = 0;
	virtual HRESULT __stdcall SetOutlineColor(COLORREF color, COLORREF* pOldColor) = 0;
	virtual HRESULT __stdcall GetFrozenItem(PINT pItemIndex) = 0;
	// one parameter will be the item index; works in Icons view only
	virtual HRESULT __stdcall SetFrozenItem(BOOL freeze, int item) = 0;
	virtual HRESULT __stdcall GetFrozenSlot(RECT* rc) const = 0;
	virtual HRESULT __stdcall SetFrozenSlot(BOOL freeze, POINT const* pt) = 0;
	virtual HRESULT __stdcall GetViewMargin(RECT* pMargin) const = 0;
	virtual HRESULT __stdcall SetViewMargin(RECT const* pMargin) = 0;
	virtual HRESULT __stdcall SetKeyboardSelected(LVITEMINDEX itemIndex) = 0;
	virtual HRESULT __stdcall MapIndexToId(int itemIndex, PINT pItemID) = 0;
	virtual HRESULT __stdcall MapIdToIndex(int itemID, PINT pItemIndex) = 0;
	virtual HRESULT __stdcall IsItemVisible(LVITEMINDEX itemIndex, BOOL* pVisible) = 0;
	virtual HRESULT __stdcall EnableAlphaShadow(BOOL enable) = 0;
	virtual HRESULT __stdcall GetGroupSubsetCount(PINT pNumberOfRowsDisplayed) const = 0;
	virtual HRESULT __stdcall SetGroupSubsetCount(int numberOfRowsToDisplay) = 0;
	virtual HRESULT __stdcall GetVisibleSlotCount(PINT pCount) const = 0;
	virtual HRESULT __stdcall GetColumnMargin(RECT* pMargin) const = 0;
	virtual HRESULT __stdcall SetSubItemCallback(LPVOID/*ISubItemCallback**/ pCallback) = 0;
	virtual HRESULT __stdcall GetVisibleItemRange(LVITEMINDEX* pFirstItem, LVITEMINDEX* pLastItem) const = 0;
	virtual HRESULT __stdcall SetTypeAheadFlags(UINT mask, UINT flags) = 0;

	int GetSelectedIndex() const;
	int GetTopIndex() const;
};
