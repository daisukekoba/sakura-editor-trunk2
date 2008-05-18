#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_CtrlCode : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);

private:
	EColorIndexType			nCOMMENTMODE_OLD;
	int						nCOMMENTEND_OLD;
};
