#pragma once

#include <unordered_map>

template<typename T>
class CCustomDrawListView : public CCustomDraw<T> {
public:
	BEGIN_MSG_MAP(CCustomDrawListView)
		CHAIN_MSG_MAP(CCustomDraw<T>)
	ALT_MSG_MAP(1)
		CHAIN_MSG_MAP_ALT(CCustomDraw<T>, 1)
	END_MSG_MAP()

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/) {
		return CDRF_NOTIFYITEMDRAW;
	}

	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW cd) {
		int row = (int)cd->dwItemSpec;
		if (auto it = m_ColorRowData.find(row); it != m_ColorRawData.end()) {
			if (it->TargetRevertTime && it->TargetRevertTime >= ::GetTickCount64()) {
				if (it->RevertCallback)
					it->RevertCallback(it->second);
				m_ColorRowData.erase(row);
			}
			else {
				auto lv = (NMLVCUSTOMDRAW*)cd;
				lv->clrText = it->TextColor;
				lv->clrTextBk = it->BackColor;
			}
		}
		return CDRF_SUBITEMPAINT;
	}
		
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW cd) {
		auto lv = (NMLVCUSTOMDRAW*)cd;
		auto pair = std::make_pair((int)cd->dwItemSpec, lv->iSubItem);

		if (auto it = m_ColorCellData.find(pair); it != m_ColorCellData.end()) {
			if (it->TargetRevertTime && it->TargetRevertTime >= ::GetTickCount64()) {
				if (it->RevertCallback)
					it->RevertCallback(it->second);
				m_ColorRowData.erase(pair);
			}
			else {
				lv->clrText = it->TextColor;
				lv->clrTextBk = it->BackColor;
			}
		}
	}

protected:
	bool AddRowColorData(ColorData& data) {
		data.Column = -1;
		m_ColorRowData[data.Row] = data;
		return true;
	}

	bool AddCellColorData(ColorData& data) {
		m_ColorCellData[std::make_pair(data.Row, data.Column)] = data;
		return true;
	}

private:
	struct ColorData {
		int Row, Column;
		COLORREF BackColor, TextColor;
		//COLORREF OldBackColor, OldTextColor;
		DWORD64 TargetRevertTime;
		std::function<void(const ColorData&)> RevertCallback;
	};
	std::unorderd_map<int, ColorData> m_ColorRowData;
	std::unorderd_map<std::pair<int, int>, ColorData> m_ColorCellData;
};
