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
		&&WCODE::IsControlCode(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]))
	{
		this->nCOMMENTMODE_OLD = pInfo->nCOMMENTMODE;
		this->nCOMMENTEND_OLD = pInfo->nCOMMENTEND;

		/* コントロールコード列の終端を探す */
		int i;
		for( i = pInfo->nPosInLogic + 1; i <= pInfo->nLineLenOfLayoutWithNexts - 1; ++i ){
			if(!WCODE::IsControlCode(pInfo->pLineOfLayout[i])){
				break;
			}
		}
		pInfo->nCOMMENTEND = i;
		return COLORIDX_CTRLCODE;
	}

	return _COLORIDX_NOCHANGE;
}

bool CColor_CtrlCode::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->nPosInLogic == pInfo->nCOMMENTEND ){
		pInfo->nCOMMENTMODE = this->nCOMMENTMODE_OLD;
		pInfo->nCOMMENTEND = this->nCOMMENTEND_OLD;
		return true;
	}
	return false;
}

