#pragma once

class SecurityInfo : public ISecurityInformation {
public:
	SecurityInfo(HANDLE hObject, PCWSTR name);

private:
	// Inherited via ISecurityInformation
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	ULONG __stdcall AddRef(void) override;
	ULONG __stdcall Release(void) override;
	HRESULT __stdcall GetObjectInformation(PSI_OBJECT_INFO pObjectInfo) override;
	HRESULT __stdcall GetSecurity(SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) override;
	HRESULT __stdcall SetSecurity(SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor) override;
	HRESULT __stdcall GetAccessRights(const GUID* pguidObjectType, DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess) override;
	HRESULT __stdcall MapGeneric(const GUID* pguidObjectType, UCHAR* pAceFlags, ACCESS_MASK* pMask) override;
	HRESULT __stdcall GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG* pcInheritTypes) override;
	HRESULT __stdcall PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage) override;

private:
	CString _name;
	HANDLE _hObject;
	BYTE _buffer[1 << 10];
};

