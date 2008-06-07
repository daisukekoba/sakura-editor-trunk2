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

	if(pInfo->pLine[pInfo->nPos] == L'Å@' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		_DispZenkakuSpace(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ï`âÊé¿ëï                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	//ÉNÉäÉbÉsÉìÉOãÈå`ÇåvéZÅBâÊñ äOÇ»ÇÁï`âÊÇµÇ»Ç¢
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
#ifdef NEW_ZENSPACE
		//îwåi
		gr.SetBrushColor(::GetBkColor(gr));
		::FillRect(gr,&rc,gr.GetCurrentBrush());
		
		//éläpå`
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
		//ï`âÊ
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L"Å†":L"Å@";
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

	//à íuêiÇﬂÇÈ
	pDispPos->ForwardDrawCol(2);
}
