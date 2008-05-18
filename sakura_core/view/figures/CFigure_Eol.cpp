#include "stdafx.h"
#include "CFigure_Eol.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

//�܂�Ԃ��`��
void _DispWrap(HDC hdc, DispPos* pDispPos, const CEditView* pcView);

//EOF�`��֐�
//���ۂɂ� pX �� nX ���X�V�����B
//2004.05.29 genta
//2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
//2007.08.25 kobake �������� nCharWidth, nLineHeight ���폜
//2007.08.28 kobake ���� fuOptions ���폜
void _DispEOF( HDC hdc, DispPos* pDispPos, const CEditView* pcView);

//��(����)�s�`��
bool _DispEmptyLine(HDC hdc, DispPos* pDispPos, const CEditView* pcView);

//���s�L���`��
//2007.08.30 kobake �ǉ�
void _DispEOL(HDC hdc, DispPos* pDispPos, CEol cEol, bool bSearchStringMode, const CEditView* pcView);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CFigure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
bool CFigure_Eol::DrawImp(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	// NULL == pLine�̏ꍇ
	if(!pInfo->pLine){
		if(pInfo->nPos==0){
			_DispEmptyLine(pInfo->hdc,pInfo->pDispPos,pInfo->pcView);
			pInfo->nPos++;
			return true;
		}
		else{
			return false;
		}
	}

	//�R���t�B�O
	CTypeSupport		cTextType	(pcView,COLORIDX_TEXT);
	int					nLineHeight	= pcView->GetTextMetrics().GetHankakuDy();
	const CLayoutInt	nWrapKeta	= pcDoc->m_cLayoutMgr.GetMaxLineKetas();	// �܂�Ԃ���

	// ���s�擾
	const CLayout*	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	CEol cEol = pcLayout2->GetLayoutEol();

	// ���傤�Ǎs���Ȃ�A�ȉ��֐i��
	if( pInfo->nPos != pInfo->nLineLen - cEol.GetLen() ){
		return false;
	}

	// �s���L����`��
	{
		// ���s�����݂����ꍇ�́A���s�L����\��
		if(cEol.GetLen()){
			_DispEOL(pInfo->hdc,pInfo->pDispPos,pcLayout2->GetLayoutEol(),pInfo->bSearchStringMode,pInfo->pcView);
			pInfo->nPos+=cEol.GetLen();
		}
		// �ŏI�s�̏ꍇ�́AEOF��\��
		else if(pInfo->pDispPos->GetLayoutLineRef()+1==CEditDoc::GetInstance(0)->m_cLayoutMgr.GetLineCount() && pInfo->pDispPos->GetDrawCol() < nWrapKeta){
			if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
				_DispEOF(pInfo->hdc,pInfo->pDispPos,pInfo->pcView);
			}
			pInfo->nPos+=CLogicInt(1);
		}
		// ����ȊO�ł́A�܂�Ԃ��L����\��
		else{
			_DispWrap(pInfo->hdc,pInfo->pDispPos,pcView);
			pInfo->nPos+=CLogicInt(1);
		}
	}

	// �s���w�i�`��
	RECT rcClip;
	if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
		cTextType.FillBack(pInfo->hdc,rcClip);
	}

	// �c���`��
	pInfo->pcView->GetTextDrawer().DispVerticalLines(
		pInfo->hdc,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		CLayoutInt(0),
		CLayoutInt(-1)
	);

	// ���]�`��
	if( pInfo->pcView->GetSelectionInfo().IsTextSelected() ){
		pInfo->pcView->DispTextSelected(
			pInfo->hdc,
			pInfo->pDispPos->GetLayoutLineRef(),
			CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
			pInfo->pDispPos->GetDrawCol()
		);
	}

	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ��(����)�s�`�����                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ��s��`��BEOF��`�悵���ꍇ��true��Ԃ��B
bool _DispEmptyLine(HDC hdc, DispPos* pDispPos, const CEditView* pcView)
{
	bool bEof=false;

	CTypeSupport cEofType(pcView,COLORIDX_EOF);
	CTypeSupport cTextType(pcView,COLORIDX_TEXT);

	const CLayoutInt nWrapKetas = pcView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int nYPrev = pDispPos->GetDrawPos().y;
	
	if( !pcView->IsBkBitmap() ){
		// �w�i�`��
		RECT rcClip;
		pcView->GetTextArea().GenerateClipRectLine(&rcClip,*pDispPos);
		cTextType.FillBack(hdc,rcClip);
	}

	// EOF�L���̕\��
	CLayoutInt nCount = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	// �h�L�������g����(nCount==0)�B������1�s��(pDispPos->GetLayoutLineRef() == 0)�B�\�����1�s��(m_nViewTopLine==0)
	if( nCount == 0 && pcView->GetTextArea().GetViewTopLine() == 0 && pDispPos->GetLayoutLineRef() == 0 ){
		// EOF�L���̕\��
		if( cEofType.IsDisp() ){
			_DispEOF(hdc,pDispPos,pcView);
		}

		bEof = true;
	}
	else{
		//�ŏI�s�̎��̍s
		if( nCount > 0 && pDispPos->GetLayoutLineRef() == nCount ){
			//�ŏI�s�̎擾
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - CLayoutInt(1), &nLineLen, &pcLayout );
			
			//�ŏI�s�̌���
			CLayoutInt nLineCols = pcView->LineIndexToColmn( pcLayout, nLineLen );

			if( WCODE::IsLineDelimiter(pLine[nLineLen-1]) || nLineCols >= nWrapKetas ){
				// EOF�L���̕\��
				if( cEofType.IsDisp() ){
					_DispEOF(hdc,pDispPos,pcView);
				}

				bEof = true;
			}
		}
	}

	// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
	pcView->GetTextDrawer().DispVerticalLines(
		hdc,
		nYPrev,
		nYPrev + pcView->GetTextMetrics().GetHankakuDy(),
		CLayoutInt(0),
		CLayoutInt(-1)
	);

	return bEof;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �܂�Ԃ��`�����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �܂�Ԃ��`��
void _DispWrap(HDC hdc, DispPos* pDispPos, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,1))
	{
		//�T�|�[�g�N���X
		CTypeSupport cWrapType(pcView,COLORIDX_WRAP);

		//�`�敶����ƐF�̌���
		const wchar_t* szText;
		if( cWrapType.IsDisp() )
		{
			szText = L"<";
			cWrapType.SetFont(hdc);
			cWrapType.SetColors(hdc);
		}
		else
		{
			szText = L" ";
		}

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szText,
			wcslen(szText),
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       EOF�`�����                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
EOF�L���̕`��
@date 2004.05.29 genta  MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
@date 2007.08.28 kobake ���� nCharWidth �폜
@date 2007.08.28 kobake ���� fuOptions �폜
@date 2007.08.30 kobake ���� EofColInfo �폜
*/
void _DispEOF(
	HDC					hdc,		//!< [in] �`��Ώۂ�Device Context
	DispPos*			pDispPos,	//!< [in] �\�����W
	const CEditView*	pcView
)
{
	// �`��Ɏg���F���
	CTypeSupport cEofType(pcView,COLORIDX_EOF);

	//�K�v�ȃC���^�[�t�F�[�X���擾
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	//�萔
	const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	//�N���b�s���O�̈���v�Z
	RECT rcClip;
	if(pArea->GenerateClipRect(&rcClip,*pDispPos,nEofLen))
	{
		//�F�ݒ�
		cEofType.SetColors(hdc);
		cEofType.SetFont(hdc);

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip,
			szEof,
			nEofLen,
			pMetrics->GetDxArray_AllHankaku()
		);
	}

	//�`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nEofLen);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���s�`�����                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//��ʕ`��⏕�֐�
//May 23, 2000 genta
//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
void _DrawEOL(
	HDC				hdc,
	const CMyRect&	rcEol,
	CEol			cEol,
	bool			bBold,
	COLORREF		pColor
);

//2007.08.30 kobake �ǉ�
void _DispEOL(HDC hdc, DispPos* pDispPos, CEol cEol, bool bSearchStringMode, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){

		// �F����
		CTypeSupport cSupport(pcView,pcView->GetTextDrawer()._GetColorIdx(COLORIDX_EOL,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);

		// 2003.08.17 ryoji ���s�����������Ȃ��悤��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			L"  ",
			2,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// ���s�L���̕\��
		if( CTypeSupport(pcView,COLORIDX_EOL).IsDisp() ){
			// From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��

			// ���[�W�����쐬�A�I���B
			HRGN hRgn = ::CreateRectRgnIndirect(&rcClip2);
			::SelectClipRgn(hdc, hRgn);
			
			// �`��̈�
			CMyRect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(pcView->GetTextMetrics().GetHankakuWidth(), pcView->GetTextMetrics().GetHankakuHeight());

			// �`��
			_DrawEOL(hdc, rcEol, cEol, cSupport.IsFatFont(), cSupport.GetTextColor());

			// ���[�W�����j��
			::SelectClipRgn(hdc, NULL);
			::DeleteObject(hRgn);
			
			// To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
		}
	}

	//�`��ʒu��i�߂�(2��)
	pDispPos->ForwardDrawCol(2);
}


//	May 23, 2000 genta
/*!
��ʕ`��⏕�֐�:
�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
���܂葾�������Ȃ��B

@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
						���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void _DrawEOL(
	HDC				hdc,		//!< Device Context Handle
	const CMyRect&	rcEol,		//!< �`��̈�
	CEol			cEol,		//!< �s���R�[�h���
	bool			bBold,		//!< TRUE: ����
	COLORREF		pColor		//!< �F
)
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	�������
		sx = rcEol.left;						//X���[
		sy = rcEol.top + ( rcEol.Height() / 2);	//Y���S
		::MoveToEx( hdc, sx + rcEol.Width(), sy - rcEol.Height() / 4, NULL );	//	���
		::LineTo(   hdc, sx + rcEol.Width(), sy );			//	����
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		if ( bBold ) {
			::MoveToEx( hdc, sx + rcEol.Width() + 1, sy - rcEol.Height() / 4, NULL );	//	��ցi�E�ւ��炷�j
			++sy;
			::LineTo( hdc, sx + rcEol.Width() + 1, sy );	//	�E�ցi�E�ɂЂƂ���Ă���j
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = rcEol.left;
		sy = rcEol.top + ( rcEol.Height() / 2 );
		::MoveToEx( hdc, sx + rcEol.Width(), sy, NULL );	//	�E��
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		if ( bBold ) {
			++sy;
			::MoveToEx( hdc, sx + rcEol.Width(), sy, NULL );	//	�E��
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = rcEol.left + ( rcEol.Width() / 2 );
		sy = rcEol.top + ( rcEol.Height() * 3 / 4 );
		::MoveToEx( hdc, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );	//	���
		::LineTo(   hdc, sx, sy );								//	�ォ�牺��
		::LineTo(   hdc, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	���̂܂܍����
		::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
		::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	�����ĉE���
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );
			::LineTo(   hdc, sx, sy );								//	�ォ�牺��
			::LineTo(   hdc, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	���̂܂܍����
			::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
			::LineTo(   hdc, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	�����ĉE���
		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}
