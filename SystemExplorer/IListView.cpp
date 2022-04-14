#include "pch.h"
#include "IListView.h"

int IListView::GetSelectedIndex() const {
    int selected = -1;
    GetSelectedCount(&selected);
    if (selected != 1)
        return -1;
    LVITEMINDEX idx;
    return GetNextItem(LVITEMINDEX{ -1, -1 }, LVNI_SELECTED, &idx) == S_OK ? idx.iItem : -1;
}

int IListView::GetTopIndex() const {
    int top;
    return GetTopIndex(&top) == S_OK ? top : -1;
}
