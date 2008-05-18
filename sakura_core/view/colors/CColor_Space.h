#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_Tab : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};


class CColor_ZenSpace : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};

class CColor_HanSpace : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};

