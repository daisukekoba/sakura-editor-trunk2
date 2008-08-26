#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_KeywordSet : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_KEYWORD1; } //##########‰¼
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};
