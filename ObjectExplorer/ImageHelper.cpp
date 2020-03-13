#include "stdafx.h"
#include "ImageHelper.h"

HICON ImageHelper::GetSystemIcon(SHSTOCKICONID icon, bool big) {
	SHSTOCKICONINFO ssii = { sizeof(ssii) };
	if (FAILED(::SHGetStockIconInfo(icon, (big ? SHGSI_LARGEICON : SHGSI_SMALLICON) | SHGSI_ICON, &ssii)))
		return nullptr;

	return ssii.hIcon;
}
