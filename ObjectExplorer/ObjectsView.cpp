// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include <execution>
#include "ObjectsView.h"
#include "ClipboardHelper.h"
#include "ProcessHelper.h"
#include "Interfaces.h"
#include "SortHelper.h"
#include "ObjectHandlesDlg.h"
#include "ObjectType.h"
#include "ObjectTypeFactory.h"
#include "SecurityInfo.h"
#include "DriverHelper.h"

int CObjectsView::ColumnCount;

CObjectsView::CObjectsView(IMainFrame* pFrame, PCWSTR type) : CViewBase(pFrame),
	m_pUpdateUI(pFrame->GetUpdateUI()), m_Typename(type) {
}

BOOL CObjectsView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CObjectsView::DoSort(const SortInfo* si) {
	std::sort(std::execution::seq, m_Objects.begin(), m_Objects.end(), [this, si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

bool CObjectsView::IsSortable(int col) const {
	return col < 4;
}

void CObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfo>& CObjectsView::GetItem(int index) {
	return m_Objects[index];
}

bool CObjectsView::CompareItems(const ObjectInfo& o1, const ObjectInfo& o2, const SortInfo* si) {
	switch (si->SortColumn) {
		case 0:		// type
			return SortHelper::SortStrings(m_ObjMgr.GetType(o1.TypeIndex)->TypeName, m_ObjMgr.GetType(o2.TypeIndex)->TypeName, si->SortAscending);

		case 1:		// address
			return SortHelper::SortNumbers(o1.Object, o2.Object, si->SortAscending);

		case 2:		// name
			return SortHelper::SortStrings(o1.Name, o2.Name, si->SortAscending);

		case 3:		// handles
			return SortHelper::SortNumbers(o1.HandleCount, o2.HandleCount, si->SortAscending);

	}

	//ATLASSERT(false);
	return false;
}

CString CObjectsView::GetObjectDetails(ObjectInfo* info) const {
	auto h = ObjectManager::DupHandle(info);	// info->LocalHandle.get();
	if (!h)
		return L"";

	auto type = ObjectTypeFactory::CreateObjectType(info->TypeIndex, info->TypeName);
	CString details = type ? type->GetDetails(h) : L"";
	::CloseHandle(h);
	return details;
}

CString CObjectsView::GetProcessHandleInfo(const HandleInfo& hi) const {
	CString info;
	info.Format(L"H: %d, PID: %d (%s)",
		hi.HandleValue, hi.ProcessId, m_ProcMgr.GetProcessNameById(hi.ProcessId).c_str());
	return info;
}

HWND CObjectsView::CreateToolBar() {
	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
	tb.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

	CImageList tbImages;
	tbImages.Create(24, 24, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
		int state = TBSTATE_ENABLED;
		PCWSTR text = nullptr;
	} buttons[] = {
		{ ID_HANDLES_NAMEDOBJECTSONLY, IDI_FONT, BTNS_CHECK | BTNS_SHOWTEXT, TBSTATE_ENABLED, L"Named Objects Only" },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = b.image == 0 ? I_IMAGENONE : tbImages.AddIcon(AtlLoadIconImage(b.image, 0, 24, 24));
			tb.AddButton(b.id, b.style, b.state, image, b.text, 0);
		}
	}

	return hWndToolBar;
}

LRESULT CObjectsView::OnActivatePage(UINT, WPARAM, LPARAM, BOOL&) {
	return LRESULT();
}

LRESULT CObjectsView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		Refresh();
		auto si = GetSortInfo();
		if (si && si->SortColumn >= 0)
			DoSort(si);
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
	return 0;
}

LRESULT CObjectsView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0)
		return 0;

	CString text;
	for (int i = 0; i < ColumnCount; i++) {
		CString temp;
		m_List.GetItemText(selected, i, temp);
		text += temp + ", ";
	}

	ClipboardHelper::CopyText(*this, text.Left(text.GetLength() - 2));

	return 0;
}

LRESULT CObjectsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto hWndToolBar = CreateToolBar();
	AddSimpleReBarBand(hWndToolBar);

	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle);

	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 140 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 330 },
		{ L"Handles", 80, LVCFMT_RIGHT },
		{ L"First Handle", 220, LVCFMT_LEFT },
		{ L"Details", 500 },
	};

	ColumnCount = _countof(columns);

	int i = 0;
	for (auto& c : columns)
		m_List.InsertColumn(i++, c.Header, c.Format, c.Width);

	m_List.SetImageList(GetFrame()->GetImageList(), LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CObjectsView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CObjectsView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = GetItem(item.iItem);

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:	// type
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				item.pszText = (PWSTR)(PCWSTR)data->Name;
				break;

			case 3:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->HandleCount);
				break;

			case 4:	// first handle
				::StringCchCopy(item.pszText, item.cchTextMax, GetProcessHandleInfo(*data->Handles[0].get()));
				break;

			case 5:	// details
				::StringCchCopy(item.pszText, item.cchTextMax, GetObjectDetails(data.get()));

		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = GetFrame()->GetIconIndexByType((PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName);
	}
	return 0;
}

LRESULT CObjectsView::OnContextMenu(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMITEMACTIVATE*)hdr;
	CMenu menu = AtlLoadMenu(IDR_CONTEXT);
	GetFrame()->TrackPopupMenu(menu.GetSubMenu(2), *this);
	return FALSE;
}

LRESULT CObjectsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CObjectsView::OnItemChanged(int, LPNMHDR, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	m_pUpdateUI->UIEnable(ID_EDIT_COPY, selected >= 0);
	m_pUpdateUI->UIEnable(ID_OBJECTS_ALLHANDLESFOROBJECT, selected >= 0);
	m_pUpdateUI->UIEnable(ID_EDIT_SECURITY, selected >= 0);

	return 0;
}

LRESULT CObjectsView::OnShowAllHandles(WORD, WORD, HWND, BOOL&) {
	ATLASSERT(m_List.GetSelectedIndex() >= 0);
	auto& item = GetItem(m_List.GetSelectedIndex());
	CObjectHandlesDlg dlg(item.get(), m_ProcMgr);
	CImageList il = GetFrame()->GetImageList();
	dlg.DoModal(*this, (LPARAM)il.GetIcon(GetFrame()->GetIconIndexByType(item->TypeName)));

	return 0;
}

LRESULT CObjectsView::OnShowNamedObjectsOnly(WORD, WORD, HWND, BOOL&) {
	m_NamedObjectsOnly = !m_NamedObjectsOnly;
	m_pUpdateUI->UISetCheck(ID_HANDLES_NAMEDOBJECTSONLY, m_NamedObjectsOnly);
	Refresh();

	return 0;
}

LRESULT CObjectsView::OnEditSecurity(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	auto& item = m_Objects[selected];
	auto& handle = item->Handles[0];

	auto hDup = DriverHelper::DupHandle(UlongToHandle(handle->HandleValue), handle->ProcessId, READ_CONTROL | WRITE_DAC);
	if (!hDup)
		hDup = DriverHelper::DupHandle(UlongToHandle(handle->HandleValue), handle->ProcessId, READ_CONTROL);

	if (!hDup) {
		AtlMessageBox(*this, L"Error in handle duplication", IDR_MAINFRAME, MB_ICONERROR);
		return 0;
	}
	SecurityInfo info(hDup, item->Name);
	if (!::EditSecurity(*this, &info)) {
		AtlMessageBox(*this, L"Error launching security dialog box", IDR_MAINFRAME, MB_ICONERROR);
	}
	::CloseHandle(hDup);

	return 0;
}

void CObjectsView::Refresh() {
	CWaitCursor wait;
	m_ObjMgr.EnumHandlesAndObjects(m_Typename, 0, nullptr, m_NamedObjectsOnly);
	m_ProcMgr.EnumProcesses();
	m_Objects = m_ObjMgr.GetObjects();
	if (GetSortColumn(m_List) >= 0)
		DoSort(GetSortInfo());

	m_List.SetItemCountEx(static_cast<int>(m_Objects.size()), LVSICF_NOSCROLL);
}
