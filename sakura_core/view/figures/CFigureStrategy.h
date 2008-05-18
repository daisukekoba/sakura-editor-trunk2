#pragma once

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo

struct SColorStrategyInfo;

class CFigureStrategy{
public:
	virtual ~CFigureStrategy(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
};

bool DrawChar(SColorStrategyInfo* pInfo);
