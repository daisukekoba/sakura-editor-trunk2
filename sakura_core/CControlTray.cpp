/*!	@file
	@brief �풓��
	
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2001/06/03 N.Nakatani grep�P��P�ʂŌ�������������Ƃ��̂��߂ɃR�}���h���C���I�v�V�����̏����ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, Stonee, jepro, genta, aroka, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, YAZAKI, towest
	Copyright (C) 2003, MIK, Moca, KEITA, genta, aroka
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, novice

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#define ID_HOTKEY_TRAYMENU	0x1234

#include <HtmlHelp.h>
#include <io.h>
#include "CControlTray.h"
#include "CEditApp.h"
#include "Debug.h"
#include "CEditWnd.h"		//Nov. 21, 2000 JEPROtest
#include "CDlgAbout.h"		//Nov. 21, 2000 JEPROtest
#include "mymessage.h"
#include "CDlgOpenFile.h"
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "sakura_rc.h"

/////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK CControlTrayWndProc( HWND, UINT, WPARAM, LPARAM );

static CControlTray*	g_m_pCControlTray;

//Stonee, 2001/03/21
//Stonee, 2001/07/01  ���d�N�����ꂽ�ꍇ�͑O��̃_�C�A���O��O�ʂɏo���悤�ɂ����B
void CControlTray::DoGrep()
{
	//Stonee, 2001/06/30
	//�O��̃_�C�A���O������ΑO�ʂ� (suggested by genta)
	if ( ::IsWindow(m_cDlgGrep.m_hWnd) ){
		::OpenIcon(m_cDlgGrep.m_hWnd);
		::BringWindowToTop(m_cDlgGrep.m_hWnd);
		return;
	}

	_tcscpy( m_cDlgGrep.m_szText, m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] );

	/* Grep�_�C�A���O�̕\�� */
	int nRet = m_cDlgGrep.DoModal( m_hInstance, NULL, _T("") );
	if( !nRet || m_hWnd == NULL ){
		return;
	}


	/*======= Grep�̎��s =============*/
	/* Grep���ʃE�B���h�E�̕\�� */

	CMemory			cmWork1;
	CMemory			cmWork2;
	CMemory			cmWork3;
	cmWork1.SetString( m_cDlgGrep.m_szText );
	cmWork2.SetString( m_cDlgGrep.m_szFile );
	cmWork3.SetString( m_cDlgGrep.m_szFolder );
	cmWork1.Replace( "\"", "\"\"" );
	cmWork2.Replace( _T("\""), _T("\"\"") );
	cmWork3.Replace( _T("\""), _T("\"\"") );

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	TCHAR* pCmdLine = new char[1024];
	wsprintf( pCmdLine, _T("-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d"),
		cmWork1.GetStringPtr(),
		cmWork2.GetStringPtr(),
		cmWork3.GetStringPtr(),
		m_cDlgGrep.m_nGrepCharSet
	);

	//GOPT�I�v�V����
	TCHAR pOpt[64] = _T("");
	if( m_cDlgGrep.m_bSubFolder					)_tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
	if( m_cDlgGrep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
	if( m_cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// ���K�\��
	if( m_cDlgGrep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// �s���o�͂��邩�Y�����������o�͂��邩
	if( m_cDlgGrep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
	if( 1 == m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
	if( 2 == m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
	if( pOpt[0] != _T('\0') ){
		_tcscat( pCmdLine, _T(" -GOPT=") );
		_tcscat( pCmdLine, pOpt );
	}

	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
	CControlTray::OpenNewEditor( m_hInstance, m_pShareData->m_sHandles.m_hwndTray, pCmdLine, 0, false,
		false, NULL, m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false );

	delete [] pCmdLine;
}


/* �E�B���h�E�v���V�[�W������ */
static LRESULT CALLBACK CControlTrayWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CControlTray* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CControlTray* )g_m_pCControlTray;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		// Modified by KEITA for WIN64 2003.9.6
		//RELPRINT( _T("dispatch\n") );
		pSApp = ( CControlTray* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CControlTray
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CControlTray::CControlTray()
//	Apr. 24, 2001 genta
: m_uCreateTaskBarMsg( ::RegisterWindowMessage( TEXT("TaskbarCreated") ) )
, m_bCreatedTrayIcon( FALSE )	//�g���C�ɃA�C�R���������
, m_hInstance( NULL )
, m_hWnd( NULL )
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	// �A�N�Z�����[�^�e�[�u���쐬
	CreateAccelTbl();

	m_bUseTrayMenu = false;

	return;
}


CControlTray::~CControlTray()
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CControlTray �����o�֐�




/* �쐬 */
HWND CControlTray::Create( HINSTANCE hInstance )
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlTray::Create" );

	//�������N���X�̃E�B���h�E�����ɑ��݂��Ă�����A���s
	m_hInstance = hInstance;
	HWND hwndWork = ::FindWindow( GSTR_CEDITAPP, GSTR_CEDITAPP );
	if( NULL != hwndWork ){
		return NULL;
	}

	//�E�B���h�E�N���X�o�^
	WNDCLASS	wc;
	{
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= CControlTrayWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= GSTR_CEDITAPP;
		ATOM	atom = RegisterClass( &wc );
		if( 0 == atom ){
			ErrorMessage( NULL, _T("CControlTray::Create()\n�E�B���h�E�N���X��o�^�ł��܂���ł����B") );
		}
	}
	g_m_pCControlTray = this;

	::CreateWindow(
		GSTR_CEDITAPP,						// pointer to registered class name
		GSTR_CEDITAPP,						// pointer to window name
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		m_hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);

	// �őO�ʂɂ���i�g���C����̃|�b�v�A�b�v�E�B���h�E���őO�ʂɂȂ�悤�Ɂj
	::SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
	MY_TRACETIME( cRunningTimer, "Window is created" );

	// �^�X�N�g���C�A�C�R���쐬
	m_hIcons.Create( m_hInstance );	//	Oct. 16, 2000 genta
	MY_TRACETIME( cRunningTimer, "Icons are created" );
	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_hIcons );
	if( m_hWnd ){
		CreateTrayIcon( m_hWnd );
	}

	return m_hWnd;
}

//! �^�X�N�g���C�ɃA�C�R����o�^����
bool CControlTray::CreateTrayIcon( HWND hWnd )
{
	// �^�X�N�g���C�̃A�C�R�������
	if( m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray ){	/* �^�X�N�g���C�̃A�C�R�����g�� */
		//	Dec. 02, 2002 genta
		HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
//From Here Jan. 12, 2001 JEPRO �g���C�A�C�R���Ƀ|�C���g����ƃo�[�W����no.���\�������悤�ɏC��
//			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, GSTR_APPNAME );
		/* �o�[�W������� */
		//	UR version no.��ݒ� (cf. cDlgAbout.cpp)
		TCHAR	pszTips[64];
		//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
		DWORD dwVersionMS, dwVersionLS;
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&dwVersionMS, &dwVersionLS );

		wsprintf( pszTips, _T("%s %d.%d.%d.%d"),		//Jul. 06, 2001 jepro UR �͂����t���Ȃ��Ȃ����̂�Y��Ă���
			GSTR_APPNAME,
			HIWORD( dwVersionMS ),
			LOWORD( dwVersionMS ),
			HIWORD( dwVersionLS ),
			LOWORD( dwVersionLS )
		);
		TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, pszTips );
//To Here Jan. 12, 2001
		m_bCreatedTrayIcon = TRUE;	/* �g���C�ɃA�C�R��������� */
	}
	return true;
}




/* ���b�Z�[�W���[�v */
void CControlTray::MessageLoop( void )
{
//�����v���Z�X��
	MSG	msg;
	int ret;
	
	//2004.02.17 Moca GetMessage�̃G���[�`�F�b�N
	while ( m_hWnd != NULL && (ret = ::GetMessage(&msg, NULL, 0, 0 )) ){
		if( ret == -1 ){
			break;
		}
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return;

}




/* �^�X�N�g���C�̃A�C�R���Ɋւ��鏈�� */
BOOL CControlTray::TrayMessage( HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const TCHAR* pszTip )
{
	BOOL			res;
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof( tnd );
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if( pszTip ){
		lstrcpyn( tnd.szTip, pszTip, _countof( tnd.szTip ) );
	}else{
		tnd.szTip[0] = _T('\0');
	}
	res = Shell_NotifyIcon( dwMessage, &tnd );
	if( hIcon ){
		DestroyIcon( hIcon );
	}
	return res;
}





/* ���b�Z�[�W���� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
LRESULT CControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int				nId;
	HWND			hwndWork;
	LPHELPINFO		lphi;

	int			nRowNum;
	EditNode*	pEditNodeArr;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
	LPMEASUREITEMSTRUCT	lpmis;	/* ���ڃT�C�Y��� */
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;
	static bool			bLDClick = false;	/* ���_�u���N���b�N�������� 03/02/20 ai */

	switch ( uMsg ){
	case WM_MENUCHAR:
		/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );
	case WM_DRAWITEM:
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
			/* ���j���[�A�C�e���`�� */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
			/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( 0 < nItemWidth ){
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;

	/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
	case WM_HOTKEY:
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			TCHAR	szClassName[100];
			TCHAR	szText[256];

			hwndWork = ::GetForegroundWindow();
			szClassName[0] = '\0';
			::GetClassName( hwndWork, szClassName, _countof( szClassName ) - 1 );
			::GetWindowText( hwndWork, szText, _countof( szText ) - 1 );
			if( 0 == _tcscmp( szText, _T("���ʐݒ�") ) ){
				return -1;
			}

			if( ID_HOTKEY_TRAYMENU == idHotKey
			 &&	( wHotKeyMods )  == fuModifiers
			 && wHotKeyCode == uVirtKey
			){
				// Jan. 1, 2003 AROKA
				// �^�X�N�g���C���j���[�̕\���^�C�~���O��LBUTTONDOWN��LBUTTONUP�ɕύX�������Ƃɂ��
				::PostMessage( m_hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONUP );
			}
		}
		return 0;

	case MYWM_UIPI_CHECK:
		/* �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W */	// 2007.06.07 ryoji
		::SendMessage( (HWND)lParam, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)0 );	// �Ԏ���Ԃ�
		return 0L;

	case MYWM_HTMLHELP:
		{
			TCHAR* pWork = m_pShareData->m_sWorkBuffer.m_szWork;

			//szHtmlFile�擾
			TCHAR	szHtmlHelpFile[1024];
			_tcscpy( szHtmlHelpFile, pWork );
			int		nLen = _tcslen( szHtmlHelpFile );

			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD_PTR)0,
				true
			);

			HH_AKLINK	link;
			link.cbStruct		= sizeof(link);
			link.fReserved		= FALSE;
			link.pszKeywords	= &pWork[nLen+1];
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;

			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link,
				false
			);
		}
		return (LRESULT)hwndHtmlHelp;


	/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
	case MYWM_DELETE_ME:
		// �^�X�N�g���C�̃A�C�R�����풓���Ȃ��A�܂��́A�g���C�ɃA�C�R��������Ă��Ȃ�
		if( !m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray || !m_bCreatedTrayIcon ){
			// ���݊J���Ă���ҏW���̃��X�g
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( 0 < nRowNum ){
				delete [] pEditNodeArr;
			}
			// �ҏW�E�B���h�E�̐���0�ɂȂ�����I��
			if( 0 == nRowNum ){
				::SendMessage( hwnd, WM_CLOSE, 0, 0 );
			}
		}
		return 0;

	case WM_CREATE:
		m_hWnd = hwnd;
		hwndHtmlHelp = NULL;
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)this );

		/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
		wHotKeyMods = 0;
		if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_SHIFT;
		}
		if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_CONTROL;
		}
		if( HOTKEYF_ALT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_ALT;
		}
		wHotKeyCode = m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode;
		::RegisterHotKey(
			m_hWnd,
			ID_HOTKEY_TRAYMENU,
			wHotKeyMods,
			wHotKeyCode
		);

		// 2006.07.09 ryoji �Ō�̕��ŃV���b�g�_�E������A�v���P�[�V�����ɂ���
		BOOL (WINAPI *pfnSetProcessShutdownParameters)( DWORD dwLevel, DWORD dwFlags );
		HINSTANCE hDll;
		hDll = ::GetModuleHandle(_T("KERNEL32"));
		if( NULL != hDll ){
			*(FARPROC*)&pfnSetProcessShutdownParameters = ::GetProcAddress( hDll, "SetProcessShutdownParameters" );
			if( NULL != pfnSetProcessShutdownParameters ){
				pfnSetProcessShutdownParameters( 0x180, 0 );
			}
		}
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			MyWinHelp( hwnd, HELP_CONTEXT, FuncID_To_HelpContextID( lphi->iCtrlId ) );
			break;
		}
		return TRUE;
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
		case MYWM_CHANGESETTING:
			switch( (e_PM_CHANGESETTING_SELECT)lParam ){
			case PM_CHANGESETTING_ALL:
				::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );
				/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
				wHotKeyMods = 0;
				if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_SHIFT;
				}
				if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_CONTROL;
				}
				if( HOTKEYF_ALT & m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ){
					wHotKeyMods |= MOD_ALT;
				}
				wHotKeyCode = m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode;
				::RegisterHotKey(
					m_hWnd,
					ID_HOTKEY_TRAYMENU,
					wHotKeyMods,
					wHotKeyCode
				);

//@@			/* ���L�f�[�^�̕ۑ� */
//@@			m_cShareData.SaveShareData();

				/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
				// �A�N�Z�����[�^�e�[�u���j��
				DeleteAccelTbl();
				// �A�N�Z�����[�^�e�[�u���쐬
				CreateAccelTbl();
				break;
			default:
				break;
			}
			return 0L;

		case MYWM_NOTIFYICON:
//			MYTRACE( _T("MYWM_NOTIFYICON\n") );
			switch (lParam){
//�L�[���[�h�F�g���C�E�N���b�N���j���[�ݒ�
//	From Here Oct. 12, 2000 JEPRO ���E�Ƃ����ꏈ���ɂȂ��Ă����̂�ʁX�ɏ�������悤�ɕύX
			case WM_RBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */
				nId = CreatePopUpMenu_R();
				switch( nId ){
				case F_HELP_CONTENTS:
					/* �w���v�ڎ� */
					ShowWinHelpContents( m_hWnd );	//	�ڎ���\������
					break;
				case F_HELP_SEARCH:
					/* �w���v�L�[���[�h���� */
					MyWinHelp( m_hWnd, HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
					break;
				case F_EXTHELP1:
					/* �O���w���v�P */
					do{
						if( CShareData::getInstance()->ExtWinHelpIsSet() ) {	//	���ʐݒ�̂݊m�F
							break;
						}
						else{
							ErrorBeep();
						}
					}while(IDYES == ::MYMESSAGEBOX( 
							NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
							GSTR_APPNAME,
							_T("�O���w���v�P���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?"))
					);/*do-while*/

					break;
				case F_EXTHTMLHELP:
					/* �O��HTML�w���v */
					{
//						CEditView::Command_EXTHTMLHELP();
					}
					break;
				case F_ABOUT:
					/* �o�[�W������� */
					{
						CDlgAbout cDlgAbout;
						cDlgAbout.DoModal( m_hInstance, m_hWnd );
					}
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
					/* �T�N���G�f�B�^�̑S�I�� */
					CControlTray::TerminateApplication( m_hWnd );	// 2006.12.25 ryoji �����ǉ�
					break;
				default:
					break;
				}
				return 0L;
//	To Here Oct. 12, 2000

			case WM_LBUTTONDOWN:
				//	Mar. 29, 2003 genta �O�̂��߃t���O�N���A
				bLDClick = false;
				return 0L;
			case WM_LBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
//				MYTRACE( _T("WM_LBUTTONDOWN\n") );
				/* 03/02/20 ���_�u���N���b�N��̓��j���[��\�����Ȃ� ai Start */
				if( bLDClick ){
					bLDClick = false;
					return 0L;
				}
				/* 03/02/20 ai End */
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* �|�b�v�A�b�v���j���[(�g���C���{�^��) */
				nId = CreatePopUpMenu_L();
				switch( nId ){
				case F_FILENEW:	/* �V�K�쐬 */
					/* �V�K�ҏW�E�B���h�E�̒ǉ� */
					OnNewEditor( false );
					break;
				case F_FILEOPEN:	/* �J�� */
					{
						HWND			hWndOwner;

						// MRU���X�g�̃t�@�C���̃��X�g
						const CMRUFile cMRU;
						std::vector<LPCTSTR> vMRU = cMRU.GetPathList();

						// �t�@�C���I�[�v���_�C�A���O�̏�����
						TCHAR szPath[_MAX_PATH + 1];
						_tcscpy( szPath, _T("") );
						ECodeType nCharCode = CODE_AUTODETECT;	// �����R�[�h��������
						bool bReadOnly = false;
						CDlgOpenFile	cDlgOpenFile;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL,
							_T("*.*"),
							vMRU.empty()? NULL: vMRU[0],//@@@ 2001.12.26 YAZAKI m_fiMRUArr�ɂ̓A�N�Z�X���Ȃ�
							vMRU,
							CMRUFolder().GetPathList()	// OPENFOLDER���X�g�̃t�@�C���̃��X�g
						);
						if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
							break;
						}
						if( NULL == m_hWnd ){
							break;
						}
						/* �w��t�@�C�����J����Ă��邩���ׂ� */
						if( CShareData::getInstance()->ActiveAlreadyOpenedWindow( szPath, &hWndOwner, nCharCode )){
							// 2007.03.13 maru ���d�I�[�v���ɑ΂��鏈����CShareData::IsPathOpened�ֈړ�
						}else{
							if( strchr( szPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", szPath );
								strcpy( szPath, szFile2 );
							}
							// �V���ȕҏW�E�B���h�E���N��
							CControlTray::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly,
								true, NULL, m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false );
						}
					}
					break;
				case F_GREP_DIALOG:
					/* Grep */
					DoGrep();  //Stonee, 2001/03/21  Grep��ʊ֐���
					break;
				case F_FILESAVEALL:	// Jan. 24, 2005 genta �S�ď㏑���ۑ�
					CShareData::getInstance()->PostMessageToAllEditors(
						WM_COMMAND,
						MAKELONG( F_FILESAVE_QUIET, 0 ),
						(LPARAM)0,
						NULL
					);
					break;
				case F_EXITALLEDITORS:	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	// 2007.02.13 ryoji ��F_EXITALLEDITORS
					/* �ҏW�̑S�I�� */
					CControlTray::CloseAllEditor( TRUE, m_hWnd, TRUE, 0 );	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
					break;
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
					/* �T�N���G�f�B�^�̑S�I�� */
					CControlTray::TerminateApplication( m_hWnd );	// 2006.12.25 ryoji �����ǉ�
					break;
				default:
					if( nId - IDM_SELWINDOW  >= 0 && nId - IDM_SELWINDOW  < m_pShareData->m_sNodes.m_nEditArrNum ){
						hwndWork = m_pShareData->m_sNodes.m_pEditArr[nId - IDM_SELWINDOW].m_hWnd;

						/* �A�N�e�B�u�ɂ��� */
						ActivateFrameWindow( hwndWork );
					}
					else if( nId-IDM_SELMRU >= 0 && nId-IDM_SELMRU < 999 ){

						/* �V�����ҏW�E�B���h�E���J�� */
						//	From Here Oct. 27, 2000 genta	�J�[�\���ʒu�𕜌����Ȃ��@�\
						const CMRUFile cMRU;
						EditInfo openEditInfo;
						cMRU.GetEditInfo(nId - IDM_SELMRU, &openEditInfo);

						if( m_pShareData->m_Common.m_sFile.GetRestoreCurPosition() ){
							CControlTray::OpenNewEditor2( m_hInstance, m_hWnd, &openEditInfo, false );
						}
						else {
							CControlTray::OpenNewEditor(
								m_hInstance,
								m_hWnd,
								openEditInfo.m_szPath,
								openEditInfo.m_nCharCode,
								false,
								false,
								NULL,
								m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false
							);

						}
						//	To Here Oct. 27, 2000 genta
					}
					else if( nId - IDM_SELOPENFOLDER  >= 0 && nId - IDM_SELOPENFOLDER  < 999 ){
						HWND			hWndOwner;

						/* MRU���X�g�̃t�@�C���̃��X�g */
						const CMRUFile cMRU;
						std::vector<LPCTSTR> vMRU = cMRU.GetPathList();

						/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
						const CMRUFolder cMRUFolder;
						std::vector<LPCTSTR> vOPENFOLDER = cMRUFolder.GetPathList();

						//Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
						NetConnect( cMRUFolder.GetPath( nId - IDM_SELOPENFOLDER ) );

						/* �t�@�C���I�[�v���_�C�A���O�̏����� */
						TCHAR szPath[_MAX_PATH + 1];
						_tcscpy( szPath, _T("") );
						ECodeType nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
						bool bReadOnly = false;
						CDlgOpenFile	cDlgOpenFile;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL,
							_T("*.*"),
							vOPENFOLDER[ nId - IDM_SELOPENFOLDER ],
							vMRU,
							vOPENFOLDER
						);
						if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
							break;
						}
						if( NULL == m_hWnd ){
							break;
						}
						/* �w��t�@�C�����J����Ă��邩���ׂ� */
						if( CShareData::getInstance()->ActiveAlreadyOpenedWindow( szPath, &hWndOwner, nCharCode )){
							// 2007.03.13 maru ���d�I�[�v���ɑ΂��鏈����CShareData::IsPathOpened�ֈړ�
						} else {
							if( strchr( szPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", szPath );
								strcpy( szPath, szFile2 );
							}

							// �V���ȕҏW�E�B���h�E���N��
							CControlTray::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly,
								true, NULL, m_pShareData->m_Common.m_sTabBar.m_bNewWindow? true : false );
						}
					}
					break;
				}
				return 0L;
			case WM_LBUTTONDBLCLK:
				bLDClick = true;		/* 03/02/20 ai */
				/* �V�K�ҏW�E�B���h�E�̒ǉ� */
				OnNewEditor( m_pShareData->m_Common.m_sTabBar.m_bNewWindow == TRUE  );
				// Apr. 1, 2003 genta ���̌�ŕ\�����ꂽ���j���[�͕���
				::PostMessage( m_hWnd, WM_CANCELMODE, 0, 0 );
				return 0L;
			case WM_RBUTTONDBLCLK:
				return 0L;
			}
			break;

		case WM_QUERYENDSESSION:
			/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
			if( CloseAllEditor( FALSE, m_hWnd, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
				//	Jan. 31, 2000 genta
				//	���̎��_�ł�Windows�̏I�����m�肵�Ă��Ȃ��̂ŏ풓�������ׂ��ł͂Ȃ��D
				//	::DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
			if( CloseAllEditor( FALSE, m_hWnd, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
				::DestroyWindow( hwnd );
			}
			return 0L;

		//	From Here Jan. 31, 2000 genta	Windows�I�����̌㏈���D
		//	Windows�I������WM_CLOSE���Ă΂�Ȃ���CDestroyWindow��
		//	�Ăяo���K�v���Ȃ��D�܂��C���b�Z�[�W���[�v�ɖ߂�Ȃ��̂�
		//	���b�Z�[�W���[�v�̌��̏����������Ŋ���������K�v������D
		case WM_ENDSESSION:
			//	����Windows�̏I�������f���ꂽ�̂Ȃ牽�����Ȃ�
			if( wParam == TRUE )
				OnDestroy();	// 2006.07.09 ryoji WM_DESTROY �Ɠ�������������i�g���C�A�C�R���̔j���Ȃǂ�NT�n�ł͕K�v�j

			return 0;	//	�������̃v���Z�X�ɐ��䂪�߂邱�Ƃ͂Ȃ�
		//	To Here Jan. 31, 2000 genta
		case WM_DESTROY:
			OnDestroy();

			/* Windows �ɃX���b�h�̏I����v�����܂��B*/
			::PostQuitMessage( 0 );
			return 0L;
		default:
// << 20010412 by aroka
//	Apr. 24, 2001 genta RegisterWindowMessage���g���悤�ɏC��
			if( uMsg == m_uCreateTaskBarMsg ){
				/* TaskTray Icon�̍ēo�^��v�����郁�b�Z�[�W�D
					Explorer���ċN�������Ƃ��ɑ��o�����D*/
				CreateTrayIcon( m_hWnd ) ;
			}
			break;	/* default */
// >> by aroka
	}
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}




/* WM_COMMAND���b�Z�[�W���� */
void CControlTray::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* ���j���[����̃��b�Z�[�W */
	case 0:
		break;
	}
	return;
}

/*!
	@brief �V�K�E�B���h�E���쐬����
	
	�^�X�N�g���C����̐V�K�쐬�̏ꍇ�ɂ̓J�����g�f�B���N�g����
	�ۑ����̃f�t�H���g�f�B���N�g�����Ō�Ɏg��ꂽ�f�B���N�g���Ƃ���D
	�������Ō�Ɏg��ꂽ�f�B���N�g�������݂��Ȃ��ꍇ�͎��Ɏg��ꂽ�f�B���N�g���Ƃ��C
	�������݂���f�B���N�g����������܂ŗ��������Ɏ����D
	
	�ǂ̗�����������Ȃ������ꍇ�ɂ͌��݂̃J�����g�f�B���N�g���ō쐬����D

	@author genta
	@date 2003.05.30 �V�K�쐬
*/
void CControlTray::OnNewEditor( bool bNewWindow )
{

	const TCHAR* szCurDir = NULL;

	//
	//  szCurDir ��ݒ�
	//
	//	�ŋߎg�����t�H���_�����Ԃɂ��ǂ�
	const CMRUFolder mrufolder;

	// �V�K�E�B���h�E�ŊJ���I�v�V�����́A�^�u�o�[���O���[�v����O��Ƃ���
	bNewWindow = bNewWindow
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd == TRUE
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin == FALSE;

	int nCount = mrufolder.Length();
	for( int i = 0; i < nCount ; i++ ){
		const TCHAR* recentdir = mrufolder.GetPath( i );
		DWORD attr = GetFileAttributes( recentdir );

		if( attr != -1 ){
			if(( attr & FILE_ATTRIBUTE_DIRECTORY ) != 0 ){
				szCurDir = recentdir;
				break;
			}
		}
	}


	// �ҏW�E�C���h�E���J��
	OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, false, false, szCurDir, bNewWindow );
}

/*!
	�V�K�ҏW�E�B���h�E�̒ǉ� ver 0

	@date 2000.10.24 genta WinExec -> CreateProcess�D�����@�\��t��
	@date 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2003.05.30 genta �O���v���Z�X�N�����̃J�����g�f�B���N�g���w����\�ɁD
	@date 2007.06.26 ryoji �V�K�ҏW�E�B���h�E�� hWndParent �Ɠ����O���[�v���w�肵�ċN������
	@date 2008.04.19 ryoji MYWM_FIRST_IDLE �҂���ǉ�
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
bool CControlTray::OpenNewEditor(
	HINSTANCE			hInstance,			//!< [in] �C���X�^���XID (���͖��g�p)
	HWND				hWndParent,			//!< [in] �e�E�B���h�E�n���h���D�G���[���b�Z�[�W�\���p
	const TCHAR*		pszPath,			//!< [in] �V�K�G�f�B�^�ŊJ���t�@�C�����ƃI�v�V�����DNULL�ŐV�K�G�f�B�^�쐬�D
	int					nCharCode,			//!< [in] �V�K�G�f�B�^�̕����R�[�h
	bool				bReadOnly,			//!< [in] FALSE�łȂ���Γǂݎ���p�ŊJ��
	bool				sync,				//!< [in] true�Ȃ�V�K�G�f�B�^�̋N���܂őҋ@����
	const TCHAR*		szCurDir,			//!< [in] �V�K�G�f�B�^�̃J�����g�f�B���N�g��
	bool				bNewWindow			//!< [in] �V�K�G�f�B�^��V�����E�C���h�E�ŊJ��
)
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	DLLSHAREDATA*	pShareData = CShareData::getInstance()->GetShareData();

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		return false;
	}

	TCHAR szCmdLineBuf[1024];	//	�R�}���h���C��
	int nPos = 0;				//	�R�}���h���C���\�z�p�|�C���^

	//�A�v���P�[�V�����p�X
	TCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName( NULL, szEXE, _countof( szEXE ) );
	nPos += wsprintf( szCmdLineBuf + nPos, _T("\"%s\""), szEXE );

	// �t�@�C����
	if( pszPath ) nPos += wsprintf( szCmdLineBuf + nPos, _T(" %s"), pszPath );

	// �R�[�h�w��
	if( nCharCode != CODE_AUTODETECT )	nPos += wsprintf( szCmdLineBuf + nPos, _T(" -CODE=%d"), nCharCode );

	//	�ǂݎ���p�w��
	if( bReadOnly )		nPos += wsprintf( szCmdLineBuf + nPos, _T(" -R") );

	// �O���[�vID
	if( false == bNewWindow ){	// �V�K�G�f�B�^���E�C���h�E�ŊJ��
		// �O���[�vID��e�E�B���h�E����擾
		HWND hwndAncestor = MyGetAncestor( hWndParent, GA_ROOTOWNER2 );	// 2007.10.22 ryoji GA_ROOTOWNER -> GA_ROOTOWNER2
		int nGroup = CShareData::getInstance()->GetGroupId( hwndAncestor );
		if( nGroup > 0 ){
			nPos += wsprintf( szCmdLineBuf + nPos, _T(" -GROUP=%d"), nGroup );
		}
	}else{
		// �󂢂Ă���O���[�vID���g�p����
		nPos += wsprintf( szCmdLineBuf + nPos, _T(" -GROUP=%d"), CShareData::getInstance()->GetFreeGroupId() );
	}

	// -- -- -- -- �v���Z�X���� -- -- -- -- //

	//	�v���Z�X�̋N��
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	//	May 30, 2003 genta �J�����g�f�B���N�g���w����\��
	//�G�f�B�^�v���Z�X���N��
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	BOOL bCreateResult = CreateProcess(
		szEXE,					// ���s�\���W���[���̖��O
		szCmdLineBuf,			// �R�}���h���C���̕�����
		NULL,					// �Z�L�����e�B�L�q�q
		NULL,					// �Z�L�����e�B�L�q�q
		FALSE,					// �n���h���̌p���I�v�V����
		dwCreationFlag,			// �쐬�̃t���O
		NULL,					// �V�������u���b�N
		szCurDir,				// �J�����g�f�B���N�g���̖��O
		&s,						// �X�^�[�g�A�b�v���
		&p						// �v���Z�X���
	);
	if( !bCreateResult ){
		//	���s
		TCHAR* pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			_T("\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B\n%s"),
			szEXE,
			pMsg
		);
		::LocalFree( (HLOCAL)pMsg );	//	�G���[���b�Z�[�W�o�b�t�@�����
		return false;
	}

	bool bRet = true;
	if( sync ){
		//	�N�������v���Z�X�����S�ɗ����オ��܂ł�����Ƒ҂D
		int nResult = WaitForInputIdle( p.hProcess, 10000 );	//	�ő�10�b�ԑ҂�
		if( nResult != 0 ){
			ErrorMessage(
				hWndParent,
				_T("\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B"),
				szEXE
			);
			bRet = false;
		}
	}
	else{
		// �^�u�܂Ƃߎ��͋N�������v���Z�X�������オ��܂ł��΂炭�^�C�g���o�[���A�N�e�B�u�ɕۂ�	// 2007.02.03 ryoji
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
			WaitForInputIdle( p.hProcess, 3000 );
			sync = true;
		}
	}

	// MYWM_FIRST_IDLE ���͂��܂ł�����Ƃ����]���ɑ҂�	// 2008.04.19 ryoji
	// Note. �N����v���Z�X���������������� COM �֐��iSHGetFileInfo API �Ȃǂ��܂ށj�����s����ƁA
	//       ���̎��_�� COM �̓����@�\�������� WaitForInputIdle �͏I�����Ă��܂��\��������i�炵���j�B
	if( sync && bRet )
	{
		int i;
		for( i = 0; i < 200; i++ ){
			MSG msg;
			DWORD dwExitCode;
			if( ::PeekMessage( &msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE ) ){
				if( msg.message == WM_QUIT ){	// �w��͈͊O�ł� WM_QUIT �͎��o�����
					::PostQuitMessage( msg.wParam );
					break;
				}
				// �Ď��Ώۃv���Z�X����̃��b�Z�[�W�Ȃ甲����
				// �����łȂ���Δj�����Ď������o��
				if( msg.wParam == p.dwProcessId ){
					break;
				}
			}
			if( ::GetExitCodeProcess( p.hProcess, &dwExitCode ) && dwExitCode != STILL_ACTIVE ){
				break;	// �Ď��Ώۃv���Z�X���I������
			}
			::Sleep(10);
		}
	}

	CloseHandle( p.hThread );
	CloseHandle( p.hProcess );

	return bRet;
}


/*!	�V�K�ҏW�E�B���h�E�̒ǉ� ver 2:

	@date Oct. 24, 2000 genta create.
	@date Feb. 25, 2012 novice -CODE/-R��OpenNewEditor���ŏ�������̂ō폜
*/
bool CControlTray::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo*	pfi,
	bool			bReadOnly,
	bool			sync,
	bool			bNewWindow			//!< [in] �V�K�G�f�B�^��V�����E�C���h�E�ŊJ��
)
{
	DLLSHAREDATA*	pShareData;
	int				nPos = 0;		//	�����쐬�p�|�C���^

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	pShareData = CShareData::getInstance()->GetShareData();

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		return false;
	}

	// �ǉ��̃R�}���h���C���I�v�V����
	char pszCmdLine[1024];
	if( pfi != NULL ){
		if( pfi->m_szPath != NULL ){
			if( pfi->m_szPath[0] != '\0' ){
				nPos += wsprintf( pszCmdLine + nPos, _T(" \"%s\""), pfi->m_szPath );
			}
		}
		if( pfi->m_nX >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, _T(" -X=%d"), pfi->m_nX +1 );
		}
		if( pfi->m_nY >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, _T(" -Y=%d"), pfi->m_nY +1 );
		}
		if( pfi->m_nViewLeftCol >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, _T(" -VX=%d"), pfi->m_nViewLeftCol +1 );
		}
		if( pfi->m_nViewTopLine >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, _T(" -VY=%d"), pfi->m_nViewTopLine +1 );
		}
	}

	int nCharCode = pfi ? pfi->m_nCharCode : CODE_NONE;

	return OpenNewEditor( hInstance, hWndParent, pszCmdLine, nCharCode, bReadOnly, sync, NULL, bNewWindow );

}
//	To Here Oct. 24, 2000 genta



void CControlTray::ActiveNextWindow(HWND hwndParent)
{
	/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* �����̃E�B���h�E�𒲂ׂ� */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( hwndParent == pEditNodeArr[i].m_hWnd )
			{
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// �O�̃E�B���h�E
			int		j;
			for( j = i - 1; j >= 0; --j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j < 0 ){
				for( j = nRowNum - 1; j > i; --j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* �O�̃E�B���h�E���A�N�e�B�u�ɂ��� */
			HWND	hwndWork = pEditNodeArr[j].m_hWnd;
			ActivateFrameWindow( hwndWork );
			/* �Ō�̃y�C�����A�N�e�B�u�ɂ��� */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
		}
		delete [] pEditNodeArr;
	}
}

void CControlTray::ActivePrevWindow(HWND hwndParent)
{
	/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* �����̃E�B���h�E�𒲂ׂ� */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( hwndParent == pEditNodeArr[i].m_hWnd ){
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// ���̃E�B���h�E
			int		j;
			for( j = i + 1; j < nRowNum; ++j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j >= nRowNum ){
				for( j = 0; j < i; ++j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* ���̃E�B���h�E���A�N�e�B�u�ɂ��� */
			HWND	hwndWork = pEditNodeArr[j].m_hWnd;
			ActivateFrameWindow( hwndWork );
			/* �ŏ��̃y�C�����A�N�e�B�u�ɂ��� */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
		}
		delete [] pEditNodeArr;
	}
}



/*!	�T�N���G�f�B�^�̑S�I��

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2006.12.25 ryoji �����̕ҏW�E�B���h�E�����Ƃ��̊m�F�i�����ǉ��j
*/
void CControlTray::TerminateApplication(
	HWND hWndFrom	//!< [in] �Ăяo�����̃E�B���h�E�n���h��
)
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */

	/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	if( pShareData->m_Common.m_sGeneral.m_bExitConfirm ){	//�I�����̊m�F
		if( 0 < CShareData::getInstance()->GetEditorWindowsNum( 0 ) ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				_T("���݊J���Ă���ҏW�p�̃E�B���h�E�����ׂĕ��ďI�����܂���?")
			) ){
				return;
			}
		}
	}
	/* �u���ׂẴE�B���h�E�����v�v�� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	BOOL bCheckConfirm = (pShareData->m_Common.m_sGeneral.m_bExitConfirm)? FALSE: TRUE;	// 2006.12.25 ryoji �I���m�F�ς݂Ȃ炻��ȏ�͊m�F���Ȃ�
	if( CloseAllEditor( bCheckConfirm, hWndFrom, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
		::PostMessage( pShareData->m_sHandles.m_hwndTray, WM_CLOSE, 0, 0 );
	}
	return;
}




/*!	���ׂẴE�B���h�E�����

	@date Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2006.12.25 ryoji �����̕ҏW�E�B���h�E�����Ƃ��̊m�F�i�����ǉ��j
	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.20 ryoji nGroup������ǉ�
*/
BOOL CControlTray::CloseAllEditor(
	BOOL	bCheckConfirm,	//!< [in] [���ׂĕ���]�m�F�I�v�V�����ɏ]���Ė₢���킹�����邩�ǂ���
	HWND	hWndFrom,		//!< [in] �Ăяo�����̃E�B���h�E�n���h��
	BOOL	bExit,			//!< [in] TRUE: �ҏW�̑S�I�� / FALSE: ���ׂĕ���
	int		nGroup			//!< [in] �O���[�vID
)
{
	EditNode*	pWndArr;
	int		n;

	n = CShareData::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	/* �S�ҏW�E�B���h�E�֏I���v�����o�� */
	BOOL ret = CShareData::getInstance()->RequestCloseEditor( pWndArr, n, bExit, nGroup, bCheckConfirm, hWndFrom );	// 2007.02.13 ryoji bExit�������p��

	delete[] pWndArr;
	return ret;
}




/*! �|�b�v�A�b�v���j���[(�g���C���{�^��) */
int	CControlTray::CreatePopUpMenu_L( void )
{
	int			i;
	int			j;
	int			nId;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	TCHAR		szMenu[100 + MAX_PATH * 2];	//	Jan. 19, 2001 genta
	POINT		po;
	RECT		rc;
	EditInfo*	pfi;

	//�{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();
	CShareData::getInstance()->TransformFileName_MakeCache();

	// ���\�[�X���g��Ȃ��悤��
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_CMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenu, "TrayL", "" );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, _T(""), _T("G"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	const CMRUFile cMRU;
	hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	�t�@�C�����j���[
	if ( cMRU.MenuLength() > 0 ){
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�ŋߎg�����t�@�C��"), _T("F") );
	}
	else {
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("�ŋߎg�����t�@�C��"), _T("F") );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
	const CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.MenuLength() > 0 ){
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_"), _T("D") );
	}
	else {
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_"), _T("D") );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL, _T(""), _T("Z"), FALSE );	// Jan. 24, 2005 genta

	/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
	j = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			++j;
		}
	}

	if( j > 0 ){
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
		j = 0;
		for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
				/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
				::SendMessage( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
				pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

				// ���j���[���x���B1����A�N�Z�X�L�[��U��
				CShareData::getInstance()->GetMenuFullLabel_WinList( szMenu, _countof(szMenu), pfi, m_pShareData->m_sNodes.m_pEditArr[i].m_nId, i );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMenu, _T(""), FALSE );
				++j;
			}
		}
	}
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T(""), _T("Q"), FALSE );	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)	// 2006.10.21 ryoji �\��������ύX	// 2007.02.13 ryoji ��F_EXITALLEDITORS
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_EXITALLEDITORS, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	// 2007.02.13 ryoji ��F_EXITALLEDITORS
		::EnableMenuItem( hMenu, F_FILESAVEALL, MF_BYCOMMAND | MF_GRAYED );	// Jan. 24, 2005 genta
	}

	//	Jun. 9, 2001 genta �\�t�g�E�F�A������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), FALSE );	//Dec. 26, 2000 JEPRO F_�ɕύX

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( _T("nId=%d\n"), nId );

	m_bUseTrayMenu = false;

	return nId;
}

//�L�[���[�h�F�g���C�E�N���b�N���j���[����
//	Oct. 12, 2000 JEPRO �|�b�v�A�b�v���j���[(�g���C���{�^��) ���Q�l�ɂ��ĐV���ɒǉ���������

/*! �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */
int	CControlTray::CreatePopUpMenu_R( void )
{
	int		nId;
	HMENU	hMenuTop;
	HMENU	hMenu;
	POINT	po;
	RECT	rc;

	//�{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();

	// ���\�[�X���g��Ȃ��悤��
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_CMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenu, "TrayR", "" );

	/* �g���C�E�N���b�N�́u�w���v�v���j���[ */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T("�w���v�ڎ�"), _T("O"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , _T("�w���v�L�[���[�h����"), _T("S"), FALSE );	//Nov. 25, 2000 JEPRO �u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, _T("�o�[�W�������"), _T("A"), FALSE );	//Dec. 25, 2000 JEPRO F_�ɕύX
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
	//	Jun. 18, 2001 genta �\�t�g�E�F�A������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T("�T�N���G�f�B�^�̑S�I��"), _T("X"), FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( _T("nId=%d\n"), nId );

	m_bUseTrayMenu = false;

	return nId;
}

/*! �A�N�Z�����[�^�e�[�u���쐬
	@date 2013.04.20 novice ���ʏ������֐���
*/
void CControlTray::CreateAccelTbl( void )
{
	m_pShareData->m_sHandles.m_hAccel = CKeyBind::CreateAccerelator(
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);

	if( NULL == m_pShareData->m_sHandles.m_hAccel ){
		ErrorMessage(
			NULL,
			_T("CControlTray::CreateAccelTbl()\n")
			_T("�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n")
			_T("�V�X�e�����\�[�X���s�����Ă��܂��B")
		);
	}
}

/*! �A�N�Z�����[�^�e�[�u���j��
	@date 2013.04.20 novice ���ʏ������֐���
*/
void CControlTray::DeleteAccelTbl( void )
{
	if( m_pShareData->m_sHandles.m_hAccel ){
		::DestroyAcceleratorTable( m_pShareData->m_sHandles.m_hAccel );
		m_pShareData->m_sHandles.m_hAccel = NULL;
	}
}

/*!
	@brief WM_DESTROY ����
	@date 2006.07.09 ryoji �V�K�쐬
*/
void CControlTray::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (m_hWnd == NULL)
		return;	// ���ɔj������Ă���

	// �z�b�g�L�[�̔j��
	::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );

	// 2006.07.09 ryoji ���L�f�[�^�ۑ��� CControlProcess::Terminate() ����ړ�
	//
	// �u�^�X�N�g���C�ɏ풓���Ȃ��v�ݒ�ŃG�f�B�^��ʁiNormal Process�j�𗧂��グ���܂�
	// �Z�b�V�����I������悤�ȏꍇ�ł����L�f�[�^�ۑ����s���Ȃ������蒆�f����邱�Ƃ�
	// �����悤�A�����ŃE�B���h�E���j�������O�ɕۑ�����
	//

	/* �I���_�C�A���O��\������ */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* �I�����_�C�A���O�̕\�� */
		hwndExitingDlg = ::CreateDialog(
			m_hInstance,
			MAKEINTRESOURCE( IDD_EXITING ),
			m_hWnd/*::GetDesktopWindow()*/,
			(DLGPROC)ExitingDlgProc
		);
		::ShowWindow( hwndExitingDlg, SW_SHOW );
	}

	/* ���L�f�[�^�̕ۑ� */
	CShareData::getInstance()->SaveShareData();

	/* �I���_�C�A���O��\������ */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* �I�����_�C�A���O�̔j�� */
		::DestroyWindow( hwndExitingDlg );
	}

	if( m_bCreatedTrayIcon ){	/* �g���C�ɃA�C�R��������� */
		TrayMessage( m_hWnd, NIM_DELETE, 0, NULL, NULL );
	}

	// �A�N�Z�����[�^�e�[�u���̍폜
	DeleteAccelTbl();

	m_hWnd = NULL;
}

/*!
	@brief �I���_�C�A���O�p�v���V�[�W��
	@date 2006.07.02 ryoji CControlProcess ����ړ�
*/
INT_PTR CALLBACK CControlTray::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
)
{
	switch( uMsg ){
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

/*[EOF]*/
