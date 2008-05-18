#include "stdafx.h"
#include "CColor_Eol.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

EColorIndexType CColor_Eol::BeginColor(SColorStrategyInfo* pInfo)
{
	CTypeSupport cTextType(pInfo->pcView,COLORIDX_TEXT);
	const CLayout*	pcLayout2; //ワーク用CLayoutポインタ
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	int nLineHeight = pInfo->pcView->GetTextMetrics().GetHankakuDy();  //行の縦幅？

	if(!pInfo->pLine){
		if(pInfo->pDispPos->GetLayoutLineRef()==CEditDoc::GetInstance(0)->m_cLayoutMgr.GetLineCount()){
			return COLORIDX_EOL;
		}
		else{
			return _COLORIDX_NOCHANGE;
		}
	}
	else if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
		pInfo->nCOMMENTEND = pInfo->nPos + pcLayout2->GetLayoutEol().GetLen();
		return COLORIDX_EOL;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Eol::EndColor(SColorStrategyInfo* pInfo)
{
	if(pInfo->nPos>=pInfo->nCOMMENTEND){
		return true;
	}
	return false;
}
