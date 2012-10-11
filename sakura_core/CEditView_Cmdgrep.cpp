/*!	@file
	@brief CEditView�N���X��grep�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CEditApp.h"
#include "charcode.h"
#include "Debug.h"
#include "sakura_rc.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(grep mode��)
*/
void CEditView::TranslateCommand_grep(
	int&	nCommand,
	BOOL&	bRedraw,
	LPARAM&	lparam1,
	LPARAM&	lparam2,
	LPARAM&	lparam3,
	LPARAM&	lparam4
)
{
	if( ! m_pcEditDoc->m_bGrepMode )
		return;

	if( nCommand == F_CHAR ){
		//	Jan. 23, 2005 genta ��������Y��
		if(( lparam1 == CR || lparam1 == LF ) && m_pShareData->m_Common.m_bGTJW_RETURN ){
			nCommand = F_TAGJUMP;
			lparam1 = GetKeyState_Control();
		}
	}
}

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void CEditView::Command_GREP_DIALOG( void )
{
	CMemory		cmemCurText;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	/* �L�[���Ȃ��Ȃ�A��������Ƃ��Ă��� */
	if( 0 == cmemCurText.GetStringLength() ){
//		cmemCurText.SetData( m_pShareData->m_szSEARCHKEYArr[0], lstrlen( m_pShareData->m_szSEARCHKEYArr[0] ) );
		cmemCurText.SetString( m_pShareData->m_szSEARCHKEYArr[0] );
	}
	_tcscpy( m_pcEditDoc->m_cDlgGrep.m_szText, cmemCurText.GetStringPtr() );

	/* Grep�_�C�A���O�̕\�� */
	int nRet = m_pcEditDoc->m_cDlgGrep.DoModal( m_hInstance, m_hWnd, m_pcEditDoc->GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void CEditView::Command_GREP( void )
{
	CMemory		cmWork1;
	CMemory		cmWork2;
	CMemory		cmWork3;
	CMemory		cmemCurText;

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		::MYMESSAGEBOX( m_hWnd, MB_OK, GSTR_APPNAME, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );

		return;
	}
	cmWork1.SetString( m_pcEditDoc->m_cDlgGrep.m_szText );
	cmWork2.SetString( m_pcEditDoc->m_cDlgGrep.m_szFile );
	cmWork3.SetString( m_pcEditDoc->m_cDlgGrep.m_szFolder );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��B�܂��́A�ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ�B
		Grep���ƃA�E�g�v�b�g�E�B���h�E�����O����
	*/
	if( (  m_pcEditDoc->m_bGrepMode &&
		   !m_pcEditDoc->m_bGrepRunning ) ||
	    ( !m_pcEditDoc->m_bGrepMode &&
		   !m_pcEditDoc->IsModified() &&
		   !m_pcEditDoc->IsFilePathAvailable() &&		/* ���ݕҏW���̃t�@�C���̃p�X */
		   !m_pcEditDoc->m_bDebugMode
		)
	){
		DoGrep(
			&cmWork1,
			&cmWork2,
			&cmWork3,
			m_pcEditDoc->m_cDlgGrep.m_bSubFolder,
			m_pcEditDoc->m_cDlgGrep.m_sSearchOption,
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet,
			m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine,
			m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle
		);
	}
	else{
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
		char*	pCmdLine = new char[1024];
		char*	pOpt = new char[64];
		cmWork1.Replace( "\"", "\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );

		// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		wsprintf( pCmdLine,
			_T("-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d"),
			cmWork1.GetStringPtr(),
			cmWork2.GetStringPtr(),
			cmWork3.GetStringPtr(),
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet
		);

		//GOPT�I�v�V����
		pOpt[0] = _T('\0');
		if( m_pcEditDoc->m_cDlgGrep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
		if( m_pcEditDoc->m_cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// ���K�\��
		if( m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// �s���o�͂��邩�Y�����������o�͂��邩
		if( 1 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
		if( 2 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
		if( 0 < _tcslen( pOpt ) ){
			_tcscat( pCmdLine, _T(" -GOPT=") );
			_tcscat( pCmdLine, pOpt );
		}

//		MYTRACE_A( "pCmdLine=[%s]\n", pCmdLine );
		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
		CEditApp::OpenNewEditor( m_hInstance, m_hWnd, pCmdLine, 0, FALSE );
		delete [] pCmdLine;
		delete [] pOpt;
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
	}
	return;
}
/*[EOF]*/
