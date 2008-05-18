#pragma once

#include "view/colors/CColorStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_LineComment : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_BlockComment : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�Q                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CColor_BlockComment2 : public CColorStrategy{
public:
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
};

