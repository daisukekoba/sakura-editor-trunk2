/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�̃t�@�C���ց^����̓��o��

	@author Norio Nakatani

	@date 2005.04.07 D.S.Koba �e�Z�N�V�����̓��o�͂��֐��Ƃ��ĕ���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, Stonee, jepro, MIK, asa-o, YAZAKI, hor
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, genta, Moca
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, MIK, Moca, D.S.Koba, genta
	Copyright (C) 2005, MIK, genta, D.S.Koba, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon, maru
	Copyright (C) 2007, ryoji, genta, maru, Moca, nasukoji
	Copyright (C) 2008, nasukoji, novice
	Copyright (C) 2009, nasukoji, ryoji, salarm

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <imagehlp.h>
#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "mymessage.h"
#include "Debug.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CProfile.h"
#include "etc_uty.h"
#include "COsVersionInfo.h"
#include "CCommandLine.h"

#ifndef CSIDL_PROFILE
#define CSIDL_PROFILE                   0x0028        // USERPROFILE
#endif

#define STR_COLORDATA_HEAD3		" �e�L�X�g�G�f�B�^�F�ݒ� Ver3"	//Jan. 15, 2001 Stonee  �F�ݒ�Ver3�h���t�g(�ݒ�t�@�C���̃L�[��A�ԁ��������)	//Feb. 11, 2001 JEPRO �L���ɂ���

void ShareData_IO_Sub_LogFont( CProfile& cProfile, const char* pszSecName,
	const char* pszKeyLf, const char* pszKeyPointSize, const char* pszKeyFaceName, LOGFONT& lf, int& pointSize );

template <typename T>
void SetValueLimit(T& target, int minval, int maxval)
{
	target = t_max<T>(minval, t_min<T>(maxval, target));
}

template <typename T>
void SetValueLimit(T& target, int maxval)
{
	SetValueLimit( target, 0, maxval );
}

/* static */ TCHAR CShareData::GetAccessKeyByIndex(int index, bool bZeroOrigin)
{
	if( index < 0 ) return 0;
	int accKeyIndex = ((bZeroOrigin? index: index+1) % 36);
	TCHAR c = (TCHAR)((accKeyIndex < 10) ? (_T('0') + accKeyIndex) : (_T('A') + accKeyIndex - 10));
	return c;
}

static void GetAccessKeyLabelByIndex(TCHAR* pszLabel, bool bEspaceAmp, int index, bool bZeroOrigin)
{
	TCHAR c = CShareData::GetAccessKeyByIndex(index, bZeroOrigin);
	if( c ){
		if( bEspaceAmp ){
			pszLabel[0] = _T('&');
			pszLabel[1] = c;
			pszLabel[2] = _T(' ');
			pszLabel[3] = _T('\0');
		}else{
			pszLabel[0] = c;
			pszLabel[1] = _T(' ');
			pszLabel[2] = _T('\0');
		}
	}else{
		pszLabel[0] = _T('\0');
	}
}

/*
	@param editInfo      �E�B���h�E���BNUL�ŕs������
	@param index         ����0origin�Ŏw��B -1�Ŕ�\��
	@param bZeroOrigin   �A�N�Z�X�L�[��0����U��
*/
bool CShareData::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const EditInfo* editInfo, int nId, bool bFavorite,
	int index, bool bAccKeyZeroOrigin
){
	const EditInfo* pfi = editInfo;
	TCHAR szAccKey[4];
	int ret = 0;
	if( NULL == pfi ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = _stprintf( pszOutput, _T("%s �s��(�����Ȃ�)"), szAccKey );
		return true; // true�ɂ��Ă���
	}else if( pfi->m_bIsGrep ){
		
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		//pfi->m_szGrepKeyShort �� cmemDes
		CMemory	cmemDes;
		int nGrepKeyLen = _tcslen(pfi->m_szGrepKey);
		const int GREPKEY_LIMIT_LEN = 64;
		// CEditDoc::ExpandParameter �ł� 32��������
		// ���j���[�� 64��������
		LimitStringLengthB( pfi->m_szGrepKey, nGrepKeyLen, GREPKEY_LIMIT_LEN, cmemDes );
		
		const TCHAR* pszKey;
		TCHAR szMenu2[GREPKEY_LIMIT_LEN*2*2+1]; // WCHAR=>ACHAR��2�{�A&��2�{
		if( bEspaceAmp ){
			dupamp( cmemDes.GetStringPtr(), szMenu2 );
			pszKey = szMenu2;
		}else{
			pszKey = cmemDes.GetStringPtr();
		}

		//szMenu�����
		//	Jan. 19, 2002 genta
		//	&�̏d��������ǉ��������ߌp��������኱�ύX
		//	20100729 ExpandParameter�ɂ��킹�āA�E�E�E��...�ɕύX
		ret = _stprintf( pszOutput, _T("%s �yGrep�z\"%s%s\""),
			szAccKey, pszKey,
			( nGrepKeyLen > cmemDes.GetStringLength() ) ? _T("..."):_T("")
		);
	}else if( pfi->m_bIsDebug ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = _stprintf( pszOutput, _T("%s �A�E�g�v�b�g"), szAccKey );
	}else{
		return GetMenuFullLabel(pszOutput, nBuffSize, bEspaceAmp, pfi->m_szPath, nId, pfi->m_bIsModified, pfi->m_nCharCode, bFavorite,
			 index, bAccKeyZeroOrigin);
	}
	return 0 < ret;
}

bool CShareData::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const TCHAR* pszFile, int nId, bool bModified, ECodeType nCharCode, bool bFavorite,
	int index, bool bAccKeyZeroOrigin
){
	TCHAR szAccKey[4];
	TCHAR szFileName[_MAX_PATH];
	TCHAR szMenu2[_MAX_PATH * 2];
	const TCHAR* pszName;

	GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
	if( pszFile[0] ){
		this->GetTransformFileNameFast( pszFile, szFileName, _MAX_PATH );

		// szFileName �� szMenu2
		//	Jan. 19, 2002 genta
		//	���j���[�������&���l��
		if( bEspaceAmp ){
			dupamp( szFileName, szMenu2 );
			pszName = szMenu2;
		}else{
			pszName = szFileName;
		}
	}else{
		wsprintf( szFileName, _T("(����)%d"), nId);
		pszName = szFileName;
	}
	const TCHAR* pszCharset = _T("");
	if( IsValidCodeTypeExceptSJIS(nCharCode)){
		pszCharset = gm_pszCodeNameArr_3[nCharCode];
	}
	
	int ret = _stprintf( pszOutput, _T("%s%s%s %s%s"),
		szAccKey, (bFavorite ? _T("�� ") : _T("")), pszName,
		(bModified ? _T("*"):_T(" ")), pszCharset
	);
	return 0 < ret;
}


/**
	�\���ݒ�t�@�C������ini�t�@�C�������擾����

	sakura.exe.ini����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�

	@param[out] pszPrivateIniFile �}���`���[�U�p��ini�t�@�C���p�X
	@param[out] pszIniFile EXE���ini�t�@�C���p�X

	@author ryoji
	@date 2007.09.04 ryoji �V�K�쐬
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void CShareData::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		NULL,
		szPath, _countof(szPath)
	);
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );

	_snprintf( pszIniFile, _MAX_PATH - 1, _T("%s%s%s%s"), szDrive, szDir, szFname, _T(".ini") );
	pszIniFile[_MAX_PATH - 1] = _T('\0');

	// �}���`���[�U�p��ini�t�@�C���p�X
	//		exe�Ɠ����t�H���_�ɒu���ꂽ�}���`���[�U�\���ݒ�t�@�C���isakura.exe.ini�j�̓��e
	//		�ɏ]���ă}���`���[�U�p��ini�t�@�C���p�X�����߂�
	pszPrivateIniFile[0] = _T('\0');
	if( IsWin2000_or_later() ){
		_snprintf( szPath, _MAX_PATH - 1, _T("%s%s%s%s"), szDrive, szDir, szFname, _T(".exe.ini") );
		szPath[_MAX_PATH - 1] = _T('\0');
		int nEnable = ::GetPrivateProfileInt(_T("Settings"), _T("MultiUser"), 0, szPath );
		if( nEnable ){
			int nFolder = ::GetPrivateProfileInt(_T("Settings"), _T("UserRootFolder"), 0, szPath );
			switch( nFolder ){
			case 1:
				nFolder = CSIDL_PROFILE;			// ���[�U�̃��[�g�t�H���_
				break;
			case 2:
				nFolder = CSIDL_PERSONAL;			// ���[�U�̃h�L�������g�t�H���_
				break;
			case 3:
				nFolder = CSIDL_DESKTOPDIRECTORY;	// ���[�U�̃f�X�N�g�b�v�t�H���_
				break;
			default:
				nFolder = CSIDL_APPDATA;			// ���[�U�̃A�v���P�[�V�����f�[�^�t�H���_
				break;
			}
			::GetPrivateProfileString(_T("Settings"), _T("UserSubFolder"), _T("sakura"), szDir, _MAX_DIR, szPath );
			if( szDir[0] == _T('\0') )
				::lstrcpy( szDir, _T("sakura") );
			if( GetSpecialFolderPath( nFolder, szPath ) ){
				_snprintf( pszPrivateIniFile, _MAX_PATH - 1, _T("%s\\%s\\%s%s"), szPath, szDir, szFname, _T(".ini") );
				pszPrivateIniFile[_MAX_PATH - 1] = _T('\0');
			}
		}
	}
	return;
}

/**
	ini�t�@�C�����̎擾

	���L�f�[�^����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�
	�i���L�f�[�^���ݒ�̂Ƃ��͋��L�f�[�^�ݒ���s���j

	@param[out] pszIniFileName ini�t�@�C�����i�t���p�X�j
	@param[in] bRead true: �ǂݍ��� / false: ��������

	@author ryoji
	@date 2007.05.19 ryoji �V�K�쐬
*/
void CShareData::GetIniFileName( LPTSTR pszIniFileName, BOOL bRead/*=FALSE*/ )
{
	if( !m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit ){
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = true;			// �������σt���O
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = false;	// �}���`���[�U�pini����̓ǂݏo���t���O
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;	// �}���`���[�U�pini�ւ̏������݃t���O

		GetIniFileNameDirect( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
		if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile[0] != _T('\0') ){
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = true;
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = true;
			if( CCommandLine::getInstance()->IsNoWindow() && CCommandLine::getInstance()->IsWriteQuit() )
				m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;

			// �}���`���[�U�p��ini�t�H���_���쐬���Ă���
			if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate ){
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, szDrive, szDir, NULL, NULL );
				_snprintf( szPath, _MAX_PATH - 1, _T("%s\\%s"), szDrive, szDir );
				szPath[_MAX_PATH - 1] = _T('\0');
				::MakeSureDirectoryPathExists( szPath );
			}
		}
	}

	bool bPrivate = bRead? m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate: m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile: m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
}

/*!
	���L�f�[�^�̓ǂݍ���/�ۑ� 2

	@param[in] bRead true: �ǂݍ��� / false: ��������

	@date 2004-01-11 D.S.Koba CProfile�ύX�ɂ��R�[�h�ȗ���
	@date 2005-04-05 D.S.Koba �e�Z�N�V�����̓��o�͂��֐��Ƃ��ĕ���
*/
bool CShareData::ShareData_IO_2( bool bRead )
{
	//MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	CProfile	cProfile;

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

	TCHAR		szIniFileName[_MAX_PATH + 1];
	GetIniFileName( szIniFileName, bRead );	// 2007.05.19 ryoji ini�t�@�C�������擾����

//	MYTRACE_A( "Ini�t�@�C������-1 ���v����(�~���b) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( !cProfile.ReadProfile( szIniFileName ) ){
			/* �ݒ�t�@�C�������݂��Ȃ� */
			return false;
		}
	}
//	MYTRACE_A( "Ini�t�@�C������ 0 ���v����(�~���b) = %d\n", cRunningTimer.Read() );
	
	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru( cProfile );
	ShareData_IO_Keys( cProfile );
	ShareData_IO_Grep( cProfile );
	ShareData_IO_Folders( cProfile );
	ShareData_IO_Cmd( cProfile );
	ShareData_IO_Nickname( cProfile );
	ShareData_IO_Common( cProfile );
	ShareData_IO_Toolbar( cProfile );
	ShareData_IO_CustMenu( cProfile );
	ShareData_IO_Font( cProfile );
	ShareData_IO_KeyBind( cProfile );
	ShareData_IO_Print( cProfile );
	ShareData_IO_Types( cProfile );
	ShareData_IO_KeyWords( cProfile );
	ShareData_IO_Macro( cProfile );
	ShareData_IO_Other( cProfile );
	
	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, _T(" sakura.ini �e�L�X�g�G�f�B�^�ݒ�t�@�C��") );
	}

//	MYTRACE_A( "Ini�t�@�C������ 8 ���v����(�~���b) = %d\n", cRunningTimer.Read() );
//	MYTRACE_A( "Ini�t�@�C������ ���v����(�~���b) = %d\n", cRunningTimerStart.Read() );

	return true;
}

/*!
	@brief ���L�f�[�^��Mru�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Mru( CProfile& cProfile )
{
	const char* pszSecName = "MRU";
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	char		szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_MRU_Counts", m_pShareData->m_sHistory.m_nMRUArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nMRUArrNum, MAX_MRU );
	nSize = m_pShareData->m_sHistory.m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_sHistory.m_fiMRUArr[i];
		if( cProfile.IsReadingMode() ){
			pfiWork->m_nType = -1;
		}
		wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		wsprintf( szKeyName, "MRU[%02d].nX", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nX );
		wsprintf( szKeyName, "MRU[%02d].nY", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nY );
		wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
		cProfile.IOProfileData( pszSecName, szKeyName, (int&)pfiWork->m_nCharCode );
		wsprintf( szKeyName, "MRU[%02d].szPath", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
		wsprintf( szKeyName, "MRU[%02d].szMark", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		wsprintf( szKeyName, "MRU[%02d].nType", i );
		int nType = pfiWork->m_nType;
		cProfile.IOProfileData( pszSecName, szKeyName, nType );
		pfiWork->m_nType = nType;
		//���C�ɓ���	//@@@ 2003.04.08 MIK
		wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI �c���m_fiMRUArr���������B
	if ( cProfile.IsReadingMode() ){
		EditInfo	fiInit;
		//	�c���fiInit�ŏ��������Ă����B
		fiInit.m_nCharCode = CODE_DEFAULT;
		fiInit.m_nViewLeftCol = 0;
		fiInit.m_nViewTopLine = 0;
		fiInit.m_nX = 0;
		fiInit.m_nY = 0;
		_tcscpy( fiInit.m_szPath, _T("") );
		strcpy( fiInit.m_szMarkLines, "" );	// 2002.01.16 hor
		for( ; i < MAX_MRU; ++i){
			m_pShareData->m_sHistory.m_fiMRUArr[i] = fiInit;
			m_pShareData->m_sHistory.m_bMRUArrFavorite[i] = false;	//���C�ɓ���	//@@@ 2003.04.08 MIK
		}
	}

	cProfile.IOProfileData( pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_sHistory.m_nOPENFOLDERArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nOPENFOLDERArrNum, MAX_OPENFOLDER );
	nSize = m_pShareData->m_sHistory.m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_szOPENFOLDERArr[i],
			sizeof( m_pShareData->m_sHistory.m_szOPENFOLDERArr[0] ));
		//���C�ɓ���	//@@@ 2003.04.08 MIK
		strcat( szKeyName, ".bFavorite" );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_bOPENFOLDERArrFavorite[i] );
	}
	//�ǂݍ��ݎ��͎c���������
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_sHistory.m_szOPENFOLDERArr[i][0] = '\0';
			m_pShareData->m_sHistory.m_bOPENFOLDERArrFavorite[i] = false;	//���C�ɓ���	//@@@ 2003.04.08 MIK
		}
	}
}

/*!
	@brief ���L�f�[�^��Keys�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Keys( CProfile& cProfile )
{
	const char* pszSecName = "Keys";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum, MAX_SEARCHKEY );
	nSize = m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] ));
	}

	cProfile.IOProfileData( pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum, MAX_REPLACEKEY );
	nSize = m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0] ) );
	}
}

/*!
	@brief ���L�f�[�^��Grep�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Grep( CProfile& cProfile )
{
	const char* pszSecName = "Grep";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_GREPFILE_Counts", m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum, MAX_GREPFILE );
	nSize = m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[0] ));
	}

	cProfile.IOProfileData( pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum, MAX_GREPFOLDER );
	nSize = m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[0] ));
	}
}

/*!
	@brief ���L�f�[�^��Folders�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Folders( CProfile& cProfile )
{
	const char* pszSecName = "Folders";
	/* �}�N���p�t�H���_ */
	cProfile.IOProfileData( pszSecName, "szMACROFOLDER",
		m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, sizeof( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ));
	/* �ݒ�C���|�[�g�p�t�H���_ */
	cProfile.IOProfileData( pszSecName, "szIMPORTFOLDER",
		m_pShareData->m_sHistory.m_szIMPORTFOLDER, sizeof( m_pShareData->m_sHistory.m_szIMPORTFOLDER ));
}

/*!
	@brief ���L�f�[�^��Cmd�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Cmd( CProfile& cProfile )
{
	const char* pszSecName = "Cmd";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "nCmdArrNum", m_pShareData->m_sHistory.m_nCmdArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nCmdArrNum, MAX_CMDARR );
	int nSize = m_pShareData->m_sHistory.m_nCmdArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sHistory.m_szCmdArr[i], sizeof( m_pShareData->m_sHistory.m_szCmdArr[0] ));
	}
}

/*!
	@brief ���L�f�[�^��Nickname�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Nickname( CProfile& cProfile )
{
	const char* pszSecName = "Nickname";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "ArrNum", m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum );
	SetValueLimit( m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum, MAX_TRANSFORM_FILENAME );
	int nSize = m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "From%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[0] ));
		wsprintf( szKeyName, "To%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i], sizeof( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[0] ));
	}
	// �ǂݍ��ݎ��C�c���NULL�ōď�����
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] = '\0';
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i][0]   = '\0';
		}
	}
}

/*!
	@brief ���L�f�[�^��Common�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Common( CProfile& cProfile )
{
	const char* pszSecName = "Common";
	// 2005.04.07 D.S.Koba
	CommonSetting& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nCaretType"				, common.m_sGeneral.m_nCaretType );
	//	Oct. 2, 2005 genta
	//	�����l��}�����[�h�ɌŒ肷�邽�߁C�ݒ�̓ǂݏ�������߂�
	//cProfile.IOProfileData( pszSecName, "bIsINSMode"			, common.m_sGeneral.m_bIsINSMode );
	cProfile.IOProfileData( pszSecName, "bIsFreeCursorMode"		, common.m_sGeneral.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchWord"	, common.m_sGeneral.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchParagraph"	, common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( pszSecName, "m_bRestoreCurPosition"	, common.m_sFile.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( pszSecName, "m_bRestoreBookmarks"	, common.m_sFile.m_bRestoreBookmarks );
	cProfile.IOProfileData( pszSecName, "bAddCRLFWhenCopy"		, common.m_sEdit.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( pszSecName, "eOpenDialogDir"		, (int&)common.m_sEdit.m_eOpenDialogDir );
	cProfile.IOProfileData( pszSecName, "szOpenDialogSelDir"		, common.m_sEdit.m_OpenDialogSelDir, sizeof(common.m_sEdit.m_OpenDialogSelDir) );
	cProfile.IOProfileData( pszSecName, "nRepeatedScrollLineNum"	, common.m_sGeneral.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, "nRepeatedScroll_Smooth"	, common.m_sGeneral.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, "nPageScrollByWheel"	, common.m_sGeneral.m_nPageScrollByWheel );	// 2009.01.12 nasukoji
	cProfile.IOProfileData( pszSecName, "nHorizontalScrollByWheel"	, common.m_sGeneral.m_nHorizontalScrollByWheel );	// 2009.01.12 nasukoji
	cProfile.IOProfileData( pszSecName, "bCloseAllConfirm"		, common.m_sGeneral.m_bCloseAllConfirm );	/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
	cProfile.IOProfileData( pszSecName, "bExitConfirm"			, common.m_sGeneral.m_bExitConfirm );
	cProfile.IOProfileData( pszSecName, "bSearchRegularExp"		, common.m_sSearch.m_sSearchOption.bRegularExp );
	cProfile.IOProfileData( pszSecName, "bSearchLoHiCase"		, common.m_sSearch.m_sSearchOption.bLoHiCase );
	cProfile.IOProfileData( pszSecName, "bSearchWordOnly"		, common.m_sSearch.m_sSearchOption.bWordOnly );
	cProfile.IOProfileData( pszSecName, "bSearchConsecutiveAll"	, common.m_sSearch.m_bConsecutiveAll );	// 2007.01.16 ryoji
	cProfile.IOProfileData( pszSecName, "bSearchNOTIFYNOTFOUND"	, common.m_sSearch.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( pszSecName, "bSearchAll"			, common.m_sSearch.m_bSearchAll );
	cProfile.IOProfileData( pszSecName, "bSearchSelectedArea"	, common.m_sSearch.m_bSelectedArea );
	cProfile.IOProfileData( pszSecName, "bGrepSubFolder"		, common.m_sSearch.m_bGrepSubFolder );
	cProfile.IOProfileData( pszSecName, "bGrepOutputLine"		, common.m_sSearch.m_bGrepOutputLine );
	cProfile.IOProfileData( pszSecName, "nGrepOutputStyle"		, common.m_sSearch.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, "bGrepDefaultFolder"	, common.m_sSearch.m_bGrepDefaultFolder );
	// 2002/09/21 Moca �ǉ�
	cProfile.IOProfileData( pszSecName, "nGrepCharSet"			, (int&)common.m_sSearch.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, "bGrepRealTime"			, common.m_sSearch.m_bGrepRealTimeView ); // 2003.06.16 Moca
	cProfile.IOProfileData( pszSecName, "bCaretTextForSearch"	, common.m_sSearch.m_bCaretTextForSearch );	// 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	
	/* ���K�\��DLL 2007.08.12 genta */
	cProfile.IOProfileData( pszSecName, "szRegexpLib"			, common.m_sSearch.m_szRegexpLib, sizeof( common.m_sSearch.m_szRegexpLib ) );
	
	cProfile.IOProfileData( pszSecName, "bGTJW_RETURN"			, common.m_sSearch.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, "bGTJW_LDBLCLK"			, common.m_sSearch.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, "bBackUp"				, common.m_sBackup.m_bBackUp );
	cProfile.IOProfileData( pszSecName, "bBackUpDialog"			, common.m_sBackup.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, "bBackUpFolder"			, common.m_sBackup.m_bBackUpFolder );
	
	if( !cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	cProfile.IOProfileData( pszSecName, "szBackUpFolder",
		common.m_sBackup.m_szBackUpFolder, sizeof( common.m_sBackup.m_szBackUpFolder ));
	if( cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	
	
	cProfile.IOProfileData( pszSecName, "nBackUpType"			, common.m_sBackup.m_nBackUpType );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt1"		, common.m_sBackup.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt2"		, common.m_sBackup.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt3"		, common.m_sBackup.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt4"		, common.m_sBackup.m_nBackUpType_Opt4 );
	cProfile.IOProfileData( pszSecName, "bBackUpDustBox"		, common.m_sBackup.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( pszSecName, "bBackUpPathAdvanced"	, common.m_sBackup.m_bBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "szBackUpPathAdvanced"	,
		common.m_sBackup.m_szBackUpPathAdvanced, sizeof( common.m_sBackup.m_szBackUpPathAdvanced ));	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "nFileShareMode"		, common.m_sFile.m_nFileShareMode );
	cProfile.IOProfileData( pszSecName, "szExtHelp",
		common.m_sHelper.m_szExtHelp, sizeof( common.m_sHelper.m_szExtHelp ));
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp",
		common.m_sHelper.m_szExtHtmlHelp, sizeof( common.m_sHelper.m_szExtHtmlHelp ));
	
	cProfile.IOProfileData( pszSecName, "szMigemoDll",
		common.m_sHelper.m_szMigemoDll, sizeof( common.m_sHelper.m_szMigemoDll ));
	cProfile.IOProfileData( pszSecName, "szMigemoDict",
		common.m_sHelper.m_szMigemoDict, sizeof( common.m_sHelper.m_szMigemoDict ));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		ShareData_IO_Sub_LogFont( cProfile, pszSecName, "khlf", "khps", "khlfFaceName",
			common.m_sHelper.m_lf_kh, common.m_sHelper.m_ps_kh );
	}// Keword Help Font
	

	cProfile.IOProfileData( pszSecName, "nMRUArrNum_MAX"		, common.m_sGeneral.m_nMRUArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nMRUArrNum_MAX, MAX_MRU );
	cProfile.IOProfileData( pszSecName, "nOPENFOLDERArrNum_MAX"	, common.m_sGeneral.m_nOPENFOLDERArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nOPENFOLDERArrNum_MAX, MAX_OPENFOLDER );
	cProfile.IOProfileData( pszSecName, "bDispTOOLBAR"			, common.m_sWindow.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, "bDispSTATUSBAR"		, common.m_sWindow.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, "bDispFUNCKEYWND"		, common.m_sWindow.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_Place"		, common.m_sWindow.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_GroupNum"	, common.m_sWindow.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	
	cProfile.IOProfileData( pszSecName, "bDispTabWnd"			, common.m_sTabBar.m_bDispTabWnd );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "bDispTabWndMultiWin"	, common.m_sTabBar.m_bDispTabWndMultiWin );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "szTabWndCaption"		, common.m_sTabBar.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, "bSameTabWidth"			, common.m_sTabBar.m_bSameTabWidth );	// 2006.01.28 ryoji �^�u�𓙕��ɂ���
	cProfile.IOProfileData( pszSecName, "bDispTabIcon"			, common.m_sTabBar.m_bDispTabIcon );	// 2006.01.28 ryoji �^�u�ɃA�C�R����\������
	cProfile.IOProfileData( pszSecName, "bSortTabList"			, common.m_sTabBar.m_bSortTabList );	// 2006.05.10 ryoji �^�u�ꗗ���\�[�g����
	cProfile.IOProfileData( pszSecName, "bTab_RetainEmptyWin"	, common.m_sTabBar.m_bTab_RetainEmptyWin );	// �Ō�̃t�@�C��������ꂽ�Ƃ�(����)���c��	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, "bTab_CloseOneWin"		, common.m_sTabBar.m_bTab_CloseOneWin );	// �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, "bTab_ListFull"			, common.m_sTabBar.m_bTab_ListFull );	// �^�u�ꗗ���t���p�X�\������	// 2007.02.28 ryoji
	cProfile.IOProfileData( pszSecName, "bChgWndByWheel"		, common.m_sTabBar.m_bChgWndByWheel );	// 2006.03.26 ryoji �}�E�X�z�C�[���ŃE�B���h�E�؂�ւ�
	cProfile.IOProfileData( pszSecName, "bNewWindow"			, common.m_sTabBar.m_bNewWindow );	// �O������N������Ƃ��͐V�����E�C���h�E�ŊJ��

	ShareData_IO_Sub_LogFont( cProfile, pszSecName, "lfTabFont", "lfTabFontPs", "lfTabFaceName",
		common.m_sTabBar.m_tabFont, common.m_sTabBar.m_tabFontPs );
	
	// 2001/06/20 asa-o �����E�B���h�E�̃X�N���[���̓������Ƃ�
	cProfile.IOProfileData( pszSecName, "bSplitterWndHScroll"	, common.m_sWindow.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, "bSplitterWndVScroll"	, common.m_sWindow.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, "szMidashiKigou"		, common.m_sFormat.m_szMidashiKigou, 0 );
	cProfile.IOProfileData( pszSecName, "szInyouKigou"			, common.m_sFormat.m_szInyouKigou, 0 );
	
	// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜�F�R�s
	cProfile.IOProfileData( pszSecName, "bUseHokan"				, common.m_sHelper.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect �� bGrepCharSet�ɓ��������̂ō폜
	// 2001/06/19 asa-o �^�C�v�ʂɈړ������̂ō폜�F1�s
	cProfile.IOProfileData( pszSecName, "bSaveWindowSize"		, (int&)common.m_sWindow.m_eSaveWindowSize );
	cProfile.IOProfileData( pszSecName, "nWinSizeType"			, common.m_sWindow.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, "nWinSizeCX"			, common.m_sWindow.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, "nWinSizeCY"			, common.m_sWindow.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*��ǉ�
	cProfile.IOProfileData( pszSecName, "nSaveWindowPos"		, (int&)common.m_sWindow.m_eSaveWindowPos );
	cProfile.IOProfileData( pszSecName, "nWinPosX"				, common.m_sWindow.m_nWinPosX );
	cProfile.IOProfileData( pszSecName, "nWinPosY"				, common.m_sWindow.m_nWinPosY );
	cProfile.IOProfileData( pszSecName, "bTaskTrayUse"			, common.m_sGeneral.m_bUseTaskTray );
	cProfile.IOProfileData( pszSecName, "bTaskTrayStay"			, common.m_sGeneral.m_bStayTaskTray );

	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyCode"		, common.m_sGeneral.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyMods"		, common.m_sGeneral.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DragDrop"			, common.m_sEdit.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DropSource"			, common.m_sEdit.m_bUseOLE_DropSource );
	cProfile.IOProfileData( pszSecName, "bDispExitingDialog"			, common.m_sGeneral.m_bDispExitingDialog );
	cProfile.IOProfileData( pszSecName, "bEnableUnmodifiedOverwrite"	, common.m_sFile.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( pszSecName, "bSelectClickedURL"			, common.m_sEdit.m_bSelectClickedURL );
	cProfile.IOProfileData( pszSecName, "bGrepExitConfirm"			, common.m_sSearch.m_bGrepExitConfirm );/* Grep���[�h�ŕۑ��m�F���邩 */
//	cProfile.IOProfileData( pszSecName, "bRulerDisp"				, common.m_bRulerDisp );/* ���[���[�\�� */
	cProfile.IOProfileData( pszSecName, "nRulerHeight"				, common.m_sWindow.m_nRulerHeight );/* ���[���[���� */
	cProfile.IOProfileData( pszSecName, "nRulerBottomSpace"			, common.m_sWindow.m_nRulerBottomSpace );/* ���[���[�ƃe�L�X�g�̌��� */
	cProfile.IOProfileData( pszSecName, "nRulerType"				, common.m_sWindow.m_nRulerType );/* ���[���[�̃^�C�v */
	//	Sep. 18, 2002 genta �ǉ�
	cProfile.IOProfileData( pszSecName, "nLineNumberRightSpace"		, common.m_sWindow.m_nLineNumRightSpace );/* �s�ԍ��̉E���̌��� */
	cProfile.IOProfileData( pszSecName, "nVertLineOffset"			, common.m_sWindow.m_nVertLineOffset ); // 2005.11.10 Moca
	cProfile.IOProfileData( pszSecName, "bUseCompotibleBMP"			, common.m_sWindow.m_bUseCompatibleBMP ); // 2007.09.09 Moca
	cProfile.IOProfileData( pszSecName, "bCopyAndDisablSelection"	, common.m_sEdit.m_bCopyAndDisablSelection );/* �R�s�[������I������ */
	cProfile.IOProfileData( pszSecName, "bEnableNoSelectCopy"		, common.m_sEdit.m_bEnableNoSelectCopy );/* �I���Ȃ��ŃR�s�[���\�ɂ��� */	// 2007.11.18 ryoji
	cProfile.IOProfileData( pszSecName, "bEnableLineModePaste"		, common.m_sEdit.m_bEnableLineModePaste );/* ���C�����[�h�\��t�����\�ɂ��� */	// 2007.10.08 ryoji
	cProfile.IOProfileData( pszSecName, "bConvertEOLPaste"			, common.m_sEdit.m_bConvertEOLPaste );	/* ���s�R�[�h��ϊ����ē\��t���� */	// 2009.02.28 salarm
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"			, common.m_sHelper.m_bHtmlHelpIsSingle );/* HtmlHelp�r���[�A�͂ЂƂ� */
	cProfile.IOProfileData( pszSecName, "bCompareAndTileHorz"		, common.m_sCompare.m_bCompareAndTileHorz );/* ������r��A���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
	cProfile.IOProfileData( pszSecName, "bDropFileAndClose"			, common.m_sFile.m_bDropFileAndClose );/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	cProfile.IOProfileData( pszSecName, "nDropFileNumMax"			, common.m_sFile.m_nDropFileNumMax );/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	cProfile.IOProfileData( pszSecName, "bCheckFileTimeStamp"		, common.m_sFile.m_bCheckFileTimeStamp );/* �X�V�̊Ď� */
	cProfile.IOProfileData( pszSecName, "bNotOverWriteCRLF"			, common.m_sEdit.m_bNotOverWriteCRLF );/* ���s�͏㏑�����Ȃ� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFind"			, common.m_sSearch.m_bAutoCloseDlgFind );/* �����_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFuncList"		, common.m_sOutline.m_bAutoCloseDlgFuncList );/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgReplace"		, common.m_sSearch.m_bAutoCloseDlgReplace );/* �u�� �_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoColmnPaste"			, common.m_sEdit.m_bAutoColmnPaste );/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
	cProfile.IOProfileData( pszSecName, "NoCaretMoveByActivation"	, common.m_sGeneral.m_bNoCaretMoveByActivation );/* �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji (add by genta) */
	cProfile.IOProfileData( pszSecName, "bScrollBarHorz"			, common.m_sWindow.m_bScrollBarHorz );/* �����X�N���[���o�[���g�� */
	
	cProfile.IOProfileData( pszSecName, "bHokanKey_RETURN"			, common.m_sHelper.m_bHokanKey_RETURN );/* VK_RETURN �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_TAB"				, common.m_sHelper.m_bHokanKey_TAB );/* VK_TAB    �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_RIGHT"			, common.m_sHelper.m_bHokanKey_RIGHT );/* VK_RIGHT  �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_SPACE"			, common.m_sHelper.m_bHokanKey_SPACE );/* VK_SPACE  �⊮����L�[���L��/���� */
	
	cProfile.IOProfileData( pszSecName, "nDateFormatType"			, common.m_sFormat.m_nDateFormatType );/* ���t�����̃^�C�v */
	cProfile.IOProfileData( pszSecName, "szDateFormat"				, common.m_sFormat.m_szDateFormat, 0 );//���t����
	cProfile.IOProfileData( pszSecName, "nTimeFormatType"			, common.m_sFormat.m_nTimeFormatType );/* ���������̃^�C�v */
	cProfile.IOProfileData( pszSecName, "szTimeFormat"				, common.m_sFormat.m_szTimeFormat, 0 );//��������
	
	cProfile.IOProfileData( pszSecName, "bMenuIcon"					, common.m_sWindow.m_bMenuIcon );//���j���[�ɃA�C�R����\������
	cProfile.IOProfileData( pszSecName, "bMenuWChar"				, common.m_sWindow.m_bMenuWChar );//���j���[�̎������΍�
	cProfile.IOProfileData( pszSecName, "bAutoMIMEdecode"			, common.m_sFile.m_bAutoMIMEdecode );//�t�@�C���ǂݍ��ݎ���MIME��decode���s����
	cProfile.IOProfileData( pszSecName, "bQueryIfCodeChange"		, common.m_sFile.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s����
	cProfile.IOProfileData( pszSecName, "bAlertIfFileNotExist"		, common.m_sFile.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveNew"			, common.m_sFile.m_bNoFilterSaveNew );	// �V�K����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveFile"			, common.m_sFile.m_bNoFilterSaveFile );	// �V�K�ȊO����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bAlertIfLargeFile"			, common.m_sFile.m_bAlertIfLargeFile ); // �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
	cProfile.IOProfileData( pszSecName, "nAlertFileSize"   			, common.m_sFile.m_nAlertFileSize );    // �x�����J�n����t�@�C���T�C�Y(MB�P��)

	cProfile.IOProfileData( pszSecName, "bCreateAccelTblEachWin"	, common.m_sKeyBind.m_bCreateAccelTblEachWin);	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)	// 2009.08.15 nasukoji
	
	/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
	const char* pszKeyName = "rcOpenDialog";
	const char* pszForm = "%d,%d,%d,%d";
	char		szKeyData[1024];
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_sOthers.m_rcOpenDialog.left,
				&common.m_sOthers.m_rcOpenDialog.top,
				&common.m_sOthers.m_rcOpenDialog.right,
				&common.m_sOthers.m_rcOpenDialog.bottom
			);
		}
	}else{
		wsprintf(
			szKeyData,
			pszForm,
			common.m_sOthers.m_rcOpenDialog.left,
			common.m_sOthers.m_rcOpenDialog.top,
			common.m_sOthers.m_rcOpenDialog.right,
			common.m_sOthers.m_rcOpenDialog.bottom
		);
		cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, 0 );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( pszSecName, "bMarkUpBlankLineEnable"	, common.m_sOutline.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( pszSecName, "bFunclistSetFocusOnJump"	, common.m_sOutline.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	cProfile.IOProfileData( pszSecName, "szWinCaptionActive"
		, common.m_sWindow.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
	cProfile.IOProfileData( pszSecName, "szWinCaptionInactive"
		, common.m_sWindow.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );
	
	// �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��  20060201 aroka
	cProfile.IOProfileData( pszSecName, "bRememberOutlineWindowPos", common.m_sOutline.m_bRememberOutlineWindowPos);
	if( common.m_sOutline.m_bRememberOutlineWindowPos ){
		cProfile.IOProfileData( pszSecName, "widthOutlineWindow"	, common.m_sOutline.m_widthOutlineWindow);
		cProfile.IOProfileData( pszSecName, "heightOutlineWindow", common.m_sOutline.m_heightOutlineWindow);
		cProfile.IOProfileData( pszSecName, "xOutlineWindowPos"	, common.m_sOutline.m_xOutlineWindowPos);
		cProfile.IOProfileData( pszSecName, "yOutlineWindowPos"	, common.m_sOutline.m_yOutlineWindowPos);
	}
	
}

/*!
	@brief ���L�f�[�^��Toolbar�Z�N�V�����̓��o��
	@param[in]		bRead		true: �ǂݍ��� / false: ��������
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Toolbar( CProfile& cProfile )
{
	const char* pszSecName = "Toolbar";
	int		i;
	char	szKeyName[64];
	CommonSetting_ToolBar& toolbar = m_pShareData->m_Common.m_sToolBar;

	cProfile.IOProfileData( pszSecName, "bToolBarIsFlat", toolbar.m_bToolBarIsFlat );

	cProfile.IOProfileData( pszSecName, "nToolBarButtonNum", toolbar.m_nToolBarButtonNum );
	SetValueLimit( toolbar.m_nToolBarButtonNum, MAX_TOOLBAR_BUTTON_ITEMS );
	int	nSize = toolbar.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "nTBB[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i] );
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i< MAX_TOOLBAR_BUTTON_ITEMS; ++i){
			toolbar.m_nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief ���L�f�[�^��CustMenu�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_CustMenu( CProfile& cProfile )
{
	const char* pszSecName = "CustMenu";
	int		i, j;
	char	szKeyName[64];
	CommonSetting_CustomMenu& menu = m_pShareData->m_Common.m_sCustomMenu;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		wsprintf( szKeyName, "szCMN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta �ő咷�w��
		wsprintf( szKeyName, "nCMIN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemNumArr[i] );
		SetValueLimit( menu.m_nCustMenuItemNumArr[i], _countof(menu.m_nCustMenuItemFuncArr[0]) );
		int nSize = menu.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemFuncArr[i][j] );
			wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemKeyArr[i][j] );
		}
	}
}

/*!
	@brief ���L�f�[�^��Font�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Font( CProfile& cProfile )
{
	const char* pszSecName = "Font";
	CommonSetting_View& view = m_pShareData->m_Common.m_sView;
	ShareData_IO_Sub_LogFont( cProfile, pszSecName, "lf", "nPointSize", "lfFaceName",
		view.m_lf, view.m_nPointSize );

	cProfile.IOProfileData( pszSecName, "bFontIs_FIXED_PITCH", view.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief ���L�f�[�^��KeyBind�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_KeyBind( CProfile& cProfile )
{
	const char* pszSecName = "KeyBind";
	int		i;
	char	szKeyName[64];
	char	szKeyData[1024];
	int		nSize = m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum;
	for( i = 0; i < nSize; ++i ){
		// 2005.04.07 D.S.Koba
		KEYDATA& keydata = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[i];
		strcpy( szKeyName, keydata.m_szKeyName );
		
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
					&keydata.m_nFuncCodeArr[0],
					&keydata.m_nFuncCodeArr[1],
					&keydata.m_nFuncCodeArr[2],
					&keydata.m_nFuncCodeArr[3],
					&keydata.m_nFuncCodeArr[4],
					&keydata.m_nFuncCodeArr[5],
					&keydata.m_nFuncCodeArr[6],
					&keydata.m_nFuncCodeArr[7]
				 );
			}
		}else{
			wsprintf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
				keydata.m_nFuncCodeArr[0],
				keydata.m_nFuncCodeArr[1],
				keydata.m_nFuncCodeArr[2],
				keydata.m_nFuncCodeArr[3],
				keydata.m_nFuncCodeArr[4],
				keydata.m_nFuncCodeArr[5],
				keydata.m_nFuncCodeArr[6],
				keydata.m_nFuncCodeArr[7]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}

/*!
	@brief ���L�f�[�^��Print�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Print( CProfile& cProfile )
{
	const char* pszSecName = "Print";
	int		i, j;
	char	szKeyName[64];
	char	szKeyData[1024];
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = m_pShareData->m_PrintSettingArr[i];
		wsprintf( szKeyName, "PS[%02d].nInts", i );
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData ) ) ){
				sscanf( szKeyData, pszForm,
					&printsetting.m_nPrintFontWidth		,
					&printsetting.m_nPrintFontHeight		,
					&printsetting.m_nPrintDansuu			,
					&printsetting.m_nPrintDanSpace		,
					&printsetting.m_nPrintLineSpacing		,
					&printsetting.m_nPrintMarginTY		,
					&printsetting.m_nPrintMarginBY		,
					&printsetting.m_nPrintMarginLX		,
					&printsetting.m_nPrintMarginRX		,
					&printsetting.m_nPrintPaperOrientation,
					&printsetting.m_nPrintPaperSize		,
					&printsetting.m_bPrintWordWrap		,
					&printsetting.m_bPrintLineNumber		,
					&printsetting.m_bHeaderUse[0]			,
					&printsetting.m_bHeaderUse[1]			,
					&printsetting.m_bHeaderUse[2]			,
					&printsetting.m_bFooterUse[0]			,
					&printsetting.m_bFooterUse[1]			,
					&printsetting.m_bFooterUse[2]
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				printsetting.m_nPrintFontWidth		,
				printsetting.m_nPrintFontHeight		,
				printsetting.m_nPrintDansuu			,
				printsetting.m_nPrintDanSpace			,
				printsetting.m_nPrintLineSpacing		,
				printsetting.m_nPrintMarginTY			,
				printsetting.m_nPrintMarginBY			,
				printsetting.m_nPrintMarginLX			,
				printsetting.m_nPrintMarginRX			,
				printsetting.m_nPrintPaperOrientation	,
				printsetting.m_nPrintPaperSize		,
				printsetting.m_bPrintWordWrap			,
				printsetting.m_bPrintLineNumber		,
				printsetting.m_bHeaderUse[0]			,
				printsetting.m_bHeaderUse[1]			,
				printsetting.m_bHeaderUse[2]			,
				printsetting.m_bFooterUse[0]			,
				printsetting.m_bFooterUse[1]			,
				printsetting.m_bFooterUse[2]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
		
		wsprintf( szKeyName, "PS[%02d].szSName"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintSettingName	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
		wsprintf( szKeyName, "PS[%02d].szFF"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceHan	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
		wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceZen	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
		for( j = 0; j < 3; ++j ){
			wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szHeaderForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
			wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szFooterForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
		}
		wsprintf( szKeyName, "PS[%02d].szDriver", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDriverName,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
		wsprintf( szKeyName, "PS[%02d].szDevice", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDeviceName, 
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
		wsprintf( szKeyName, "PS[%02d].szOutput", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterOutputName,
			sizeof( printsetting.m_mdmDevMode.m_szPrinterOutputName ));
		
		// 2002.02.16 hor �Ƃ肠�������ݒ��ϊ����Ƃ�
		if(0==strcmp(printsetting.m_szHeaderForm[0],"&f") &&
		   0==strcmp(printsetting.m_szFooterForm[0],"&C- &P -")
		){
			strcpy( printsetting.m_szHeaderForm[0], "$f" );
			strcpy( printsetting.m_szFooterForm[0], "" );
			strcpy( printsetting.m_szFooterForm[1], "- $p -" );
		}
		
		//�֑�	//@@@ 2002.04.09 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead );
		wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail );
		wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK
	}
}

/*!
	@brief ���L�f�[�^��STypeConfig�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
	@date 2010/04/17 Uchi ���[�v����ShareData_IO_Type_One�ɕ����B
*/
void CShareData::ShareData_IO_Types( CProfile& cProfile )
{
	int		i;
	char	szKey[32];

	for( i = 0; i < MAX_TYPES; ++i ){
		wsprintf( szKey, "Types(%d)", i );
		ShareData_IO_Type_One( cProfile, i, szKey);
	}
}

/*!
@brief ���L�f�[�^��STypeConfig�Z�N�V�����̓��o��(�P��)
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X
	@param[in]		nType		STypeConfig�Z�N�V�����ԍ�
	@param[in]		pszSecName	�Z�N�V������

	@date 2010/04/17 Uchi ShareData_IO_TypesOne���番���B
*/
void CShareData::ShareData_IO_Type_One( CProfile& cProfile, int nType, const char* pszSecName)
{
	int		j;
	char	szKeyName[64];
	char	szKeyData[1024];

	// 2005.04.07 D.S.Koba
	STypeConfig& types = m_pShareData->m_Types[nType];
	static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
	strcpy( szKeyName, "nInts" );
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&types.m_nIdx,
				&types.m_nMaxLineKetas,
				&types.m_nColmSpace,
				&types.m_nTabSpace,
				&types.m_nKeyWordSetIdx[0],
				&types.m_nKeyWordSetIdx[1],	//MIK
				&types.m_nStringType,
				&types.m_bLineNumIsCRLF,
				&types.m_nLineTermType,
				&types.m_bWordWrap,
				&types.m_nCurrentPrintSetting
			 );
		}
		// �܂�Ԃ����̍ŏ��l��10�B���Ȃ��Ƃ��S�Ȃ��ƃn���O�A�b�v����B // 20050818 aroka
		if( types.m_nMaxLineKetas < MINLINEKETAS ){
			types.m_nMaxLineKetas = MINLINEKETAS;
		}
	}
	else{
		wsprintf( szKeyData, pszForm,
			types.m_nIdx,
			types.m_nMaxLineKetas,
			types.m_nColmSpace,
			types.m_nTabSpace,
			types.m_nKeyWordSetIdx[0],
			types.m_nKeyWordSetIdx[1],	//MIK
			types.m_nStringType,
			types.m_bLineNumIsCRLF,
			types.m_nLineTermType,
			types.m_bWordWrap,
			types.m_nCurrentPrintSetting
		);
		cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
	}
	// 2005.01.13 MIK Keywordset 3-10
	cProfile.IOProfileData( pszSecName, "nKeywordSelect3",  types.m_nKeyWordSetIdx[2] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect4",  types.m_nKeyWordSetIdx[3] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect5",  types.m_nKeyWordSetIdx[4] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect6",  types.m_nKeyWordSetIdx[5] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect7",  types.m_nKeyWordSetIdx[6] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect8",  types.m_nKeyWordSetIdx[7] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect9",  types.m_nKeyWordSetIdx[8] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect10", types.m_nKeyWordSetIdx[9] );

	/* �s�Ԃ̂����� */
	cProfile.IOProfileData( pszSecName, "nLineSpace", types.m_nLineSpace );
	if( cProfile.IsReadingMode() ){
		if( types.m_nLineSpace < /* 1 */ 0 ){
			types.m_nLineSpace = /* 1 */ 0;
		}
		if( types.m_nLineSpace > LINESPACE_MAX ){
			types.m_nLineSpace = LINESPACE_MAX;
		}
	}


	cProfile.IOProfileData( pszSecName, "szTypeName",
		types.m_szTypeName,
		sizeof( m_pShareData->m_Types[0].m_szTypeName ));
	cProfile.IOProfileData( pszSecName, "szTypeExts",
		types.m_szTypeExts,
		sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	cProfile.IOProfileData( pszSecName, "szTabViewString",
		/*&*/types.m_szTabViewString,
		sizeof( types.m_szTabViewString ));
//#endif
	cProfile.IOProfileData( pszSecName, "bTabArrow"			, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
	cProfile.IOProfileData( pszSecName, "bInsSpace"			, types.m_bInsSpace );	// 2001.12.03 hor

	cProfile.IOProfileData( pszSecName, "nTextWrapMethod", types.m_nTextWrapMethod );		// 2008.05.30 nasukoji

	// From Here Sep. 28, 2002 genta / YAZAKI
	if( cProfile.IsReadingMode() ){
		//	Block Comment
		char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
		//	2004.10.02 Moca �΂ɂȂ�R�����g�ݒ肪�Ƃ��ɓǂݍ��܂ꂽ�Ƃ������L���Ȑݒ�ƌ��Ȃ��D
		//	�u���b�N�R�����g�̎n�܂�ƏI���D�s�R�����g�̋L���ƌ��ʒu
		bool bRet1, bRet2;
		buffer[0][0] = buffer[1][0] = '\0';
		bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
			buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
		bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
			buffer[1], BLOCKCOMMENT_BUFFERSIZE );
		if( bRet1 && bRet2 ) types.m_cBlockComments[0].SetBlockCommentRule(buffer[0], buffer[1] );

		//@@@ 2001.03.10 by MIK
		buffer[0][0] = buffer[1][0] = '\0';
		bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
			buffer[0], BLOCKCOMMENT_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
			buffer[1], BLOCKCOMMENT_BUFFERSIZE );
		if( bRet1 && bRet2 ) types.m_cBlockComments[1].SetBlockCommentRule(buffer[0], buffer[1] );
		
		//	Line Comment
		char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
		int  pos;

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 0, lbuf, pos );

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment2"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 1, lbuf, pos );

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment3"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO �ǉ�
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 2, lbuf, pos );
	}
	else { // write
		//	Block Comment
		cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
			const_cast<char*>(types.m_cBlockComments[0].getBlockCommentFrom()), 0 );
		cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
			const_cast<char*>(types.m_cBlockComments[0].getBlockCommentTo()), 0 );

		//@@@ 2001.03.10 by MIK
		cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
			const_cast<char*>(types.m_cBlockComments[1].getBlockCommentFrom()), 0 );
		cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
			const_cast<char*>(types.m_cBlockComments[1].getBlockCommentTo()), 0 );

		//	Line Comment
	cProfile.IOProfileData( pszSecName, "szLineComment"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 0 )), 0 );
	cProfile.IOProfileData( pszSecName, "szLineComment2"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 1 )), 0 );
	cProfile.IOProfileData( pszSecName, "szLineComment3"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO �ǉ�

	//	From here May 12, 2001 genta
	int pos;
	pos = types.m_cLineComment.getLineCommentPos( 0 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
	pos = types.m_cLineComment.getLineCommentPos( 1 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
	pos = types.m_cLineComment.getLineCommentPos( 2 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
	//	To here May 12, 2001 genta

	}
	// To Here Sep. 28, 2002 genta / YAZAKI

	cProfile.IOProfileData( pszSecName, "szIndentChars"		,
		types.m_szIndentChars,
		sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
	cProfile.IOProfileData( pszSecName, "cLineTermChar"		, types.m_cLineTermChar );

	cProfile.IOProfileData( pszSecName, "nDefaultOutline"	, types.m_nDefaultOutline );/* �A�E�g���C����͕��@ */
	cProfile.IOProfileData( pszSecName, "szOutlineRuleFilename"	,
		types.m_szOutlineRuleFilename,
		sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* �A�E�g���C����̓��[���t�@�C�� */
	cProfile.IOProfileData( pszSecName, "nSmartIndent"		, types.m_nSmartIndent );/* �X�}�[�g�C���f���g��� */
	//	Nov. 20, 2000 genta
	cProfile.IOProfileData( pszSecName, "nImeState"			, types.m_nImeState );	//	IME����

	//	2001/06/14 Start By asa-o: �^�C�v�ʂ̕⊮�t�@�C��
	//	Oct. 5, 2002 genta sizeof()�Ō���ă|�C���^�̃T�C�Y���擾���Ă����̂��C��
	cProfile.IOProfileData( pszSecName, "szHokanFile"		,
		types.m_szHokanFile,
		sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	�⊮�t�@�C��
	//	2001/06/14 End

	//	2001/06/19 asa-o
	cProfile.IOProfileData( pszSecName, "bHokanLoHiCase"		, types.m_bHokanLoHiCase );

	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	cProfile.IOProfileData( pszSecName, "bUseHokanByFile"		, types.m_bUseHokanByFile );

	//@@@ 2002.2.4 YAZAKI
	cProfile.IOProfileData( pszSecName, "szExtHelp"			,
		types.m_szExtHelp,
		sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
		
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp"		,
		types.m_szExtHtmlHelp,
		sizeof( types.m_szExtHtmlHelp ));
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"	, types.m_bHtmlHelpIsSingle ); // 2012.06.30 Fix m_bHokanLoHiCase -> m_bHtmlHelpIsSingle

	cProfile.IOProfileData( pszSecName, "eDefaultCodetype"		, (int&)types.m_eDefaultCodetype );
	cProfile.IOProfileData( pszSecName, "eDefaultEoltype"		, types.m_eDefaultEoltype );
	cProfile.IOProfileData( pszSecName, "bDefaultBom"			, types.m_bDefaultBom );

	cProfile.IOProfileData( pszSecName, "bAutoIndent"			, types.m_bAutoIndent );
	cProfile.IOProfileData( pszSecName, "bAutoIndent_ZENSPACE"	, types.m_bAutoIndent_ZENSPACE );
	cProfile.IOProfileData( pszSecName, "bRTrimPrevLine"		, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
	cProfile.IOProfileData( pszSecName, "nIndentLayout"			, types.m_nIndentLayout );

	/* �F�ݒ� I/O */
	IO_ColorSet( &cProfile, pszSecName, types.m_ColorInfoArr  );

	// 2005.11.08 Moca �w�茅�c��
	for(j = 0; j < MAX_VERTLINES; j++ ){
		wsprintf( szKeyName, "nVertLineIdx%d", j + 1 );
		cProfile.IOProfileData( pszSecName, szKeyName, types.m_nVertLineIdx[j] );
		if( types.m_nVertLineIdx[j] == 0 ){
			break;
		}
	}

//@@@ 2001.11.17 add start MIK
	{	//���K�\���L�[���[�h
		char	*p;
		cProfile.IOProfileData( pszSecName, "bUseRegexKeyword", types.m_bUseRegexKeyword );/* ���K�\���L�[���[�h�g�p���邩�H */
		for(j = 0; j < 100; j++)
		{
			wsprintf( szKeyName, "RxKey[%03d]", j );
			if( cProfile.IsReadingMode() )
			{
				types.m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
				types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
				if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
				{
					p = strchr(szKeyData, ',');
					if( p )
					{
						*p = '\0';
						types.m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(szKeyData);	//@@@ 2002.04.30
						if( types.m_RegexKeywordArr[j].m_nColorIndex == -1 )	//���O�łȂ�
							types.m_RegexKeywordArr[j].m_nColorIndex = atoi(szKeyData);
						p++;
						strcpy(types.m_RegexKeywordArr[j].m_szKeyword, p);
						if( types.m_RegexKeywordArr[j].m_nColorIndex < 0
						 || types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
						{
							types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
						}
					}
				}
			}
			// 2002.02.08 hor ����`�l�𖳎�
			else if(lstrlen(types.m_RegexKeywordArr[j].m_szKeyword))
			{
				wsprintf( szKeyData, "%s,%s",
					GetColorNameByIndex( types.m_RegexKeywordArr[j].m_nColorIndex ),
					types.m_RegexKeywordArr[j].m_szKeyword);
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
			}
		}
	}
//@@@ 2001.11.17 add end MIK

	/* �֑� */
	cProfile.IOProfileData( pszSecName, "bKinsokuHead"	, types.m_bKinsokuHead );
	cProfile.IOProfileData( pszSecName, "bKinsokuTail"	, types.m_bKinsokuTail );
	cProfile.IOProfileData( pszSecName, "bKinsokuRet"	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
	cProfile.IOProfileData( pszSecName, "bKinsokuKuto"	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
	cProfile.IOProfileData( pszSecName, "szKinsokuHead"	,
		types.m_szKinsokuHead,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
	cProfile.IOProfileData( pszSecName, "szKinsokuTail"	,
		types.m_szKinsokuTail,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
	cProfile.IOProfileData( pszSecName, "szKinsokuKuto"	,
		types.m_szKinsokuKuto,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuKuto ));	// 2009.08.07 ryoji
	cProfile.IOProfileData( pszSecName, "bUseDocumentIcon"	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta �ϐ������C��

//@@@ 2006.04.10 fon ADD-start
	{	/* �L�[���[�h���� */
		static const char* pszForm = "%d,%s,%s";
		char	*pH, *pT;	/* <pH>keyword<pT> */
		cProfile.IOProfileData( pszSecName, "bUseKeyWordHelp", types.m_bUseKeyWordHelp );	/* �L�[���[�h�����I�����g�p���邩�H */
//		cProfile.IOProfileData( pszSecName, "nKeyHelpNum", types.m_nKeyHelpNum );				/* �o�^������ */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpAllSearch", types.m_bUseKeyHelpAllSearch );	/* �q�b�g�������̎���������(&A) */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpKeyDisp", types.m_bUseKeyHelpKeyDisp );		/* 1�s�ڂɃL�[���[�h���\������(&W) */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpPrefix", types.m_bUseKeyHelpPrefix );		/* �I��͈͂őO����v����(&P) */
		for(j = 0; j < MAX_KEYHELP_FILE; j++){
			wsprintf( szKeyName, "KDct[%02d]", j );
			/* �ǂݏo�� */
			if( cProfile.IsReadingMode() ){
				types.m_KeyHelpArr[j].m_nUse = 0;
				types.m_KeyHelpArr[j].m_szAbout[0] = '\0';
				types.m_KeyHelpArr[j].m_szPath[0] = '\0';
				if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
					pH = szKeyData;
					if( NULL != (pT=strchr(pH, ',')) ){
						*pT = '\0';
						types.m_KeyHelpArr[j].m_nUse = atoi( pH );
						pH = pT+1;
						if( NULL != (pT=strchr(pH, ',')) ){
							*pT = '\0';
							strcpy( types.m_KeyHelpArr[j].m_szAbout, pH );
							pH = pT+1;
							if( '\0' != (*pH) ){
								strcpy( types.m_KeyHelpArr[j].m_szPath, pH );
								types.m_nKeyHelpNum = j+1;	// ini�ɕۑ������ɁA�ǂݏo�����t�@�C�������������Ƃ���
							}
						}
					}
				}
			}/* �������� */
			else{
				if(_tcslen(types.m_KeyHelpArr[j].m_szPath)){
					wsprintf( szKeyData, pszForm,
						types.m_KeyHelpArr[j].m_nUse,
						types.m_KeyHelpArr[j].m_szAbout,
						types.m_KeyHelpArr[j].m_szPath
					);
					cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
		/* ���o�[�W����ini�t�@�C���̓ǂݏo���T�|�[�g */
		if( cProfile.IsReadingMode() ){
			cProfile.IOProfileData( pszSecName, "szKeyWordHelpFile",
			types.m_KeyHelpArr[0].m_szPath, sizeof( types.m_KeyHelpArr[0].m_szPath ) );
		}
	}
//@@@ 2006.04.10 fon ADD-end
}

/*!
	@brief ���L�f�[�^��KeyWords�Z�N�V�����̓��o��
	@param[in]		bRead		true: �ǂݍ��� / false: ��������
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_KeyWords( CProfile& cProfile )
{
	const char*		pszSecName = "KeyWords";
	int				i, j;
	char			szKeyName[64];
	char			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( pszSecName, "nCurrentKeyWordSetIdx"	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( pszSecName, "nKeyWordSetNum", nKeyWordSetNum );
	if( cProfile.IsReadingMode() ){
		// nKeyWordSetNum ���ǂݍ��߂Ă���΁A���ׂĂ̏�񂪂�����Ă���Ɖ��肵�ď�����i�߂�
		if( bIOSuccess ){
			// 2004.11.25 Moca �L�[���[�h�Z�b�g�̏��́A���ڏ��������Ȃ��Ŋ֐��𗘗p����
			// �����ݒ肳��Ă��邽�߁A��ɍ폜���Ȃ��ƌŒ胁�����̊m�ۂɎ��s����\��������
			int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
			char *pszMem = new char[nMemLen];
			pCKeyWordSetMgr->ResetAllKeyWordSet();
			for( i = 0; i < nKeyWordSetNum; ++i ){
				bool bKEYWORDCASE = false;
				int nKeyWordNum = 0;
				//�l�̎擾
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, bKEYWORDCASE );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKeyWordNum );

				//�ǉ�
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, bKEYWORDCASE, nKeyWordNum );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen ) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, pszMem );
				}
			}
			delete [] pszMem;
		}
	}else{
		int nSize = pCKeyWordSetMgr->m_nKeyWordSetNum;
		for( i = 0; i < nSize; ++i ){
			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName,
				pCKeyWordSetMgr->m_szSetNameArr[i],
				sizeof( pCKeyWordSetMgr->m_szSetNameArr[0] ));
			wsprintf( szKeyName, "nCASE[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_bKEYWORDCASEArr[i] );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKeyWordNumArr[i] );
			
			int nMemLen = 0;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				nMemLen += strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				nMemLen ++;
			}
			nMemLen ++;
			wsprintf( szKeyName, "szKW[%02d].Size", i );
			cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
			char* pszMem = new char[nMemLen + 1];	//	May 25, 2003 genta ��؂��TAB�ɕύX�����̂ŁC�Ō��\0�̕���ǉ�
			char* pMem = pszMem;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				//	May 25, 2003 genta ��؂��TAB�ɕύX
				int kwlen = strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				memcpy( pMem, pCKeyWordSetMgr->GetKeyWord( i, j ), kwlen );
				pMem += kwlen;
				*pMem++ = '\t';
			}
			*pMem = '\0';
			wsprintf( szKeyName, "szKW[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen );
			delete [] pszMem;
		}
	}
}

/*!
	@brief ���L�f�[�^��Macro�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Macro( CProfile& cProfile )
{
	const char* pszSecName = "Macro";
	int		i;	
	char	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = m_pShareData->m_Common.m_sMacro.m_MacroTable[i];
		//	Oct. 4, 2001 genta ���܂�Ӗ����Ȃ������Ȃ̂ō폜�F3�s
		// 2002.02.08 hor ����`�l�𖳎�
		if( !cProfile.IsReadingMode() && !_tcslen(macrorec.m_szName) && !_tcslen(macrorec.m_szFile) ) continue;
		wsprintf( szKeyName, "Name[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szName, MACRONAME_MAX - 1 );
		wsprintf( szKeyName, "File[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szFile, _MAX_PATH );
		wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
	cProfile.IOProfileData( pszSecName, "nMacroOnOpened", m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );	/* �I�[�v���㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, "nMacroOnTypeChanged", m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged );	/* �^�C�v�ύX�㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, "nMacroOnSave", m_pShareData->m_Common.m_sMacro.m_nMacroOnSave );	/* �ۑ��O�������s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
}

/*!
	@brief ���L�f�[�^��Other�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Other( CProfile& cProfile )
{
	const char* pszSecName = "Other";	//�Z�N�V������1�쐬�����B2003.05.12 MIK
	int		i;	
	char	szKeyName[64];

	/* **** ���̑��̃_�C�A���O **** */
	/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
	if(!cProfile.IOProfileData( pszSecName, "nExecFlgOpt"	, m_pShareData->m_nExecFlgOpt ) ){ //	2006.12.03 maru �I�v�V�����g��
		cProfile.IOProfileData( pszSecName, "bGetStdout"		, m_pShareData->m_nExecFlgOpt );
	}

	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */
	cProfile.IOProfileData( pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );
	
	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
	cProfile.IOProfileData( pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum );
	for( i = 0; i < m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum; ++i ){
		wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
		if( i >= m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum ){
			strcpy( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], "" );
		}
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[0] ));
	}
	cProfile.IOProfileData( pszSecName, "m_bTagJumpICase"		, m_pShareData->m_sTagJump.m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_sTagJump.m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��

	//	MIK �o�[�W�������i�������݂̂݁j
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		wsprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_sVersion.m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_sVersion.m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_sVersion.m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_sVersion.m_dwProductVersionLS ) );
		cProfile.IOProfileData( pszSecName, _T("szVersion")	, iniVer, sizeof( iniVer ) );
	}
}

/*!
	@brief �F�ݒ� I/O

	�w�肳�ꂽ�F�ݒ���w�肳�ꂽ�Z�N�V�����ɏ������ށB�܂���
	�w�肳�ꂽ�Z�N�V�������炢��ݒ��ǂݍ��ށB

	@param[in,out]	pcProfile		�����o���A�ǂݍ��ݐ�Profile object (���o�͕�����bRead�Ɉˑ�)
	@param[in]		pszSecName		�Z�N�V������
	@param[in,out]	pColorInfoArr	�����o���A�ǂݍ��ݑΏۂ̐F�ݒ�ւ̃|�C���^ (���o�͕�����bRead�Ɉˑ�)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, const char* pszSecName, ColorInfo* pColorInfoArr )
{
	char	szKeyName[256];
	char	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const char* pszForm = "%d,%d,%06x,%06x,%d";
		wsprintf( szKeyName, "C[%s]", g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
		if( pcProfile->IsReadingMode() ){
			if( pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}
			else{
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 �ȑO��ini�t�@�C����ǂ񂾂Ƃ��ɃL�����b�g���e�L�X�g�w�i�F�Ɠ����ɂȂ��
				// ������ƍ���̂ŃL�����b�g�F���ǂ߂Ȃ��Ƃ��̓L�����b�g�F���e�L�X�g�F�Ɠ����ɂ���
				if( COLORIDX_CARET == j )
					pColorInfoArr[j].m_colTEXT = pColorInfoArr[COLORIDX_TEXT].m_colTEXT;
			}
			// 2006.12.18 ryoji
			// �����ݒ肪����ΏC������
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if( 0 != (fAttribute & COLOR_ATTRIB_FORCE_DISP) )
				pColorInfoArr[j].m_bDisp = TRUE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_BOLD) )
				pColorInfoArr[j].m_bFatFont = FALSE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )
				pColorInfoArr[j].m_bUnderLine = FALSE;
		}
		else{
			wsprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp,
				pColorInfoArr[j].m_bFatFont,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine
			);
			pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}


/*!
	@brief �^�O�W�����v���̕ۑ�

	�^�O�W�����v����Ƃ��ɁA�^�O�W�����v��̏���ۑ�����B

	@param[in] pTagJump �ۑ�����^�O�W�����v���
	@retval true	�ۑ�����
	@retval false	�ۑ����s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca ��t�ɂȂ������ԌÂ������폜�������ɐV������������
*/
void CShareData::PushTagJump(const TagJump *pTagJump)
{
	int i = m_pShareData->m_sTagJump.m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_sTagJump.m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_sTagJump.m_TagJumpNum++;
	}
	m_pShareData->m_sTagJump.m_TagJump[i] = *pTagJump;
	m_pShareData->m_sTagJump.m_TagJumpTop = i;
}


/*!
	@brief �^�O�W�����v���̎Q��

	�^�O�W�����v�o�b�N����Ƃ��ɁA�^�O�W�����v���̏����Q�Ƃ���B

	@param[out] pTagJump �Q�Ƃ���^�O�W�����v���
	@retval true	�Q�Ɛ���
	@retval false	�Q�Ǝ��s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca SetTagJump�ύX�ɂ��C��
*/
bool CShareData::PopTagJump(TagJump *pTagJump)
{
	if( 0 < m_pShareData->m_sTagJump.m_TagJumpNum ){
		*pTagJump = m_pShareData->m_sTagJump.m_TagJump[m_pShareData->m_sTagJump.m_TagJumpTop--];
		if( m_pShareData->m_sTagJump.m_TagJumpTop < 0 ){
			m_pShareData->m_sTagJump.m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_sTagJump.m_TagJumpNum--;
		return true;
	}
	return false;
}

void ShareData_IO_Sub_LogFont( CProfile& cProfile, const char* pszSecName,
	const char* pszKeyLf, const char* pszKeyPointSize, const char* pszKeyFaceName, LOGFONT& lf, int& pointSize )
{
	const TCHAR* pszForm = _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
	TCHAR		szKeyData[1024];

	cProfile.IOProfileData( pszSecName, pszKeyPointSize, pointSize );	// 2009.10.01 ryoji
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyLf, szKeyData, sizeof(szKeyData) ) ){
			sscanf( szKeyData, pszForm,
				&lf.lfHeight,
				&lf.lfWidth,
				&lf.lfEscapement,
				&lf.lfOrientation,
				&lf.lfWeight,
				&lf.lfItalic,
				&lf.lfUnderline,
				&lf.lfStrikeOut,
				&lf.lfCharSet,
				&lf.lfOutPrecision,
				&lf.lfClipPrecision,
				&lf.lfQuality,
				&lf.lfPitchAndFamily
			);
			if( pointSize != 0 ){
				// DPI�ύX���Ă��t�H���g�̃|�C���g�T�C�Y���ς��Ȃ��悤��
				// �|�C���g������s�N�Z�����ɕϊ�����
				lf.lfHeight = -DpiPointsToPixels( abs(pointSize), 10 );	// pointSize: 1/10�|�C���g�P�ʂ̃T�C�Y
			}else{
				// ����܂��͌Â��o�[�W��������̍X�V���̓|�C���g�����s�N�Z��������t�Z���ĉ��ݒ�
				pointSize = DpiPixelsToPoints( abs(lf.lfHeight) ) * 10;	// �����_�����̓[���̈����i�]���t�H���g�_�C�A���O�ŏ����_�͎w��s�j
			}
		}
	}else{
		wsprintf( szKeyData, pszForm,
			lf.lfHeight,
			lf.lfWidth,
			lf.lfEscapement,
			lf.lfOrientation,
			lf.lfWeight,
			lf.lfItalic,
			lf.lfUnderline,
			lf.lfStrikeOut,
			lf.lfCharSet,
			lf.lfOutPrecision,
			lf.lfClipPrecision,
			lf.lfQuality,
			lf.lfPitchAndFamily
		);
		cProfile.IOProfileData( pszSecName, pszKeyLf, szKeyData, 0 );
	}
	
	cProfile.IOProfileData( pszSecName, pszKeyFaceName, lf.lfFaceName, sizeof(lf.lfFaceName) );
}

/*[EOF]*/
