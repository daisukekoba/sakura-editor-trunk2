#include "stdafx.h"
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

void _DispZenkakuSpace( HDC hdc, DispPos* pDispPos, CEditView* pcView );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pInfo->pLine[pInfo->nPos] == L'　' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		_DispZenkakuSpace(pInfo->hdc,pInfo->pDispPos,pInfo->pcView);
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DispZenkakuSpace( HDC hdc, DispPos* pDispPos, CEditView* pcView )
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2))
	{
		//描画文字列
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L"□":L"　";

		//描画
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szZenSpace,
			wcslen(szZenSpace),
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(2);
}
