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

	if(pInfo->pLine[pInfo->nPos] == L'@' && TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp){
		_DispZenkakuSpace(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         •`‰æŽÀ‘•                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DispZenkakuSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView )
{
	//ƒNƒŠƒbƒsƒ“ƒO‹éŒ`‚ðŒvŽZB‰æ–ÊŠO‚È‚ç•`‰æ‚µ‚È‚¢
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
//		::FillRect(gr,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

		//•`‰æ•¶Žš—ñ
		const wchar_t* szZenSpace =
			CTypeSupport(pcView,COLORIDX_ZENSPACE).IsDisp()?L" ":L"@";

		//”wŒi
//		PatBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SRCCOPY);FillRect(hdc,&rc,A);

//		Rectangle(hdc,rc.left+2,rc.top+2,rc.right-2,rc.bottom-2);
		//*
		//•`‰æ
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
		//*/
	}

	//ˆÊ’ui‚ß‚é
	pDispPos->ForwardDrawCol(2);
}
