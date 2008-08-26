#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Found : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SEARCH; }
	//色替え
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
	//イベント
	virtual void OnStartScanLogic();

private:
	bool			m_bSearchFlg;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
};
