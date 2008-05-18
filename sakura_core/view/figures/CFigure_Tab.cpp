#include "stdafx.h"
#include "CFigure_Tab.h"
#include "types/CTypeSupport.h"

//2007.08.28 kobake �ǉ�
void _DispTab( HDC hdc, DispPos* pDispPos, const CEditView* pcView );
//�^�u���`��֐�	//@@@ 2003.03.26 MIK
void _DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CFigure_Tab                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Tab::DrawImp(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == WCODE::TAB ){
		_DispTab( pInfo->hdc, pInfo->pDispPos, pInfo->pcView );
		return true;
	}
	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	Sep. 22, 2002 genta ���ʎ��̂����肾��
//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
//@@@ 2001.03.16 by MIK
//@@@ 2003.03.26 MIK �^�u���\��
void _DispTab( HDC hdc, DispPos* pDispPos, const CEditView* pcView )
{
	DispPos& sPos=*pDispPos;

	//�萔
	static const wchar_t* pszSPACES = L"        ";

	//�K�v�ȃC���^�[�t�F�[�X
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();
	STypeConfig* TypeDataPtr = &pcView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();


	CTypeSupport cTabType(pcView,COLORIDX_TAB);

	// ���ꂩ��`�悷��^�u��
	int tabDispWidth = (Int)pcView->m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( sPos.GetDrawCol() );

	// �^�u�L���̈�
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if( pArea->IsRectIntersected(rcClip2) ){
		//$$note: Strategy���g���Ƃ����Ɣ������Ȃ�܂� by kobake

		//�`��f�[�^����
		const wchar_t* pData = NULL;
		if( cTabType.IsDisp() ){
			if(TypeDataPtr->m_bTabArrow){
				pData = pszSPACES;
			}
			else{
				pData = TypeDataPtr->m_szTabViewString;
			}
		}
		else{
			pData = pszSPACES;
		}

		//�^�u�e�L�X�g (�܂��͔w�i�̂�)
		::ExtTextOutW_AnyBuild(
			hdc,
			sPos.GetDrawPos().x,
			sPos.GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			TypeDataPtr->m_szTabViewString,
			tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
			pMetrics->GetDxArray_AllHankaku()
		);

		//�^�u���\��
		if( cTabType.IsDisp() && TypeDataPtr->m_bTabArrow && rcClip2.left <= sPos.GetDrawPos().x ){ // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
			_DrawTabArrow(
				hdc,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				pMetrics->GetHankakuWidth(),
				pMetrics->GetHankakuHeight(),
				cTabType.IsFatFont(),
				cTabType.GetTextColor()
			);
		}
	}

	//X��i�߂�
	sPos.ForwardDrawCol(tabDispWidth);
}

/*
	�^�u���`��֐�
*/
void _DrawTabArrow(
	HDC hdc,
	int nPosX,   //�s�N�Z��X
	int nPosY,   //�s�N�Z��Y
	int nWidth,  //�s�N�Z��W
	int nHeight, //�s�N�Z��H
	int bBold,
	COLORREF pColor
	){
	HPEN hPen    = ::CreatePen( PS_SOLID, 1, pColor );
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;

	//	���̐擪
	int sx = nPosX + nWidth;
	int sy = nPosY + ( nHeight / 2 );

	::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	����
	::LineTo(   hdc, sx, sy );								//	�Ō��
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
	::MoveToEx( hdc, sx, sy, NULL);							//	�Ō�֖߂�
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	����
		::LineTo(   hdc, sx, sy );								//	�Ō��
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
		::MoveToEx( hdc, sx, sy, NULL);							//	�Ō�֖߂�
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}
