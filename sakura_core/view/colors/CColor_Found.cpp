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
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if(!pInfo->pcView->m_sCurSearchOption.bRegularExp || (m_bSearchFlg && m_nSearchStart < pInfo->nPosInLogic)){
		m_bSearchFlg = pInfo->pcView->IsSearchString(
			pcDocLine->GetPtr(),
			pcDocLine->GetLengthWithoutEOL(),
			pInfo->nPosInLogic,
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	//マッチ文字列検出
	if( m_bSearchFlg && m_nSearchStart==pInfo->nPosInLogic){
		return COLORIDX_SEARCH;
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_Found::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	//マッチ文字列終了検出
	if( m_nSearchEnd <= pInfo->nPosInLogic ){ //+ == では行頭文字の場合、m_nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
		// -- -- マッチ文字列を描画 -- -- //

		return true;
	}

	return false;
}
