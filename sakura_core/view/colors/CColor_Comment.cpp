#include "stdafx.h"
#include "CColor_Comment.h"
#include "doc/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_LineComment::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// �s�R�����g
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( pInfo->GetPosInLayout(), pInfo->nLineLenOfLayoutWithNexts, pInfo->pLineOfLayout )	//@@@ 2002.09.22 YAZAKI
	){
		return COLORIDX_COMMENT;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_LineComment::EndColor(SColorStrategyInfo* pInfo)
{
	const CLayout*	pcLayout2;
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	if( pInfo->nPosInLogic >= pInfo->pDispPos->GetLayoutRef()->GetDocLineRef()->GetLengthWithoutEOL() ){
		return true;
	}

	return false; //�������Ȃ�
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_BlockComment::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	const CDocLine* pcDocLine = pInfo->GetDocLine();

	// �u���b�N�R�����g
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComments[m_nType].Match_CommentFrom( pInfo->nPosInLogic, pcDocLine->GetLengthWithoutEOL(), pcDocLine->GetPtr() )	//@@@ 2002.09.22 YAZAKI
	){
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */	//@@@ 2002.09.22 YAZAKI
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComments[m_nType].Match_CommentTo(
			pInfo->nPosInLogic + (int)wcslen( TypeDataPtr->m_cBlockComments[m_nType].getBlockCommentFrom() ),
			pcDocLine->GetLengthWithoutEOL(),
			pcDocLine->GetPtr()
		);

		return this->GetStrategyColor();
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_BlockComment::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	const CDocLine* pcDocLine = pInfo->GetDocLine();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComments[m_nType].Match_CommentTo(
			0,
			pcDocLine->GetLengthWithoutEOL(),
			pcDocLine->GetPtr()
		);
	}
	else if( pInfo->nPosInLogic >= pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

