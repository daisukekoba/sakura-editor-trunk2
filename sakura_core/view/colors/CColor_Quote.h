#pragma once

#include "view/colors/CColorStrategy.h"



class CColor_Quote : public CColorStrategy{
public:
	CColor_Quote(wchar_t cQuote) : m_cQuote(cQuote), m_bLastEscape(false), m_bDone(false) { }
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual EColorIndexType BeginColor(SColorStrategyInfo* pInfo);
	virtual bool EndColor(SColorStrategyInfo* pInfo);
private:
	wchar_t		m_cQuote;
	bool		m_bLastEscape;
	bool		m_bDone;
};


class CColor_SingleQuote : public CColor_Quote{
public:
	CColor_SingleQuote() : CColor_Quote(L'\'') { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SSTRING; }
};

class CColor_DoubleQuote : public CColor_Quote{
public:
	CColor_DoubleQuote() : CColor_Quote(L'"') { }
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_WSTRING; }
};
