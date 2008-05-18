#pragma once

#include "view/figures/CFigureStrategy.h"

//! 半角スペース描画
class CFigure_HanSpace : public CFigureStrategy{
public:
	bool DrawImp(SColorStrategyInfo* pInfo);
};
