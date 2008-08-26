#include "stdafx.h"
#include "CColor_CtrlCode.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        制御コード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_CtrlCode::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
		&& WCODE::IsControlCode(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]))
	{
		return COLORIDX_CTRLCODE;
	}

	return _COLORIDX_NOCHANGE;
}

bool CColor_CtrlCode::EndColor(SColorStrategyInfo* pInfo)
{
	if( !WCODE::IsControlCode(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]) ){
		return true;
	}
	return false;
}

