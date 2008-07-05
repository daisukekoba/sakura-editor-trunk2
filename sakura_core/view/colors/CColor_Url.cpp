#include "stdafx.h"
#include "CColor_Url.h"
#include "parse/CWordParse.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_Url::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nUrlLen;
	
	if( pInfo->IsPosKeywordHead() && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
	 && IsURL( &pInfo->pLineOfLayout[pInfo->GetPosInLayout()], pInfo->nLineLenOfLayoutWithNexts - pInfo->nPosInLogic, &nUrlLen )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
	){
		pInfo->nCOMMENTEND = pInfo->nPosInLogic + nUrlLen;
		return COLORIDX_URL;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Url::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->nPosInLogic == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

