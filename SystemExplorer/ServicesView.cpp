#include "pch.h"
#include "ServicesView.h"
#include "SecurityHelper.h"
#include "ProgressDlg.h"
#include <algorithm>
#include <SortHelper.h>
#include "SelectColumnsDlg.h"
#include "ProcessPropertiesDlg.h"
#include "ProcessInfoEx.h"

using namespace WinSys;

const CString AccessDenied(L"<access denied>");

CServicesView::CServicesView(IMainFrame* pFrame, bool services) : CViewBase(pFrame), m_ViewServices(services) {
}

void CServicesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Services.begin(), m_Services.end(), [&](const auto& s1, const auto& s2) {
		return CompareItems(s1, s2, GetColumnManager(m_List)->GetColumn(si->SortColumn).Tag, si->SortAscending);
		});
}

bool CServicesView::IsSortable(int col) const {
	// columns from the configuration sort services whose configuration can't be read as empty
	return true;
}

int CServicesView::GetRowImage(HWND, int row) const {
	return ServiceStatusToImage(m_Services[row].GetStatusProcess().CurrentState);
}

LRESULT CServicesView::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	InitToolBar();

	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 170, ColumnFlags::Visible | ColumnFlags::Mandatory, 0);
	cm->AddColumn(L"Display Name", LVCFMT_LEFT, 280, ColumnFlags::Visible | ColumnFlags::Mandatory, 1);
	cm->AddColumn(L"State", LVCFMT_LEFT, 80, ColumnFlags::Visible, 2);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnFlags::Visible, 3);
	if (m_ViewServices) {
		cm->AddColumn(L"PID", LVCFMT_RIGHT, 100, ColumnFlags::Visible, 4);
		cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150, ColumnFlags::Visible, 5);
	}
	cm->AddColumn(L"Start Type", LVCFMT_LEFT, 150, ColumnFlags::Visible, 6);
	cm->AddColumn(L"Binary Path", LVCFMT_LEFT, 350, SecurityHelper::IsRunningElevated() ? ColumnFlags::Visible : ColumnFlags::None, 7);
	cm->AddColumn(L"Account Name", LVCFMT_LEFT, 150, SecurityHelper::IsRunningElevated() && m_ViewServices ? ColumnFlags::Visible : ColumnFlags::None, 8);
	cm->AddColumn(L"Error Control", LVCFMT_LEFT, 80, ColumnFlags::Visible, 9);
	cm->AddColumn(L"Description", LVCFMT_LEFT, 350, ColumnFlags::None, 10);
	if (m_ViewServices) {
		cm->AddColumn(L"Privileges", LVCFMT_LEFT, 180, ColumnFlags::None, 11);
		cm->AddColumn(L"Triggers", LVCFMT_LEFT, 200, ColumnFlags::None, 12);
		cm->AddColumn(L"Dependencies", LVCFMT_LEFT, 200, ColumnFlags::None, 13);
		cm->AddColumn(L"Controls Accepted", LVCFMT_LEFT, 180, ColumnFlags::Visible, 14);
		cm->AddColumn(L"Service SID", LVCFMT_LEFT, 250, ColumnFlags::None, 15);
		cm->AddColumn(L"SID Type", LVCFMT_LEFT, 80, ColumnFlags::None, 16);
	}

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

void CServicesView::OnActivate(bool activate) {
	if (activate)
		Refresh();
}

CString CServicesView::GetColumnText(HWND, int row, int col) const {
	auto& data = m_Services[row];
	auto& pdata = data.GetStatusProcess();
	auto& svcex = GetServiceInfoEx(data.GetName());
	auto config = svcex.GetConfiguration();

	CString text;
	switch (GetColumnManager(m_List)->GetColumn(col).Tag) {
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
		case 7:	return config ? CString(config->BinaryPathName.c_str()) : AccessDenied;
		case 8: return config ? CString(config->AccountName.c_str()) : AccessDenied;
		case 9: return config ? ErrorControlToString(config->ErrorControl) : AccessDenied;
		case 10: return svcex.GetDescription();
		case 11: return svcex.GetPrivileges();
		case 12: return svcex.GetTriggers();
		case 13: return config ? svcex.GetDependencies() : AccessDenied;
		case 14: return ServiceControlsAcceptedToString(pdata.ControlsAccepted);
		case 15: return svcex.GetSID();
		case 16: return ServiceSidTypeToString(svcex.GetSidType());
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
		Frame()->GetUpdateUI()->UIEnable(ID_HEADER_HIDECOLUMN,
			(cm->GetColumn(m_SelectedHeader).Flags & ColumnFlags::Mandatory) == ColumnFlags::None);
		pt = pt2;
	}
	else {
		index = m_List.GetSelectedIndex();
		if (index >= 0) {
			auto item = (NMITEMACTIVATE*)hdr;
			hSubMenu = menu.GetSubMenu(m_ViewServices ? 6 : 10);
			pt = item->ptAction;
			m_List.ClientToScreen(&pt);
			UpdateUI(Frame()->GetUpdateUI());
		}
	}
	if (hSubMenu) {
		auto cmd = (UINT)Frame()->TrackPopupMenu(hSubMenu, nullptr, &pt, TPM_RETURNCMD);
		if (cmd)
			PostMessage(WM_COMMAND, cmd);
	}
	return 0;
}

LRESULT CServicesView::OnItemStateChanged(int, LPNMHDR, BOOL&) {
	UpdateUI(this);
	return 0;
}

LRESULT CServicesView::OnServiceStart(WORD, WORD, HWND, BOOL&) {
	ControlServiceAndWait(ServiceAccessMask::Start, [](Service& s) { return s.Start(); }, ServiceState::Running,
		L"Starting", L"Failed to start service", L"starting", 5000);
	return 0;
}

LRESULT CServicesView::OnServiceStop(WORD, WORD, HWND, BOOL&) {
	ControlServiceAndWait(ServiceAccessMask::Stop, [](Service& s) { return s.Stop(); }, ServiceState::Stopped,
		L"Stopping", L"Failed to stop service", L"stopping", 30000);
	return 0;
}

LRESULT CServicesView::OnServicePause(WORD, WORD, HWND, BOOL&) {
	ControlServiceAndWait(ServiceAccessMask::PauseContinue, [](Service& s) { return s.Pause(); }, ServiceState::Paused,
		L"Pausing", L"Failed to pause service", L"pausing", 30000);
	return 0;
}

LRESULT CServicesView::OnServiceContinue(WORD, WORD, HWND, BOOL&) {
	ControlServiceAndWait(ServiceAccessMask::PauseContinue, [](Service& s) { return s.Continue(); }, ServiceState::Running,
		L"Resuming", L"Failed to resume service", L"resuming", 30000);
	return 0;
}

//
// Asks the selected service to start/stop/pause/resume, then waits (showing progress) for it to get to the state
// that leads to. The wait can be cancelled, and it gives up after timeout ms: a service can stay in a pending state
// for good, and the dialog used to have no way out then.
//
void CServicesView::ControlServiceAndWait(ServiceAccessMask access, bool (*action)(Service&), ServiceState target,
	PCWSTR doing, PCWSTR failedText, PCWSTR pendingText, DWORD timeout) {
	auto index = m_List.GetSelectedIndex();
	if (index < 0 || index >= (int)m_Services.size())
		return;		// toolbar buttons and accelerators work without a selection too

	// a copy: m_Services may be replaced while the dialog is up
	auto svc = m_Services[index];
	auto service = Service::Open(svc.GetName(), access | ServiceAccessMask::QueryStatus);
	if (service == nullptr) {
		AtlMessageBox(*this, L"Failed to open service", IDS_TITLE, MB_ICONERROR);
		return;
	}

	if (!action(*service)) {
		AtlMessageBox(*this, failedText, IDS_TITLE, MB_ICONERROR);
		return;
	}

	CProgressDlg dlg;
	dlg.SetMessageText((doing + std::wstring(L" service ") + svc.GetName() + L"...").c_str());
	dlg.SetProgressMarquee(true);
	auto start = ::GetTickCount64();
	bool timedOut = false;
	dlg.SetTimerCallback([&]() {
		service->Refresh(svc);
		if (svc.GetStatusProcess().CurrentState == target)
			dlg.Close();
		else if (::GetTickCount64() - start > timeout) {
			timedOut = true;
			dlg.Close(IDCANCEL);
		}
		}, 500);
	dlg.DoModal();
	if (timedOut) {
		CString text;
		text.Format(L"The service is still %s after %u seconds.", pendingText, timeout / 1000);
		AtlMessageBox(*this, (PCWSTR)text, IDS_TITLE, MB_ICONEXCLAMATION);
	}
	Refresh();
	UpdateUI(this);
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

LRESULT CServicesView::OnServiceProperties(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CServicesView::OnServiceDelete(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	auto& svc = m_Services[index];
	CString text;
	text.Format(L"Uninstall the service %s?\n", svc.GetName().c_str());
	if (AtlMessageBox(*this, (PCWSTR)text, IDS_TITLE, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		auto success = WinSys::ServiceManager::Uninstall(svc.GetName());
		AtlMessageBox(*this, success ? L"Service uninstalled successfully" : L"Failed to uninstall service", 
			IDS_TITLE, success ? MB_ICONINFORMATION : MB_ICONERROR);
		Refresh();
	}
	return 0;
}

LRESULT CServicesView::OnProcessProperties(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0 || selected >= (int)m_Services.size())
		return 0;	// toolbar buttons and accelerators work without a selection too

	auto& svc = m_Services[selected];
	ProcessInfoEx px(m_ProcMgr.GetProcessById(svc.GetStatusProcess().ProcessId).get());
	CProcessPropertiesDlg dlg(m_ProcMgr, px);
	dlg.SetModal(true);
	dlg.DoModal();

	return 0;
}

bool CServicesView::CompareItems(const ServiceInfo& s1, const ServiceInfo& s2, int col, bool asc) {
	switch (col) {
		case 0:	return SortHelper::Sort(s1.GetName(), s2.GetName(), asc);
		case 1: return SortHelper::Sort(s1.GetDisplayName(), s2.GetDisplayName(), asc);
		case 2: return SortHelper::Sort(
			ServiceStateToString(s1.GetStatusProcess().CurrentState), ServiceStateToString(s2.GetStatusProcess().CurrentState), asc);
		case 3: return SortHelper::Sort(s1.GetStatusProcess().Type, s2.GetStatusProcess().Type, asc);
		case 4: return SortHelper::Sort(s1.GetStatusProcess().ProcessId, s2.GetStatusProcess().ProcessId, asc);
		case 5: return SortHelper::Sort(
			m_ProcMgr.GetProcessNameById(s1.GetStatusProcess().ProcessId), 
			m_ProcMgr.GetProcessNameById(s2.GetStatusProcess().ProcessId), asc);
		case 6: case 7: case 8: case 9:
		{
			// the configuration is nullptr if it can't be read; such services sort as empty
			auto c1 = GetServiceInfoEx(s1.GetName()).GetConfiguration();
			auto c2 = GetServiceInfoEx(s2.GetName()).GetConfiguration();
			switch (col) {
				case 6: return SortHelper::Sort(c1 ? ServiceStartTypeToString(*c1) : CString(), c2 ? ServiceStartTypeToString(*c2) : CString(), asc);
				case 7: return SortHelper::Sort(c1 ? c1->BinaryPathName : std::wstring(), c2 ? c2->BinaryPathName : std::wstring(), asc);
				case 8: return SortHelper::Sort(c1 ? c1->AccountName : std::wstring(), c2 ? c2->AccountName : std::wstring(), asc);
				default: return SortHelper::Sort(c1 ? (int)c1->ErrorControl : -1, c2 ? (int)c2->ErrorControl : -1, asc);
			}
		}
		case 10: return SortHelper::Sort(
			GetServiceInfoEx(s1.GetName()).GetDescription(),
			GetServiceInfoEx(s2.GetName()).GetDescription(), asc);
		case 11: return SortHelper::Sort(
			GetServiceInfoEx(s1.GetName()).GetPrivileges(),
			GetServiceInfoEx(s2.GetName()).GetPrivileges(), asc);
		case 12: return SortHelper::Sort(
			GetServiceInfoEx(s1.GetName()).GetTriggers(),
			GetServiceInfoEx(s2.GetName()).GetTriggers(), asc);
		case 13: return SortHelper::Sort(
			GetServiceInfoEx(s1.GetName()).GetDependencies(),
			GetServiceInfoEx(s2.GetName()).GetDependencies(), asc);
		case 14: return SortHelper::Sort(s1.GetStatusProcess().ControlsAccepted, s2.GetStatusProcess().ControlsAccepted, asc);
		case 15: return SortHelper::Sort(GetServiceInfoEx(s1.GetName()).GetSID(), GetServiceInfoEx(s2.GetName()).GetSID(), asc);
		case 16: return SortHelper::Sort(GetServiceInfoEx(s1.GetName()).GetSidType(), GetServiceInfoEx(s2.GetName()).GetSidType(), asc);
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
	using enum ServiceState;
	switch (state) {
		case Running: return L"Running";
		case Stopped: return L"Stopped";
		case Paused: return L"Paused";
		case StartPending: return L"Start Pending";
		case ContinuePending: return L"Continue Pending";
		case StopPending: return L"Stop Pending";
		case PausePending: return L"Pause Pending";
	}
	return L"Unknown";
}

CString CServicesView::ServiceStartTypeToString(const ServiceConfiguration& config) {
	CString type;

	switch (config.StartType) {
		using enum ServiceStartType;
		case Boot: type = L"Boot"; break;
		case System: type = L"System"; break;
		case Auto: type = L"Automatic"; break;
		case Demand: type = L"Manual"; break;
		case Disabled: type = L"Disabled"; break;
	}

	if (config.DelayedAutoStart)
		type += L" (Delayed)";
	if (config.TriggerStart)
		type += L" (Trigger)";

	return type;
}

CString CServicesView::ServiceControlsAcceptedToString(ServiceControlsAccepted accepted) {
	using enum ServiceControlsAccepted;
	if (accepted == None)
		return L"(None)";

	static struct {
		ServiceControlsAccepted type;
		PCWSTR text;
	} types[] = {
		{ Stop, L"Stop" },
		{ PauseContinue, L"Pause, Continue" },
		{ Shutdown, L"Shutdown" },
		{ ParamChange, L"Param Change" },
		{ NetBindChange, L"NET Bind Change" },
		{ HardwareProfileChange, L"Hardware Profile Change" },
		{ PowerEvent, L"Power Event" },
		{ SessionChange, L"Session Change" },
		{ Preshutdown, L"Pre Shutdown" },
		{ TimeChanged, L"Time Change" },
		{ TriggerEvent, L"Trigger Event" },
		{ UserLogOff, L"User Log off" },
		{ LowResources, L"Low Resources" },
		{ SystemLowResources, L"System Low Resources" },
		{ InternalReserved, L"(Internal)" },
	};

	CString text;
	for (auto& item : types)
		if ((item.type & accepted) == item.type)
			text += CString(item.text) + L", ";

	text = text.Left(text.GetLength() - 2);
	text.Format(L"%s (0x%X)", (PCWSTR)text, (DWORD)accepted);
	return text;
}

CString CServicesView::ErrorControlToString(ServiceErrorControl ec) {
	using enum ServiceErrorControl;

	switch (ec) {
		case Normal: return L"Normal (1)";
		case Ignore: return L"Ignore (0)";
		case Critical: return L"Critical (3)";
		case Severe: return L"Severe (2)";
	}
	ATLASSERT(false);
	return L"";
}

CString CServicesView::ServiceTypeToString(WinSys::ServiceType type) {
	using enum ServiceType;

	static struct {
		ServiceType type;
		PCWSTR text;
	} types[] = {
		{ KernelDriver, L"Kernel Driver" },
		{ FileSystemDriver, L"FS/Filter Driver" },
		{ Win32OwnProcess, L"Own" },
		{ Win32SharedProcess, L"Shared" },
		{ InteractiveProcess, L"Interactive" },
		{ UserService, L"User" },
		{ UserServiceInstance, L"Instance" },
		{ PackageService, L"Packaged" },
	};

	CString text;
	for (auto& item : types)
		if ((item.type & type) == item.type)
			text += CString(item.text) + L", ";

	return text.Left(text.GetLength() - 2);
}

PCWSTR CServicesView::ServiceSidTypeToString(WinSys::ServiceSidType type) {
	switch (type) {
		case ServiceSidType::None: return L"None";
		case ServiceSidType::Restricted: return L"Restricted";
		case ServiceSidType::Unrestricted: return L"Unrestricted";
	}
	return L"(Unknown)";
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
	m_Services = WinSys::ServiceManager::EnumServices(m_ViewServices ? ServiceEnumType::AllServices : ServiceEnumType::AllDrivers);
	m_ServicesEx.reserve(m_Services.size());
	m_List.SetItemCountEx(static_cast<int>(m_Services.size()), LVSICF_NOSCROLL);
	DoSort(GetSortInfo(m_List));
	UpdateUI(this);
}

void CServicesView::UpdateUI(CUpdateUIBase* ui) {
	auto selected = m_List.GetSelectedIndex();
	ui->UIEnable(ID_EDIT_PROPERTIES, selected >= 0);

	if (selected < 0 || !SecurityHelper::IsRunningElevated()) {
		ui->UIEnable(ID_SERVICE_START, FALSE);
		ui->UIEnable(ID_SERVICE_STOP, FALSE);
		ui->UIEnable(ID_SERVICE_PAUSE, FALSE);
		ui->UIEnable(ID_SERVICE_CONTINUE, FALSE);
		ui->UIEnable(ID_SERVICE_UNINSTALL, FALSE);
		ui->UIEnable(ID_SERVICE_PROCESSPROPERTIES, FALSE);
	}
	else {
		auto& svc = m_Services[selected];
		auto& state = svc.GetStatusProcess().CurrentState;
		auto config = GetServiceInfoEx(svc.GetName()).GetConfiguration();
		ui->UIEnable(ID_SERVICE_START, state == ServiceState::Stopped &&
			(config == nullptr || config->StartType != ServiceStartType::Disabled));
		ui->UIEnable(ID_SERVICE_STOP, state == ServiceState::Running);
		ui->UIEnable(ID_SERVICE_PAUSE, state == ServiceState::Running);
		ui->UIEnable(ID_SERVICE_CONTINUE, state == ServiceState::Paused);
		ui->UIEnable(ID_SERVICE_UNINSTALL, TRUE);
		auto active = svc.GetStatusProcess().ProcessId != 0;
		ui->UIEnable(ID_SERVICE_PROCESSPROPERTIES, active);
	}
}


PCWSTR CServicesView::TriggerToText(const WinSys::ServiceTrigger& trigger) {
	using namespace WinSys;
	switch (trigger.Type) {
		using enum ServiceTriggerType;
		case Custom: return L"Custom";
		case DeviceInterfaceArrival: return L"Device Arrival";
		case DomainJoin: return L"Domain Join";
		case FirewallPortEvent: return L"Firewall Port Event";
		case GroupPolicy: return L"Group Policy";
		case IpAddressAvailability: return L"IP Address Availability";
		case NetworkEndpoint: return L"Network Endpoint";
		case SystemStateChanged: return L"System State Changed";
		case Aggregate: return L"Aggregate";
	}
	ATLASSERT(false);
	return L"";
}

LRESULT CServicesView::OnProcessItem(WORD, WORD id, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0)
		return 0;

	auto& svc = m_Services[selected];
	auto pid = svc.GetStatusProcess().ProcessId;
	if (pid) {
		auto pi = m_ProcMgr.GetProcessById(svc.GetStatusProcess().ProcessId);
		Frame()->SendFrameMessage(WM_COMMAND, id, reinterpret_cast<LPARAM>(pi.get()));
	}
	else {
		AtlMessageBox(*this, L"Service is not running", IDS_TITLE, MB_ICONWARNING);
	}
	return 0;
}
