/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date   2005/09/02 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, �����, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta, maru
	Copyright (C) 2007, ryoji, ���イ��, maru, genta, Moca, nasukoji, D.S.Koba
	Copyright (C) 2008, ryoji, nasukoji, bosagami, Moca, genta
	Copyright (C) 2009, nasukoji, ryoji, syat
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditView.h"
#include "CEditWnd.h"

/*!	IME ON��

	@date  2006.12.04 ryoji �V�K�쐬�i�֐����j
*/
bool CEditView::IsImeON( void )
{
	bool bRet;
	HIMC	hIme;
	DWORD	conv, sent;

	//	From here Nov. 26, 2006 genta
	hIme = ImmGetContext( m_hwndParent );
	if( ImmGetOpenStatus( hIme ) != FALSE ){
		ImmGetConversionStatus( hIme, &conv, &sent );
		if(( conv & IME_CMODE_NOCONVERSION ) == 0 ){
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	else {
		bRet = false;
	}
	ImmReleaseContext( m_hwndParent, hIme );
	//	To here Nov. 26, 2006 genta

	return bRet;
}

/* IME�ҏW�G���A�̈ʒu��ύX */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( m_hWnd );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + m_nCaretHeight - m_nCharHeight;

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
}


/* IME�ҏW�G���A�̕\���t�H���g��ύX */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( m_hWnd );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_sView.m_lf) );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
}
//  2002.04.09 minfu from here
/*�ĕϊ��p �J�[�\���ʒu����O��200byte�����o����RECONVERTSTRING�𖄂߂� */
/*  ����  pReconv RECONVERTSTRING�\���̂ւ̃|�C���^�B                     */
/*        bUnicode true�Ȃ��UNICODE�ō\���̂𖄂߂�                      */
/*  �߂�l   RECONVERTSTRING�̃T�C�Y                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	const char	*pLine;
	int			nCurrentLine;
	
	//�s�̒��ōĕϊ���API�ɂ킽���Ƃ��镶����̊J�n�ʒu�ƒ����i�l��������j
	int			nReconvIndex, nReconvLenWithNull;
	
	//�s�̒��ōĕϊ��̒��ڂ��镶�߂Ƃ��镶����̊J�n�ʒu�A�I���ʒu�A�����i�Ώە�����j
	int			nSelectedIndex, nSelectedEndIndex, nSelectedLen;
	
	int			nSelectColumnFrom;
	int			nSelectLineFrom, nSelectLineTo;
	
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	
	CMemory		cmemBuf1;
	const char*		pszReconv;
	CDocLine*	pcCurDocLine;
	
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//��`�I�𒆂͉������Ȃ�
	if( m_bBeginBoxSelect )
		return 0;
	
	if( IsTextSelected() ){
		//�e�L�X�g���I������Ă���Ƃ�
		nSelectColumnFrom = m_sSelect.m_ptFrom.x;
		nSelectLineFrom   = m_sSelect.m_ptFrom.y;
		
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, &nSelectedIndex, &nCurrentLine);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, &nSelectedEndIndex, &nSelectLineTo);
		
		//�I��͈͂������s�̎���
		if (nSelectLineTo != nCurrentLine){
			//�s���܂łɐ���
			pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(nCurrentLine);
			nSelectedEndIndex = pcCurDocLine->m_cLine.GetStringLength();
		}
		
		nSelectedLen = nSelectedEndIndex - nSelectedIndex;
		
	}else{
		//�e�L�X�g���I������Ă��Ȃ��Ƃ�
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_ptCaretPos.x ,m_ptCaretPos.y , &nSelectedIndex, &nCurrentLine);
		nSelectedLen = 0;
	}
	
	pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(nCurrentLine);
	if (NULL == pcCurDocLine )
		return 0;
	
	const int nLineLen = pcCurDocLine->m_cLine.GetStringLength() - pcCurDocLine->m_cEol.GetLen() ; //���s�R�[�h���̂���������
	if ( 0 == nLineLen )
		return 0;
	
	pLine = pcCurDocLine->m_cLine.GetStringPtr();

	//�ĕϊ��l��������J�n
	nReconvIndex = 0;
	if ( nSelectedIndex > 200 ) {
		const char* pszWork = pLine;
		while( (nSelectedIndex - nReconvIndex) > 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//�ĕϊ��l��������I��
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - nSelectedIndex) > 200 ){
		const char* pszWork = pLine + nSelectedIndex;
		nReconvLen = nSelectedIndex - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - nSelectedIndex) <= 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//�Ώە�����̒���
	if ( nSelectedIndex + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - nSelectedIndex;
	}
	
	pszReconv =  pLine + nReconvIndex;
	
	if(bUnicode){
		
		//�l��������̊J�n����Ώە�����̊J�n�܂�
		if( nSelectedIndex - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconv, nSelectedIndex - nReconvIndex);
			cmemBuf1.SJISToUnicode();
			dwCompStrOffset = cmemBuf1.GetStringLength();  //Offset ��byte
		}else{
			dwCompStrOffset = 0;
		}
		
		//�Ώە�����̊J�n����Ώە�����̏I���܂�
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconv + nSelectedIndex, nSelectedLen);  
			cmemBuf1.SJISToUnicode();
			dwCompStrLen = cmemBuf1.GetStringLength() / sizeof(wchar_t);
		}else{
			dwCompStrLen = 0;
		}
		
		//�l�������񂷂ׂ�
		cmemBuf1.SetString(pszReconv , nReconvLen );
		cmemBuf1.SJISToUnicode();
		
		dwReconvTextLen =  cmemBuf1.GetStringLength() / sizeof(wchar_t);
		nReconvLenWithNull =  cmemBuf1.GetStringLength()  + sizeof(wchar_t);
		
		pszReconv = cmemBuf1.GetStringPtr();
	}else{
		dwReconvTextLen = nReconvLen;
		nReconvLenWithNull = nReconvLen + 1;
		dwCompStrOffset = nSelectedIndex - nReconvIndex;
		dwCompStrLen    = nSelectedLen;
	}
	
	if ( NULL != pReconv) {
		//�ĕϊ��\���̂̐ݒ�
		pReconv->dwSize = sizeof(*pReconv) + nReconvLenWithNull ;
		pReconv->dwVersion = 0;
		pReconv->dwStrLen = dwReconvTextLen ;
		pReconv->dwStrOffset = sizeof(*pReconv) ;
		pReconv->dwCompStrLen = dwCompStrLen;			//�����P��
		pReconv->dwCompStrOffset = dwCompStrOffset;		//�o�C�g�P��
		pReconv->dwTargetStrLen = dwCompStrLen;			//�����P��
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//�o�C�g�P��
		
		// 2004.01.28 Moca �k���I�[�̏C��
		if( bUnicode ){
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - sizeof(wchar_t) );
			*((wchar_t *)(pReconv + 1) + nReconvLenWithNull - sizeof(wchar_t) ) = L'\0';
		}else{
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - 1 );
			*((char *)(pReconv + 1) + nReconvLenWithNull - 1 ) = '\0';
		}
	}
	
	// �ĕϊ����̕ۑ�
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = nCurrentLine;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*�ĕϊ��p �G�f�B�^��̑I��͈͂�ύX���� 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	CMemory		cmemBuf;
	
	// �ĕϊ���񂪕ۑ�����Ă��邩
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( IsTextSelected()) 
		DisableSelectArea( true );

	DWORD		dwOffset, dwLen;
	
	if(bUnicode){
		
		//�l��������̊J�n����Ώە�����̊J�n�܂�
		if( pReconv->dwCompStrOffset > 0){
			cmemBuf.SetString((const char *)((const wchar_t *)(pReconv + 1)), 
								pReconv->dwCompStrOffset ); 
			cmemBuf.UnicodeToSJIS();
			dwOffset = cmemBuf.GetStringLength();
			
		}else{
			dwOffset = 0;
		}

		//�Ώە�����̊J�n����Ώە�����̏I���܂�
		if( pReconv->dwCompStrLen > 0 ){
			cmemBuf.SetString((const char *)(const wchar_t *)(pReconv + 1) + pReconv->dwCompStrOffset , 
								pReconv->dwCompStrLen * sizeof(wchar_t)); 
			cmemBuf.UnicodeToSJIS();
			dwLen = cmemBuf.GetStringLength();
		}else{
			dwLen = 0;
		}
	}else{
		dwOffset = pReconv->dwCompStrOffset;
		dwLen =  pReconv->dwCompStrLen;
	}
	
	//�I���J�n�̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_nLastReconvIndex + dwOffset 
												, m_nLastReconvLine, &m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y);
	//�I���I���̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_nLastReconvIndex + dwOffset + dwLen
												, m_nLastReconvLine, &m_sSelect.m_ptTo.x, &m_sSelect.m_ptTo.y);

	// �P��̐擪�ɃJ�[�\�����ړ�
	MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );

	//�I��͈͍ĕ`�� 
	DrawSelectArea();

	// �ĕϊ����̔j��
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}

/*[EOF]*/
