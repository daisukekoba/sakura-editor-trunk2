#include "stdafx.h"
#include "CColor_RegexKeyword.h"


EColorIndexType CColor_RegexKeyword::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nMatchLen;
	int		nMatchColor;

	//���K�\���L�[���[�h
	if( TypeDataPtr->m_bUseRegexKeyword
	 && pInfo->pcView->m_cRegexKeyword->RegexIsKeyword( pInfo->pLine, pInfo->nPos, pInfo->nLineLen, &nMatchLen, &nMatchColor )
	){
		pInfo->nCOMMENTEND = pInfo->nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
		return MakeColorIndexType_RegularExpression(nMatchColor);
	}
	return _COLORIDX_NOCHANGE;
}


bool CColor_RegexKeyword::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

