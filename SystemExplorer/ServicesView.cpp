#include "pch.h"
#include "ServicesView.h"
#include "SecurityHelper.h"
#include "ProgressDlg.h"
#include <algorithm>
#include "SortHelper.h"
#include "SelectColumnsDlg.h"

using namespace WinSys;

PCWSTR AccessDenied = L"<access denied>";

CServicesView::CServicesView(IMainFrame* pFrame) : CViewBase(pFrame) {
}

void CServicesView::DoSort(const SortInfo* si) {
	std::sort(m_Services.begin(), m_Services.end(), [&](const auto& s1, const auto& s2) {
		return CompareItems(s1, s2, si->SortColumn, si->SortAscending);
		});
}

bool CServicesView::IsSortable(int col) const {
	if (SecurityHelper::IsRunningElevated())
		return true;

	static auto dummyConfig = GetServiceInfoEx(m_Services[0].GetName()).GetConfiguration();
	return dummyConfig != nullptr;
}

int CServicesView::GetRowImage(int row) const {
	return ServiceStatusToImage(m_Services[row].GetStatusProcess().CurrentState);
}

LRESULT CServicesView::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	m_RunningElevated = SecurityHelper::IsRunningElevated();

	InitToolBar();

	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 170, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Display Name", LVCFMT_LEFT, 280, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"State", LVCFMT_LEFT, 80, ColumnFlags::Visible);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150, ColumnFlags::Visible);
	cm->AddColumn(L"Start Type", LVCFMT_LEFT, 150, ColumnFlags::Visible);
	cm->AddColumn(L"Binary Path", LVCFMT_LEFT, 250, SecurityHelper::IsRunningElevated() ? ColumnFlags::Visible : ColumnFlags::None);
	cm->AddColumn(L"Account Name", LVCFMT_LEFT, 150, SecurityHelper::IsRunningElevated() ? ColumnFlags::Visible : ColumnFlags::None);
	cm->AddColumn(L"Error Control", LVCFMT_LEFT, 80, ColumnFlags::None);
	cm->AddColumn(L"Description", LVCFMT_LEFT, 250, ColumnFlags::None);
	cm->AddColumn(L"Privileges", LVCFMT_LEFT, 180, ColumnFlags::None);
	cm->AddColumn(L"Triggers", LVCFMT_LEFT, 200, ColumnFlags::None);
	cm->AddColumn(L"Dependencies", LVCFMT_LEFT, 200, ColumnFlags::None);

	cm->UpdateColumns();

	UINT icons[] = { 
		IDI_SERVICE, IDI_SERVICE_RUNNING, IDI_SERVICE_STOP, IDI_SERVICE_PAUSE
	};
	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_COLOR, 8, 2);
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 64, 16, 16));

	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();
	bHandled = FALSE;

	return 0;
}

LRESULT CServicesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (activate)
		Refresh();
	return 0;
}

CString CServicesView::GetColumnText(HWND, int row, int col) const {
	auto& data = m_Services[row];
	auto& pdata = data.GetStatusProcess();
	auto& svcex = GetServiceInfoEx(data.GetName());
	auto config = svcex.GetConfiguration();

	CString text;
	switch (col) {
		case 0: return data.GetName().c_str();
		case 1:	return data.GetDisplayName().c_str();
		case 2: return ServiceStateToString(pdata.CurrentState);
		case 3: return ServiceTypeToString(pdata.Type);
		case 4:
			if (pdata.ProcessId > 0) {
				text.Format(L"%u (0x%X)", pdata.ProcessId, pdata.ProcessId);
			}
			break;

		case 5:	return pdata.ProcessId > 0 ? m_ProcMgr.GetProcessNameById(pdata.ProcessId).c_str() : L"";
		case 6:	return config ? ServiceStartTypeToString(*config) : AccessDenied;
		case 7:	return config ? config->BinaryPathName.c_str() : AccessDenied;
		case 8: return config ? config->AccountName.c_str() : AccessDenied;
		case 9: return config ? ErrorControlToString(config->ErrorControl) : AccessDenied;
		case 10: return svcex.GetDescription();
		case 11: return svcex.GetPrivileges();
		case 12: return svcex.GetTriggers();
		case 13: return config ? svcex.GetDependencies() : AccessDenied;
	}

	return text;
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
		auto cm = GetColumnManager(m_List);
		m_SelectedHeader = cm->GetRealColumn(index);
		GetFrame()->GetUpdateUI()->UIEnable(ID_HEADER_HIDECOLUMN,
			(cm->GetColumn(m_SelectedHeader).Flags & ColumnFlags::Mandatory) == ColumnFlags::None);
		pt = pt2;
	}
	else {
		index = m_List.GetSelectedIndex();
		if (index >= 0) {
			auto item = (NMITEMACTIVATE*)hdr;
			hSubMenu = menu.GetSubMenu(6);
			pt = item->ptAction;
			m_List.ClientToScreen(&pt);
			UpdateUI(GetFrame()->GetUpdateUI());
		}
	}
	if(hSubMenu)
		GetFrame()->TrackPopupMenu(hSubMenu, nullptr, &pt);
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
	UpdateUI(this);

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
	UpdateUI(this);

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
	UpdateUI(this);

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
	UpdateUI(this);

	return 0;
}

LRESULT CServicesView::OnHideColumn(WORD, WORD, HWND, BOOL&) {
	auto cm = GetColumnManager(m_List);
	cm->SetVisible(m_SelectedHeader, false);
	cm->UpdateColumns();
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());

	return 0;
}

LRESULT CServicesView::OnSelectColumns(WORD, WORD, HWND, BOOL&) {
	CSelectColumnsDlg dlg(GetColumnManager(m_List));
	if (IDOK == dlg.DoModal()) {
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}

	return 0;
}

LRESULT CServicesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

bool CServicesView::CompareItems(const ServiceInfo& s1, const ServiceInfo& s2, int col, bool asc) {
	switch (col) {
		case 0:	return SortHelper::SortStrings(s1.GetName(), s2.GetName(), asc);
		case 1: return SortHelper::SortStrings(s1.GetDisplayName(), s2.GetDisplayName(), asc);
		case 2: return SortHelper::SortStrings(
			ServiceStateToString(s1.GetStatusProcess().CurrentState), ServiceStateToString(s2.GetStatusProcess().CurrentState), asc);
		case 3: return SortHelper::SortNumbers(s1.GetStatusProcess().Type, s2.GetStatusProcess().Type, asc);
		case 4: return SortHelper::SortNumbers(s1.GetStatusProcess().ProcessId, s2.GetStatusProcess().ProcessId, asc);
		case 5: return SortHelper::SortStrings(
			m_ProcMgr.GetProcessNameById(s1.GetStatusProcess().ProcessId), 
			m_ProcMgr.GetProcessNameById(s2.GetStatusProcess().ProcessId), asc);
		case 6:	return SortHelper::SortStrings(ServiceStartTypeToString(*GetServiceInfoEx(s1.GetName()).GetConfiguration()),
					ServiceStartTypeToString(*GetServiceInfoEx(s2.GetName()).GetConfiguration()), asc);
		case 7: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetConfiguration()->BinaryPathName,
			GetServiceInfoEx(s2.GetName()).GetConfiguration()->BinaryPathName, asc);
		case 8: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetConfiguration()->AccountName,
			GetServiceInfoEx(s2.GetName()).GetConfiguration()->AccountName, asc);
		case 9: return SortHelper::SortNumbers(
			GetServiceInfoEx(s1.GetName()).GetConfiguration()->ErrorControl,
			GetServiceInfoEx(s2.GetName()).GetConfiguration()->ErrorControl, asc);
		case 10: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetDescription(),
			GetServiceInfoEx(s2.GetName()).GetDescription(), asc);
		case 11: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetPrivileges(),
			GetServiceInfoEx(s2.GetName()).GetPrivileges(), asc);
		case 12: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetTriggers(),
			GetServiceInfoEx(s2.GetName()).GetTriggers(), asc);
		case 13: return SortHelper::SortStrings(
			GetServiceInfoEx(s1.GetName()).GetDependencies(),
			GetServiceInfoEx(s2.GetName()).GetDependencies(), asc);

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

PCWSTR CServicesView::ErrorControlToString(WinSys::ServiceErrorControl ec) {
	using namespace WinSys;

	switch (ec) {
		case ServiceErrorControl::Normal: return L"Normal (1)";
		case ServiceErrorControl::Ignore: return L"Ignore (0)";
		case ServiceErrorControl::Critical: return L"Critical (3)";
		case ServiceErrorControl::Severe: return L"Severe (2)";
	}
	ATLASSERT(false);
	return L"";
}

CString CServicesView::ServiceTypeToString(WinSys::ServiceType type) {
	using namespace WinSys;

	static struct {
		ServiceType type;
		PCWSTR text;
	} types[] = {
		{ ServiceType::Win32OwnProcess, L"Own" },
		{ ServiceType::Win32SharedProcess, L"Shared" },
		{ ServiceType::InteractiveProcess, L"Interactive" },
		{ ServiceType::UserService, L"User" },
		{ ServiceType::UserServiceInstance, L"Instance" },
		{ ServiceType::PackageService, L"Packaged" },
	};

	CString text;
	for (auto& item : types)
		if ((item.type & type) == item.type)
			text += CString(item.text) + L", ";

	return text.Left(text.GetLength() - 2);
}

CString CServicesView::DependenciesToString(const std::vector<std::wstring>& deps) {
	CString text;
	for (auto& dep : deps)
		text += CString(dep.c_str()) + L", ";
	return text.Left(text.GetLength() - 2);
}

ServiceInfoEx& CServicesView::GetServiceInfoEx(const std::wstring& name) const {
	auto it = m_ServicesEx.find(name);
	if (it != m_ServicesEx.end())
		return it->second;

	ServiceInfoEx infox(name.c_str());
	auto pos = m_ServicesEx.insert({ name, std::move(infox) });
	return pos.first->second;
}

HWND CServicesView::InitToolBar() {
	const ToolBarButtonInfo buttons[] = {
		{ ID_SERVICE_START, IDI_PLAY, 0, L"Start" },
		{ ID_SERVICE_STOP, IDI_STOP, 0, L"Stop" },
		{ ID_SERVICE_PAUSE, IDI_PAUSE },
		{ ID_SERVICE_CONTINUE, IDI_RESUME },
		{ 0 },
		{ ID_HEADER_COLUMNS, IDI_EDITCOLUMNS, 0, L"Columns" },
	};
	return CreateAndInitToolBar(buttons, _countof(buttons));
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
		ui->UIEnable(ID_SERVICE_START, state == ServiceState::Stopped && 
			GetServiceInfoEx(svc.GetName()).GetConfiguration()->StartType != ServiceStartType::Disabled);
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

const CString& ServiceInfoEx::GetDescription() const {
	if (_desc.IsEmpty()) {
		_desc = WinSys::ServiceManager::GetServiceDescription(_name).c_str();
	}
	return _desc;
}

const CString& ServiceInfoEx::GetPrivileges() const {
	if (!_flagPriveleges) {
		auto svc = Service::Open(_name, ServiceAccessMask::QueryConfig);
		if (svc) {
			auto privs = svc->GetRequiredPrivileges();
			CString text;
			for (auto& priv : privs) {
				if (!text.IsEmpty())
					text += L", ";
				text += priv.substr(2, priv.size() - 11).c_str();
			}
			_privileges = std::move(text);
		}
		_flagPriveleges = true;
	}
	return _privileges;
}

PCWSTR CServicesView::TriggerToText(const WinSys::ServiceTrigger& trigger) {
	using namespace WinSys;
	switch (trigger.Type) {
		case ServiceTriggerType::Custom: return L"Custom";
		case ServiceTriggerType::DeviceInterfaceArrival: return L"Device Arrival";
		case ServiceTriggerType::DomainJoin: return L"Domain Join";
		case ServiceTriggerType::FirewallPortEvent: return L"Firewall Port Event";
		case ServiceTriggerType::GroupPolicy: return L"Group Policy";
		case ServiceTriggerType::IpAddressAvailability: return L"IP Address Availability";
		case ServiceTriggerType::NetworkEndpoint: return L"Network Endpoint";
		case ServiceTriggerType::SystemStateChanged: return L"System State Changed";
		case ServiceTriggerType::Aggregate: return L"Aggregate";
	}
	ATLASSERT(false);
	return L"";
}

const CString& ServiceInfoEx::GetTriggers() const {
	if (!_flagTriggers) {
		_flagTriggers = true;
		auto svc = WinSys::Service::Open(_name, ServiceAccessMask::QueryConfig);
		if (svc) {
			auto triggers = svc->GetTriggers();
			CString text;
			for (auto& trigger : triggers) {
				auto desc = CServicesView::TriggerToText(trigger);
				if (text.Find(desc) < 0) {
					if (!text.IsEmpty())
						text += L", ";
					text += desc;
				}
			}
			_triggers = std::move(text);
		}
	}
	return _triggers;
}

const CString& ServiceInfoEx::GetDependencies() const {
	if (!_flagDependencies) {
		_flagDependencies = true;
		if(GetConfiguration())
			_dependencies = CServicesView::DependenciesToString(GetConfiguration()->Dependencies);
	}
	return _dependencies;
}
