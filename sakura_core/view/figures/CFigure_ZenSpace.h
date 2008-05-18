#pragma once

#include "view/figures/CFigureStrategy.h"

//! 全角スペース描画
class CFigure_ZenSpace : public CFigureStrategy{
public:
	bool DrawImp(SColorStrategyInfo* pInfo);
};
