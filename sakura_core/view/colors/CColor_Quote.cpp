#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �N�H�[�e�[�V����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_Quote::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] == m_cQuote &&
		TypeDataPtr->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
	){
		m_bLastEscape = false;
		m_bDone = false;
		return this->GetStrategyColor();
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Quote::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(m_bDone){
		return true;
	}

	//�������؂�L���G�X�P�[�v���@ 0=[\"][\']
	if( TypeDataPtr->m_nStringType == 0 ){
		//�G�X�P�[�v����
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//�I�[
		if(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]==m_cQuote){
			m_bDone = true;
			return false;
		}
		//�G�X�P�[�v���o
		if(pInfo->pLineOfLayout[pInfo->GetPosInLayout()-1]==L'\\'){
			m_bLastEscape = true;
		}
	}
	//�������؂�L���G�X�P�[�v���@ 1=[""]['']
	else if(TypeDataPtr->m_nStringType == 1){
		//�G�X�P�[�v����
		if(m_bLastEscape){
			m_bLastEscape = false;
			return false;
		}
		//�I�[
		if(pInfo->pLineOfLayout[pInfo->GetPosInLayout()]==m_cQuote){
			//�G�X�P�[�v���o
			if(pInfo->pLineOfLayout[pInfo->GetPosInLayout()+1]==m_cQuote){
				m_bLastEscape = true;
				return false;
			}
			else{
				m_bDone = true;
				return false;
			}
		}
	}
	//�z��O
	else{
		assert(0);
	}

	return false;
}
