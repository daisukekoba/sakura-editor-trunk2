#include "stdafx.h"
#include "CColor_RegexKeyword.h"


EColorIndexType CColor_RegexKeyword::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	//正規表現キーワード
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pInfo->pcView->m_cRegexKeyword->RegexIsKeyword( pInfo->pLineOfLayout, pInfo->nPosInLogic, pInfo->nLineLenOfLayoutWithNexts, &nMatchLen, &nMatchColor )
	){
		pInfo->nCOMMENTEND = pInfo->nPosInLogic + nMatchLen;  /* キーワード文字列の終端をセットする */
		return MakeColorIndexType_RegularExpression(nMatchColor);
	}
	return _COLORIDX_NOCHANGE;
}


bool CColor_RegexKeyword::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->nPosInLogic == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

