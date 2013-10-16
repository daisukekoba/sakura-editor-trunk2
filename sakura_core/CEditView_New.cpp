/*!	@file
	@brief CEditView�N���X

	@author Norio Nakatani
	@date	1998/12/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, hor, novice, YAZAKI, aroka, KK
	Copyright (C) 2003, MIK, ryoji, �����
	Copyright (C) 2004, genta, Moca, MIK
	Copyright (C) 2005, genta, Moca, MIK, D.S.Koba
	Copyright (C) 2006, Moca
	Copyright (C) 2007, ryoji, Moca
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "Debug.h"
#include "Funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "CLayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka

/*! �ʏ�̕`�揈�� new 
	@param pPs  pPs.rcPaint �͐������K�v������
	@param bDrawFromComptibleBmp  TRUE ��ʃo�b�t�@����hdc�ɍ�悷��(�R�s�[���邾��)�B
			TRUE�̏ꍇ�ApPs.rcPaint�̈�O�͍�悳��Ȃ����AFALSE�̏ꍇ�͍�悳��鎖������B
			�݊�DC/BMP�������ꍇ�́A���ʂ̍�揈��������B
	@date 2007.09.09 Moca ���X����������Ă�����O�p�����[�^��bUseMemoryDC��bDrawFromComptibleBmp�ɕύX�B
	@date 2009.03.26 ryoji �s�ԍ��̂ݕ`���ʏ�̍s�`��ƕ����i�������j
*/
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == hdc ){
		return;
	}
//	if( m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		return;
//	}
	if( !m_bDrawSWITCH ){
		return;
	}

	//@@@
#if 0
	::MYTRACE( _T("OnPaint(%d,%d)-(%d,%d) : %d\n"),
		pPs->rcPaint.left,
		pPs->rcPaint.top,
		pPs->rcPaint.right,
		pPs->rcPaint.bottom,
		bDrawFromComptibleBmp
		);
#endif
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �݊�BMP����̓]���݂̂ɂ����
	if( bDrawFromComptibleBmp
		&& m_hdcCompatDC && m_hbmpCompatBMP ){
		::BitBlt(
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			m_hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if ( m_pcEditWnd->GetActivePane() == m_nMyIndex ){
			/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
			m_cUnderLine.CaretUnderLineON( true );
		}
		return;
	}
	if( m_hdcCompatDC && NULL == m_hbmpCompatBMP
		 || (pPs->rcPaint.right - pPs->rcPaint.left) < m_nCompatBMPWidth
		 || (pPs->rcPaint.bottom - pPs->rcPaint.top) < m_nCompatBMPHeight ){
		RECT rect;
		::GetWindowRect( m_hWnd, &rect );
		CreateOrUpdateCompatibleBitmap( rect.right - rect.left, rect.bottom - rect.top );
	}
	// To Here 2007.09.09 Moca

	/* �L�����b�g���B�� */
	bool bCaretShowFlag_Old = m_bCaretShowFlag;	// 2008.06.09 ryoji
	HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int				i;
	HFONT			hFontOld;
	HBRUSH			hBrush;
	COLORREF		crBackOld;
	COLORREF		crTextOld;
	HDC				hdcOld;
	int				nTop;
	RECT			rc;
	RECT			rcBack;
	BOOL			bEOF;
	int				nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	int				nCharWidth = m_nCharWidth + TypeDataPtr->m_nColumnSpace;
	int				nLineTo;
	int				nX = m_nViewAlignLeft - m_nViewLeftCol * nCharWidth;
	int				nY;
	const CLayout*	pcLayout;
	HPEN			hPen;
	HPEN			hPenOld;

//@@@ 2001.11.17 add start MIK
	//�ύX������΃^�C�v�ݒ���s���B
	if( TypeDataPtr->m_bUseRegexKeyword
	 || m_cRegexKeyword->m_bUseRegexKeyword )	//OFF�Ȃ̂ɑO��̃f�[�^���c���Ă�
	{
		//�^�C�v�ʐݒ������B�ݒ�ς݂��ǂ����͌Ăѐ�Ń`�F�b�N����B
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	/* �L�����b�g�̍s���ʒu��\������ */
//	DrawCaretPosInfo();

	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�͕`���̂c�b��؂�ւ��� */
	// 2007.09.09 Moca bUseMemoryDC��L�����B
	// bUseMemoryDC = FALSE;
	BOOL bUseMemoryDC = (m_hdcCompatDC != NULL);
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 �Ί��ʂ̋����\��(����) ai */
	DrawBracketPair( false );

	/* ���[���[�ƃe�L�X�g�̊Ԃ̗]�� */
	//@@@ 2002.01.03 YAZAKI �]����0�̂Ƃ��͖��ʂł����B
	if ( m_nTopYohaku ){
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	
	/* �s�ԍ��̕\�� */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta �s�ԍ���\���ł��s�ԍ��F�̑т�����̂Ō��Ԃ𖄂߂�
	if( m_nTopYohaku ){ 
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		//	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rc.right = m_nViewAlignLeft - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_sColorAttr.m_cBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, m_pcViewFont->GetFontHan() );
	m_hFontOld = NULL;


//	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_colorBACK );
//	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_colorTEXT );
	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT );

//	::FillRect( hdc, &pPs->rcPaint, ::GetStockObject( WHITE_BRUSH ) );


	nTop = pPs->rcPaint.top;
	bEOF = FALSE;
//	i = m_nViewTopLine;

	if( 0 > nTop - m_nViewAlignTop ){
		i = m_nViewTopLine;
	}else{
		i = m_nViewTopLine + ( ( nTop - m_nViewAlignTop ) / nLineHeight );
	}

	int nMaxRollBackLineNum = 260 / nWrapWidth + 1;
	int nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->m_ptLogicPos.x ){	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
			break;
		}
		i--;
		nRollBackLineNum++;
	}

	nY = ( i - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
	nLineTo = m_nViewTopLine + ( ( pPs->rcPaint.bottom - m_nViewAlignTop + (nLineHeight - 1) ) / nLineHeight ) - 1;	// 2009.02.17 ryoji �v�Z�𐸖���

	BOOL bSelected;
	bSelected = IsTextSelected();
	if( pPs->rcPaint.right <= m_nViewAlignLeft ){	// 2009.03.26 ryoji �s�ԍ��̂ݕ`���ʏ�̍s�`��ƕ����i�������j
		for( ; i <= nLineTo; ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout )
				break;
			DispLineNumber( hdc, pcLayout, i, nY );
			nY += nLineHeight;	//�`��Y���W�{�{
			i++;				//���C�A�E�g�s�{�{
		}
	}else{
		for( ; i <= nLineTo; ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( DispLineNew(
				hdc,
				pcLayout,
				i,
				nX,
				nY,
				nLineTo,
				bSelected
			) ){
				pPs->rcPaint.bottom += nLineHeight;	/* EOF�ĕ`��Ή� */
				bEOF = TRUE;
				break;
			}
			if( NULL == pcLayout ){
				bEOF = TRUE;
				break;
			}
		}
	}
	if( NULL != m_hFontOld ){
		::SelectObject( hdc, m_hFontOld );
		m_hFontOld = NULL;
	}

	/* �e�L�X�g�̂Ȃ�������w�i�F�œh��Ԃ� */
	if( nY < pPs->rcPaint.bottom ){
		rcBack.left = pPs->rcPaint.left;
		rcBack.right = pPs->rcPaint.right;
		rcBack.top = nY;
		rcBack.bottom = pPs->rcPaint.bottom;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
		::FillRect( hdc, &rcBack, hBrush );
		::DeleteObject( hBrush );

		// 2006.04.29 �s�����͍s���Ƃɍ�悵�A�����ł͏c���̎c������
		DispVerticalLines( hdc, nY, pPs->rcPaint.bottom, 0, -1 );
	}

	::SetTextColor( hdc, crTextOld );
	::SetBkColor( hdc, crBackOld );
	::SelectObject( hdc, hFontOld );

	/* �܂�Ԃ��ʒu�̕\�� */
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
		int nXPos = m_nViewAlignLeft + ( nWrapWidth - m_nViewLeftCol ) * nCharWidth;
		//	2005.11.08 Moca �������ύX
		if( m_nViewAlignLeft < nXPos && nXPos < m_nViewCx + m_nViewAlignLeft ){
			/* �܂�Ԃ��L���̐F�̃y�����쐬 */
			hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, nXPos, m_nViewAlignTop, NULL );
			::LineTo( hdc, nXPos, m_nViewAlignTop + m_nViewCy );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
	}


	/* ���[���[�`�� */
	if ( pPs->rcPaint.top < m_nViewAlignTop - m_nTopYohaku ) { // ���[���[���ĕ`��͈͂ɂ���Ƃ��̂ݍĕ`�悷�� 2002.02.25 Add By KK
		m_bRedrawRuler = true; //2002.02.25 Add By KK ���[���[�S�̂�`��B
		DispRuler( hdc );
	}

	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�̓������c�b�ɕ`�悵�����e����ʂփR�s�[���� */
	if( bUseMemoryDC ){
		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	//     �A���_�[���C���`���������DC����̃R�s�[�O���������Ɉړ�
	if ( m_pcEditWnd->GetActivePane() == m_nMyIndex ){
		/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
		m_cUnderLine.CaretUnderLineON( true );
	}
	// To Here 2007.09.09 Moca

	/* 03/02/18 �Ί��ʂ̋����\��(�`��) ai */
	DrawBracketPair( true );

	/* �L�����b�g�����݈ʒu�ɕ\�����܂� */
	if( bCaretShowFlag_Old )	// 2008.06.09 ryoji
		ShowCaret_( m_hWnd ); // 2002/07/22 novice
	return;
}









//@@@ 2001.02.17 Start by MIK
/*! �s�̃e�L�X�g�^�I����Ԃ̕`��
	1���1�_���s������悷��B

	@return EOF����悵����true

	@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX
	@date 2007.08.31 kobake ���� bDispBkBitmap ���폜
*/
bool CEditView::DispLineNew(
	HDC						hdc,			//!< ���Ώ�
	const CLayout*			pcLayout,		//!< �\�����J�n���郌�C�A�E�g
	int&					nLineNum,		//!< ��悷�郌�C�A�E�g�s�ԍ�(0�J�n), ���̕����s�ɑΉ����郌�C�A�E�g�s�ԍ�
	int						x,				//!< ���C�A�E�g0���ڂ̍����Wx
	int&					y,				//!< �����Wy, ���̍����Wy
	int						nLineTo,		//!< ���I�����郌�C�A�E�g�s�ԍ�
	BOOL					bSelected		//!< �I�𒆂�
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	EColorIndexType			nCOMMENTMODE;
	EColorIndexType			nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						nColorIndex;
	bool					bDispEOF = false;

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->m_pCDocLine->m_cLine.GetStringLength() - pcLayout->m_ptLogicPos.x;
		pLine = pcLayout->m_pCDocLine->m_cLine.GetStringPtr() + pcLayout->m_ptLogicPos.x;

		nCOMMENTMODE = pcLayout->m_nTypePrev;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}

	/* ���݂̐F���w�� */
	SetCurrentColor( hdc, nCOMMENTMODE );

	int					nX = 0;
	//	�e�L�X�g�`�惂�[�h
	const UINT			fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	const int			nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	const int			nCharWidth = m_nCharWidth + TypeDataPtr->m_nColumnSpace;

	static const char*	pszSPACES = "        ";
	static const char*	pszZENSPACE	= "��";
	static const char*	pszWRAP	= "<";


	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		y -= nLineHeight;
		nLineNum--;
//		MYTRACE( _T("\n\n=======================================") );

		int			nBgn = 0;
		int			nPos = 0;
		int			nLineBgn =0;
		int			nCharChars = 0;
		BOOL		bSearchStringMode = FALSE;
		BOOL		bSearchFlg = TRUE;	// 2002.02.08 hor
		int			nSearchStart = -1;	// 2002.02.08 hor
		int			nSearchEnd	= -1;	// 2002.02.08 hor
		bool		bKeyWordTop	= true;	//	Keyword Top

		int			nNumLen;
		int			nUrlLen;
		//@@@ 2001.11.17 add start MIK
		int			nMatchLen;
		int			nMatchColor;

		//@@@ 2001.12.21 YAZAKI
		HBRUSH		hBrush;
		COLORREF	colTextColorOld;
		COLORREF	colBkColorOld;
		RECT		rcClip;
		RECT		rcClip2;

		while( nPos < nLineLen ){
//			MYTRACE( _T("nLineNum = %d\n"), nLineNum );

			y += nLineHeight;
			nLineNum++;
			if( m_nViewTopLine + m_nViewRowNum < nLineNum ){
				nLineNum = nLineTo + 1;
				goto end_of_func;
			}
			if( nLineTo < nLineNum ){
				goto end_of_func;
			}
			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* �s�ԍ��\�� */
				DispLineNumber( hdc, pcLayout2, nLineNum, y );
			}
			nBgn = nPos;
			nLineBgn = nBgn;
			nX = 0;

			//	�s���w�i
			if (pcLayout2 && pcLayout2->GetIndent()){
				rcClip.left = x;
				rcClip.right = x + pcLayout2->GetIndent() * ( nCharWidth );
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}
				nX += pcLayout2->GetIndent();
			}

			while( nPos - nLineBgn < pcLayout2->m_nLength ){
				/* ����������̐F���� */
				if( m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* ���݂̐F���w�� */
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}else
					if( bSearchStringMode
					 && nSearchEnd <= nPos		//+ == �ł͍s�������̏ꍇ�AnSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
					){
						// ��������������̏I���܂ł�����A�����F��W���ɖ߂�����
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						/* ���݂̐F���w�� */
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						nBgn = nPos + 1;
						/* �s���w�i�`�� */
						rcClip.left = x + nX * ( nCharWidth );
						rcClip.right = m_nViewAlignLeft + m_nViewCx;
						rcClip.top = y;
						rcClip.bottom = y + nLineHeight;
						if( rcClip.left < m_nViewAlignLeft ){
							rcClip.left = m_nViewAlignLeft;
						}
						if( rcClip.left < rcClip.right &&
							rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
							hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
							::FillRect( hdc, &rcClip, hBrush );
							::DeleteObject( hBrush );
						}

						/* ���s�L���̕\�� */
						rcClip2.left = x + nX * ( nCharWidth );
						// Jul. 20, 2003 ryoji ���X�N���[�����ɉ��s�R�[�h�������Ȃ��悤��
						rcClip2.right = rcClip2.left + ( nCharWidth ) * ( 2 );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							// 2006.04.30 Moca �F�I�������ʓ��Ɉړ�
							if( bSearchStringMode ){
								nColorIndex = COLORIDX_SEARCH;
							}else{
								nColorIndex = COLORIDX_EOL;
							}
							HFONT	hFontOld;
							/* �t�H���g��I�� */
							hFontOld = (HFONT)::SelectObject( hdc,
								m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
							);
							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT ); /* CRLF�̐F */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* CRLF�w�i�̐F */
							//	2003.08.17 ryoji ���s�����������Ȃ��悤��
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, (const char *)"  ", 2, m_pnDx );
							/* ���s�L���̕\�� */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOL].m_bDisp ){
								int nPosX = x + nX * ( nCharWidth );
								int nPosY = y;
								//	From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
								HRGN hRgn;
								hRgn = ::CreateRectRgnIndirect(&rcClip2);
								::SelectClipRgn(hdc, hRgn);
								//@@@ 2001.12.21 YAZAKI
								DrawEOL(hdc, nPosX + 1, nPosY, m_nCharWidth, m_nCharHeight,
									pcLayout2->m_cEol, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr.m_bBoldFont,
										TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );
								::SelectClipRgn(hdc, NULL);
								::DeleteObject(hRgn);
								//	To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
							}
							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}

						nX++;


						// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
						DispVerticalLines( hdc, y, y + nLineHeight, 0, -1 );
						if( bSelected ){
							/* �e�L�X�g���] */
							DispTextSelected( hdc, nLineNum, x, y, nX );
						}
					}

					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//���K�\���L�[���[�h
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = (EColorIndexType)(COLORIDX_REGEX_FIRST + nMatchColor);	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[0].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[0].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[0].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[1].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[1].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[1].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == '"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (nNumLen = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						nNumLen = nPos + nNumLen;
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = nNumLen;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* �L�[���[�h������̏I�[��T�� */
						int nKeyEnd;
						for( nKeyEnd = nPos + 1; nKeyEnd<= nLineLen - 1; ++nKeyEnd ){
							if( !IS_KEYWORD_CHAR( pLine[nKeyEnd] ) ){
								break;
							}
						}
						int nKeyLen = nKeyEnd - nPos;

						/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
						// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z�� //MIK 2000.12.01 second keyword & binary search
						for( int n = 0; n < MAX_KEYWORDSET_PER_TYPE; n++ )
						{
							// �����L�[���[�h�͑O�l�߂Őݒ肳���̂ŁA���ݒ��Index������Β��f
							if(TypeDataPtr->m_nKeyWordSetIdx[n] == -1 ){
									break;
							}
							else if(TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + n].m_bDisp)
							{
								/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
								int nIdx = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2(
									TypeDataPtr->m_nKeyWordSetIdx[n],
									&pLine[nPos],
									nKeyLen
								);
								if( nIdx >= 0 ){
									if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
										/* �e�L�X�g�\�� */
										nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
									}
									/* ���݂̐F���w�� */
									nBgn = nPos;
									nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + n);
									nCOMMENTEND = nKeyEnd;
									if( !bSearchStringMode ){
										SetCurrentColor( hdc, nCOMMENTMODE );
									}
									break;
								}
							}
						}
						//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URL���[�h�ł��� */
				case COLORIDX_KEYWORD1:	/* �����L�[���[�h1 */
				case COLORIDX_DIGIT:	/* ���p���l�ł��� */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* �����L�[���[�h2 */	//MIK
				case COLORIDX_KEYWORD3:	// 2005.01.13 MIK �����L�[���[�h3-10
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[0].Match_CommentTo(nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[1].Match_CommentTo(nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE & COLORIDX_REGEX_BIT ){	//���K�\���L�[���[�h1�`10
						if( nPos == nCOMMENTEND ){
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* �e�L�X�g�\�� */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == TAB ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						//	Sep. 22, 2002 genta ���ʎ��̂����肾��
						//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
						int tabDispWidth = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nX );
						/* �^�u�L����\������ */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * tabDispWidth;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							/* TAB��\�����邩�H */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
							 && !TypeDataPtr->m_bTabArrow ){	//�^�u�ʏ�\��	//@@@ 2003.03.26 MIK
								if( bSearchStringMode ){
									nColorIndex = COLORIDX_SEARCH;
								}else{
									nColorIndex = COLORIDX_TAB;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* TAB�����̐F */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* TAB�����w�i�̐F */

								HFONT	hFontOld;
								/* �t�H���g��I�� */
								hFontOld = (HFONT)::SelectObject( hdc,
									m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
								);
								
								//@@@ 2001.03.16 by MIK
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, /*pszTAB*/ TypeDataPtr->m_szTabViewString,
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );
							}else{
								if( bSearchStringMode ){
									nColorIndex = COLORIDX_SEARCH;
									colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* TAB�����w�i�̐F */
								}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES,
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
								if( bSearchStringMode ){
									::SetBkColor( hdc, colBkColorOld );
								}
								
								//�^�u���\��	//@@@ 2003.03.26 MIK
								if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
								 && TypeDataPtr->m_bTabArrow
								 && rcClip2.left <= x + nX * nCharWidth ) // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
								{
									DrawTabArrow( hdc, x + nX * ( nCharWidth ), y, m_nCharWidth, m_nCharHeight,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_sFontAttr.m_bBoldFont,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_sColorAttr.m_cTEXT );
								}
							}
						}
						nX += tabDispWidth ;//	Sep. 22, 2002 genta
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );

						/* �S�p�󔒂�\������ */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * 2;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;

							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp ){	/* ���{��󔒂�\�����邩 */
								if( bSearchStringMode ){
									nColorIndex = COLORIDX_SEARCH;
								}else{
									nColorIndex = COLORIDX_ZENSPACE;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* �S�p�X�y�[�X�����̐F */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* �S�p�X�y�[�X�����w�i�̐F */


								HFONT	hFontOld;
								/* �t�H���g��I�� */
								hFontOld = (HFONT)::SelectObject( hdc,
									m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
								);

								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszZENSPACE, lstrlen( pszZENSPACE ), m_pnDx );

								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );

							}else{
							if( bSearchStringMode ){
								nColorIndex = COLORIDX_SEARCH;
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );	/* �����w�i�̐F */
							}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES, 2, m_pnDx );
							}
							if( bSearchStringMode ){
								::SetBkColor( hdc, colBkColorOld );
							}
						}
						nX += 2;
					}
					nBgn = nPos + 2;
					nCharChars = 2;
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )
				{
					nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
					if( y >= m_nViewAlignTop ){
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){

							if( bSearchStringMode ){
								nColorIndex = COLORIDX_SEARCH;
							}else{
								nColorIndex = COLORIDX_SPACE;
							}

							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cTEXT );	/* ���p�X�y�[�X�����̐F */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sColorAttr.m_cBACK );		/* ���p�X�y�[�X�����w�i�̐F */
							HFONT	hFontOld = (HFONT)::SelectObject( hdc,
								m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[nColorIndex].m_sFontAttr )
							);

							//������"o"�̉��������o��
							rcClip2.top = y + m_nCharHeight/2;
							rcClip2.bottom = y + nLineHeight;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, "o", 1, m_pnDx );

							//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
							rcClip2.top = y ;
							rcClip2.bottom = y + m_nCharHeight/2;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, " ", 1, m_pnDx );

							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}
						nX++;
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 &&	(
								//	Jan. 23, 2002 genta �x���}��
							( (unsigned char)pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < (unsigned char)pLine[nPos] && (unsigned char)pLine[nPos] < (unsigned char)'�' ) ||
							( (unsigned char)'�' < (unsigned char)pLine[nPos] )
						)
					 && pLine[nPos] != TAB && pLine[nPos] != CR && pLine[nPos] != LF
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						int nCtrlEnd;
						for( nCtrlEnd = nPos + 1; nCtrlEnd <= nLineLen - 1; ++nCtrlEnd ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, nCtrlEnd );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta �x���}��
								( (unsigned char)pLine[nCtrlEnd] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[nCtrlEnd] && (unsigned char)pLine[nCtrlEnd] < (unsigned char)'�' ) ||
									( (unsigned char)'�' < (unsigned char)pLine[nCtrlEnd] )
								) &&
								pLine[nCtrlEnd] != TAB && pLine[nCtrlEnd] != CR && pLine[nCtrlEnd] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = nCtrlEnd;
						/* ���݂̐F���w�� */
						SetCurrentColor( hdc, nCOMMENTMODE );
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength )
			if( nPos >= nLineLen ){
				break;
			}
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* �e�L�X�g�\�� */
				nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
				nBgn = nPos;

				/* �s���w�i�`�� */
				rcClip.left = x + nX * ( nCharWidth );
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}

				/* �܂�Ԃ��L�� */
				/* �܂�Ԃ��L����\������ */
				rcClip2.left = x + nX * ( nCharWidth );
				rcClip2.right = rcClip2.left + ( nCharWidth ) * 1;
				if( rcClip2.left < m_nViewAlignLeft ){
					rcClip2.left = m_nViewAlignLeft;
				}
				if( rcClip2.left < rcClip2.right &&
					rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
					rcClip2.top = y;
					rcClip2.bottom = y + nLineHeight;

					/* �܂�Ԃ��L����\������ */
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
						HFONT	hFontOld;
						colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cTEXT );	/* �܂�Ԃ��L���̐F */
						colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sColorAttr.m_cBACK );		/* �܂�Ԃ��L���w�i�̐F */
						/* �t�H���g��I�� */
						hFontOld = (HFONT)::SelectObject( hdc,
							m_pcViewFont->ChooseFontHandle( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_sFontAttr )
						);
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, pszWRAP, lstrlen( pszWRAP ), m_pnDx );

						::SelectObject( hdc, hFontOld );
						::SetTextColor( hdc, colTextColorOld );
						::SetBkColor( hdc, colBkColorOld );
					}else{
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, " ", 1, m_pnDx );
					}
				}
				// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
				DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
				if( bSelected ){
					/* �e�L�X�g���] */
					DispTextSelected( hdc, nLineNum, x, y, nX );
				}
			}
		}
		
		if( y >= m_nViewAlignTop ){
			/* �e�L�X�g�\�� */
			nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
			/* EOF�L���̕\�� */
			if( nLineNum + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				nX < nWrapWidth
			){
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					nX += DispEOF( hdc, x + nX * ( nCharWidth ), y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				bDispEOF = true;
			}

			/* �s���w�i�`�� */
			rcClip.left = x + nX * ( nCharWidth );
			rcClip.right = m_nViewAlignLeft + m_nViewCx;
			rcClip.top = y;
			rcClip.bottom = y + nLineHeight;
			if( rcClip.left < m_nViewAlignLeft ){
				rcClip.left = m_nViewAlignLeft;
			}
			if( rcClip.left < rcClip.right &&
				rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
				hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
				::FillRect( hdc, &rcClip, hBrush );
				::DeleteObject( hBrush );
			}

			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
			if( bSelected ){
				/* �e�L�X�g���] */
				DispTextSelected( hdc, nLineNum, x, y, nX );
			}
		}
end_of_line:;
		nLineNum++;
		y += nLineHeight;
	}else{ // NULL == pLine�̏ꍇ
		if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
			int nYPrev = y;

			/* �w�i�`�� */
			RECT		rcClip;
			rcClip.left = 0;
			rcClip.right = m_nViewAlignLeft + m_nViewCx;
			rcClip.top = y;
			rcClip.bottom = y + nLineHeight;
			HBRUSH hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
			::FillRect( hdc, &rcClip, hBrush );
			::DeleteObject( hBrush );

			/* EOF�L���̕\�� */
			int nCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( nCount == 0 && m_nViewTopLine == 0 && nLineNum == 0 ){
				/* EOF�L���̕\�� */
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				y += nLineHeight;
				bDispEOF = true;
			}else{
				if( nCount > 0 && nLineNum == nCount ){
					const char*	pLine;
					int			nLineLen;
					const CLayout* pcLayout;
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - 1, &nLineLen, &pcLayout );
					int nLineCols = LineIndexToColumn( pcLayout, nLineLen );
					if( ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ||
						nLineCols >= nWrapWidth
					 ){
						/* EOF�L���̕\�� */
						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
							//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
							nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
						}
						y += nLineHeight;
						bDispEOF = true;
					}
				}
			}
			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			DispVerticalLines( hdc, nYPrev, nYPrev + nLineHeight,  0, -1 );
		}
	}

end_of_func:;
	return bDispEOF;
}

//	May 23, 2000 genta
/*!	��ʕ`��⏕�֐�:
	�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

	@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
	���܂葾�������Ȃ��B
	
	@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
							���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void CEditView::DrawEOL(
	HDC hdc,			//!< Device Context Handle
	int nPosX,			//!< �`����WX
	int nPosY,			//!< �`����WY
	int nWidth,			//!< �`��G���A�̃T�C�YX
	int nHeight,		//!< �`��G���A�̃T�C�YY
	CEol cEol,			//!< �s���R�[�h���
	int bBold,			//!< TRUE: ����
	COLORREF pColor		//!< �F
)
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	�������
		{
			sx = nPosX;
			sy = nPosY + ( nHeight / 2);
			DWORD pp[] = { 3, 3 };
			POINT pt[6];
			pt[0].x = sx + nWidth;	//	���
			pt[0].y = sy - nHeight / 4;
			pt[1].x = sx + nWidth;	//	����
			pt[1].y = sy;
			pt[2].x = sx;	//	�擪��
			pt[2].y = sy;
			pt[3].x = sx + nHeight / 4;	//	�擪���牺��
			pt[3].y = sy + nHeight / 4;
			pt[4].x = sx;	//	�擪�֖߂�
			pt[4].y = sy;
			pt[5].x = sx + nHeight / 4;	//	�擪������
			pt[5].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 1;	//	��ցi�E�ւ��炷�j
				pt[0].y += 0;
				pt[1].x += 1;	//	�E�ցi�E�ɂЂƂ���Ă���j
				pt[1].y += 1;
				pt[2].x += 0;	//	�擪��
				pt[2].y += 1;
				pt[3].x += 0;	//	�擪���牺��
				pt[3].y += 1;
				pt[4].x += 0;	//	�擪�֖߂�
				pt[4].y += 1;
				pt[5].x += 0;	//	�擪������
				pt[5].y += 1;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		{
			sx = nPosX;
			sy = nPosY + ( nHeight / 2 );
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx + nWidth;	//	�E��
			pt[0].y = sy;
			pt[1].x = sx;	//	�擪��
			pt[1].y = sy;
			pt[2].x = sx + nHeight / 4;	//	�擪���牺��
			pt[2].y = sy + nHeight / 4;
			pt[3].x = sx;	//	�擪�֖߂�
			pt[3].y = sy;
			pt[4].x = sx + nHeight / 4;	//	�擪������
			pt[4].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	�E��
				pt[0].y += 1;
				pt[1].x += 0;	//	�擪��
				pt[1].y += 1;
				pt[2].x += 0;	//	�擪���牺��
				pt[2].y += 1;
				pt[3].x += 0;	//	�擪�֖߂�
				pt[3].y += 1;
				pt[4].x += 0;	//	�擪������
				pt[4].y += 1;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		{
			sx = nPosX + ( nWidth / 2 );
			sy = nPosY + ( nHeight * 3 / 4 );

			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx;	//	���
			pt[0].y = nPosY + nHeight / 4 + 1;
			pt[1].x = sx;	//	�ォ�牺��
			pt[1].y = sy;
			pt[2].x = sx - nHeight / 4;	//	���̂܂܍����
			pt[2].y = sy - nHeight / 4;
			pt[3].x = sx;	//	���̐�[�ɖ߂�
			pt[3].y = sy;
			pt[4].x = sx + nHeight / 4;	//	�����ĉE���
			pt[4].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if( bBold ){
				pt[0].x += 1;	//	���
				pt[0].y += 0;
				pt[1].x += 1;	//	�ォ�牺��
				pt[1].y += 0;
				pt[2].x += 1;	//	���̂܂܍����
				pt[2].y += 0;
				pt[3].x += 1;	//	���̐�[�ɖ߂�
				pt[3].y += 0;
				pt[4].x += 1;	//	�����ĉE���
				pt[4].y += 0;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_LFCR:
		{
			sx = nPosX + ( nWidth / 2 );
			sy = nPosY + ( nHeight * 3 / 4 );
			DWORD pp[] = { 4, 2 };
			POINT pt[6];
			pt[0].x = sx + nWidth / 2;	//	�E���
			pt[0].y = nPosY + nHeight / 4 + 1;
			pt[1].x = sx;	//	�E���獶��
			pt[1].y = nPosY + nHeight / 4 + 1;
			pt[2].x = sx;	//	�ォ�牺��
			pt[2].y = sy;
			pt[3].x = sx - nHeight / 4;	//	���̂܂܍����
			pt[3].y = sy - nHeight / 4;
			pt[4].x = sx;	//	���̐�[�ɖ߂�
			pt[4].y = sy;
			pt[5].x = sx + nHeight / 4;	//	���̐�[�ɖ߂�
			pt[5].y = sy - nHeight / 4;
			::PolyPolyline( hdc, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	�E���
				pt[0].y += 1;
				pt[1].x += 1;	//	�E���獶��
				pt[1].y += 1;
				pt[2].x += 1;	//	�ォ�牺��
				pt[2].y += 0;
				pt[3].x += 1;	//	���̂܂܍����
				pt[3].y += 0;
				pt[4].x += 1;	//	���̐�[�ɖ߂�
				pt[4].y += 0;
				pt[5].x += 1;	//	�����ĉE���
				pt[5].y += 0;
				::PolyPolyline( hdc, pt, pp, _countof(pp));
			}

//			::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 1, NULL );	//	�E���
//			::LineTo(   hdc, sx, nPosY + nHeight / 4 + 1 );			//	�E���獶��
//			::LineTo(   hdc, sx, sy );								//	�ォ�牺��
//			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
//			::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
//			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
//			if( bBold ){
//				::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 2, NULL );	//	�E���
//				++sx;
//				::LineTo(   hdc, sx, nPosY + nHeight / 4 + 2 );			//	�E���獶��
//				::LineTo(   hdc, sx, sy );								//	�ォ�牺��
//				::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
//				::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
//				::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
//			}
		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*
	�^�u���`��֐�
*/
void CEditView::DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor )
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;

	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;
	sx = nPosX + nWidth;
	sy = nPosY + ( nHeight / 2 );
	::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	����
	::LineTo(   hdc, sx, sy );					//	�Ō��
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
	::MoveToEx( hdc, sx, sy, NULL);				//	�Ō�֖߂�
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	����
		::LineTo(   hdc, sx, sy );					//	�Ō��
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�Ō�֖߂�
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*! EOF�L���̕`��

	@param hdc	[in] �`��Ώۂ�Device Context
	@param x	[in] �\�����Wx
	@param y	[in] �\�����Wy
	@param nCharWidth	[in] 1�����̉���(pixel)
	@param fuOptions	[in] ExtTextOut()�ɓn���`��I�v�V����
	@param EofColInfo	[in] �`��Ɏg���F���

  @date 2004.05.29 genta MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
*/
int CEditView::DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo )
{
	//	���̐F��ۑ����邽��
	COLORREF	colTextColorOld;
	COLORREF	colBkColorOld;
	RECT		rcClip;
	static const char	pszEOF[] = "[EOF]";
	const int	szEOFlen = sizeof( pszEOF) - 1;

	colTextColorOld = ::SetTextColor( hdc, EofColInfo.m_sColorAttr.m_cTEXT );	/* EOF�̐F */
	colBkColorOld = ::SetBkColor( hdc, EofColInfo.m_sColorAttr.m_cBACK );		/* EOF�w�i�̐F */
	rcClip.left = /*m_nViewAlignLeft*/x;
	rcClip.right = rcClip.left + ( nCharWidth ) * szEOFlen;
	if( rcClip.left < m_nViewAlignLeft ){
		rcClip.left = m_nViewAlignLeft;
	}
	if( rcClip.left < rcClip.right &&
		rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;

		HFONT	hFontOld;
		/* �t�H���g��I�� */
		hFontOld = (HFONT)::SelectObject( hdc,
			m_pcViewFont->ChooseFontHandle( EofColInfo.m_sFontAttr )
		);

		::ExtTextOut( hdc, x , y, fuOptions,
			&rcClip, pszEOF, szEOFlen, m_pnDx );

		::SelectObject( hdc, hFontOld );
	}

	::SetTextColor( hdc, colTextColorOld );
	::SetBkColor( hdc, colBkColorOld );
	
	return szEOFlen;
}
// �@�@�@

/*!	�w�茅�c���̕`��
	@param hdc     ��悷��E�B���h�E��DC
	@param nTop    ����������[�̃N���C�A���g���Wy
	@param nButtom �����������[�̃N���C�A���g���Wy
	@param nColLeft  ���������͈͂̍����̎w��
	@param nColRight ���������͈͂̉E���̎w��(-1�Ŗ��w��)

	@date 2005.11.08 Moca �V�K�쐬
	@date 2006.04.29 Moca �����E�_���̃T�|�[�g�B�I�𒆂̔��]�΍�ɍs���Ƃɍ�悷��悤�ɕύX
	    �c���̐F���e�L�X�g�̔w�i�F�Ɠ����ꍇ�́A�c���̔w�i�F��EXOR�ō�悷��
	@note Common::m_sWindow::m_nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void CEditView::DispVerticalLines( HDC hdc, int nTop, int nBottom, int nLeftCol, int nRightCol )
{
	const STypeConfig&	typeData = m_pcEditDoc->GetDocumentAttribute();
	if( !typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp ){
		return;
	}
	nLeftCol = t_max( m_nViewLeftCol, nLeftCol );
	const int nWrapWidth  = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharWidth  = m_nCharWidth + typeData.m_nColumnSpace;
	if( nRightCol < 0 ){
		nRightCol = nWrapWidth;
	}
	const int nPosXOffset = m_pShareData->m_Common.m_sWindow.m_nVertLineOffset + m_nViewAlignLeft;
	const int nPosXLeft   = t_max( m_nViewAlignLeft + (nLeftCol  - m_nViewLeftCol) * nCharWidth, m_nViewAlignLeft );
	const int nPosXRight  = t_min( m_nViewAlignLeft + (nRightCol - m_nViewLeftCol) * nCharWidth, m_nViewCx + m_nViewAlignLeft );
	const int nLineHeight = m_nCharHeight + typeData.m_nLineSpace;
	bool bOddLine = ((((nLineHeight % 2) ? m_nViewTopLine : 0) + m_nViewAlignTop + nTop) % 2 == 1);

	// ����
	const BOOL bBold = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sFontAttr.m_bBoldFont;
	// �h�b�g��(����������]�p/�e�X�g�p)
	const BOOL bDot = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sFontAttr.m_bUnderLine;
	const bool bExorPen = ( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cTEXT 
		== typeData.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cBACK );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_sColorAttr.m_cTEXT );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXCol��1�J�n�Bm_nViewLeftCol��0�J�n�Ȃ̂Œ��ӁB
		int nXCol = typeData.m_nVertLineIdx[k];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		// nXCol���}�C�i�X���ƌJ��Ԃ��Bk+1���I���l�Ak+2���X�e�b�v���Ƃ��ė��p����
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// ���͈͂̎n�߂܂ŃX�L�b�v
				if( nXCol < m_nViewLeftCol ){
					nXCol = m_nViewLeftCol + nXColAdd - (m_nViewLeftCol - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapWidth < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + ( nXCol - 1 - m_nViewLeftCol ) * nCharWidth;
			// 2006.04.30 Moca ���̈����͈́E���@��ύX
			// �����̏ꍇ�A����������悷��\��������B
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// �_���ō��B1�h�b�g�̐����쐬
					int y = nTop;
					// �X�N���[�����Ă������؂�Ȃ��悤�ɍ��W�𒲐�
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}


/*[EOF]*/
