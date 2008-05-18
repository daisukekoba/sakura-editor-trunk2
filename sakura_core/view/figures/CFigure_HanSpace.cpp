#include "stdafx.h"
#include "CFigure_HanSpace.h"
#include "types/CTypeSupport.h"

void _DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode, CEditView* pcView );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_HanSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pInfo->pLine[pInfo->nPos] == L' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp){
		_DispHankakuSpace(pInfo->hdc,pInfo->pDispPos,pInfo->bSearchStringMode,pInfo->pcView);
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode, CEditView* pcView )
{
	//クリッピング矩形を計算。画面外なら描画しない
	CMyRect rcClip;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		// 色決定
		CTypeSupport cSupport(pcView,pcView->GetTextDrawer()._GetColorIdx(COLORIDX_SPACE,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);
		
		//小文字"o"の下半分を出力
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipBottom,
			L"o",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//上半分は普通の空白で出力（"o"の上半分を消す）
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipTop,
			L" ",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(1);
}
