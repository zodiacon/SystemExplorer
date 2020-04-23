#include "stdafx.h"
#include "ServicesView.h"
#include "SecurityHelper.h"
#include "ProgressDlg.h"
#include <algorithm>
#include "SortHelper.h"

using namespace WinSys;

CServicesView::CServicesView(IMainFrame* pFrame) : m_pFrame(pFrame) {
}

BOOL CServicesView::OnIdle() {
	UIUpdateToolBar();
	return FALSE;
}

void CServicesView::DoSort(const SortInfo* si) {
	std::sort(m_Services.begin(), m_Services.end(), [&](const auto& s1, const auto& s2) {
		return CompareItems(s1, s2, si->SortColumn, si->SortAscending);
		});
}

bool CServicesView::IsSortable(int col) const {
	if (SecurityHelper::IsRunningElevated())
		return true;

	return col != 5;
}

LRESULT CServicesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	m_RunningElevated = SecurityHelper::IsRunningElevated();
	auto hWndToolBar = m_ToolBar.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, ATL_IDW_TOOLBAR);
	InitToolBar(m_ToolBar);
	UIAddToolBar(hWndToolBar);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(m_ToolBar);

	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 170, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Display Name", LVCFMT_LEFT, 280, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"State", LVCFMT_LEFT, 80, ColumnFlags::Visible);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150, ColumnFlags::Visible);
	cm->AddColumn(L"Start Type", LVCFMT_LEFT, 150, ColumnFlags::Visible);

	cm->UpdateColumns();

	UINT icons[] = { 
		IDI_SERVICE, IDI_SERVICE_RUNNING, IDI_SERVICE_STOP, IDI_SERVICE_PAUSE
	};
	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_COLOR, 8, 2);
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 64, 16, 16));

	m_List.SetImageList(images, LVSIL_SMALL);

	auto pLoop = _Module.GetMessageLoop();
	pLoop->AddIdleHandler(this);

	Refresh();

	return 0;
}

LRESULT CServicesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL& handled) {
	handled = FALSE;
	_Module.GetMessageLoop()->RemoveIdleHandler(this);
	return 0;
}

LRESULT CServicesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	return 0;
}

LRESULT CServicesView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = m_Services[item.iItem];
	auto& pdata = data.GetStatusProcess();

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:		// name
				item.pszText = (PWSTR)data.GetName().c_str();
				break;

			case 1:		// display name
				item.pszText = (PWSTR)data.GetDisplayName().c_str();
				break;

			case 2:		// state
				item.pszText = (PWSTR)ServiceStateToString(pdata.CurrentState);
				break;

			case 3:		// PID
				if (pdata.ProcessId > 0) {
					::StringCchPrintf(item.pszText, item.cchTextMax, L"%u (0x%X)", pdata.ProcessId, pdata.ProcessId);
				}
				break;

			case 4:		// process name
				if (pdata.ProcessId > 0) {
					::StringCchCopy(item.pszText, item.cchTextMax, m_ProcMgr.GetProcessNameById(pdata.ProcessId).c_str());
				}
				break;

			case 5:		// start type
				auto config = GetServiceInfoEx(data.GetName()).GetConfiguration();
				::StringCchCopy(item.pszText, item.cchTextMax, config ? ServiceStartTypeToString(*config) : L"<access denied>");
				break;

		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = ServiceStatusToImage(data.GetStatusProcess().CurrentState);
	}

	return 0;
}

LRESULT CServicesView::OnListRightClick(int, LPNMHDR hdr, BOOL&) {
	POINT pt;
	::GetCursorPos(&pt);
	CPoint pt2(pt);
	auto header = m_List.GetHeader();
	header.ScreenToClient(&pt);
	HDHITTESTINFO hti;
	hti.pt = pt;
	int index = header.HitTest(&hti);
	CMenuHandle hSubMenu;
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	if (index >= 0) {
		// right-click header
		hSubMenu = menu.GetSubMenu(7);
		pt = pt2;
	}
	else {
		index = m_List.GetSelectedIndex();
		if (index >= 0) {
			auto item = (NMITEMACTIVATE*)hdr;
			hSubMenu = menu.GetSubMenu(6);
			pt = item->ptAction;
			m_List.ClientToScreen(&pt);
			UpdateUI(m_pFrame->GetUpdateUI());
		}
	}
	if(hSubMenu)
		m_pFrame->TrackPopupMenu(hSubMenu, nullptr, &pt);
	return 0;
}

LRESULT CServicesView::OnItemStateChanged(int, LPNMHDR, BOOL&) {
	UpdateUI(this);
	return 0;
}

LRESULT CServicesView::OnServiceStart(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	ATLASSERT(index >= 0);
	auto& svc = m_Services[index];
	ATLASSERT(svc.GetStatusProcess().CurrentState == ServiceState::Stopped);

	auto service = Service::Open(svc.GetName(), ServiceAccessMask::Start | ServiceAccessMask::QueryStatus);
	if (service == nullptr) {
		AtlMessageBox(*this, L"Failed to open service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	if (!service->Start()) {
		AtlMessageBox(*this, L"Failed to start service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}
	CProgressDlg dlg;
	dlg.ShowCancelButton(false);
	dlg.SetMessageText((L"Starting service " + svc.GetName() + L"...").c_str());
	dlg.SetProgressMarquee(true);
	dlg.SetTimerCallback([&]() {
		service->Refresh(svc);
		if (svc.GetStatusProcess().CurrentState == ServiceState::Running)
			dlg.Close();
		}, 500);
	dlg.DoModal();
	m_List.RedrawItems(index, index);

	return 0;
}

LRESULT CServicesView::OnServiceStop(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	ATLASSERT(index >= 0);
	auto& svc = m_Services[index];
	ATLASSERT(svc.GetStatusProcess().CurrentState == ServiceState::Running);

	auto service = Service::Open(svc.GetName(), ServiceAccessMask::Stop | ServiceAccessMask::QueryStatus);
	if (service == nullptr) {
		AtlMessageBox(*this, L"Failed to open service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	if (!service->Stop()) {
		AtlMessageBox(*this, L"Failed to stop service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	CProgressDlg dlg;
	dlg.ShowCancelButton(false);
	dlg.SetMessageText((L"Stopping service " + svc.GetName() + L"...").c_str());
	dlg.SetProgressMarquee(true);
	dlg.SetTimerCallback([&]() {
		service->Refresh(svc);
		if (svc.GetStatusProcess().CurrentState == ServiceState::Stopped)
			dlg.Close();
		}, 500);
	dlg.DoModal();
	m_List.RedrawItems(index, index);

	return 0;
}

LRESULT CServicesView::OnServicePause(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	ATLASSERT(index >= 0);
	auto& svc = m_Services[index];
	ATLASSERT(svc.GetStatusProcess().CurrentState == ServiceState::Running);

	auto service = Service::Open(svc.GetName(), ServiceAccessMask::PauseContinue | ServiceAccessMask::QueryStatus);
	if (service == nullptr) {
		AtlMessageBox(*this, L"Failed to open service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	if (!service->Pause()) {
		AtlMessageBox(*this, L"Failed to pause service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	CProgressDlg dlg;
	dlg.ShowCancelButton(false);
	dlg.SetMessageText((L"Pausing service " + svc.GetName() + L"...").c_str());
	dlg.SetProgressMarquee(true);
	dlg.SetTimerCallback([&]() {
		service->Refresh(svc);
		if (svc.GetStatusProcess().CurrentState == ServiceState::Paused)
			dlg.Close();
		}, 500);
	dlg.DoModal();
	m_List.RedrawItems(index, index);
	
	return 0;
}

LRESULT CServicesView::OnServiceContinue(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	ATLASSERT(index >= 0);
	auto& svc = m_Services[index];
	ATLASSERT(svc.GetStatusProcess().CurrentState == ServiceState::Paused);

	auto service = Service::Open(svc.GetName(), ServiceAccessMask::PauseContinue | ServiceAccessMask::QueryStatus);
	if (service == nullptr) {
		AtlMessageBox(*this, L"Failed to open service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	if (!service->Continue()) {
		AtlMessageBox(*this, L"Failed to resume service", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	CProgressDlg dlg;
	dlg.ShowCancelButton(false);
	dlg.SetMessageText((L"Resuming service " + svc.GetName() + L"...").c_str());
	dlg.SetProgressMarquee(true);
	dlg.SetTimerCallback([&]() {
		service->Refresh(svc);
		if (svc.GetStatusProcess().CurrentState == ServiceState::Running)
			dlg.Close();
		}, 500);
	dlg.DoModal();
	m_List.RedrawItems(index, index);

	return 0;
}

bool CServicesView::CompareItems(const ServiceInfo& s1, const ServiceInfo& s2, int col, bool asc) {
	switch (col) {
		case 0:	return SortHelper::SortStrings(s1.GetName(), s2.GetName(), asc);
		case 1: return SortHelper::SortStrings(s1.GetDisplayName(), s2.GetDisplayName(), asc);
		case 2: return SortHelper::SortStrings(
			ServiceStateToString(s1.GetStatusProcess().CurrentState), ServiceStateToString(s2.GetStatusProcess().CurrentState), asc);
		case 3: return SortHelper::SortNumbers(s1.GetStatusProcess().ProcessId, s2.GetStatusProcess().ProcessId, asc);
		case 4: return SortHelper::SortStrings(m_ProcMgr.GetProcessNameById(s1.GetStatusProcess().ProcessId), m_ProcMgr.GetProcessNameById(s2.GetStatusProcess().ProcessId), asc);
		case 5:	return SortHelper::SortStrings(ServiceStartTypeToString(*GetServiceInfoEx(s1.GetName()).GetConfiguration()),
					ServiceStartTypeToString(*GetServiceInfoEx(s2.GetName()).GetConfiguration()), asc);
	}
	return false;
}

int CServicesView::ServiceStatusToImage(ServiceState state) {
	switch (state) {
		case ServiceState::Running: return 1;
		case ServiceState::Stopped: return 2;
		case ServiceState::Paused: return 3;
	}

	return 0;
}

PCWSTR CServicesView::ServiceStateToString(ServiceState state) {
	switch (state) {
		case ServiceState::Running: return L"Running";
		case ServiceState::Stopped: return L"Stopped";
		case ServiceState::Paused: return L"Paused";
		case ServiceState::StartPending: return L"Start Pending";
		case ServiceState::ContinuePending: return L"Continue Pending";
		case ServiceState::StopPending: return L"Stop Pending";
		case ServiceState::PausePending: return L"Pause Pending";
	}
	return L"Unknown";
}

CString CServicesView::ServiceStartTypeToString(const ServiceConfiguration& config) {
	CString type;

	switch (config.StartType) {
		case ServiceStartType::Auto: type = L"Automatic"; break;
		case ServiceStartType::Demand: type = L"Manual"; break;
		case ServiceStartType::Disabled: type = L"Disabled"; break;
	}

	if (config.DelayedAutoStart)
		type += L" (Delayed)";
	if (config.TriggerStart)
		type += L" (Trigger)";

	return type;
}

ServiceInfoEx& CServicesView::GetServiceInfoEx(const std::wstring& name) {
	auto it = m_ServicesEx.find(name);
	if (it != m_ServicesEx.end())
		return it->second;

	ServiceInfoEx infox(name.c_str());
	auto pos = m_ServicesEx.insert({ name, std::move(infox) });
	return pos.first->second;
}

void CServicesView::InitToolBar(CToolBarCtrl& tb) {
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
		{ ID_SERVICE_START, IDI_PLAY },
		{ ID_SERVICE_STOP, IDI_STOP },
		{ ID_SERVICE_PAUSE, IDI_PAUSE },
		{ ID_SERVICE_CONTINUE, IDI_RESUME },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = b.image == 0 ? I_IMAGENONE : tbImages.AddIcon(AtlLoadIconImage(b.image, 0, 24, 24));
			tb.AddButton(b.id, b.style, b.state, image, b.text, 0);
		}
	}
}

void CServicesView::Refresh() {
	m_ProcMgr.EnumProcesses();
	m_ServicesEx.clear();
	m_Services = WinSys::ServiceManager::EnumServices();
	m_ServicesEx.reserve(m_Services.size());
	m_List.SetItemCount(static_cast<int>(m_Services.size()));
	UpdateUI(this);
}

void CServicesView::UpdateUI(CUpdateUIBase* ui) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0 || !m_RunningElevated) {
		ui->UIEnable(ID_SERVICE_START, FALSE);
		ui->UIEnable(ID_SERVICE_STOP, FALSE);
		ui->UIEnable(ID_SERVICE_PAUSE, FALSE);
		ui->UIEnable(ID_SERVICE_CONTINUE, FALSE);
	}
	else {
		auto& svc = m_Services[selected];
		auto& state = svc.GetStatusProcess().CurrentState;
		ui->UIEnable(ID_SERVICE_START, state == ServiceState::Stopped && GetServiceInfoEx(svc.GetName()).GetConfiguration()->StartType != ServiceStartType::Disabled);
		ui->UIEnable(ID_SERVICE_STOP, state == ServiceState::Running);
		ui->UIEnable(ID_SERVICE_PAUSE, state == ServiceState::Running);
		ui->UIEnable(ID_SERVICE_CONTINUE, state == ServiceState::Paused);
	}
}

ServiceInfoEx::ServiceInfoEx(PCWSTR name) : _name(name) {
}

const WinSys::ServiceConfiguration* ServiceInfoEx::GetConfiguration() const {
	if (_config == nullptr)
		_config = ServiceManager::GetServiceConfiguration(_name);
	return _config.get();
}
