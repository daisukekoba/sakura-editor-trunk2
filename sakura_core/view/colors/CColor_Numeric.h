#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Numeric : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};
