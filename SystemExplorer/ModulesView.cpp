#include "pch.h"
#include "ModulesView.h"
#include <algorithm>
#include "SortHelper.h"
#include "resource.h"

CModulesView::CModulesView(DWORD pid, IMainFrame* frame) : CViewBase(frame), m_Tracker(pid) {
}

CModulesView::CModulesView(HANDLE hProcess, IMainFrame* frame) : CViewBase(frame), m_Tracker(hProcess) {
}

CString CModulesView::GetColumnText(HWND, int row, int col) const {
	const auto& mi = m_Modules[row];
	CString text;

	switch (col) {
		case 0: return mi->Name.empty() ? L"<Pagefile Backed>" : mi->Name.c_str();
		case 1: return mi->Type == WinSys::MapType::Image ? L"Image" : L"Data";
		case 2: text.Format(L"0x%08X", mi->ModuleSize); break;
		case 3: text.Format(L"0x%p", mi->Base); break;
		case 4:
			if (mi->Type == WinSys::MapType::Image)
				text.Format(L"0x%p", mi->ImageBase);
			else
				text = L"N/A";
			break;
		case 5: return mi->Path.c_str();
		case 6: return mi->Type == WinSys::MapType::Image ? CharacteristicsToString(mi->Characteristics) : L"";
	}

	return text;
}

int CModulesView::GetRowImage(HWND, int row) const {
	auto& mi = m_Modules[row];

	if (mi->Type == WinSys::MapType::Data)
		return 1;

	return mi->ImageBase == mi->Base ? 0 : 2;
}

void CModulesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Modules.begin(), m_Modules.end(), [=](auto& m1, auto& m2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(m1->Name, m2->Name, si->SortAscending);
			case 1: return SortHelper::SortNumbers(m1->Type, m2->Type, si->SortAscending);
			case 2: return SortHelper::SortNumbers(m1->ModuleSize, m2->ModuleSize, si->SortAscending);
			case 3: return SortHelper::SortNumbers(m1->Base, m2->Base, si->SortAscending);
			case 4: return SortHelper::SortNumbers(m1->ImageBase, m2->ImageBase, si->SortAscending);
			case 5: return SortHelper::SortStrings(m1->Path, m2->Path, si->SortAscending);
			case 6: return SortHelper::SortNumbers(m1->Characteristics, m2->Characteristics, si->SortAscending);
		}
		return false;
		});

}

LRESULT CModulesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 4);
	UINT ids[] = { IDI_DLL, IDI_DLL_DB, IDI_DLL_RELOC };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 60, ColumnFlags::Visible);
	cm->AddColumn(L"Size", LVCFMT_RIGHT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"Base Address", LVCFMT_RIGHT, 130, ColumnFlags::Visible);
	cm->AddColumn(L"Image Base", LVCFMT_RIGHT, 130, ColumnFlags::Visible);
	cm->AddColumn(L"Path", LVCFMT_LEFT, 450, ColumnFlags::Visible);
	cm->AddColumn(L"Characteristics", LVCFMT_LEFT, 250, ColumnFlags::Visible);

	cm->UpdateColumns();

	Refresh();
	SetTimer(1, m_UpdateInterval, nullptr);

	return 0;
}

LRESULT CModulesView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id != 1)
		return 0;

	Refresh();

	return 0;
}

LRESULT CModulesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	KillTimer(1);

	return 0;
}

LRESULT CModulesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (activate) {
		Refresh();
		SetTimer(1, m_UpdateInterval, nullptr);
	}
	else
		KillTimer(1);

	return 0;
}

LRESULT CModulesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CModulesView::Refresh() {
	auto first = m_Modules.empty();
	m_Tracker.EnumModules();
	if (first) {
		m_Modules = m_Tracker.GetModules();
	}
	else {
		auto count = static_cast<int>(m_Modules.size());
		for (int i = 0; i < count; i++) {
			auto& mi = m_Modules[i];
			auto& mx = GetModuleEx(mi.get());
			if (mx.IsUnloaded && ::GetTickCount64() > mx.TargetTime) {
				ATLTRACE(L"Module unload end: %u %s (0x%p)\n", ::GetTickCount(), mi->Name.c_str(), mi->Base);
				m_ModulesEx.erase(mi.get());
				m_Modules.erase(m_Modules.begin() + i);
				i--;
				count--;
			}
			else if (mx.IsNew && !mx.IsUnloaded && ::GetTickCount64() > mx.TargetTime) {
				mx.IsNew = false;
			}
		}

		for (auto& mi : m_Tracker.GetNewModules()) {
			m_Modules.push_back(mi);
			auto& mx = GetModuleEx(mi.get());
			mx.IsNew = true;
			mx.TargetTime = ::GetTickCount64() + 2000;
		}
		for (auto& mi : m_Tracker.GetUnloadedModules()) {
			ATLTRACE(L"Module unload start: %u %s (0x%p)\n", ::GetTickCount(), mi->Name.c_str(), mi->Base);
			auto& mx = GetModuleEx(mi.get());
			ATLASSERT(!mx.IsUnloaded);
			mx.IsNew = false;
			mx.IsUnloaded = true;
			mx.TargetTime = ::GetTickCount64() + 2000;
		}
	}

	auto count = static_cast<int>(m_Modules.size());
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	DoSort(GetSortInfo(m_List));

	if (!first) {
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
}

CModulesView::ModuleInfoEx& CModulesView::GetModuleEx(WinSys::ModuleInfo* mi) {
	auto it = m_ModulesEx.find(mi);
	if (it != m_ModulesEx.end())
		return it->second;

	ModuleInfoEx mx;
	m_ModulesEx.insert({ mi, mx });
	return GetModuleEx(mi);
}

CString CModulesView::CharacteristicsToString(WinSys::DllCharacteristics ch) {
	using namespace WinSys;

	CString result;
	if ((ch & DllCharacteristics::HighEntropyVA) != DllCharacteristics::None)
		result += L"High Entropy VA, ";
	if ((ch & DllCharacteristics::DynamicBase) != DllCharacteristics::None)
		result += L"Dynamic Base, ";
	if ((ch & DllCharacteristics::ForceIntegrity) != DllCharacteristics::None)
		result += L"Force Integrity, ";
	if ((ch & DllCharacteristics::NxCompat) != DllCharacteristics::None)
		result += L"NX Compat, ";
	if ((ch & DllCharacteristics::NoIsolation) != DllCharacteristics::None)
		result += L"No Isolation, ";
	if ((ch & DllCharacteristics::NoSEH) != DllCharacteristics::None)
		result += L"No SEH, ";
	if ((ch & DllCharacteristics::NoBind) != DllCharacteristics::None)
		result += L"No Bind, ";
	if ((ch & DllCharacteristics::AppContainer) != DllCharacteristics::None)
		result += L"App Container, ";
	if ((ch & DllCharacteristics::WDMDriver) != DllCharacteristics::None)
		result += L"WDM Driver, ";
	if ((ch & DllCharacteristics::ControlFlowGuard) != DllCharacteristics::None)
		result += L"CFG, ";
	if ((ch & DllCharacteristics::TerminalServerAware) != DllCharacteristics::None)
		result += L"TS Aware, ";

	if (!result.IsEmpty())
		result = result.Left(result.GetLength() - 2);
	return result;
}

DWORD CModulesView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	int index = (int)cd->dwItemSpec;
	auto& mi = m_Modules[index];
	auto& mx = GetModuleEx(mi.get());
	if (mx.IsUnloaded)
		lcd->clrTextBk = RGB(255, 0, 0);
	else if (mx.IsNew)
		lcd->clrTextBk = RGB(0, 255, 0);
	else if (mi->Type == WinSys::MapType::Image && mi->Base != mi->ImageBase)
		lcd->clrTextBk = RGB(255, 255, 128);

	return CDRF_DODEFAULT;
}

DWORD CModulesView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	return CDRF_NOTIFYITEMDRAW;
}

