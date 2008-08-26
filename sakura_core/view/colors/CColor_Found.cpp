#include "stdafx.h"
#include "CColor_Found.h"
#include "types/CTypeSupport.h"


void CColor_Found::OnStartScanLogic()
{
	m_bSearchFlg	= true;
	m_nSearchStart	= CLogicInt(-1);
	m_nSearchEnd	= CLogicInt(-1);
}

EColorIndexType CColor_Found::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	const CDocLine* pcDocLine = pInfo->GetDocLine();

	if( !pInfo->pcView->m_bCurSrchKeyMark || !CTypeSupport(pInfo->pcView,COLORIDX_SEARCH).IsDisp() ){
		return _COLORIDX_NOCHANGE;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if(!pInfo->pcView->m_sCurSearchOption.bRegularExp || (m_bSearchFlg && m_nSearchStart < pInfo->nPosInLogic)){
		m_bSearchFlg = pInfo->pcView->IsSearchString(
			pcDocLine->GetPtr(),
			pcDocLine->GetLengthWithoutEOL(),
			pInfo->nPosInLogic,
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//�}�b�`�����񌟏o
	if( m_bSearchFlg && m_nSearchStart==pInfo->nPosInLogic){
		return COLORIDX_SEARCH;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Found::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	//�}�b�`������I�����o
	if( m_nSearchEnd <= pInfo->nPosInLogic ){ //+ == �ł͍s�������̏ꍇ�Am_nSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
		// -- -- �}�b�`�������`�� -- -- //

		return true;
	}

	return false;
}
