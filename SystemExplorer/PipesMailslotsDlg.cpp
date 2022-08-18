#include "pch.h"
#include "PipesMailslotsDlg.h"
#include "SortHelper.h"

CPipesMailslotsDlg::CPipesMailslotsDlg(Type type) : m_Type(type) {
}

CString CPipesMailslotsDlg::GetColumnText(HWND, int row, int col) {
	auto& item = m_Objects[row];
	CString text;

	auto& h = item->Handles[0];

	switch (col) {
		case 0: return item->Name.Mid(m_Prefix.GetLength());
		case 1: text.Format(L"0x%p", item->Object); break;
		case 2: text.Format(L"%u", item->HandleCount); break;
		case 3: text.Format(L"%u (0x%X)", h->HandleValue, h->HandleValue); break;
		case 4: text.Format(L"%u (0x%X)", h->ProcessId, h->ProcessId); break;
		case 5: return GetProcessName(item.get()).c_str();
	}
	return text;
}

int CPipesMailslotsDlg::GetRowImage(HWND, int row) {
	return 0;
}

void CPipesMailslotsDlg::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Objects.begin(), m_Objects.end(), [=](const auto& i1, const auto& i2) {
		return CompareItems(i1.get(), i2.get(), si->SortColumn, si->SortAscending);
		});
}

LRESULT CPipesMailslotsDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	UINT icon = 0;
	switch (m_Type) {
		case Type::Pipes:
			SetWindowText(L"Pipes");
			icon = IDI_PIPE;
			m_Prefix = LR"(\Device\NamedPipe\)";
			break;

		case Type::Mailslots:
			SetWindowText(L"Mailslots");
			icon = IDI_MAILBOX;
			m_Prefix = LR"(\Device\Mailslot\)";
			break;

		default:
			ATLASSERT(false);
	}
	SetIcon(AtlLoadIcon(icon), FALSE);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 2);
	images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 240);
	m_List.InsertColumn(1, L"Object", LVCFMT_RIGHT, 140);
	m_List.InsertColumn(2, L"Handles", LVCFMT_RIGHT, 70);
	m_List.InsertColumn(3, L"First Handle", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(4, L"Process ID", LVCFMT_RIGHT, 120);
	m_List.InsertColumn(5, L"Process Name", LVCFMT_LEFT, 200);

	DlgResize_Init(true);

	EnumObjects();

	return 0;
}

LRESULT CPipesMailslotsDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

void CPipesMailslotsDlg::EnumObjects() {
	m_ProcMgr.EnumProcesses();
	ObjectManager om;
	om.EnumHandlesAndObjects(L"File", 0, m_Prefix);
	m_Objects = om.GetObjects();
	m_ProcessNames.clear();
	m_ProcessNames.reserve(m_Objects.size());
	m_List.SetItemCountEx(static_cast<int>(m_Objects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}

bool CPipesMailslotsDlg::CompareItems(const ObjectInfo* o1, const ObjectInfo* o2, int col, bool asc) const {
	switch (col) {
		case 0: return SortHelper::SortStrings(o1->Name, o2->Name, asc);
		case 1: return SortHelper::SortNumbers(o1->Object, o2->Object, asc);
		case 2: return SortHelper::SortNumbers(o1->HandleCount, o2->HandleCount, asc);
		case 3: return SortHelper::SortNumbers(o1->Handles[0]->HandleValue, o2->Handles[0]->HandleValue, asc);
		case 4: return SortHelper::SortNumbers(o1->Handles[0]->ProcessId, o2->Handles[0]->ProcessId, asc);
		case 5: return SortHelper::SortStrings(GetProcessName(o1), GetProcessName(o2), asc);
	}
	return false;
}

std::wstring CPipesMailslotsDlg::GetProcessName(const ObjectInfo* obj) const {
	if (const auto it = m_ProcessNames.find(obj); it != m_ProcessNames.end())
		return it->second;

	auto name = m_ProcMgr.GetProcessNameById(obj->Handles[0]->ProcessId);
	m_ProcessNames.insert({ obj, name });
	return name;
}
