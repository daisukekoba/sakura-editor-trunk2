#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Found : public CColorStrategy{
public:
	//�F�ւ�
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
	//�C�x���g
	virtual void OnStartScanLogic();

private:
	bool			m_bSearchFlg;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
};
