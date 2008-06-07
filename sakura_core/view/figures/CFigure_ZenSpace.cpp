#include "stdafx.h"
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(pInfo->pLine[pInfo->nPos] == L'　' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		_DispZenkakuSpace(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
#ifdef NEW_ZENSPACE
		//背景
		gr.SetBrushColor(::GetBkColor(gr));
		::FillRect(gr,&rc,gr.GetCurrentBrush());
		
		//四角形
		COLORREF c = ::GetTextColor(gr);
		rc.left+=1;
		rc.top+=1;
		rc.right-=1;
		rc.bottom-=1;
//		gr.SetNullBrush();
//		gr.PushPen(c,1,PS_DOT);
//		Rectangle(gr,rc.left,rc.top,rc.right,rc.bottom);
//		gr.PopPen();
		for(int x=rc.left+1;x<rc.right-1;x+=2){
			::SetPixel(gr,x,rc.top,c);
			::SetPixel(gr,x,rc.bottom-1,c);
		}
		for(int y=rc.top+1;y<rc.bottom-1;y+=2){
			::SetPixel(gr,rc.left,y,c);
			::SetPixel(gr,rc.right-1,y,c);
		}
#else
		//描画
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L"□":L"　";
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rc,
			szZenSpace,
			wcslen(szZenSpace),
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
#endif
	}

	//位置進める
	pDispPos->ForwardDrawCol(2);
}
