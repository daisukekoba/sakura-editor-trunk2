#include "stdafx.h"
#include "view/figures/CFigureStrategy.h"
#include "CFigure_Tab.h"
#include "CFigure_Eol.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_HanSpace.h"
#include "doc/CLayout.h"


#if 1


//! ’ÊíƒeƒLƒXƒg•`‰æ
class CDraw_Text : public CFigureStrategy{
public:
	bool DrawImp(SColorStrategyInfo* pInfo)
	{
		if(!pInfo->pLine)return false;

		const CLayout*	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
		if(pInfo->nPos < pcLayout2->GetLengthWithoutEOL()){
			pInfo->pcView->GetTextDrawer().DispText(
				pInfo->gr,
				pInfo->pDispPos,
				&pInfo->pLine[pInfo->nPos],
				1
			);
			pInfo->nPos++;
			return true;
		}
		return false;
	}
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         •`‰æ“‡                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 1•¶Žš•`‰æB‚à‚¤•`‰æ‚·‚é•¶Žš‚ª–³‚­‚È‚Á‚½‚çtrue‚ð•Ô‚·
bool DrawChar(SColorStrategyInfo* pInfo)
{
	if( pInfo->pDispPos->GetDrawPos().y >= pInfo->pcView->GetTextArea().GetAreaTop() ){
		if(CFigure_Tab().DrawImp(pInfo)){}
		else if(CFigure_HanSpace().DrawImp(pInfo)){}
		else if(CFigure_ZenSpace().DrawImp(pInfo)){}
		else if(CFigure_Eol().DrawImp(pInfo)){}
		else if(CDraw_Text().DrawImp(pInfo)){}
		else{
			return true;
		}
		return false;
	}
	else{
		return false; //####”÷–­
	}
}



#endif
