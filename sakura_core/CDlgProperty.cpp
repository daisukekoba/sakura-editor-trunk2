/*!	@file
	@brief �t�@�C���v���p�e�B�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji, rastiv
	Copyright (C) 2008, Uchi
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
#include "shell.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "global.h"		// Moca, 2002/05/26
#include "charcode.h"	// rastiv, 2006/06/28

// �v���p�e�B CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
//	IDCANCEL,				HIDCANCEL_PROP,			// ���g�p del 2008/7/4 Uchi
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT_PROPERTY,		HIDC_PROP_EDIT1,		// IDC_EDIT1->IDC_EDIT_PROPERTY	2008/7/3 Uchi
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
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
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
	TCHAR			szWork[500];

	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;

	//	Aug. 16, 2000 genta	�S�p��
	cmemProp.AppendString( _T("�t�@�C����  ") );
	cmemProp.AppendString( pCEditDoc->GetFilePath() );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("�ݒ�̃^�C�v  ") );
	cmemProp.AppendString( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("�����R�[�h  ") );
	cmemProp.AppendString( gm_pszCodeNameArr_1[pCEditDoc->m_nCharCode] );
	cmemProp.AppendString( _T("\r\n") );

	wsprintf( szWork, _T("�s��  %d�s\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, _T("���C�A�E�g�s��  %d�s\r\n"), pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendString( _T("�ǂݎ���p���[�h�ŊJ���Ă��܂��B\r\n") );	// 2009.04.11 ryoji �u�㏑���֎~���[�h�v���u�ǂݎ���p���[�h�v
	}
	if( pCEditDoc->IsModified() ){
		cmemProp.AppendString( _T("�ύX����Ă��܂��B\r\n") );
	}else{
		cmemProp.AppendString( _T("�ύX����Ă��܂���B\r\n") );
	}

	wsprintf( szWork, _T("\r\n�R�}���h���s��    %d��\r\n"), pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, _T("--�t�@�C�����-----------------\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->GetFilePath(), (WIN32_FIND_DATA*)&wfd ) ) ){
		if( INVALID_HANDLE_VALUE != pCEditDoc->m_hLockedFile ){
			if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ){
				wsprintf( szWork, _T("���Ȃ��͂��̃t�@�C�����A���v���Z�X����̏㏑���֎~���[�h�Ń��b�N���Ă��܂��B\r\n") );
			}
			else if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
				wsprintf( szWork, _T("���Ȃ��͂��̃t�@�C�����A���v���Z�X����̓ǂݏ����֎~���[�h�Ń��b�N���Ă��܂��B\r\n") );
			}
			else{
				wsprintf( szWork, _T("���Ȃ��͂��̃t�@�C�������b�N���Ă��܂��B\r\n") );
			}
			cmemProp.AppendString( szWork );
		}
		else{
			wsprintf( szWork, _T("���Ȃ��͂��̃t�@�C�������b�N���Ă��܂���B\r\n") );
			cmemProp.AppendString( szWork );
		}

		wsprintf( szWork, _T("�t�@�C������  "), pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendString( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendString( _T("/�A�[�J�C�u") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendString( _T("/���k") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendString( _T("/�t�H���_") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendString( _T("/�B��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendString( _T("/�m�[�}��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendString( _T("/�I�t���C��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendString( _T("/�ǂݎ���p") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendString( _T("/�V�X�e��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendString( _T("/�e���|����") );
		}
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("�쐬����  ") );
		CFileTime ctimeCreation = wfd.ftCreationTime;
		wsprintf( szWork, _T("%d�N%d��%d�� %02d:%02d:%02d"),
			ctimeCreation->wYear,
			ctimeCreation->wMonth,
			ctimeCreation->wDay,
			ctimeCreation->wHour,
			ctimeCreation->wMinute,
			ctimeCreation->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		cmemProp.AppendString( _T("�X�V����  ") );
		CFileTime ctimeLastWrite = wfd.ftLastWriteTime;
		wsprintf( szWork, _T("%d�N%d��%d�� %02d:%02d:%02d"),
			ctimeLastWrite->wYear,
			ctimeLastWrite->wMonth,
			ctimeLastWrite->wDay,
			ctimeLastWrite->wHour,
			ctimeLastWrite->wMinute,
			ctimeLastWrite->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("�A�N�Z�X��  ") );
		CFileTime ctimeLastAccess = wfd.ftLastAccessTime;
		wsprintf( szWork, _T("%d�N%d��%d��"),
			ctimeLastAccess->wYear,
			ctimeLastAccess->wMonth,
			ctimeLastAccess->wDay
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		wsprintf( szWork, _T("MS-DOS�t�@�C����  %s\r\n"), wfd.cAlternateFileName );
		cmemProp.AppendString( szWork );

		wsprintf( szWork, _T("�t�@�C���T�C�Y  %d �o�C�g\r\n"), wfd.nFileSizeLow );
		cmemProp.AppendString( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	
	HFILE					hFile;
	HGLOBAL					hgData;
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
	::SetDlgItemText( m_hWnd, IDC_EDIT_PROPERTY, cmemProp.GetStringPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
