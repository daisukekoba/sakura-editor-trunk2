#include "stdafx.h"
#include "CColor_Space.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           タブ                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_Tab::BeginColor(SColorStrategyInfo* pInfo)
{
#ifdef NEW_ZENSPACE
	return _COLORIDX_NOCHANGE;
#endif
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] == WCODE::TAB ){
		return COLORIDX_TAB;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Tab::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] != WCODE::TAB ){
		return true;
	}
	return false;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       全角スペース                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_ZenSpace::BeginColor(SColorStrategyInfo* pInfo)
{
#ifdef NEW_ZENSPACE
	return _COLORIDX_NOCHANGE;
#endif
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( WCODE::IsZenkakuSpace(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]) )
	{
		return COLORIDX_ZENSPACE;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_ZenSpace::EndColor(SColorStrategyInfo* pInfo)
{
	if( !WCODE::IsZenkakuSpace(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]) ){
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       半角スペース                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_HanSpace::BeginColor(SColorStrategyInfo* pInfo)
{
#ifdef NEW_ZENSPACE
	return _COLORIDX_NOCHANGE;
#endif
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if (pInfo->pLineOfLayout[pInfo->GetPosInLayout()] == L' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp )
	{
		return COLORIDX_SPACE;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_HanSpace::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] != L' ' ){
		return true;
	}
	return false;
}

