/*!	@file
	@brief ���ʊ֐��Q

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, shoji masami, Misaka, Stonee, MIK, YAZAKI
	Copyright (C) 2002, genta, aroka, hor, MIK, �S, Moca, YAZAKI
	Copyright (C) 2003, genta, matsumo, Moca, MIK
	Copyright (C) 2004, genta, novice, Moca, MIK
	Copyright (C) 2005, genta, D.S.Koba, Moca, ryoji, aroka
	Copyright (C) 2006, genta, ryoji, rastiv
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, ryoji, nasukoji, novice
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//	Sep. 10, 2005 genta GetLongPathName�̃G�~�����[�V�����֐��̎��̐����̂���
#define COMPILE_NEWAPIS_STUBS
// 2006.04.21 ryoji �}���`���j�^�̃G�~�����[�V�����֐��̎��̐����̂���
#define COMPILE_MULTIMON_STUBS

#include "StdAfx.h"
#include <Shlwapi.h>	// 2006.06.17 ryoji
#include <HtmlHelp.h>
#include <io.h>
#include <memory.h>		// Apr. 03, 2003 genta
#include "etc_uty.h"
#include "Debug.h"
#include "CMemory.h"
#include "Funccode.h"	//Stonee, 2001/02/23
#include "mymessage.h"	// 2007.04.03 ryoji

#include "WinNetWk.h"	//Stonee, 2001/12/21
#include "sakura.hh"	//YAZAKI, 2001/12/11
#include "CEol.h"// 2002/2/3 aroka
#include "CBregexp.h"// 2002/2/3 aroka
#include "COsVersionInfo.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include "charcode.h"  // 2006/08/28 rastiv

#include "CShareData.h"
#include "CMRU.h"
#include "CMRUFolder.h"
#include "CUxTheme.h"	// 2007.04.01 ryoji

/*!	WinHelp �̂����� HtmlHelp ���Ăяo��

	@author ryoji
	@date 2006.07.22 ryoji �V�K
*/
BOOL MyWinHelp(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
	UINT uCommandOrg = uCommand;	// WinHelp �̃R�}���h
	bool bDesktop = false;	// �f�X�N�g�b�v��e�ɂ��ăw���v��ʂ��o�����ǂ���
	HH_POPUP hp;	// �|�b�v�A�b�v�w���v�p�̍\����

	// Note: HH_TP_HELP_CONTEXTMENU �� HELP_WM_HELP �ł̓w���v�R���p�C������
	// �g�s�b�N�t�@�C���� Cshelp.txt �ȊO�ɂ��Ă���ꍇ�A
	// ���̃t�@�C������ .chm �p�X���ɒǉ��w�肷��K�v������B
	//     ��jsakura.chm::/xxx.txt

	switch( uCommandOrg )
	{
	case HELP_COMMAND:	// [�w���v]-[�ڎ�]
	case HELP_CONTENTS:
		uCommand = HH_DISPLAY_TOC;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_KEY:	// [�w���v]-[�L�[���[�h����]
		uCommand = HH_DISPLAY_INDEX;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXT:	// ���j���[��ł�[F1]�L�[�^�_�C�A���O��[�w���v]�{�^��
		uCommand = HH_HELP_CONTEXT;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXTMENU:	// �R���g���[����ł̉E�N���b�N
	case HELP_WM_HELP:		// [�H]�{�^���������ăR���g���[�����N���b�N�^�R���g���[���Ƀt�H�[�J�X��u����[F1]�L�[
		uCommand = HH_DISPLAY_TEXT_POPUP;
		{
			// �|�b�v�A�b�v�w���v�p�̍\���̂ɒl���Z�b�g����
			HWND hwndCtrl;	// �w���v�\���Ώۂ̃R���g���[��
			int nCtrlID;	// �ΏۃR���g���[���� ID
			DWORD* pHelpIDs;	// �R���g���[�� ID �ƃw���v ID �̑Ή��\�ւ̃|�C���^

			memset(&hp, 0, sizeof(hp));	// �\���̂��[���N���A
			hp.cbStruct = sizeof(hp);
			hp.pszFont = _T("�l�r �o�S�V�b�N, 9");
			hp.clrForeground = hp.clrBackground = -1;
			hp.rcMargins.left = hp.rcMargins.top = hp.rcMargins.right = hp.rcMargins.bottom = -1;
			if( uCommandOrg == HELP_CONTEXTMENU ){
				// �}�E�X�J�[�\���ʒu����ΏۃR���g���[���ƕ\���ʒu�����߂�
				if( !::GetCursorPos(&hp.pt) )
					return FALSE;
				hwndCtrl = ::WindowFromPoint(hp.pt);
			}
			else{
				// �ΏۃR���g���[���� hwndCaller
				// [F1]�L�[�̏ꍇ������̂őΏۃR���g���[���̈ʒu����\���ʒu�����߂�
				RECT rc;
				hwndCtrl = hwndCaller;
				if( !::GetWindowRect( hwndCtrl, &rc ) )
					return FALSE;
				hp.pt.x = (rc.left + rc.right) / 2;
				hp.pt.y = rc.top;
			}
			// �ΏۃR���g���[���ɑΉ�����w���v ID ��T��
			nCtrlID = ::GetDlgCtrlID( hwndCtrl );
			if( nCtrlID <= 0 )
				return FALSE;
			pHelpIDs = (DWORD*)dwData;
			while( true ){
				if( *pHelpIDs == 0 )
					return FALSE;	// ������Ȃ�����
				if( *pHelpIDs == nCtrlID )
					break;			// ��������
				pHelpIDs += 2;
			}
			hp.idString = *(pHelpIDs + 1);	// �������w���v ID ��ݒ肷��
			dwData = (DWORD_PTR)&hp;	// �������|�b�v�A�b�v�w���v�p�̍\���̂ɍ����ւ���
		}
		break;
	default:
		return FALSE;
	}

	if( IsFileExists( lpszHelp, true ) ){
		// HTML �w���v���Ăяo��
		HWND hWnd = OpenHtmlHelp( hwndCaller, lpszHelp, uCommand, dwData );
		if (bDesktop && hWnd != NULL){
			::SetForegroundWindow( hWnd );	// �w���v��ʂ���O�ɏo��
		}
	}
	else {
		if( uCommandOrg == HELP_CONTEXTMENU)
			return FALSE;	// �E�N���b�N�ł͉������Ȃ��ł���

		// �I�����C���w���v���Ăяo��
		if( uCommandOrg != HELP_CONTEXT )
			dwData = 1;	// �ڎ��y�[�W

		TCHAR buf[256];
		_stprintf( buf, _T("http://sakura-editor.sourceforge.net/cgi-bin/hid.cgi?%d"), dwData );
		ShellExecute( ::GetActiveWindow(), NULL, buf, NULL, NULL, SW_SHOWNORMAL );
	}

	return TRUE;
}

int CALLBACK MYBrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
)
{
	switch( uMsg ){
	case BFFM_INITIALIZED:
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
		break;
	}
	return 0;

}


/* �t�H���_�I���_�C�A���O */
BOOL SelectDir( HWND hWnd, const TCHAR* pszTitle, const TCHAR* pszInitFolder, TCHAR* strFolderName )
{
	BOOL	bRes;
	TCHAR	szInitFolder[MAX_PATH];

	_tcscpy( szInitFolder, pszInitFolder );
	/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
	CutLastYenFromDirectoryPath( szInitFolder );

	// 2010.08.28 �t�H���_���J���ƃt�b�N���܂߂ĐF�XDLL���ǂݍ��܂��̂ňړ�
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	// SHBrowseForFolder()�֐��ɓn���\����
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = strFolderName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS/* | BIF_EDITBOX*//* | BIF_STATUSTEXT*/;
	bi.lpfn = MYBrowseCallbackProc;
	bi.lParam = (LPARAM)szInitFolder;
	bi.iImage = 0;
	// �A�C�e���h�c���X�g��Ԃ�
	// ITEMIDLIST�̓A�C�e���̈�ӂ�\���\����
	LPITEMIDLIST pList = ::SHBrowseForFolder(&bi);
	if( NULL != pList ){
		// SHGetPathFromIDList()�֐��̓A�C�e���h�c���X�g�̕����p�X��T���Ă����
		bRes = ::SHGetPathFromIDList( pList, strFolderName );
		// :SHBrowseForFolder()�Ŏ擾�����A�C�e���h�c���X�g���폜
		::CoTaskMemFree( pList );
		if( bRes ){
			return TRUE;
		}else{
			return FALSE;
		}
	}
	return FALSE;
}

/* �g���q�𒲂ׂ� */
BOOL CheckEXT( const TCHAR* pszPath, const TCHAR* pszExt )
{
	TCHAR	szExt[_MAX_EXT];
	TCHAR*	pszWork;
	_tsplitpath( pszPath, NULL, NULL, NULL, szExt );
	pszWork = szExt;
	if( pszWork[0] == _T('.') ){
		pszWork++;
	}
	if( 0 == my_stricmp( pszExt, pszWork ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

/*!
	�󔒂��܂ރt�@�C�������l�������g�[�N���̕���
	
	�擪�ɂ���A��������؂蕶���͖�������D
	
	@param pBuffer [in] ������o�b�t�@(�I�[�����邱��)
	@param nLen [in] ������̒���
	@param pnOffset [in/out] �I�t�Z�b�g
	@param pDelimiter [in] ��؂蕶��
	@return �g�[�N��

	@date 2004.02.15 �݂� �œK��
*/
TCHAR* my_strtok( TCHAR* pBuffer, int nLen, int* pnOffset, const TCHAR* pDelimiter )
{
	int i = *pnOffset;
	TCHAR* p;

	do {
		bool bFlag = false;	//�_�u���R�[�e�[�V�����̒����H
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == _T('"') ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( _tcschr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//��̃g�[�N���Ȃ玟��T��
	return p;
}

/*! �����O�t�@�C�������擾���� 

	@param[in] pszFilePathSrc �ϊ����p�X��
	@param[out] pszFilePathDes ���ʏ������ݐ� (����MAX_PATH�̗̈悪�K�v)

	@date Oct. 2, 2005 genta GetFilePath API���g���ď�������
	@date Oct. 4, 2005 genta ���΃p�X����΃p�X�ɒ�����Ȃ�����
	@date Oct. 5, 2005 Moca  ���΃p�X���΃p�X�ɕϊ�����悤��
*/
BOOL GetLongFileName( const TCHAR* pszFilePathSrc, TCHAR* pszFilePathDes )
{
	TCHAR* name;
	TCHAR szBuf[_MAX_PATH + 1];
	int len = ::GetFullPathName( pszFilePathSrc, _MAX_PATH, szBuf, &name );
	if( len <= 0 || _MAX_PATH <= len ){
		len = ::GetLongPathName( pszFilePathSrc, pszFilePathDes, _MAX_PATH );
		if( len <= 0 || _MAX_PATH < len ){
			return FALSE;
		}
		return TRUE;
	}
	len = ::GetLongPathName( szBuf, pszFilePathDes, _MAX_PATH );
	if( len <= 0 || _MAX_PATH < len ){
		_tcscpy( pszFilePathDes, szBuf );
	}
	return TRUE;
}

/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
void SplitPath_FolderAndFile( const TCHAR* pszFilePath, TCHAR* pszFolder, TCHAR* pszFile )
{
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	int		nFolderLen;
	int		nCharChars;
	_tsplitpath( pszFilePath, szDrive, szDir, szFname, szExt );
	if( NULL != pszFolder ){
		_tcscpy( pszFolder, szDrive );
		_tcscat( pszFolder, szDir );
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	if( NULL != pszFile ){
		_tcscpy( pszFile, szFname );
		_tcscat( pszFile, szExt );
	}
	return;
}




/* �V�X�e�����\�[�X�𒲂ׂ�
	Win16 �̎��́AGetFreeSystemResources �Ƃ����֐�������܂����B�������AWin32 �ł͂���܂���B
	�T���N����邾�� DLL ����邾�͓̂�����܂��B�ȒP�ȕ��@��������܂��B
	���g���� Windows95 �� [�A�N�Z�T��]-[�V�X�e���c�[��] �Ƀ��\�[�X���[�^������̂Ȃ�A
	c:\windows\system\rsrc32.dll ������͂��ł��B����́A���\�[�X���[�^�Ƃ��� Win32 �A�v�����A
	Win16 �� GetFreeSystemResources �֐����ĂԈׂ� DLL �ł��B������g���܂��傤�B
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES	0x0000
	#define GFSR_GDIRESOURCES		0x0001
	#define GFSR_USERRESOURCES		0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibraryExedir( _T("RSRC32.dll") );
	if( (int)hlib > 32 ){
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ){
			*pnSystemResources = GetFreeSystemResources( GFSR_SYSTEMRESOURCES );
			*pnUserResources = GetFreeSystemResources( GFSR_USERRESOURCES );
			*pnGDIResources = GetFreeSystemResources( GFSR_GDIRESOURCES );
			::FreeLibrary( hlib );
			return TRUE;
		}else{
			::FreeLibrary( hlib );
			return FALSE;
		}
	}else{
		return FALSE;
	}
}




/* �V�X�e�����\�[�X�̃`�F�b�N */
BOOL CheckSystemResources( const TCHAR* pszAppName )
{
	int		nSystemResources;
	int		nUserResources;
	int		nGDIResources;
	TCHAR*	pszResourceName;
	/* �V�X�e�����\�[�X�̎擾 */
	if( GetSystemResources( &nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE_A( "nSystemResources=%d\n", nSystemResources );
//		MYTRACE_A( "nUserResources=%d\n", nUserResources );
//		MYTRACE_A( "nGDIResources=%d\n", nGDIResources );
		pszResourceName = NULL;
		if( nSystemResources <= 5 ){
			pszResourceName = _T("�V�X�e�� ");
		}else
		if( nUserResources <= 5 ){
			pszResourceName = _T("���[�U�[ ");
		}else
		if( nGDIResources <= 5 ){
			pszResourceName = _T("GDI ");
		}
		if( NULL != pszResourceName ){
			ErrorBeep();
			ErrorBeep();
			::MYMESSAGEBOX( NULL, MB_OK | /*MB_YESNO | */ MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
				_T("%s���\�[�X���ɒ[�ɕs�����Ă��܂��B\n")
				_T("���̂܂�%s���N������ƁA����ɓ��삵�Ȃ��\��������܂��B\n")
				_T("�V����%s�̋N���𒆒f���܂��B\n")
				_T("\n")
				_T("�V�X�e�� ���\�[�X\t�c��  %d%%\n")
				_T("User ���\�[�X\t�c��  %d%%\n")
				_T("GDI ���\�[�X\t�c��  %d%%\n\n"),
				pszResourceName,
				pszAppName,
				pszAppName,
				nSystemResources,
				nUserResources,
				nGDIResources
			);
//			) ){
				return FALSE;
//			}
		}
	}
	return TRUE;
}




struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	WCHAR	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/*! ���\�[�X���琻�i�o�[�W�����̎擾
	@date 2004.05.13 Moca ��x�擾������L���b�V������
*/
void GetAppVersionInfo(
	HINSTANCE	hInstance,
	int			nVersionResourceID,
	DWORD*		pdwProductVersionMS,
	DWORD*		pdwProductVersionLS
)
{
	HRSRC					hRSRC;
	HGLOBAL					hgRSRC;
	VS_VERSION_INFO_HEAD*	pVVIH;
	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
	*pdwProductVersionMS = 0;
	*pdwProductVersionLS = 0;
	static bool bLoad = false;
	static DWORD dwVersionMS = 0;
	static DWORD dwVersionLS = 0;
	if( hInstance == NULL && bLoad ){
		*pdwProductVersionMS = dwVersionMS;
		*pdwProductVersionLS = dwVersionLS;
		return;
	}
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
		dwVersionMS = pVVIH->Value.dwProductVersionMS;
		dwVersionLS = pVVIH->Value.dwProductVersionLS;
	}
	if( hInstance == NULL ){
		bLoad = true;
	}
	return;

}




/** �t���[���E�B���h�E���A�N�e�B�u�ɂ���
	@date 2007.11.07 ryoji �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������D
		�i���[�_���_�C�A���O�⃁�b�Z�[�W�{�b�N�X��\�����Ă���悤�ȂƂ��j
*/
void ActivateFrameWindow( HWND hwnd )
{
	// �ҏW�E�B���h�E�Ń^�u�܂Ƃߕ\���̏ꍇ�͕\���ʒu�𕜌�����
	CShareData* pInstance = NULL;
	DLLSHAREDATA* pShareData = NULL;
	if( (pInstance = CShareData::getInstance()) && (pShareData = pInstance->GetShareData()) ){
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
			if( IsSakuraMainWindow( hwnd ) ){
				if( pShareData->m_bEditWndChanging )
					return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
				pShareData->m_bEditWndChanging = TRUE;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

				// �ΏۃE�B���h�E�̃X���b�h�Ɉʒu���킹���˗�����	// 2007.04.03 ryoji
				DWORD_PTR dwResult;
				::SendMessageTimeout(
					hwnd,
					MYWM_TAB_WINDOW_NOTIFY,
					TWNT_WNDPL_ADJUST,
					(LPARAM)NULL,
					SMTO_ABORTIFHUNG | SMTO_BLOCK,
					10000,
					&dwResult
				);
			}
		}
	}

	// �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������
	HWND hwndActivate;
	hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

	if( pShareData )
		pShareData->m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji

	return;
}




//@@@ 2002.01.24 Start by MIK
/*!
	������URL���ǂ�������������B
	
	@retval TRUE URL�ł���
	@retval FALSE URL�łȂ�
	
	@note �֐����ɒ�`�����e�[�u���͕K�� static const �錾�ɂ��邱��(���\�ɉe�����܂�)�B
		url_char �̒l�� url_table �̔z��ԍ�+1 �ɂȂ��Ă��܂��B
		�V���� URL ��ǉ�����ꍇ�� #define �l���C�����Ă��������B
		url_table �͓��������A���t�@�x�b�g���ɂȂ�悤�ɕ��ׂĂ��������B
*/
BOOL IsURL(
	const char*	pszLine,	//!< [in] ������
	int			nLineLen,	//!< [in] ������̒���
	int*		pnMatchLen	//!< [out] URL�̒���
)
{
	struct _url_table_t {
		char	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* �A���t�@�x�b�g�� */
		"file://",		7,	false, /* 1 */
		"ftp://",		6,	false, /* 2 */
		"gopher://",	9,	false, /* 3 */
		"http://",		7,	false, /* 4 */
		"https://",		8,	false, /* 5 */
		"mailto:",		7,	true,  /* 6 */
		"news:",		5,	false, /* 7 */
		"nntp://",		7,	false, /* 8 */
		"prospero://",	11,	false, /* 9 */
		"telnet://",	9,	false, /* 10 */
		"tp://",		5,	false, /* 11 */	//2004.02.02
		"ttp://",		6,	false, /* 12 */	//2004.02.02
		"wais://",		7,	false, /* 13 */
		"{",			0,	false  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* �e�[�u���̕ێ琫�����߂邽�߂̒�` */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* ����128�o�C�g�]���ɂ����if����2�ӏ��폜�ł��� */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const unsigned char	*p = (const unsigned char*)pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( *p & 0x80 ) return FALSE;	/* 2�o�C�g���� */
	if( 0 < url_char[*p] ){	/* URL�J�n���� */
		for(urlp = &url_table[url_char[*p]-1]; urlp->name[0] == *p; urlp++){	/* URL�e�[�u����T�� */
			if( (urlp->length <= nLineLen) && (memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URL�w�b�_�͈�v���� */
				p += urlp->length;	/* URL�w�b�_�����X�L�b�v���� */
				if( urlp->is_mail ){	/* ���[����p�̉�͂� */
					if( IsMailAddress((const char*)p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* �ʏ�̉�͂� */
					if( (*p & 0x80) || (!(url_char[*p])) ) break;	/* �I�[�ɒB���� */
				}
				if( i == urlp->length ) return FALSE;	/* URL�w�b�_���� */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
}

/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
BOOL IsMailAddress( const char* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	int		j;
	int		nDotCount;
	int		nBgn;


	j = 0;
	if( (pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	 || (pszBuf[j] == '.')
	 || (pszBuf[j] == '-')
	 || (pszBuf[j] == '_')
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( '@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	while( 1 ){
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
		 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
		 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
		 || (pszBuf[j] == '-')
		 || (pszBuf[j] == '_')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( '.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}




//@@@ 2001.11.07 Start by MIK
//#ifdef COMPILE_COLOR_DIGIT
/*
 * ���l�Ȃ璷����Ԃ��B
 * 10�i���̐����܂��͏����B16�i��(����)�B
 * ������   ���l(�F����)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (�������\�[�X����#if��L���ɂ����Ƃ�)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (�������\�[�X����#if��L���ɂ�����"0.56.1"�ɂȂ�)
 * .5       5        (�������\�[�X����#if��L���ɂ�����".5"�ɂȂ�)
 * -.5      5        (�������\�[�X����#if��L���ɂ�����"-.5"�ɂȂ�)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * ���p���l
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10�i��, 16�i��, LF�ڔ���, ���������_��, ������
 *   IP�A�h���X�̃h�b�g�A��(�{���͐��l����Ȃ��񂾂��)
 */
int IsNumber(const char *buf, int offset, int length)
{
	register const char *p;
	register const char *q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = &buf[offset];
	q = &buf[length];

	if( *p == '0' )  /* 10�i��,C��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'x' ) )  /* C��16�i�� */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" �Ȃ� "0" ���������l */
			if( i == 2 ) return 1;
			
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == 'L' || *p == 'l' || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= '0' && *p <= '9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == '.' )
		{
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == 'E' || *p == 'e' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( ( *p == '+' || *p == '-' ) && ( *(p - 1) == 'E' || *(p - 1) == 'e' ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < '0' || *(p + 1) > '9' )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" �Ȃ� "0" �����l */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" ���������l */
			/*if( *p == '.' ) return i - 1;*/  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= '1' && *p <= '9' )  /* 10�i�� */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '-' )  /* �}�C�i�X */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." �����Ȃ琔�l�łȂ� */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		if( i == 1 ) return 0;
		if( *(p - 1) == '.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '.' )  /* �����_ */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." �����Ȃ琔�l�łȂ� */
		if( i == 1 ) return 0;
		if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == '&' )  /* VB��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'H' ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" �����Ȃ琔�l�łȂ� */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" �����Ȃ琔�l�łȂ� */
		return 0;
	}
#endif

	/* ���l�ł͂Ȃ� */
	return 0;
}
//@@@ 2001.11.07 End by MIK

bool fexist(LPCTSTR pszPath)
{
	return _taccess(pszPath,0)!=-1;
}

/**	�t�@�C���̑��݃`�F�b�N

	�w�肳�ꂽ�p�X�̃t�@�C�������݂��邩�ǂ������m�F����B
	
	@param path [in] ���ׂ�p�X��
	@param bFileOnly [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
	
	@retval true  �t�@�C���͑��݂���
	@retval false �t�@�C���͑��݂��Ȃ�
	
	@author genta
	@date 2002.01.04 �V�K�쐬
*/
bool IsFileExists(const TCHAR* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( path, &fd );
	if( hFind != INVALID_HANDLE_VALUE ){
		::FindClose( hFind );
		if( bFileOnly == false || ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			return true;
	}
	return false;
}

/**	�t�@�C�����̐؂�o��

	�w�蕶���񂩂�t�@�C�����ƔF������镶��������o���A
	�擪Offset�y�ђ�����Ԃ��B
	
	@retval true �t�@�C��������
	@retval false �t�@�C�����͌�����Ȃ�����
	
	@date 2002.01.04 genta �f�B���N�g���������ΏۊO�ɂ���@�\��ǉ�
	@date 2003.01.15 matsumo gcc�̃G���[���b�Z�[�W(:��؂�)�ł��t�@�C�������o�\��
	@date 2004.05.29 genta C:\����t�@�C��C���؂�o�����̂�h�~
	@date 2004.11.13 genta/Moca �t�@�C�����擪��*?���l��
	@date 2005.01.10 genta �ϐ����ύX j -> cur_pos
	@date 2005.01.23 genta �x���}���̂��߁Cgoto��return�ɕύX
	
*/
bool IsFilePath(
	const char* pLine,		//!< [in] �T���Ώە�����
	int* pnBgn,				//!< [out] �擪offset�BpLine + *pnBgn���t�@�C�����擪�ւ̃|�C���^�B
	int* pnPathLen,			//!< [out] �t�@�C�����̒���
	bool bFileOnly			//!< [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
)
{
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, _countof( szJumpToFile ) );

	int nLineLen = strlen( pLine );

	//�擪�̋󔒂�ǂݔ�΂�
	int		i;
	for( i = 0; i < nLineLen; ++i ){
		if( ' ' != pLine[i] && '\t' != pLine[i] && '\"' != pLine[i] ){
			break;
		}
	}

	//	#include <�t�@�C����>�̍l��
	//	#�Ŏn�܂�Ƃ���"�܂���<�܂œǂݔ�΂�
	if( i < nLineLen && '#' == pLine[i] ){
		for( ; i < nLineLen; ++i ){
			if( '<'  == pLine[i] || '\"' == pLine[i] ){
				++i;
				break;
			}
		}
	}

	//	���̎��_�Ŋ��ɍs���ɒB���Ă�����t�@�C�����͌�����Ȃ�
	if( i >= nLineLen ){
		return false;
	}

	*pnBgn = i;
	int cur_pos = 0;
	for( ; i <= nLineLen && cur_pos + 1 < _countof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == ' '  ||
			  pLine[i] == '\t' ||	//@@@ 2002.01.08 YAZAKI �^�u�������B
			  pLine[i] == '('  ||
			  pLine[i] == '\r' ||
			  pLine[i] == '\n' ||
			  pLine[i] == '\0' ||
			  pLine[i] == '>'  ||
			  // May 29, 2004 genta C:\��:�̓t�@�C����؂�ƌ��Ȃ��ė~�����Ȃ�
			  ( cur_pos > 1 && pLine[i] == ':' ) ||   //@@@ 2003/1/15/ matsumo (for gcc)
			  pLine[i] == '"'
			) &&
			0 < strlen( szJumpToFile )
		){
			if( IsFileExists(szJumpToFile, bFileOnly))
			{
				i--;
				break;
			}
		}
		if( pLine[i] == '\r'  ||
			pLine[i] == '\n' ){
			break;
		}

		if( ( /*pLine[i] == '/' ||*/
			 pLine[i] == '<' ||	//	0x3C
			 pLine[i] == '>' ||	//	0x3E
			 pLine[i] == '?' ||	//	0x3F
			 pLine[i] == '"' ||	//	0x22
			 pLine[i] == '|' ||	//	0x7C
			 pLine[i] == '*'	//	0x2A
			) &&
			/* ��̕�����SJIS2�o�C�g�R�[�h��2�o�C�g�ڂłȂ����Ƃ��A1�O�̕�����SJIS2�o�C�g�R�[�h��1�o�C�g�ڂłȂ����ƂŔ��f���� */
			//	Oct. 5, 2002 genta
			//	2004.11.13 Moca/genta �擪�ɏ�̕���������ꍇ�̍l����ǉ�
			( i == 0 || ( i > 0 && ! _IS_SJIS_1( (unsigned char)pLine[i - 1] ))) ){
			return false;
		}else{
		szJumpToFile[cur_pos] = pLine[i];
		cur_pos++;
		}
	}

	if( 0 < strlen( szJumpToFile ) && IsFileExists(szJumpToFile, bFileOnly)){
		*pnPathLen = strlen( szJumpToFile );
		return true;
	}

	return false;
}

/*!
	���[�J���h���C�u�̔���

	@param[in] pszDrive �h���C�u�����܂ރp�X��
	
	@retval true ���[�J���h���C�u
	@retval false �����[�o�u���h���C�u�D�l�b�g���[�N�h���C�u�D
	
	@author MIK
	@date 2001.03.29 MIK �V�K�쐬
	@date 2001.12.23 YAZAKI MRU�̕ʃN���X���ɔ����֐���
	@date 2002.01.28 genta �߂�l�̌^��BOOL����bool�ɕύX�D
	@date 2005.11.12 aroka �������蕔�ύX
	@date 2006.01.08 genta CMRU::IsRemovableDrive��CEditDoc::IsLocalDrive��
		�����I�ɓ������̂�����
*/
bool IsLocalDrive( const TCHAR* pszDrive )
{
	TCHAR	szDriveType[_MAX_DRIVE+1];	// "A:\ "�o�^�p
	long	lngRet;

	if( isalpha(pszDrive[0]) ){
		sprintf(szDriveType, _T("%c:\\"), toupper(pszDrive[0]));
		lngRet = GetDriveType( szDriveType );
		if( lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE )
		{
			return false;
		}
	}
	return true;
}

void GetLineColm( const char* pLine, int* pnJumpToLine, int* pnJumpToColm )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szNumber[32];
	nLineLen = strlen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= '0' &&
			pLine[i] <= '9' ){
			break;
		}
	}
	memset( szNumber, 0, _countof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* �s�ʒu ���s�P�ʍs�ԍ�(1�N�_)�̒��o */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= '0' &&
				pLine[i] <= '9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = atoi( szNumber );

		/* ���ʒu ���s�P�ʍs�擪����̃o�C�g��(1�N�_)�̒��o */
		if( i < nLineLen && pLine[i] == ',' ){
			memset( szNumber, 0, sizeof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= '0' &&
					pLine[i] <= '9' ){
					continue;
				}
				break;
			}
			*pnJumpToColm = atoi( szNumber );
		}
	}
	return;
}




/* CR0LF0,CRLF,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEol*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* ���s�R�[�h�������� */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			 pcEol->GetTypeFromString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}




/*! �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪����

	@param pText [in] �؂蕪���ΏۂƂȂ镶����ւ̃|�C���^
	@param nTextLen [in] �؂蕪���ΏۂƂȂ镶����S�̂̒���
	@param nLimitLen [in] �؂蕪���钷��
	@param pnLineLen [out] ���ۂɎ��o���ꂽ������̒���
	@param pnBgn [i/o] ����: �؂蕪���J�n�ʒu, �o��: ���o���ꂽ������̎��̈ʒu

	@note 2003.05.25 ���g�p�̂悤��
*/
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pText, nTextLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}




/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
int LimitStringLengthB( const char* pszData, int nDataLength, int nLimitLengthB, CMemory& cmemDes )
{
	int	i;
	int	nCharChars;
	int	nDesLen;
	nDesLen = 0;
	for( i = 0; i < nDataLength; ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pszData, nDataLength, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( nDesLen + nCharChars > nLimitLengthB ){
			break;
		}
		nDesLen += nCharChars;
		i += nCharChars;
	}
	cmemDes.SetString( pszData, nDesLen );
	return nDesLen;
}




/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ� */
void CutLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	return;
}




/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
void AddLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
			}else{
				pszFolder[nFolderLen] = _T('\\');
				pszFolder[nFolderLen + 1] = _T('\0');
			}
		}
	}
	return;
}


/*!	�����񂪎w�肳�ꂽ�����ŏI����Ă��Ȃ������ꍇ�ɂ�
	�����ɂ��̕�����t������D

	@param pszPath [i/o]���삷�镶����
	@param nMaxLen [in]�o�b�t�@��
	@param c [in]�ǉ�����������
	@retval  0 \��������t���Ă���
	@retval  1 \��t������
	@retval -1 �o�b�t�@�����肸�A\��t���ł��Ȃ�����
	@date 2003.06.24 Moca �V�K�쐬
*/
int AddLastChar( TCHAR* pszPath, int nMaxLen, TCHAR c ){
	int pos = _tcslen( pszPath );
	// �����Ȃ��Ƃ���\��t��
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = _T('\0');
		return 1;
	}
	// �Ōオ\�łȂ��Ƃ���\��t��(���{����l��)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = _T('\0');
		return 1;
	}
	return 0;
}


/*! �V���[�g�J�b�g(.lnk)�̉���
	@date 2009.01.08 ryoji CoInitialize/CoUninitialize���폜�iWinMain��OleInitialize/OleUninitialize��ǉ��j
*/
BOOL ResolveShortcutLink( HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath )
{
	BOOL			bRes;
	HRESULT			hRes;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;
	WIN32_FIND_DATA	wfd;
	/* ������ */
	pIShellLink = NULL;
	pIPersistFile = NULL;
	*lpszPath = 0; // assume failure
	bRes = FALSE;

// 2009.01.08 ryoji CoInitialize���폜�iWinMain��OleInitialize�ǉ��j

	// Get a pointer to the IShellLink interface.
//	hRes = 0;
	TCHAR szAbsLongPath[_MAX_PATH];
	if( ! ::GetLongFileName( lpszLinkFile, szAbsLongPath ) ){
		return FALSE;
	}

	// 2010.08.28 DLL �C���W�F�N�V�����΍�Ƃ���EXE�̃t�H���_�Ɉړ�����
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	if( SUCCEEDED( hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink ) ) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile ) ) ){
			// Ensure that the string is Unicode.
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar( CP_ACP, 0, szAbsLongPath, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load( wsz, STGM_READ ) ) ){
				// Resolve the link.
				if( SUCCEEDED( hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH ) ) ){
					// Get the path to the link target.
					TCHAR szGotPath[MAX_PATH];
					szGotPath[0] = _T('\0');
					if( SUCCEEDED( hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH ) ) ){
						// Get the description of the target.
						TCHAR szDescription[MAX_PATH];
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH ) ) ){
							if( _T('\0') != szGotPath[0] ){
								/* ����I�� */
								_tcscpy( lpszPath, szGotPath );
								bRes = TRUE;
							}
						}
					}
				}
			}
		}
	}
	// Release the pointer to the IPersistFile interface.
	if( NULL != pIPersistFile ){
		pIPersistFile->Release();
		pIPersistFile = NULL;
	}
	// Release the pointer to the IShellLink interface.
	if( NULL != pIShellLink ){
		pIShellLink->Release();
		pIShellLink = NULL;
	}
// 2009.01.08 ryoji CoUninitialize���폜�iWinMain��OleUninitialize�ǉ��j
	return bRes;
}




void ResolvePath(TCHAR* pszPath)
{
	// pszPath -> pSrc
	TCHAR* pSrc = pszPath;

	// �V���[�g�J�b�g(.lnk)�̉���: pSrc -> szBuf -> pSrc
	TCHAR szBuf[_MAX_PATH];
	if( ResolveShortcutLink( NULL, pSrc, szBuf ) ){
		pSrc = szBuf;
	}

	// �����O�t�@�C�������擾����: pSrc -> szBuf2 -> pSrc
	TCHAR szBuf2[_MAX_PATH];
	if( ::GetLongFileName( pSrc, szBuf2 ) ){
		pSrc = szBuf2;
	}

	// pSrc -> pszPath
	if(pSrc != pszPath){
		_tcscpy(pszPath, pSrc);
	}
}




/*!
	�������̃��[�U�[������\�ɂ���
	�u���b�L���O�t�b�N(?)�i���b�Z�[�W�z��

	@date 2003.07.04 genta ���̌Ăяo���ŕ������b�Z�[�W����������悤��
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		//	Jun. 04, 2003 genta ���b�Z�[�W�����邾����������悤��
		while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
			if ( msg.message == WM_QUIT ){
				return FALSE;
			}
			if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
			}else{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		return TRUE/*ret*/;
}

/*!�N���[�v�{�[�h��Text�`���ŃR�s�[����
	@param hwnd [in] �N���b�v�{�[�h�̃I�[�i�[
	@param pszText [in] �ݒ肷��e�L�X�g
	@param length [in] �L���ȃe�L�X�g�̒���
	
	@retval true �R�s�[����
	@retval false �R�s�[���s�B�ꍇ�ɂ���Ă̓N���b�v�{�[�h�Ɍ��̓��e���c��
	@date 2004.02.17 Moca �e���̃\�[�X�𓝍�
*/
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const char* pszText, int nLength )
{
	HGLOBAL		hgClip;
	char*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, nLength + 1 );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (char*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	memcpy( pszClip, pszText, nLength );
	pszClip[nLength] = 0;
	::GlobalUnlock( hgClip );
	if( !::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	::SetClipboardData( CF_OEMTEXT, hgClip );
	::CloseClipboard();

	return true;
}

/*!
	�^����ꂽ���K�\�����C�u�����̏��������s���D
	���b�Z�[�W�t���O��ON�ŏ������Ɏ��s�����Ƃ��̓��b�Z�[�W��\������D

	@retval true ����������
	@retval false �������Ɏ��s

	@date 2007.08.12 genta ���ʐݒ肩��DLL�����擾����
*/
bool InitRegexp(
	HWND		hWnd,			//!< [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	CBregexp&	rRegexp,		//!< [in] �`�F�b�N�ɗ��p����CBregexp�N���X�ւ̎Q��
	bool		bShowMessage	//!< [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
)
{
	//	From Here 2007.08.12 genta
	CShareData* pInstance = NULL;
	DLLSHAREDATA* pShareData = NULL;
	
	LPCTSTR RegexpDll = _T("");
	
	if( (pInstance = CShareData::getInstance()) && (pShareData = pInstance->GetShareData()) ){
		RegexpDll = pShareData->m_Common.m_szRegexpLib;
	}
	//	To Here 2007.08.12 genta

	if( !rRegexp.Init( RegexpDll ) ){
		if( bShowMessage ){
			WarningBeep();
			::MessageBox( hWnd, _T("���K�\�����C�u������������܂���B\r\n"
				"���K�\���𗘗p����ɂ�BREGEXP.DLL�݊��̃��C�u�������K�v�ł��B\r\n"
				"������@�̓w���v���Q�Ƃ��Ă��������B"),
				GSTR_APPNAME, MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}

/*!
	���K�\�����C�u�����̑��݂��m�F���A����΃o�[�W���������w��R���|�[�l���g�ɃZ�b�g����B
	���s�����ꍇ�ɂ͋󕶎�����Z�b�g����B

	@retval true �o�[�W�����ԍ��̐ݒ�ɐ���
	@retval false ���K�\�����C�u�����̏������Ɏ��s
*/
bool CheckRegexpVersion(
	HWND	hWnd,			//!< [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	int		nCmpId,			//!< [in] �o�[�W�����������ݒ肷��R���|�[�l���gID
	bool	bShowMessage	//!< [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
)
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		if( hWnd != NULL ){
			::SetDlgItemText( hWnd, nCmpId, _T(" "));
		}
		return false;
	}
	if( hWnd != NULL ){
		::SetDlgItemText( hWnd, nCmpId, cRegexp.GetVersionT() );
	}
	return true;
}

/*!
	���K�\�����K���ɏ]���Ă��邩���`�F�b�N����B

	@retval true ���K�\���͋K���ʂ�
	@retval false ���@�Ɍ�肪����B�܂��́A���C�u�������g�p�ł��Ȃ��B
*/
bool CheckRegexpSyntax(
	const char*	szPattern,		//!< [in] �`�F�b�N���鐳�K�\��
	HWND		hWnd,			//!< [in] ���b�Z�[�W�{�b�N�X�̐e�E�B���h�E
	bool		bShowMessage,	//!< [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
	int			nOption			//!< [in] �啶���Ə������𖳎����Ĕ�r����t���O // 2002/2/1 hor�ǉ�
)
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern, nOption ) ){	// 2002/2/1 hor�ǉ�
		if( bShowMessage ){
			::MessageBox( hWnd, cRegexp.GetLastMessage(),
				_T("���K�\���G���["), MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}

//	From Here Jun. 26, 2001 genta
/*!
	HTML Help�R���|�[�l���g�̃A�N�Z�X��񋟂���B
	�����ŕێ����ׂ��f�[�^�͓��ɂȂ��A���鏊����g����̂�Global�ϐ��ɂ��邪�A
	���ڂ̃A�N�Z�X��OpenHtmlHelp()�֐��݂̂���s���B
	���̃t�@�C�������CHtmlHelp�N���X�͉B����Ă���B
*/
CHtmlHelp g_cHtmlHelp;

/*!
	HTML Help���J��
	HTML Help�����p�\�ł���Έ��������̂܂ܓn���A�����łȂ���΃��b�Z�[�W��\������B

	@return �J�����w���v�E�B���h�E�̃E�B���h�E�n���h���B�J���Ȃ������Ƃ���NULL�B
*/

HWND OpenHtmlHelp(
	HWND		hWnd,		//!< [in] �Ăяo�����E�B���h�E�̃E�B���h�E�n���h��
	LPCTSTR		szFile,		//!< [in] HTML Help�̃t�@�C�����B�s�����ɑ����ăE�B���h�E�^�C�v�����w��\�B
	UINT		uCmd,		//!< [in] HTML Help �ɓn���R�}���h
	DWORD_PTR	data,		//!< [in] �R�}���h�ɉ������f�[�^
	bool		msgflag		//!< [in] �G���[���b�Z�[�W��\�����邩�B�ȗ�����true�B
)
{
	if( g_cHtmlHelp.Init() ){
		return g_cHtmlHelp.HtmlHelp( hWnd, szFile, uCmd, data );
	}
	if( msgflag ){
		::MessageBox(
			hWnd,
			_T("HHCTRL.OCX��������܂���B\r\n")
			_T("HTML�w���v�𗘗p����ɂ�HHCTRL.OCX���K�v�ł��B\r\n"),
			_T("���"),
			MB_OK | MB_ICONEXCLAMATION
		);
	}
	return NULL;
}


//	To Here Jun. 26, 2001 genta



// Stonee, 2001/12/21
// NetWork��̃��\�[�X�ɐڑ����邽�߂̃_�C�A���O���o��������
// NO_ERROR:���� ERROR_CANCELLED:�L�����Z�� ����ȊO:���s
// �v���W�F�N�g�̐ݒ�Ń����N���W���[����Mpr.lib��ǉ��̂���
DWORD NetConnect ( const TCHAR strNetWorkPass[] )
{
	//char sPassWord[] = "\0";	//�p�X���[�h
	//char sUser[] = "\0";		//���[�U�[��
	DWORD dwRet;				//�߂�l�@�G���[�R�[�h��WINERROR.H���Q��
	TCHAR sTemp[256];
	TCHAR sDrive[] = _T("");
    int i;

	if (_tcslen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B
	if (strNetWorkPass[0] != _T('\\') && strNetWorkPass[1] != _T('\\'))	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B

	//3�����ڂ��琔���čŏ���\�̒��O�܂ł�؂�o��
	sTemp[0] = _T('\\');
	sTemp[1] = _T('\\');
	for (i = 2; strNetWorkPass[i] != _T('\0'); i++) {
		if (strNetWorkPass[i] == _T('\\')) break;
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = _T('\0');	//�I�[

	//NETRESOURCE�쐬
	NETRESOURCE nr;
	ZeroMemory( &nr, sizeof( nr ) );
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwType = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName = sDrive;
	nr.lpRemoteName = sTemp;

	//���[�U�[�F�؃_�C�A���O��\��
	dwRet = WNetAddConnection3(0, &nr, NULL, NULL, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);

	return dwRet;
}

/*! �����̃G�X�P�[�v

	@param org [in] �ϊ�������������
	@param buf [out] �ԊҌ�̕����������o�b�t�@
	@param cesc  [in] �G�X�P�[�v���Ȃ��Ƃ����Ȃ�����
	@param cwith [in] �G�X�P�[�v�Ɏg������
	
	@retval �o�͂����o�C�g�� (Unicode�̏ꍇ�͕�����)

	�����񒆂ɂ��̂܂܎g���Ƃ܂�������������ꍇ�ɂ��̕����̑O��
	�G�X�P�[�v�L�����N�^��}�����邽�߂Ɏg���D

	@note �ϊ���̃f�[�^�͍ő�Ō��̕������2�{�ɂȂ�
	@note ���̊֐���2�o�C�g�����̍l�����s���Ă��Ȃ�

	@author genta
	@date 2002/01/04 �V�K�쐬
	@date 2002/01/30 genta &��p(dupamp)�����ʂ̕�����������悤�Ɋg���D
		dupamp��inline�֐��ɂ����D
	@date 2002/02/01 genta bugfix �G�X�P�[�v���镶���Ƃ���镶���̏o�͏������t������
	@date 2004/06/19 genta Generic mapping�Ή�
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}

/*! �����̃G�X�P�[�v

	@param org [in] �ϊ�������������
	@param buf [out] �ԊҌ�̕����������o�b�t�@
	@param cesc  [in] �G�X�P�[�v���Ȃ��Ƃ����Ȃ�����
	@param cwith [in] �G�X�P�[�v�Ɏg������
	
	@retval �o�͂����o�C�g��

	�����񒆂ɂ��̂܂܎g���Ƃ܂�������������ꍇ�ɂ��̕����̑O��
	�G�X�P�[�v�L�����N�^��}�����邽�߂Ɏg���D

	@note �ϊ���̃f�[�^�͍ő�Ō��̕������2�{�ɂȂ�
	@note ���̊֐���2�o�C�g�����̍l�����s���Ă���
	
	@note 2003.05.25 ���g�p�̂悤��
*/
int cescape_j(const char* org, char* buf, char cesc, char cwith)
{
	char *out = buf;
	for( ; *org != '\0'; ++org, ++out ){
		if( _IS_SJIS_1( (unsigned char)*org ) ){
			*out = *org;
			++out; ++org;
		}
		else if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = '\0';
	return out - buf;
}

/*	�w���v�̖ڎ���\��
	�ڎ��^�u��\���B��肪����o�[�W�����ł́A�ڎ��y�[�W��\���B
*/
void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp )
{
	COsVersionInfo cOsVer;
	if ( cOsVer.HasWinHelpContentsProblem() ){
		/* �ڎ��y�[�W��\������ */
		MyWinHelp( hwnd, lpszHelp, HELP_CONTENTS , 0 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return;
	}
	/* �ڎ��^�u��\������ */
	MyWinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return;
}


/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
SAKURA_CORE_API int GetColorIndexByName( const char *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( strcmp( name, (const char*)g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * �C���f�b�N�X�ԍ�����J���[���ɕϊ�����
 */
SAKURA_CORE_API const char* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}

/*!
	@brief ���W�X�g�����當�����ǂݏo���D
	
	@param Hive [in] HIVE
	@param Path [in] ���W�X�g���L�[�ւ̃p�X
	@param Item [in] ���W�X�g���A�C�e�����DNULL�ŕW���̃A�C�e���D
	@param Buffer [out] �擾��������i�[����ꏊ
	@param BufferSize [in] Buffer�̎w���̈�̃T�C�Y
	
	@retval true �l�̎擾�ɐ���
	@retval false �l�̎擾�Ɏ��s
	
	@author �S
	@date 2002.09.10 genta CWSH.cpp����ړ�
*/
bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferSize)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		ZeroMemory(Buffer, BufferSize);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = BufferSize - 1;
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<unsigned char*>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}

/*!
	@brief exe�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��D
	
	@author genta
	@date 2002.12.02 genta
	@date 2007.05.20 ryoji �֐����ύX�i���FGetExecutableDir�j�A�ėp�e�L�X�g�}�b�s���O��
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void GetExedir(
	LPTSTR pDir,		//!< [out] EXE�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D�\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR szFile		//!< [in] �f�B���N�g�����Ɍ�������t�@�C�����D  
)
{
	if( pDir == NULL )
		return;
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.exe �̃p�X���擾
	::GetModuleFileName( NULL, szPath, _countof(szPath) );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		_snprintf( pDir, _MAX_PATH, _T("%s\\%s"), szDir, szFile );
		pDir[_MAX_PATH - 1] = _T('\0');
	}
}

/*!
	@brief INI�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��D
	
	@author ryoji
	@date 2007.05.19 �V�K�쐬�iGetExedir�x�[�X�j
*/
void GetInidir(
	LPTSTR pDir,				//!< [out] INI�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D�\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR szFile	/*=NULL*/	//!< [in] �f�B���N�g�����Ɍ�������t�@�C�����D
)
{
	if( pDir == NULL )
		return;
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.ini �̃p�X���擾
	CShareData::getInstance()->GetIniFileName( szPath );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		_snprintf( pDir, _MAX_PATH, _T("%s\\%s"), szDir, szFile );
		pDir[_MAX_PATH - 1] = _T('\0');
	}
}


/*!
	@brief INI�t�@�C���܂���EXE�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��iINI��D��j�D
	
	@author ryoji
	@date 2007.05.22 �V�K�쐬
*/
void GetInidirOrExedir(
	LPTSTR pDir,							//!< [out] INI�t�@�C���܂���EXE�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D
											//         �\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR szFile				/*=NULL*/,	//!< [in] �f�B���N�g�����Ɍ�������t�@�C�����D
	BOOL bRetExedirIfFileEmpty	/*=FALSE*/	//!< [in] �t�@�C�����̎w�肪��̏ꍇ��EXE�t�@�C���̃t���p�X��Ԃ��D	
)
{
	TCHAR	szInidir[_MAX_PATH];
	TCHAR	szExedir[_MAX_PATH];

	// �t�@�C�����̎w�肪��̏ꍇ��EXE�t�@�C���̃t���p�X��Ԃ��i�I�v�V�����j
	if( bRetExedirIfFileEmpty && (szFile == NULL || szFile[0] == _T('\0')) ){
		GetExedir( szExedir, szFile );
		::lstrcpy( pDir, szExedir );
		return;
	}

	// INI��̃t���p�X�����݂���΂��̃p�X��Ԃ�
	GetInidir( szInidir, szFile );
	if( fexist(szInidir) ){
		::lstrcpy( pDir, szInidir );
		return;
	}

	// EXE��̃t���p�X�����݂���΂��̃p�X��Ԃ�
	if( CShareData::getInstance()->IsPrivateSettings() ){	// INI��EXE�Ńp�X���قȂ�ꍇ
		GetExedir( szExedir, szFile );
		if( fexist(szExedir) ){
			::lstrcpy( pDir, szExedir );
			return;
		}
	}

	// �ǂ���ɂ����݂��Ȃ����INI��̃t���p�X��Ԃ�
	::lstrcpy( pDir, szInidir );
}


/*!
	@brief �A�v���P�[�V�����A�C�R���̎擾
	
	�A�C�R���t�@�C�������݂���ꍇ�͂�������C�����ꍇ��
	���\�[�X�t�@�C������擾����
	
	@param hInst [in] Instance Handle
	@param nResource [in] �f�t�H���g�A�C�R���pResource ID
	@param szFile [in] �A�C�R���t�@�C����
	@param bSmall [in] true: small icon (16x16) / false: large icon (32x32)
	
	@return �A�C�R���n���h���D���s�����ꍇ��NULL�D
	
	@date 2002.12.02 genta �V�K�쐬
	@data 2007.05.20 ryoji ini�t�@�C���p�X��D��
	@author genta
*/
HICON GetAppIcon( HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall )
{
	// �T�C�Y�̐ݒ�
	int size = ( bSmall ? 16 : 32 );

	TCHAR szPath[_MAX_PATH];
	HICON hIcon;

	// �t�@�C������̓ǂݍ��݂��܂����݂�
	GetInidirOrExedir( szPath, szFile );

	hIcon = (HICON)::LoadImage(
		NULL,
		szPath,
		IMAGE_ICON,
		size,
		size,
		LR_SHARED | LR_LOADFROMFILE
	);
	if( hIcon != NULL ){
		return hIcon;
	}

	//	�t�@�C������̓ǂݍ��݂Ɏ��s�����烊�\�[�X����擾
	hIcon = (HICON)::LoadImage(
		hInst,
		MAKEINTRESOURCE(nResource),
		IMAGE_ICON,
		size,
		size,
		LR_SHARED
	);
	
	return hIcon;
}

/*! fname�����΃p�X�̏ꍇ�́A���s�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author Moca
	@date 2003.06.23
	@date 2007.05.20 ryoji �֐����ύX�i���Ffopen_absexe�j�A�ėp�e�L�X�g�}�b�s���O��
*/
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode)
{
	if( _IS_REL_PATH( fname ) ){
		TCHAR path[_MAX_PATH];
		GetExedir( path, fname );
		return _tfopen( path, mode );
	}
	return _tfopen( fname, mode );
	
}

/*! fname�����΃p�X�̏ꍇ�́AINI�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author ryoji
	@date 2007.05.19 �V�K�쐬�i_tfopen_absexe�x�[�X�j
*/
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, BOOL bOrExedir/*=TRUE*/ )
{
	if( _IS_REL_PATH( fname ) ){
		TCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		return _tfopen( path, mode );
	}
	return _tfopen( fname, mode );
}


/*! �����������@�\�t��strncpy

	�R�s�[��̃o�b�t�@�T�C�Y������Ȃ��悤��strncpy����B
	�o�b�t�@���s������ꍇ�ɂ�2�o�C�g�����̐ؒf�����蓾��B
	������\0�͕t�^����Ȃ����A�R�s�[�̓R�s�[��o�b�t�@�T�C�Y-1�܂łɂ��Ă����B

	@param dst [in] �R�s�[��̈�ւ̃|�C���^
	@param dst_count [in] �R�s�[��̈�̃T�C�Y
	@param src [in] �R�s�[��
	@param src_count [in] �R�s�[���镶����̖���

	@retval ���ۂɃR�s�[���ꂽ�R�s�[��̈��1����w���|�C���^

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	memcpy( dst, src, src_count );
	return dst + src_count;
}

/*! @brief �f�B���N�g���̐[�����v�Z����

	�^����ꂽ�p�X������f�B���N�g���̐[�����v�Z����D
	�p�X�̋�؂��\�D���[�g�f�B���N�g�����[��0�ŁC�T�u�f�B���N�g������
	�[����1���オ���Ă����D
 
	@date 2003.04.30 genta �V�K�쐬
*/
int CalcDirectoryDepth(
	const TCHAR* path	//!< [in] �[���𒲂ׂ����t�@�C��/�f�B���N�g���̃t���p�X
)
{
	int depth = 0;
 
	//	�Ƃ肠����\�̐��𐔂���
	for( const char *p = path; *p != _T('\0'); ++p ){
		//	2�o�C�g�����͋�؂�ł͂Ȃ�
		if( _IS_SJIS_1(*(unsigned const char*)p)){ // unsigned��cast���Ȃ��Ɣ�������
			++p;
			if( *p == '\0' )
				break;
		}
		else if( *p == _T('\\') ){
			++depth;
			//	�t���p�X�ɂ͓����Ă��Ȃ��͂������O�̂���
			//	.\�̓J�����g�f�B���N�g���Ȃ̂ŁC�[���Ɋ֌W�Ȃ��D
			while( p[1] == _T('.') && p[2] == _T('\\') ){
				p += 2;
			}
		}
	}
 
	//	�␳
	//	�h���C�u���̓p�X�̐[���ɐ����Ȃ�
	if(( _T('A') <= (path[0] & ~0x20)) && ((path[0] & ~0x20) <= _T('Z') ) && path[1] == _T(':') && path[2] == _T('\\') ){
		//�t���p�X
		--depth; // C:\ �� \ �̓��[�g�̋L���Ȃ̂ŊK�w�[���ł͂Ȃ�
	}
	else if( path[0] == _T('\\') ){
		if( path[1] == _T('\\') ){
			//	�l�b�g���[�N�p�X
			//	�擪��2�̓l�b�g���[�N��\���C���̎��̓z�X�g���Ȃ̂�
			//	�f�B���N�g���K�w�Ƃ͖��֌W
			depth -= 3;
		}
		else {
			//	�h���C�u�������̃t���p�X
			//	�擪��\�͑ΏۊO
			--depth;
		}
	}
	return depth;
}


/*!
	�w�肵���E�B���h�E�^�����`�̈�^�_�^���j�^�ɑΉ����郂�j�^��Ɨ̈���擾����

	���j�^��Ɨ̈�F��ʑS�̂���V�X�e���̃^�X�N�o�[��A�v���P�[�V�����̃c�[���o�[����L����̈���������̈�

	@param hWnd/prc/pt/hMon [in] �ړI�̃E�B���h�E�^�����`�̈�^�_�^���j�^
	@param prcWork [out] ���j�^��Ɨ̈�
	@param prcMonitor [out] ���j�^��ʑS��

	@retval true �Ή����郂�j�^�̓v���C�}�����j�^
	@retval false �Ή����郂�j�^�͔�v���C�}�����j�^

	@note �o�̓p�����[�^�� prcWork �� prcMonior �� NULL ���w�肵���ꍇ�A
	�Y������̈���͏o�͂��Ȃ��B�Ăяo�����͗~�������̂������w�肷��΂悢�B
*/
//	From Here May 01, 2004 genta MutiMonitor
bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	// 2006.04.21 ryoji Windows API �`���̊֐��Ăяo���ɕύX�i�X�^�u�� PSDK �� MultiMon.h �𗘗p�j
	HMONITOR hMon = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}
//	To Here May 01, 2004 genta

//	From Here 2006.04.21 ryoji MutiMonitor
bool GetMonitorWorkRect(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromRect( prc, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(POINT pt, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	MONITORINFO mi;
	::ZeroMemory( &mi, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( hMon, &mi );
	if( NULL != prcWork )
		*prcWork = mi.rcWork;		// work area rectangle of the display monitor
	if( NULL != prcMonitor )
		*prcMonitor = mi.rcMonitor;	// display monitor rectangle
	return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
}
//	To Here 2006.04.21 ryoji MutiMonitor


/**	�w�肵���E�B���h�E�̑c��̃n���h�����擾����

	GetAncestor() API��Win95�Ŏg���Ȃ��̂ł��̂����

	WS_POPUP�X�^�C���������Ȃ��E�B���h�E�iex.CDlgFuncList�_�C�A���O�j���ƁA
	GA_ROOTOWNER�ł͕ҏW�E�B���h�E�܂ők��Ȃ��݂����BGetAncestor() API�ł����l�B
	�{�֐��ŗL�ɗp�ӂ���GA_ROOTOWNER2�ł͑k�邱�Ƃ��ł���B

	@author ryoji
	@date 2007.07.01 ryoji �V�K
	@date 2007.10.22 ryoji �t���O�l�Ƃ���GA_ROOTOWNER2�i�{�֐��ŗL�j��ǉ�
	@date 2008.04.09 ryoji GA_ROOTOWNER2 �͉\�Ȍ���c���k��悤�ɓ���C��
*/
HWND MyGetAncestor( HWND hWnd, UINT gaFlags )
{
	HWND hwndAncestor;
	HWND hwndDesktop = ::GetDesktopWindow();
	HWND hwndWk;

	if( hWnd == hwndDesktop )
		return NULL;

	switch( gaFlags )
	{
	case GA_PARENT:	// �e�E�B���h�E��Ԃ��i�I�[�i�[�͕Ԃ��Ȃ��j
		hwndAncestor = ( (DWORD)::GetWindowLongPtr( hWnd, GWL_STYLE ) & WS_CHILD )? ::GetParent( hWnd ): hwndDesktop;
		break;

	case GA_ROOT:	// �e�q�֌W��k���Ē��ߏ�ʂ̃g�b�v���x���E�B���h�E��Ԃ�
		hwndAncestor = hWnd;
		while( (DWORD)::GetWindowLongPtr( hwndAncestor, GWL_STYLE ) & WS_CHILD )
			hwndAncestor = ::GetParent( hwndAncestor );
		break;

	case GA_ROOTOWNER:	// �e�q�֌W�Ə��L�֌W��GetParent()�ők���ď��L����Ă��Ȃ��g�b�v���x���E�B���h�E��Ԃ�
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
		}while( hwndWk != NULL );
		break;

	case GA_ROOTOWNER2:	// ���L�֌W��GetWindow()�ők���ď��L����Ă��Ȃ��g�b�v���x���E�B���h�E��Ԃ�
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
			if( hwndWk == NULL )
				hwndWk = ::GetWindow( hwndAncestor, GW_OWNER );
		}while( hwndWk != NULL );
		break;

	default:
		hwndAncestor = NULL;
		break;
	}

	return hwndAncestor;
}

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	Shift,Ctrl,Alt�L�[��Ԃ̎擾

	@retval nIdx Shift,Ctrl,Alt�L�[���
	@date 2004.10.10 �֐���
*/
int getCtrlKeyState()
{
	int nIdx = 0;

	/* Shift�L�[��������Ă���Ȃ� */
	if(GetKeyState_Shift()){
		nIdx |= _SHIFT;
	}
	/* Ctrl�L�[��������Ă���Ȃ� */
	if( GetKeyState_Control() ){
		nIdx |= _CTRL;
	}
	/* Alt�L�[��������Ă���Ȃ� */
	if( GetKeyState_Alt() ){
		nIdx |= _ALT;
	}

	return nIdx;
}

/*!	�t�@�C���̍X�V�������擾

	@return true: ����, false: FindFirstFile���s

	@author genta by assitance with ryoji
	@date 2005.10.22 new

	@note �������݌�Ƀt�@�C�����ăI�[�v�����ă^�C���X�^���v�𓾂悤�Ƃ����
	�t�@�C�����܂����b�N����Ă��邱�Ƃ�����C�㏑���֎~�ƌ�F����邱�Ƃ�����D
	FindFirstFile���g�����ƂŃt�@�C���̃��b�N��Ԃɉe�����ꂸ�Ƀ^�C���X�^���v��
	�擾�ł���D(ryoji)
*/
bool GetLastWriteTimestamp(
	const TCHAR*	pszFileName,	//!< [in] �t�@�C���̃p�X
	FILETIME*		pcFileTime		//!< [out] �X�V������Ԃ��ꏊ
)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA ffd;

	hFindFile = ::FindFirstFile( pszFileName, &ffd );
	if( INVALID_HANDLE_VALUE != hFindFile )
	{
		::FindClose( hFindFile );
		*pcFileTime = ffd.ftLastWriteTime;
		return true;
	}
	else{
		//	�t�@�C����������Ȃ�����
		pcFileTime->dwLowDateTime = pcFileTime->dwHighDateTime = 0;
		return false;
	}
}


/*!	�������t�H�[�}�b�g

	@param[out] �����ϊ���̕�����
	@param[in] �o�b�t�@�T�C�Y
	@param[in] format ����
	@param[in] systime ����������������
	@return bool true

	@note  %Y %y %m %d %H %M %S �̕ϊ��ɑΉ�

	@author aroka
	@date 2005.11.21 �V�K
	
	@todo �o�̓o�b�t�@�̃T�C�Y�`�F�b�N���s��
*/
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime )
{
	TCHAR szTime[10];
	const TCHAR *p = format;
	TCHAR *q = szResult;
	int len;
	
	while( *p ){
		if( *p == _T('%') ){
			++p;
			switch(*p){
			case _T('Y'):
				len = wsprintf(szTime,_T("%d"),systime.wYear);
				_tcscpy( q, szTime );
				break;
			case _T('y'):
				len = wsprintf(szTime,_T("%02d"),(systime.wYear%100));
				_tcscpy( q, szTime );
				break;
			case _T('m'):
				len = wsprintf(szTime,_T("%02d"),systime.wMonth);
				_tcscpy( q, szTime );
				break;
			case _T('d'):
				len = wsprintf(szTime,_T("%02d"),systime.wDay);
				_tcscpy( q, szTime );
				break;
			case _T('H'):
				len = wsprintf(szTime,_T("%02d"),systime.wHour);
				_tcscpy( q, szTime );
				break;
			case _T('M'):
				len = wsprintf(szTime,_T("%02d"),systime.wMinute);
				_tcscpy( q, szTime );
				break;
			case _T('S'):
				len = wsprintf(szTime,_T("%02d"),systime.wSecond);
				_tcscpy( q, szTime );
				break;
				// A Z
			case _T('%'):
			default:
				*q = *p;
				len = 1;
				break;
			}
			q+=len;//q += strlen(szTime);
			++p;
			
		}
		else{
			*q = *p;
			q++;
			p++;
		}
	}
	*q = *p;
	return true;
}

/*!	�V�F����R�����R���g���[�� DLL �̃o�[�W�����ԍ����擾

	@param[in] lpszDllName DLL �t�@�C���̃p�X
	@return DLL �̃o�[�W�����ԍ��i���s���� 0�j

	@author ? (from MSDN Library document)
	@date 2006.06.17 ryoji MSDN���C�u����������p
*/
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	/* For security purposes, LoadLibrary should be provided with a
	   fully-qualified path to the DLL. The lpszDllName variable should be
	   tested to ensure that it is a fully qualified path before it is used. */
	hinstDll = LoadLibraryExedir(lpszDllName);

	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll,
						  "DllGetVersion");

		/* Because some DLLs might not implement this function, you
		must test for it explicitly. Depending on the particular
		DLL, the lack of a DllGetVersion function can be a useful
		indicator of the version. */

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
			   dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}
	return dwVersion;
}

/*!	Comctl32.dll �̃o�[�W�����ԍ����擾

	@return Comctl32.dll �̃o�[�W�����ԍ��i���s���� 0�j

	@author ryoji
	@date 2006.06.17 ryoji �V�K
*/
static DWORD s_dwComctl32Version = PACKVERSION(0, 0);
DWORD GetComctl32Version()
{
	if( PACKVERSION(0, 0) == s_dwComctl32Version )
		s_dwComctl32Version = GetDllVersion(_T("Comctl32.dll"));
	return s_dwComctl32Version;
}


/*!	���������݃r�W���A���X�^�C���\����Ԃ��ǂ���������
	Win32 API �� IsAppThemed() �͂���Ƃ͈�v���Ȃ��iIsAppThemed() �� IsThemeActive() �Ƃ̍��ق͕s���j

	@return �r�W���A���X�^�C���\�����(TRUE)�^�N���b�V�b�N�\�����(FALSE)

	@author ryoji
	@date 2006.06.17 ryoji �V�K
*/
BOOL IsVisualStyle()
{
	// ���[�h���� Comctl32.dll �� Ver 6 �ȏ�ŉ�ʐݒ肪�r�W���A���X�^�C���w��ɂȂ��Ă���ꍇ����
	// �r�W���A���X�^�C���\���ɂȂ�i�}�j�t�F�X�g�Ŏw�肵�Ȃ��� Comctl32.dll �� 6 �����ɂȂ�j
	return ( (GetComctl32Version() >= PACKVERSION(6, 0)) && CUxTheme::getInstance()->IsThemeActive() );
}

/*!	�w��E�B���h�E�Ńr�W���A���X�^�C�����g��Ȃ��悤�ɂ���

	@param[in] hWnd �E�B���h�E

	@author ryoji
	@date 2006.06.23 ryoji �V�K
*/
void PreventVisualStyle( HWND hWnd )
{
	CUxTheme::getInstance()->SetWindowTheme( hWnd, L"", L"" );
	return;
}

/*!	�R�����R���g���[��������������

	@author ryoji
	@date 2006.06.21 ryoji �V�K
*/
void MyInitCommonControls()
{
	BOOL (WINAPI *pfnInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);

	BOOL bInit = FALSE;
	HINSTANCE hDll = ::GetModuleHandle(_T("COMCTL32"));
	if( NULL != hDll ){
		*(FARPROC*)&pfnInitCommonControlsEx = ::GetProcAddress( hDll, "InitCommonControlsEx" );
		if( NULL != pfnInitCommonControlsEx ){
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
			bInit = pfnInitCommonControlsEx( &icex );
		}
	}

	if( !bInit ){
		::InitCommonControls();
	}
}

/*!	����t�H���_�̃p�X���擾����
	SHGetSpecialFolderPath API�ishell32.dll version 4.71�ȏオ�K�v�j�Ɠ����̏���������

	@author ryoji
	@date 2007.05.19 �V�K
*/
BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath )
{
	BOOL bRet = FALSE;
	HRESULT hres;
	LPMALLOC pMalloc;
	LPITEMIDLIST pidl;

	hres = ::SHGetMalloc( &pMalloc );
	if( FAILED( hres ) )
		return FALSE;

	hres = ::SHGetSpecialFolderLocation( NULL, nFolder, &pidl );
	if( SUCCEEDED( hres ) ){
		bRet = ::SHGetPathFromIDList( pidl, pszPath );
		pMalloc->Free( (void*)pidl );
	}

	pMalloc->Release();

	return bRet;
}

//�R���X�g���N�^�ŃJ�����g�f�B���N�g����ۑ����A�f�X�g���N�^�ŃJ�����g�f�B���N�g���𕜌����郂�m�B
//kobake
CCurrentDirectoryBackupPoint::CCurrentDirectoryBackupPoint()
{
	int n = ::GetCurrentDirectory(_countof(m_szCurDir), m_szCurDir);
	if( n > 0 && n < _countof(m_szCurDir)){
		//ok
	}
	else{
		//ng
		m_szCurDir[0] = _T('\0');
	}
}

CCurrentDirectoryBackupPoint::~CCurrentDirectoryBackupPoint()
{
	if(m_szCurDir[0]){
		::SetCurrentDirectory(m_szCurDir);
	}
}

/*! 
	�J�����g�f�B���N�g�������s�t�@�C���̏ꏊ�Ɉړ�
	@date 2010.08.28 Moca �V�K�쐬
*/
void ChangeCurrentDirectoryToExeDir()
{
	TCHAR szExeDir[_MAX_PATH];
	szExeDir[0] = _T('\0');
	GetExedir( szExeDir, NULL );
	if( szExeDir[0] ){
		::SetCurrentDirectory( szExeDir );
	}else{
		// �ړ��ł��Ȃ��Ƃ���SYSTEM32(9x�ł�SYSTEM)�Ɉړ�
		szExeDir[0] = _T('\0');
		int n = ::GetSystemDirectory( szExeDir, _MAX_PATH );
		if( n && n < _MAX_PATH ){
			::SetCurrentDirectory( szExeDir );
		}
	}
}

/*! 
	@date 2010.08.28 Moca �V�K�쐬
*/
HMODULE LoadLibraryExedir(LPCTSTR pszDll)
{
	CCurrentDirectoryBackupPoint dirBack;
	// DLL �C���W�F�N�V�����΍�Ƃ���EXE�̃t�H���_�Ɉړ�����
	ChangeCurrentDirectoryToExeDir();
	return ::LoadLibrary( pszDll );
}

///////////////////////////////////////////////////////////////////////
// From Here 2007.05.25 ryoji �Ǝ��g���̃v���p�e�B�V�[�g�֐��Q

static WNDPROC s_pOldPropSheetWndProc;	// �v���p�e�B�V�[�g�̌��̃E�B���h�E�v���V�[�W��

/*!	�Ǝ��g���v���p�e�B�V�[�g�̃E�B���h�E�v���V�[�W��
	@author ryoji
	@date 2007.05.25 �V�K
*/
static LRESULT CALLBACK PropSheetWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ){
	case WM_SHOWWINDOW:
		// �ǉ��{�^���̈ʒu�𒲐�����
		if( wParam ){
			HWND hwndBtn;
			RECT rcOk;
			RECT rcTab;
			POINT pt;

			hwndBtn = ::GetDlgItem( hwnd, 0x02000 );
			::GetWindowRect( ::GetDlgItem( hwnd, IDOK ), &rcOk );
			::GetWindowRect( PropSheet_GetTabControl( hwnd ), &rcTab );
			pt.x = rcTab.left;
			pt.y = rcOk.top;
			::ScreenToClient( hwnd, &pt );
			::MoveWindow( hwndBtn, pt.x, pt.y, 140, rcOk.bottom - rcOk.top, FALSE );
		}
		break;

	case WM_COMMAND:
		// �ǉ��{�^���������ꂽ���͂��̏������s��
		if( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == 0x02000 ){
			HWND hwndBtn = ::GetDlgItem( hwnd, 0x2000 );
			RECT rc;
			POINT pt;

			// ���j���[��\������
			::GetWindowRect( hwndBtn, &rc );
			pt.x = rc.left;
			pt.y = rc.bottom;
			GetMonitorWorkRect( pt, &rc );	// ���j�^�̃��[�N�G���A

			HMENU hMenu = ::CreatePopupMenu();
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, _T("�J��(&O)...") );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, _T("�C���|�[�g�^�G�N�X�|�[�g�̋N�_���Z�b�g(&R)") );

			int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										( pt.x > rc.left )? pt.x: rc.left,
										( pt.y < rc.bottom )? pt.y: rc.bottom,
										0, hwnd, NULL );
			::DestroyMenu( hMenu );

			// �I�����ꂽ���j���[�̏���
			switch( nId ){
			case 100:	// �ݒ�t�H���_���J��
				TCHAR szPath[_MAX_PATH];
				GetInidir( szPath );

				// �t�H���_�� ITEMIDLIST ���擾���� ShellExecuteEx() �ŊJ��
				// Note. MSDN �� ShellExecute() �̉���ɂ�����@�Ńt�H���_���J�����Ƃ����ꍇ�A
				//       �t�H���_�Ɠ����ꏊ�� <�t�H���_��>.exe ������Ƃ��܂������Ȃ��B
				//       verb��"open"��NULL�ł�exe�̂ق������s����"explore"�ł͎��s����
				//       �i�t�H���_���̖�����'\\'��t�����Ă�Windows 2000�ł͕t�����Ȃ��̂Ɠ�������ɂȂ��Ă��܂��j
				LPSHELLFOLDER pDesktopFolder;
				if( SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder)) ){
					LPMALLOC pMalloc;
					if( SUCCEEDED(::SHGetMalloc(&pMalloc)) ){
						LPITEMIDLIST pIDL;
						LPWSTR pwszDisplayName;
#ifdef _UNICODE
						pwszDisplayName = szPath;
#else
						WCHAR wszPath[_MAX_PATH];
						::MultiByteToWideChar( CP_ACP, 0, szPath, -1, wszPath, _MAX_PATH );
						pwszDisplayName = wszPath;
#endif
						if( SUCCEEDED(pDesktopFolder->ParseDisplayName(NULL, NULL, pwszDisplayName, NULL, &pIDL, NULL)) ){
							SHELLEXECUTEINFO si;
							::ZeroMemory( &si, sizeof(si) );
							si.cbSize = sizeof(si);
							si.fMask = SEE_MASK_IDLIST;
							si.lpVerb = _T("open");
							si.lpIDList = pIDL;
							si.nShow = SW_SHOWNORMAL;
							::ShellExecuteEx( &si );	// �t�H���_���J��
							pMalloc->Free( (void*)pIDL );
						}
						pMalloc->Release();
					}
					pDesktopFolder->Release();
				}
				break;
			case 101:	// �C���|�[�g�^�G�N�X�|�[�g�̋N�_���Z�b�g�i�N�_��ݒ�t�H���_�ɂ���j
				int nMsgResult = MYMESSAGEBOX(
					hwnd,
					MB_OKCANCEL | MB_ICONINFORMATION,
					GSTR_APPNAME,
					_T("�e��ݒ�̃C���|�[�g�^�G�N�X�|�[�g�p�t�@�C���I����ʂ�\n")
					_T("�����\���t�H���_��ݒ�t�H���_�ɖ߂��܂��B")
				);
				if( IDOK == nMsgResult )
				{
					DLLSHAREDATA *pShareData = CShareData::getInstance()->GetShareData();
					GetInidir( pShareData->m_szIMPORTFOLDER );
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)s_pOldPropSheetWndProc );
		break;
	}

	return ::CallWindowProc( (WNDPROC)s_pOldPropSheetWndProc, hwnd, uMsg, wParam, lParam );
}

/*!	�Ǝ��g���v���p�e�B�V�[�g�̃R�[���o�b�N�֐�
	@author ryoji
	@date 2007.05.25 �V�K
*/
static int CALLBACK PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{
	// �v���p�e�B�V�[�g�̏��������Ƀ{�^���ǉ��A�v���p�e�B�V�[�g�̃T�u�N���X�����s��
	if( uMsg == PSCB_INITIALIZED ){
		s_pOldPropSheetWndProc = (WNDPROC)::SetWindowLongPtr( hwndDlg, GWLP_WNDPROC, (LONG_PTR)PropSheetWndProc );
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle( NULL );
		HWND hwndBtn = ::CreateWindowEx( 0, _T("BUTTON"), _T("�ݒ�t�H���_(&/) >>"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 140, 20, hwndDlg, (HMENU)0x02000, hInstance, NULL );
		::SendMessage( hwndBtn, WM_SETFONT, (WPARAM)::SendMessage( hwndDlg, WM_GETFONT, 0, 0 ), MAKELPARAM( FALSE, 0 ) );
		::SetWindowPos( hwndBtn, ::GetDlgItem( hwndDlg, IDHELP), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
	return 0;
}


/*!	�Ǝ��g���v���p�e�B�V�[�g�i���ʐݒ�^�^�C�v�ʐݒ��ʗp�j
	@author ryoji
	@date 2007.05.25 �V�K
*/
int MyPropertySheet( LPPROPSHEETHEADER lppsph )
{
	// �l�ݒ�t�H���_���g�p����Ƃ��́u�ݒ�t�H���_�v�{�^����ǉ�����
	if( CShareData::getInstance()->IsPrivateSettings() ){
		lppsph->dwFlags |= PSH_USECALLBACK;
		lppsph->pfnCallback = PropSheetProc;
	}
	return ::PropertySheet( lppsph );
}

// To Here 2007.05.25 ryoji �Ǝ��g���̃v���p�e�B�V�[�g�֐��Q
///////////////////////////////////////////////////////////////////////

/*[EOF]*/
