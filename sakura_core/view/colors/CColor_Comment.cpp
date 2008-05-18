#include "stdafx.h"
#include "CColor_Comment.h"
#include "doc/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_LineComment::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// 行コメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cLineComment.Match( pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		return COLORIDX_COMMENT;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_LineComment::EndColor(SColorStrategyInfo* pInfo)
{
	const CLayout*	pcLayout2;
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
		return true;
	}

	return false; //何もしない
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_BlockComment::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// ブロックコメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		/* この物理行にブロックコメントの終端があるか */	//@@@ 2002.09.22 YAZAKI
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo(
			0,
			pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ),
			pInfo->nLineLen,
			pInfo->pLine
		);

		return COLORIDX_BLOCK1;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_BlockComment::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo(
			0,
			pInfo->nPos,
			pInfo->nLineLen,
			pInfo->pLine
		);
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント２                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_BlockComment2::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	// ブロックコメント
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
		TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, pInfo->nPos, pInfo->nLineLen, pInfo->pLine )	//@@@ 2002.09.22 YAZAKI
	){
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo(
			1,
			pInfo->nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ),
			pInfo->nLineLen,
			pInfo->pLine
		);

		return COLORIDX_BLOCK2;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_BlockComment2::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* この物理行にブロックコメントの終端があるか */
		pInfo->nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo(
			1,
			pInfo->nPos,
			pInfo->nLineLen,
			pInfo->pLine
		);
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}
