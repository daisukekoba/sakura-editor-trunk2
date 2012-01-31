/*!	@file
	@brief �t�@�C���v���p�e�B�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "sakura_rc.h"
#include "CDlgProperty.h"
#include "Debug.h"
#include "CEditDoc.h"
#include "etc_uty.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "global.h"		// Moca, 2002/05/26
#include "charcode.h"	// rastiv, 2006/06/28

// �v���p�e�B CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
	IDCANCEL,				HIDCANCEL_PROP,
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT1,				HIDC_PROP_EDIT1,
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgProperty::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PROPERTY_FILE, lParam );
}

BOOL CDlgProperty::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�t�@�C���̃v���p�e�B�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/*! �_�C�A���O�f�[�^�̐ݒ�

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
void CDlgProperty::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CMemory			cmemProp;
//	char*			pWork;
//	char			szWork[100];
	char			szWork[500];

//	gm_pszCodeNameArr_1[] ���Q�Ƃ���悤�ɕύX Moca, 2002/05/26
#if 0
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS",
		"EUC",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
#endif
//	1����g���Ă��Ȃ� Moca
//	int				nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );
	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;
	SYSTEMTIME		systimeL;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	//	Aug. 16, 2000 genta	�S�p��
	cmemProp.AppendString( "�t�@�C����  " );
	cmemProp.AppendString( pCEditDoc->GetFilePath() );
	cmemProp.AppendString( "\r\n" );

	cmemProp.AppendString( "�ݒ�̃^�C�v  " );
	cmemProp.AppendString( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendString( "\r\n" );

	cmemProp.AppendString( "�����R�[�h  " );
	cmemProp.AppendString( gm_pszCodeNameArr_1[pCEditDoc->m_nCharCode] );
	cmemProp.AppendString( "\r\n" );

	wsprintf( szWork, "�s��  %d�s\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, "���C�A�E�g�s��  %d�s\r\n", pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendString( "�ǂݎ���p���[�h�ŊJ���Ă��܂��B\r\n" );	// 2009.04.11 ryoji �u�㏑���֎~���[�h�v���u�ǂݎ���p���[�h�v
	}
	if( pCEditDoc->IsModified() ){
		cmemProp.AppendString( "�ύX����Ă��܂��B\r\n" );
	}else{
		cmemProp.AppendString( "�ύX����Ă��܂���B\r\n" );
	}

	wsprintf( szWork, "\r\n�R�}���h���s��    %d��\r\n", pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, "--�t�@�C�����-----------------\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->GetFilePath(), (WIN32_FIND_DATA*)&wfd ) ) ){
		if( pCEditDoc->m_hLockedFile ){
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ){
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�����A���v���Z�X����̏㏑���֎~���[�h�Ń��b�N���Ă��܂��B\r\n" );
			}else
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�����A���v���Z�X����̓ǂݏ����֎~���[�h�Ń��b�N���Ă��܂��B\r\n" );
			}else{
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�������b�N���Ă��܂��B\r\n" );
			}
			cmemProp.AppendString( szWork );
		}else{
			wsprintf( szWork, "���Ȃ��͂��̃t�@�C�������b�N���Ă��܂���B\r\n" );
			cmemProp.AppendString( szWork );
		}

		wsprintf( szWork, "�t�@�C������  ", pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendString( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendString( "/�A�[�J�C�u" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendString( "/���k" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendString( "/�t�H���_" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendString( "/�B��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendString( "/�m�[�}��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendString( "/�I�t���C��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendString( "/�ǂݎ���p" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendString( "/�V�X�e��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendString( "/�e���|����" );
		}
		cmemProp.AppendString( "\r\n" );


		cmemProp.AppendString( "�쐬����  " );
		::FileTimeToLocalFileTime( &wfd.ftCreationTime, &wfd.ftCreationTime );
		::FileTimeToSystemTime( &wfd.ftCreationTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d�� %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( "\r\n" );

		cmemProp.AppendString( "�X�V����  " );
		::FileTimeToLocalFileTime( &wfd.ftLastWriteTime, &wfd.ftLastWriteTime );
		::FileTimeToSystemTime( &wfd.ftLastWriteTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d�� %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( "\r\n" );


		cmemProp.AppendString( "�A�N�Z�X��  " );
		::FileTimeToLocalFileTime( &wfd.ftLastAccessTime, &wfd.ftLastAccessTime );
		::FileTimeToSystemTime( &wfd.ftLastAccessTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d��",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( "\r\n" );

		wsprintf( szWork, "MS-DOS�t�@�C����  %s\r\n", wfd.cAlternateFileName );
		cmemProp.AppendString( szWork );

		wsprintf( szWork, "�t�@�C���T�C�Y  %d �o�C�g\r\n", wfd.nFileSizeLow );
		cmemProp.AppendString( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	
	HFILE					hFile;
	HGLOBAL					hgData;
//	const unsigned char*	pBuf;
	const char*				pBuf;
	int						nBufLen;
	/* �������m�� & �t�@�C���ǂݍ��� */
	hgData = NULL;
	hFile = _lopen( pCEditDoc->GetFilePath(), OF_READ );
	if( HFILE_ERROR == hFile ){
		goto end_of_CodeTest;
	}
	nBufLen = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		_lclose( hFile );
		goto end_of_CodeTest;
	}
//	pBuf = (const unsigned char*)::GlobalLock( hgData );
	pBuf = (const char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );

// From Here  2006.12.17  rastiv
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: Unicode���H
	*/
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UnicodeBE���H
	*/
	Charcode::GetEncdInf_Uni( pBuf, nBufLen, &uci_tmp );
	wsprintf( szWork, "Unicode�R�[�h�����F���s�o�C�g��=%d  BE���s�o�C�g��=%d ASCII���s�o�C�g��=%d\r\n"
		, uci_tmp.Uni.nCRorLF, uci_tmp.UniBe.nCRorLF, uci_tmp.nCRorLF_ascii );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: EUC���H
	*/
	Charcode::GetEncdInf_EucJp( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "EUCJP�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: SJIS���H
	*/
	Charcode::GetEncdInf_SJis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "SJIS�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: JIS���H
	*/
	Charcode::GetEncdInf_Jis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "JIS�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-8S���H
	*/
	Charcode::GetEncdInf_Utf8( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-8�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-7S���H
	*/
	Charcode::GetEncdInf_Utf7( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-7�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
// To Here rastiv 2006.12.17

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::SetDlgItemText( m_hWnd, IDC_EDIT1, cmemProp.GetStringPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
