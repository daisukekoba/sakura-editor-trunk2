/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, ao
	Copyright (C) 2001, MIK, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta, hor, aroka, minfu, �S, MIK, ai
	Copyright (C) 2003, genta, MIK, Moca, wmlhq, ryoji, KEITA
	Copyright (C) 2004, genta, Moca, yasu, MIK, novice, Kazika
	Copyright (C) 2005, genta, MIK, Moca, aroka, ryoji
	Copyright (C) 2006, genta, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji, maru, genta, Moca
	Copyright (C) 2008, ryoji, nasukoji, novice, syat
	Copyright (C) 2009, ryoji, nasukoji, Hidetaka Sakai, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"

#include <windows.h>
#include <winuser.h>
#include <io.h>
#include <mbctype.h>
#include <mbstring.h>
#include "CShareData.h"
#include "CRunningTimer.h"
#include "CEditApp.h"
#include "CEditWnd.h"
#include "CEditDoc.h"
#include "CDocLine.h"
#include "Debug.h"
#include "CDlgAbout.h"
#include "mymessage.h"
#include "CPrint.h"
#include "etc_uty.h"
#include "charcode.h"
#include "global.h"
#include "CDlgPrintSetting.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "CSMacroMgr.h" // Jun. 16, 2002 genta
#include "COsVersionInfo.h"	// Sep. 6, 2003 genta
#include "Keycode.h"
#include "sakura_rc.h"


//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ��������̂�
//	��`���폜

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif

#ifndef	WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif

// 2006.06.17 ryoji WM_THEMECHANGED
#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

#define		YOHAKU_X		4		/* �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l */
#define		YOHAKU_Y		4		/* �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ��������̂�
//	��`���폜


/*! �T�u�N���X�������c�[���o�[�̃E�B���h�E�v���V�[�W��
	@author ryoji
	@date 2006.09.06 ryoji
*/
static WNDPROC g_pOldToolBarWndProc;	// �c�[���o�[�̖{���̃E�B���h�E�v���V�[�W��

static LRESULT CALLBACK ToolBarWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	// WinXP Visual Style �̂Ƃ��Ƀc�[���o�[��ł̃}�E�X���E�{�^�����������Ŗ������ɂȂ�
	//�i�}�E�X���L���v�`���[�����܂ܕ����Ȃ��j ����������邽�߂ɉE�{�^���𖳎�����
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// �E�{�^���� UP/DOWN �͖{���̃E�B���h�E�v���V�[�W���ɓn���Ȃ�

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc );
		break;
	}
	return ::CallWindowProc( g_pOldToolBarWndProc, hWnd, msg, wParam, lParam );
}


//	/* ���b�Z�[�W���[�v */
//	DWORD MessageLoop_Thread( DWORD pCEditWndObject );

LRESULT CALLBACK CEditWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CEditWnd* pcWnd = ( CEditWnd* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
	if( pcWnd ){
		return pcWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditWnd::CEditWnd()
: m_hWnd( NULL )
, m_bDragMode( FALSE )
, m_hwndParent( NULL )
, m_hwndReBar( NULL )	// 2006.06.17 ryoji
, m_hwndToolBar( NULL )
, m_hwndStatusBar( NULL )
, m_hwndProgressBar( NULL )
, m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ) // 20020331 aroka �ĕϊ��Ή� for 95/NT
, m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
, m_pPrintPreview( NULL ) //@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
, m_pszLastCaption( NULL )
, m_hwndSearchBox( NULL )
, m_fontSearchBox( NULL )
, m_nCurrentFocus( 0 )
, m_bIsActiveApp( FALSE )
, m_IconClicked(icNone) //by �S(2)
, m_pszMenubarMessage( new TCHAR[MENUBAR_MESSAGE_MAX_LEN] )
{
	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	// Sep. 10, 2002 genta
	m_cEditDoc.m_pcEditWnd = this;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_pcDropTarget = new CDropTarget( this );	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji
	memset( m_pszMenubarMessage, ' ', MENUBAR_MESSAGE_MAX_LEN );	// null�I�[�͕s�v

	// 2009.01.12 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
	ClearMouseState();

	// 2009.08.15 Hidetaka Sakai, nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	if( m_pShareData->m_Common.m_bCreateAccelTblEachWin ){
		m_hAccel = CKeyBind::CreateAccerelator(
						m_pShareData->m_nKeyNameArrNum,
						m_pShareData->m_pKeyNameArr
		);

		if( NULL == m_hAccel ){
			::MessageBox(
				NULL,
				_T("CEditWnd::CEditWnd()\n")
				_T("�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n")
				_T("�V�X�e�����\�[�X���s�����Ă��܂��B"),
				GSTR_APPNAME, MB_OK | MB_ICONSTOP
			);
		}
	}else{
		m_hAccel = NULL;
	}
}

CEditWnd::~CEditWnd()
{
	delete[] m_pszMenubarMessage;
	delete[] m_pszLastCaption;

	//	Dec. 4, 2002 genta
	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;	// 2008.06.20 ryoji
	m_pcDropTarget = NULL;

	// 2009.08.15 Hidetaka Sakai, nasukoji	�E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u�����J������
	if( m_hAccel ){
		::DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
	}

	m_hWnd = NULL;
}





/*!
	�쐬

	@date 2002.03.07 genta nDocumentType�ǉ�
	@date 2007.06.26 ryoji nGroup�ǉ�
	@date 2008.04.19 ryoji ����A�C�h�����O���o�p�[���b�^�C�}�[�̃Z�b�g������ǉ�
*/
HWND CEditWnd::Create(
	HINSTANCE	hInstance,		//!< [in] Instance Handle
	HWND		hwndParent,		//!< [in] �e�E�B���h�E�̃n���h��
	int			nGroup			//!< [in] �O���[�vID
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	WNDCLASSEX	wc;
	HWND		hwndTop;
	WINDOWPLACEMENT	wpTop;
	ATOM		atom;

	if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		::MYMESSAGEBOX( NULL, MB_OK, GSTR_APPNAME, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		return NULL;
	}


	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= m_hInstance;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	wc.hIcon			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
	if( 0 == ( atom = RegisterClassEx( &wc ) ) ){
		//	2004.05.13 Moca return NULL��L���ɂ���
		return NULL;
	}

	/* �E�B���h�E�T�C�Y�p�� */
	int	nWinCX, nWinCY;
	//	2004.05.13 Moca m_Common.m_eSaveWindowSize��BOOL����enum�ɕς�������
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_eSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* �E�B���h�E�T�C�Y�w�� */
	EditInfo fi;
	CCommandLine::getInstance()->GetEditInfo(&fi);
	if( fi.m_nWindowSizeX >= 0 ){
		nWinCX = fi.m_nWindowSizeX;
	}
	if( fi.m_nWindowSizeY >= 0 ){
		nWinCY = fi.m_nWindowSizeY;
	}

	/* �E�B���h�E�ʒu�w�� */
	int nWinOX, nWinOY;
	nWinOX = CW_USEDEFAULT;
	nWinOY = 0;
	// �E�B���h�E�ʒu�Œ�
	//	2004.05.13 Moca �ۑ������E�B���h�E�ʒu���g���ꍇ�͋��L����������Z�b�g
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_eSaveWindowPos ){
		nWinOX =  m_pShareData->m_Common.m_nWinPosX;
		nWinOY =  m_pShareData->m_Common.m_nWinPosY;
	}

	//	2004.05.13 Moca �}���`�f�B�X�v���C�ł͕��̒l���L���Ȃ̂ŁC
	//	���ݒ�̔�����@��ύX�D(���̒l��CW_USEDEFAULT)
	if( fi.m_nWindowOriginX != CW_USEDEFAULT ){
		nWinOX = fi.m_nWindowOriginX;
	}
	if( fi.m_nWindowOriginY != CW_USEDEFAULT ){
		nWinOY = fi.m_nWindowOriginY;
	}

	//From Here @@@ 2003.05.31 MIK
	//�^�u�E�C���h�E�̏ꍇ�͌���l���w��
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		if( nGroup < 0 )	// �s���ȃO���[�vID
			nGroup = 0;	// �O���[�v�w�薳���i�ŋ߃A�N�e�B�u�̃O���[�v�ɓ����j
		EditNode*	pEditNode = CShareData::getInstance()->GetEditNodeAt( nGroup, 0 );	// �O���[�v�̐擪�E�B���h�E�����擾	// 2007.06.20 ryoji
		hwndTop = pEditNode? pEditNode->m_hWnd: NULL;

		if( hwndTop )
		{
			//	Sep. 11, 2003 MIK �V�KTAB�E�B���h�E�̈ʒu����ɂ���Ȃ��悤��
			// 2007.06.20 ryoji ��v���C�}�����j�^�܂��̓^�X�N�o�[�𓮂�������ł�����Ȃ��悤��
			RECT rcWork;
			RECT rcMon;
			GetMonitorWorkRect( hwndTop, &rcWork, &rcMon );

			wpTop.length = sizeof(wpTop);
			if( ::GetWindowPlacement( hwndTop, &wpTop ) ){	// ���݂̐擪�E�B���h�E����ʒu���擾
				nWinCX = wpTop.rcNormalPosition.right - wpTop.rcNormalPosition.left;
				nWinCY = wpTop.rcNormalPosition.bottom - wpTop.rcNormalPosition.top;
				nWinOX = wpTop.rcNormalPosition.left + (rcWork.left - rcMon.left);
				nWinOY = wpTop.rcNormalPosition.top + (rcWork.top - rcMon.top);
				if( wpTop.showCmd == SW_SHOWMINIMIZED )
					wpTop.showCmd = pEditNode->m_showCmdRestore;
			}
			else{
				hwndTop = NULL;
			}
		}
	}
	//To Here @@@ 2003.05.31 MIK

	HWND hWnd = ::CreateWindowEx(
		0,				 	// extended window style
		GSTR_EDITWINDOWNAME,		// pointer to registered class name
		GSTR_EDITWINDOWNAME,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		nWinOX,				// horizontal position of window
		nWinOY,				// vertical position of window
		nWinCX,				// window width
		nWinCY,				// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		m_hInstance,		// handle to application instance
		NULL				// pointer to window-creation data
	);
	m_hWnd = hWnd;

	// 2004.05.13 Moca �E�B���h�E�쐬���s�B�I��
	if( NULL == hWnd ){
		return NULL;
	}

	// ����A�C�h�����O���o�p�̃[���b�^�C�}�[���Z�b�g����	// 2008.04.19 ryoji
	// �[���b�^�C�}�[�������i����A�C�h�����O���o�j������ MYWM_FIRST_IDLE ���N�����v���Z�X�Ƀ|�X�g����B
	// ���N�����ł̋N����A�C�h�����O���o�ɂ��Ă� CEditApp::OpenNewEditor ���Q��
	::SetTimer( m_hWnd, IDT_FIRST_IDLE, 0, NULL );

	MyInitCommonControls();	// 2006.06.19 ryoji �R�����R���g���[���̏������� CreateToolBar() ����ړ�


	m_cIcons.Create( m_hInstance );	//	CreateImage List

	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_cIcons );

	// �e��o�[������� m_cEditDoc.Create() �����s���Ă���	// 2007.01.30 ryoji
	// �im_cEditDoc �����o�[�̏�������D��j
	if( FALSE == m_cEditDoc.Create( m_hInstance, m_hWnd, &m_cIcons/*, 1, 1, 0, 0*/ ) ){
		::MessageBox(
			m_hWnd,
			"�N���C�A���g�E�B���h�E�̍쐬�Ɏ��s���܂���", GSTR_APPNAME,
			MB_OK
		);
	}

	// 2007.03.08 ryoji �e��o�[�̍쐬�� Layoutxxx(), EndLayoutBars() �Ŋȑf��

// ����SetWindowLongPtr�ȍ~���ƃf�o�b�O���ɗ����邱�Ƃ��������̂ŏ��Ԃ����ւ����B // 2005/8/9 aroka
	/* �c�[���o�[ */
	LayoutToolBar();

	/* �X�e�[�^�X�o�[ */
	LayoutStatusBar();

	/* �t�@���N�V�����L�[ �o�[ */
	LayoutFuncKey();

	/* �^�u�E�C���h�E */
	LayoutTabBar();

	/* �o�[�̔z�u�I�� */
	EndLayoutBars( FALSE );

	// ��ʕ\�����O��DispatchEvent��L��������
	::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)this );

	/* �f�X�N�g�b�v����͂ݏo���Ȃ��悤�ɂ��� */
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( m_hWnd, &rcDesktop );
	::GetWindowRect( m_hWnd, &rcOrg );

	// 2005.11.23 Moca �}���`���j�^���Ŗ�肪���������ߌv�Z���@�ύX
	/* �E�B���h�E�ʒu���� */
	if( rcOrg.bottom > rcDesktop.bottom ){
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right > rcDesktop.right ){
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	
	if( rcOrg.top < rcDesktop.top ){
		rcOrg.bottom += rcDesktop.top - rcOrg.top;
		rcOrg.top = rcDesktop.top;
	}
	if( rcOrg.left < rcDesktop.left ){
		rcOrg.right += rcDesktop.left - rcOrg.left;
		rcOrg.left = rcDesktop.left;
	}

	/* �E�B���h�E�T�C�Y���� */
	if( rcOrg.bottom > rcDesktop.bottom ){
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right > rcDesktop.right ){
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}

	//From Here @@@ 2003.06.13 MIK
	if( m_pShareData->m_Common.m_bDispTabWnd
		&& !m_pShareData->m_Common.m_bDispTabWndMultiWin
		&& hwndTop )
	{
		// ���݂̐擪�E�B���h�E���� WS_EX_TOPMOST ��Ԃ������p��	// 2007.05.18 ryoji
		DWORD dwExStyle = (DWORD)::GetWindowLongPtr( hwndTop, GWL_EXSTYLE );
		::SetWindowPos( m_hWnd, (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

		//�^�u�E�C���h�E���͌�����ێ�
		/* �E�B���h�E�T�C�Y�p�� */
		if( wpTop.showCmd == SW_SHOWMAXIMIZED )
		{
			::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
		}
		else
		{
			::ShowWindow( m_hWnd, SW_SHOW );
		}
	}
	else
	{
		::SetWindowPos(
			m_hWnd, 0,
			rcOrg.left, rcOrg.top,
			rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		/* �E�B���h�E�T�C�Y�p�� */
		if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_nWinSizeType == SIZE_MAXIMIZED ){
			::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
		}else
		// 2004.05.14 Moca �E�B���h�E�T�C�Y�𒼐ڎw�肷��ꍇ�́A�ŏ����\�����󂯓����
		if( WINSIZEMODE_SET == m_pShareData->m_Common.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_nWinSizeType == SIZE_MINIMIZED ){
			::ShowWindow( m_hWnd, SW_SHOWMINIMIZED );
		}
		else{
			::ShowWindow( m_hWnd, SW_SHOW );
		}
	}
	//To Here @@@ 2003.06.13 MIK

	// �h���b�v���ꂽ�t�@�C�����󂯓����
	::DragAcceptFiles( m_hWnd, TRUE );
	m_pcDropTarget->Register_DropTarget( m_hWnd );	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

	//�A�N�e�B�u���
	m_bIsActiveApp = ( ::GetActiveWindow() == m_hWnd );	// 2007.03.08 ryoji

	// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F�iVista UIPI�@�\�j  2007.06.07 ryoji
	if( COsVersionInfo().IsWinVista_or_later() ){
		m_bUIPI = FALSE;
		::SendMessage( m_pShareData->m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)m_hWnd );
		if( !m_bUIPI ){	// �Ԏ����Ԃ�Ȃ�
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				_T("�G�f�B�^�Ԃ̑Θb�Ɏ��s���܂����B\n")
				_T("�������x���̈قȂ�G�f�B�^�����ɋN�����Ă���\��������܂��B")
			);
			::DestroyWindow( m_hWnd );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	/* �ҏW�E�B���h�E���X�g�ւ̓o�^ */
	if( FALSE == CShareData::getInstance()->AddEditWndList( m_hWnd, nGroup ) ){	// 2007.06.26 ryoji nGroup�����ǉ�
		::MYMESSAGEBOX( m_hWnd, MB_OK, GSTR_APPNAME, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::DestroyWindow( m_hWnd );
		m_hWnd = hWnd = NULL;
		return hWnd;
	}
	CShareData::getInstance()->SetTraceOutSource( m_hWnd );	// TraceOut()�N�����E�B���h�E�̐ݒ�	// 2006.06.26 ryoji

	//	Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	/* �^�C�}�[���N�� */ // �^�C�}�[��ID�ƊԊu��ύX 20060128 aroka
	if( 0 == ::SetTimer( m_hWnd, IDT_EDIT, 500, NULL ) ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("CEditWnd::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
	}
	// �c�[���o�[�̃^�C�}�[�𕪗����� 20060128 aroka
	Timer_ONOFF( TRUE );

	//�f�t�H���g��IME���[�h�ݒ�
	m_cEditDoc.SetImeMode( m_pShareData->m_Types[0].m_nImeState );

	return m_hWnd;
}



//! �N�����̃t�@�C���I�[�v������
void CEditWnd::OpenDocumentWhenStart(
	const char*	pszPath,		//!< [in] �ŏ��ɊJ���t�@�C���̃p�X�DNULL�̂Ƃ��J���t�@�C�������D
	ECodeType	nCharCode,		//!< [in] �����R�[�h
	BOOL		bReadOnly		//!< [in] �ǂݎ���p�ŊJ�����ǂ���
)
{
	if( pszPath ){
		char*	pszPathNew = new char[_MAX_PATH];
		strcpy( pszPathNew, pszPath );
		::ShowWindow( m_hWnd, SW_SHOW );
		//	Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
		BOOL		bOpened;
		BOOL		bReadResult = m_cEditDoc.FileRead( pszPathNew, &bOpened, nCharCode, bReadOnly, m_pShareData->m_Common.m_bQueryIfCodeChange );
		if( !bReadResult ){
			/* �t�@�C�������ɊJ����Ă��� */
			if( bOpened ){
				::PostMessage( m_hWnd, WM_CLOSE, 0, 0 );
				// 2004.07.12 Moca return NULL���ƁA���b�Z�[�W���[�v��ʂ炸�ɂ��̂܂ܔj������Ă��܂��A�^�u�̏I��������������
				//	���̌�͐��탋�[�g�Ń��b�Z�[�W���[�v�ɓ�������WM_CLOSE����M���Ē�����CLOSE & DESTROY�ƂȂ�D
				//	���̒��ŕҏW�E�B���h�E�̍폜���s����D
			}
		}
		delete [] pszPathNew;
	}
}

void CEditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		//!< [in] �����R�[�h
	BOOL			bReadOnly,		//!< [in] �ǂݎ���p�ŊJ�����ǂ���
	int				nDocumentType	//!< [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
)
{
	//	Mar. 7, 2002 genta �����^�C�v�̋����w��
	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��^�C�v�����w���L���ɂ���
	if( nDocumentType >= 0 ){
		m_cEditDoc.SetDocumentType( nDocumentType, true );
		//	2002/05/07 YAZAKI �^�C�v�ʐݒ�ꗗ�̈ꎞ�K�p�̃R�[�h�𗬗p
		m_cEditDoc.LockDocumentType();
	}

	// �����R�[�h�̎w��	2008/6/14 Uchi
	if( IsValidCodeType( nCharCode ) || nDocumentType >= 0 ){
		STypeConfig& types = m_cEditDoc.GetDocumentAttribute();
		ECodeType eDefaultCharCode = static_cast<ECodeType>(types.m_eDefaultCodetype);
		if( !IsValidCodeType( nCharCode ) ){
			nCharCode = eDefaultCharCode;	// ���ڃR�[�h�w�肪�Ȃ���΃^�C�v�w��̃f�t�H���g�����R�[�h���g�p
		}
		m_cEditDoc.m_nCharCode = nCharCode;
		if( nCharCode == eDefaultCharCode ){	// �f�t�H���g�����R�[�h�Ɠ��������R�[�h���I�����ꂽ�Ƃ�
			m_cEditDoc.m_bBomExist = ( types.m_bDefaultBom != FALSE );
			m_cEditDoc.SetNewLineCode( static_cast<EEolType>( types.m_eDefaultEoltype ) );
		}
		else{
			m_cEditDoc.m_bBomExist = ( nCharCode == CODE_UNICODE || nCharCode == CODE_UNICODEBE );
			m_cEditDoc.SetNewLineCode( EOL_CRLF );
		}
	}

	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��ǂݎ���p�����w���L���ɂ���
	m_cEditDoc.m_bReadOnly = bReadOnly;

	if( nDocumentType >= 0 ){
		/* �ݒ�ύX�𔽉f������ */
		m_cEditDoc.OnChangeSetting();
	}
}


//	�L�[���[�h�F�X�e�[�^�X�o�[����
/* �X�e�[�^�X�o�[�쐬 */
void CEditWnd::CreateStatusBar( void )
{
	if( m_hwndStatusBar )return;

	/* �X�e�[�^�X�o�[ */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE ����
		_T(""),
		m_hWnd,
		IDW_STATUSBAR
	);

	/* �v���O���X�o�[ */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		(LPSTR) NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		m_hInstance,
		0
	);

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		m_CFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}

	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1);
}


/* �X�e�[�^�X�o�[�j�� */
void CEditWnd::DestroyStatusBar( void )
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		BOOL bSizeBox;
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
			bSizeBox = FALSE;
		}
		else{
			bSizeBox = TRUE;
			/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1 );
}

/* �c�[���o�[�쐬
	@date @@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2005.08.29 aroka �c�[���o�[�̐܂�Ԃ�
	@date 2006.06.17 ryoji �r�W���A���X�^�C�����L���̏ꍇ�̓c�[���o�[�� Rebar �ɓ���ăT�C�Y�ύX���̂�����𖳂���
*/
void CEditWnd::CreateToolBar( void )
{
	if( m_hwndToolBar )return;

	REBARINFO		rbi;
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	UINT			uToolType;
	nFlag = 0;

	// 2006.06.17 ryoji
	// Rebar �E�B���h�E�̍쐬
	if( IsVisualStyle() ){	// �r�W���A���X�^�C���L��
		m_hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, //���o�[�R���g���[��
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |	// 2007.03.08 ryoji WS_VISIBLE ����
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			m_hWnd,
			NULL,
			m_hInstance,
			NULL
		);

		if( NULL == m_hwndReBar ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME, _T("Rebar �̍쐬�Ɏ��s���܂����B") );
			return;
		}

		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			PreventVisualStyle( m_hwndReBar );	// �r�W���A���X�^�C����K�p�̃t���b�g�� Rebar �ɂ���
		}

		::ZeroMemory(&rbi, sizeof(rbi));
		rbi.cbSize = sizeof(rbi);
		::SendMessage(m_hwndReBar, RB_SETBARINFO, 0, (LPARAM)&rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// �c�[���o�[�ւ̒ǉ��X�^�C��
	}

	/* �c�[���o�[�E�B���h�E�̍쐬 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */	// 2006.06.17 ryoji WS_CLIPCHILDREN �ǉ�	// 2007.03.08 ryoji WS_VISIBLE ����
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		m_hWnd,
		(HMENU)ID_TOOLBAR,
		m_hInstance,
		NULL
	);
	if( NULL == m_hwndToolBar ){
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			m_pShareData->m_Common.m_bToolBarIsFlat = FALSE;
		}
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME, _T("�c�[���o�[�̍쐬�Ɏ��s���܂����B") );
		DestroyToolBar();	// 2006.06.17 ryoji
	}
	else{
		// 2006.09.06 ryoji �c�[���o�[���T�u�N���X������
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			m_hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		::SendMessage( m_hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
		//	Oct. 12, 2000 genta
		//	���ɗp�ӂ���Ă���Image List���A�C�R���Ƃ��ēo�^
		m_cIcons.SetToolBarImages( m_hwndToolBar );
		/* �c�[���o�[�Ƀ{�^����ǉ� */
		int count = 0;	//@@@ 2002.06.15 MIK
		int nToolBarButtonNum = 0;// 2005/8/29 aroka
		//	From Here 2005.08.29 aroka
		// �͂��߂Ƀc�[���o�[�\���̂̔z�������Ă���
		TBBUTTON *pTbbArr = new TBBUTTON[m_pShareData->m_Common.m_nToolBarButtonNum];
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			nIdx = m_pShareData->m_Common.m_nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = m_CMenuDrawer.getButton(nIdx);
			// �Z�p���[�^�������Ƃ��͂ЂƂɂ܂Ƃ߂�
			// �܂�Ԃ��{�^����TBSTYLE_SEP�����������Ă���̂�
			// �܂�Ԃ��̑O�̃Z�p���[�^�͑S�č폜�����D
			if( (pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum!=0)){
				if( (pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP) ){
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					nToolBarButtonNum--;
				}
			}
			// ���z�ܕԂ��{�^���������璼�O�̃{�^���ɐܕԂ�������t����
			if( pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP ){
				if( nToolBarButtonNum!=0 ){
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			nToolBarButtonNum++;
		}
		//	To Here 2005.08.29 aroka

		for( i = 0; i < nToolBarButtonNum; ++i ){
			tbb = pTbbArr[i];

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//�h���b�v�_�E��
				//�g���X�^�C���ɐݒ�
				::SendMessage( m_hwndToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//�R���{�{�b�N�X
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT			lf;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//�Z�p���[�^���
						memset( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  //�{�^���ɂ��Ȃ��ƕ`�悪����� 2005/8/29 aroka
						my_tbb.idCommand = tbb.idCommand;	//����ID�ɂ��Ă���
						if( tbb.fsState & TBSTATE_WRAP ){   //�܂�Ԃ� 2005/8/29 aroka
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&my_tbb );
						count++;

						//�T�C�Y��ݒ肷��
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = 160;	//�{�b�N�X�̕�
						::SendMessage( m_hwndToolBar, TB_SETBUTTONINFO, (WPARAM)(tbb.idCommand), (LPARAM)&tbi );

						//�ʒu�ƃT�C�Y���擾����
						rc.right = rc.left = rc.top = rc.bottom = 0;
						::SendMessage( m_hwndToolBar, TB_GETITEMRECT, (WPARAM)(count-1), (LPARAM)&rc );

						//�R���{�{�b�N�X�����
						//	Mar. 8, 2003 genta �����{�b�N�X��1�h�b�g���ɂ��炵��
						m_hwndSearchBox = CreateWindow( _T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, m_hInstance, NULL );
						if( m_hwndSearchBox )
						{
							m_nCurrentFocus = 0;

							memset( &lf, 0, sizeof(lf) );
							lf.lfHeight			= 12; // Jan. 14, 2003 genta �_�C�A���O�ɂ��킹�Ă�����Ə�����
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							lf.lfCharSet		= SHIFTJIS_CHARSET;
							lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
							lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
							lf.lfQuality		= DEFAULT_QUALITY;
							lf.lfPitchAndFamily	= FF_MODERN | DEFAULT_PITCH;
							_tcscpy( lf.lfFaceName, _T("�l�r �o�S�V�b�N") );
							m_fontSearchBox = ::CreateFontIndirect( &lf );
							if( m_fontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (WPARAM)m_fontSearchBox, MAKELONG (TRUE, 0) );
							}

							//���͒�����
							::SendMessage( m_hwndSearchBox, CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

							//�����{�b�N�X���X�V	// �֐��� 2010/6/6 Uchi
							AcceptSharedSearchKey();
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//�{�^��
			case TBSTYLE_SEP:		//�Z�p���[�^
			default:
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			uToolType = (UINT)::GetWindowLong(m_hwndToolBar, GWL_STYLE);
			uToolType |= (TBSTYLE_FLAT);
			::SetWindowLong(m_hwndToolBar, GWL_STYLE, uToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
		delete []pTbbArr;// 2005/8/29 aroka
	}

	// 2006.06.17 ryoji
	// �c�[���o�[�� Rebar �ɓ����
	if( m_hwndReBar && m_hwndToolBar ){
		// �c�[���o�[�̍������擾����
		DWORD dwBtnSize = ::SendMessage( m_hwndToolBar, TB_GETBUTTONSIZE, 0, 0 );
		DWORD dwRows = ::SendMessage( m_hwndToolBar, TB_GETROWS, 0, 0 );

		// �o���h����ݒ肷��
		// �ȑO�̃v���b�g�t�H�[���� _WIN32_WINNT >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����	// 2007.12.21 ryoji
		rbBand.cbSize = CCSIZEOF_STRUCT( REBARBANDINFO, wID );
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = m_hwndToolBar;	// �c�[���o�[
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// �o���h��ǉ�����
		::SendMessage( m_hwndReBar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
	}

	return;
}

void CEditWnd::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_fontSearchBox )
			{
				::DeleteObject( m_fontSearchBox );
				m_fontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_nCurrentFocus = 0;
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;

		//if( m_cTabWnd.m_hWnd ) ::UpdateWindow( m_cTabWnd.m_hWnd );
		//if( m_CFuncKeyWnd.m_hWnd ) ::UpdateWindow( m_CFuncKeyWnd.m_hWnd );
	}

	// 2006.06.17 ryoji Rebar ��j������
	if( m_hwndReBar )
	{
		::DestroyWindow( m_hwndReBar );
		m_hwndReBar = NULL;
	}

	return;
}

/*! �c�[���o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutToolBar( void )
{
	if( m_pShareData->m_Common.m_bDispTOOLBAR ){	/* �c�[���o�[��\������ */
		CreateToolBar();
	}else{
		DestroyToolBar();
	}
}

/*! �X�e�[�^�X�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutStatusBar( void )
{
	if( m_pShareData->m_Common.m_bDispSTATUSBAR ){	/* �X�e�[�^�X�o�[��\������ */
		/* �X�e�[�^�X�o�[�쐬 */
		CreateStatusBar();
	}
	else{
		/* �X�e�[�^�X�o�[�j�� */
		DestroyStatusBar();
	}
}

/*! �t�@���N�V�����L�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutFuncKey( void )
{
	if( m_pShareData->m_Common.m_bDispFUNCKEYWND ){	/* �t�@���N�V�����L�[��\������ */
		if( NULL == m_CFuncKeyWnd.m_hWnd ){
			BOOL	bSizeBox;
			if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
				bSizeBox = FALSE;
			}else{
				bSizeBox = TRUE;
				/* �X�e�[�^�X�o�[������Ƃ��̓T�C�Y�{�b�N�X��\�����Ȃ� */
				if( m_hwndStatusBar ){
					bSizeBox = FALSE;
				}
			}
			m_CFuncKeyWnd.Open( m_hInstance, m_hWnd, &m_cEditDoc, bSizeBox );
		}
	}else{
		m_CFuncKeyWnd.Close();
	}
}

/*! �^�u�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutTabBar( void )
{
	if( m_pShareData->m_Common.m_bDispTabWnd ){	/* �^�u�o�[��\������ */
		if( NULL == m_cTabWnd.m_hWnd ){
			m_cTabWnd.Open( m_hInstance, m_hWnd );
		}
	}else{
		m_cTabWnd.Close();
	}
}

/*! �o�[�̔z�u�I������
	@date 2006.12.19 ryoji �V�K�쐬
	@data 2007.03.04 ryoji ����v���r���[���̓o�[���B��
*/
void CEditWnd::EndLayoutBars( BOOL bAdjust/* = TRUE*/ )
{
	int nCmdShow = m_pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;
	if( NULL != hwndToolBar )
		::ShowWindow( hwndToolBar, nCmdShow );
	if( NULL != m_hwndStatusBar )
		::ShowWindow( m_hwndStatusBar, nCmdShow );
	if( NULL != m_CFuncKeyWnd.m_hWnd )
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, nCmdShow );
	if( NULL != m_cTabWnd.m_hWnd )
		::ShowWindow( m_cTabWnd.m_hWnd, nCmdShow );

	if( bAdjust )
	{
		RECT		rc;
		m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1 );
		::GetClientRect( m_hWnd, &rc );
		::SendMessage( m_hWnd, WM_SIZE, m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		::RedrawWindow( m_hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW );	// �X�e�[�^�X�o�[�ɕK�v�H

		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].SetIMECompFormPos();
	}
}


//�����v���Z�X��
/* ���b�Z�[�W���[�v */
//2004.02.17 Moca GetMessage�̃G���[�`�F�b�N
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	int ret;
	
	while ( m_hWnd && ( ret = GetMessage( &msg, NULL, 0, 0 ) ) )
	{
		if( ret == -1 ){
			break;
		}

		if( m_pPrintPreview && NULL != m_pPrintPreview->GetPrintPreviewBarHANDLE() && ::IsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE(), &msg ) ){}	//!< ����v���r���[ ����o�[
		else if( m_cEditDoc.m_cDlgFind.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFind.m_hWnd, &msg ) ){}	//!<�u�����v�_�C�A���O
		else if( m_cEditDoc.m_cDlgFuncList.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFuncList.m_hWnd, &msg ) ){}	//!<�u�A�E�g���C���v�_�C�A���O
		else if( m_cEditDoc.m_cDlgReplace.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgReplace.m_hWnd, &msg ) ){}	//!<�u�u���v�_�C�A���O
		else if( m_cEditDoc.m_cDlgGrep.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgGrep.m_hWnd, &msg ) ){}	//!<�uGrep�v�_�C�A���O
		else if( m_cEditDoc.m_cHokanMgr.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cHokanMgr.m_hWnd, &msg ) ){}	//!<�u���͕⊮�v
		else if( m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, &msg ) ){	/* �u�����{�b�N�X�v */
			ProcSearchBox( &msg );
		}
		else{
			// 2009.08.15 Hidetaka Sakai, nasukoji
			// Wine�ł͕ʃv���Z�X�ō쐬�����A�N�Z�����[�^�e�[�u�����g�p���邱�Ƃ��ł��Ȃ��B
			// m_bCreateAccelTblEachWin�I�v�V�����I���ɂ��v���Z�X���ɃA�N�Z�����[�^�e�[�u�����쐬�����悤�ɂȂ�
			// ���߁A�V���[�g�J�b�g�L�[��J�[�\���L�[������ɏ��������悤�ɂȂ�B
			HACCEL hAccel = m_pShareData->m_Common.m_bCreateAccelTblEachWin ? m_hAccel : m_pShareData->m_hAccel;

			if( NULL != hAccel ){
				if( TranslateAccelerator( msg.hwnd, hAccel, &msg ) ){}
				else{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}
			else{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
}


LRESULT CEditWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int					nRet;
	int					idCtrl;
	LPNMHDR				pnmh;
	LPTOOLTIPTEXT		lptip;
	int					nPane;
	EditInfo*			pfi;
	int					nCaretPosX;
	int					nCaretPosY;
	POINT*				ppoCaret;
	LPHELPINFO			lphi;
	const char*			pLine;
	int					nLineLen;

	UINT				idCtl;	/* �R���g���[����ID */
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	UINT				fuFlags;
	HMENU				hmenu;
	LRESULT				lRes;

	switch( uMsg ){
	case WM_PAINTICON:
		return 0;
	case WM_ICONERASEBKGND:
		return 0;
	case WM_LBUTTONDOWN:
		return OnLButtonDown( wParam, lParam );
	case WM_MOUSEMOVE:
		return OnMouseMove( wParam, lParam );
	case WM_LBUTTONUP:
		return OnLButtonUp( wParam, lParam );
	case WM_MOUSEWHEEL:
		return OnMouseWheel( wParam, lParam );
	case WM_HSCROLL:
		return OnHScroll( wParam, lParam );
	case WM_VSCROLL:
		return OnVScroll( wParam, lParam );


	case WM_MENUCHAR:
		/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );

	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		if( !wParam ){
			m_cEditDoc.DeleteCompatibleBitmap();
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_MENUSELECT:
		if( NULL == m_hwndStatusBar ){
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		fuFlags = (UINT) HIWORD(wParam);	// menu flags
		hmenu = (HMENU) lParam;				// handle to menu clicked
		{
			/* ���j���[�@�\�̃e�L�X�g���Z�b�g */
			CMemory		cmemWork;

			/* �@�\�ɑΉ�����L�[���̎擾(����) */
			CMemory**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				m_hInstance,
				m_pShareData->m_nKeyNameArrNum,
				(KEYDATA*)m_pShareData->m_pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if( 0 < nAssignedKeyNum ){
				for( j = 0; j < nAssignedKeyNum; ++j ){
					if( j > 0 ){
						cmemWork.AppendString(_T(" , "));
					}
					cmemWork.AppendNativeData( *ppcAssignedKeyList[j] );
					delete ppcAssignedKeyList[j];
				}
				delete [] ppcAssignedKeyList;
			}

			const TCHAR* pszItemStr = cmemWork.GetStringPtr();

			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)pszItemStr );


		}
		return 0;


	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* �R���g���[����ID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		if( IDW_STATUSBAR == idCtl ){
			if( 5 == lpdis->itemID ){ // 2003.08.26 Moca id������č�悳��Ȃ�����
				int	nColor;
				if( m_pShareData->m_bRecordingKeyMacro	/* �L�[�{�[�h�}�N���̋L�^�� */
				 && m_pShareData->m_hwndRecordingKeyMacro == m_hWnd	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				){
					nColor = COLOR_BTNTEXT;
				}else{
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor( lpdis->hDC, ::GetSysColor( nColor ) );
				::SetBkMode( lpdis->hDC, TRANSPARENT );
				
				// 2003.08.26 Moca �㉺�����ʒu�ɍ��
				TEXTMETRIC tm;
				::GetTextMetrics( lpdis->hDC, &tm );
				int y = ( lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1 ) / 2 + lpdis->rcItem.top;
				::TextOut( lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen( _T("REC") ) );
				if( COLOR_BTNTEXT == nColor ){
					::TextOut( lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen( _T("REC") ) );
				}
			}
			return 0;
		}else{
			switch( lpdis->CtlType ){
			case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
				/* ���j���[�A�C�e���`�� */
				m_CMenuDrawer.DrawItem( lpdis );
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
//			CMenuDrawer* pCMenuDrawer;
//			pCMenuDrawer = (CMenuDrawer*)lpmis->itemData;


//			MYTRACE_A( "WM_MEASUREITEM  lpmis->itemID=%d\n", lpmis->itemID );
			/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( 0 < nItemWidth ){
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;




	case WM_PAINT:
		return OnPaint( hwnd, uMsg, wParam, lParam );

	case WM_PASTE:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_PASTE, TRUE, 0, 0, 0, 0 );

	case WM_COPY:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_COPY, TRUE, 0, 0, 0, 0 );

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			CEditApp::ShowFuncHelp( hwnd, lphi->iCtrlId );
			break;
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		m_bIsActiveApp = (BOOL)wParam;	// ���A�v�����A�N�e�B�u���ǂ���

		// �A�N�e�B�u���Ȃ�ҏW�E�B���h�E���X�g�̐擪�Ɉړ�����		// 2007.04.08 ryoji WM_SETFOCUS ����ړ�
		if( m_bIsActiveApp ){
			CShareData::getInstance()->AddEditWndList( m_hWnd );	// ���X�g�ړ�����

			// 2009.01.12 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
			ClearMouseState();
		}

		// �L���v�V�����ݒ�A�^�C�}�[ON/OFF		// 2007.03.08 ryoji WM_ACTIVATE����ړ�
		m_cEditDoc.UpdateCaption();
		m_CFuncKeyWnd.Timer_ONOFF( m_bIsActiveApp ); // 20060126 aroka
		this->Timer_ONOFF( m_bIsActiveApp ); // 20060128 aroka

		return 0L;

	case WM_ENABLE:
		// �E�h���b�v�t�@�C���̎󂯓���ݒ�^����	// 2009.01.09 ryoji
		// Note: DragAcceptFiles��K�p�������h���b�v�ɂ��Ă� Enable/Disable �Ŏ����I�Ɏ󂯓���ݒ�^�������؂�ւ��
		if( (BOOL)wParam ){
			m_pcDropTarget->Register_DropTarget( m_hWnd );
		}else{
			m_pcDropTarget->Revoke_DropTarget();
		}
		return 0L;

	case WM_WINDOWPOSCHANGED:
		// �|�b�v�A�b�v�E�B���h�E�̕\���ؑ֎w�����|�X�g����	// 2007.10.22 ryoji
		// �EWM_SHOWWINDOW�͂��ׂĂ̕\���ؑւŌĂ΂��킯�ł͂Ȃ��̂�WM_WINDOWPOSCHANGED�ŏ���
		//   �i�^�u�O���[�v�����Ȃǂ̐ݒ�ύX����WM_SHOWWINDOW�͌Ă΂�Ȃ��j
		// �E�����ؑւ��ƃ^�u�ؑւɊ����Č��̃^�u�ɖ߂��Ă��܂����Ƃ�����̂Ō�Ő؂�ւ���
		WINDOWPOS* pwp;
		pwp = (WINDOWPOS*)lParam;
		if( pwp->flags & SWP_SHOWWINDOW )
			::PostMessage( hwnd, MYWM_SHOWOWNEDPOPUPS, TRUE, 0 );
		else if( pwp->flags & SWP_HIDEWINDOW )
			::PostMessage( hwnd, MYWM_SHOWOWNEDPOPUPS, FALSE, 0 );

		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );

	case MYWM_SHOWOWNEDPOPUPS:
		::ShowOwnedPopups( m_hWnd, (BOOL)wParam );	// 2007.10.22 ryoji
		return 0L;

	case WM_SIZE:
//		MYTRACE_A( "WM_SIZE\n" );
		/* WM_SIZE ���� */
		if( SIZE_MINIMIZED == wParam ){
			m_cEditDoc.UpdateCaption();
		}
		return OnSize( wParam, lParam );

	//From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		//	�Ō�̈ʒu�𕜌����邽�߁C�ړ�����邽�тɋ��L�������Ɉʒu��ۑ�����D
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_eSaveWindowPos ){
			if( !::IsZoomed( m_hWnd ) && !::IsIconic( m_hWnd ) ){
				// 2005.11.23 Moca ���[�N�G���A���W���Ƃ����̂ŃX�N���[�����W�ɕύX
				RECT rcWin;
				::GetWindowRect( hwnd, &rcWin);
				m_pShareData->m_Common.m_nWinPosX = rcWin.left;
				m_pShareData->m_Common.m_nWinPosY = rcWin.top;
			}
		}
		// To Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	//To here 2003.05.31 MIK
	case WM_SYSCOMMAND:
		// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
		//	Feb. 11, 2007 genta �����I�ׂ�悤��(MDI���Ə]������)
		// 2007.02.22 ryoji Alt+F4 �̃f�t�H���g�@�\�Ń��[�h���̓��삪������悤�ɂȂ���
		if( wParam == SC_CLOSE ){
			// ����v���r���[���[�h�ŃE�B���h�E����鑀��̂Ƃ��̓v���r���[�����	// 2007.03.04 ryoji
			if( m_pPrintPreview ){
				PrintPreviewModeONOFF();	// ����v���r���[���[�h�̃I��/�I�t
				return 0L;
			}
			OnCommand( 0, CKeyBind::GetDefFuncCode( VK_F4, _ALT ), NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	case WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR ) {
			/* ���b�Z�[�W�̔z�� */
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	//case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYUP:	// 2004.04.28 Moca ALT+�L�[�̃L�[���s�[�g�����̂��ߒǉ�
	case WM_ENTERMENULOOP:
	case MYWM_IME_REQUEST:   /*  �ĕϊ��Ή� by minfu 2002.03.27  */ // 20020331 aroka
		/* ���b�Z�[�W�̔z�� */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_EXITMENULOOP:
//		MYTRACE_A( "WM_EXITMENULOOP\n" );
		if( NULL != m_hwndStatusBar ){
			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)_T(""));
		}
		/* ���b�Z�[�W�̔z�� */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_SETFOCUS:
//		MYTRACE_A( "WM_SETFOCUS\n" );

		// Aug. 29, 2003 wmlhq & ryoji�t�@�C���̃^�C���X�^���v�̃`�F�b�N���� OnTimer �Ɉڍs
		m_nTimerCount = 9;

		// �r���[�Ƀt�H�[�J�X���ړ�����	// 2007.10.16 ryoji
		::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );
		lRes = 0;

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/* ����v���r���[���[�h�̂Ƃ��́A�L�[����͑S��PrintPreviewBar�֓]�� */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//�����{�b�N�X���X�V
		AcceptSharedSearchKey();
		
		return lRes;

	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	�X�e�[�^�X�o�[�̃_�u���N���b�N�Ń��[�h�ؑւ��ł���悤�ɂ���
		if( m_hwndStatusBar && pnmh->hwndFrom == m_hwndStatusBar ){
			if( pnmh->code == NM_DBLCLK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 6 ){	//	�㏑��/�}��
					m_cEditDoc.HandleCommand( F_CHGMOD_INS );
				}
				else if( mp->dwItemSpec == 5 ){	//	�}�N���̋L�^�J�n�E�I��
					m_cEditDoc.HandleCommand( F_RECKEYMACRO );
				}
				else if( mp->dwItemSpec == 1 ){	//	���ʒu���s�ԍ��W�����v
					m_cEditDoc.HandleCommand( F_JUMP_DIALOG );
				}
			}
			else if( pnmh->code == NM_RCLICK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 2 ){	//	���͉��s���[�h
					m_CMenuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						_T("���͉��s�R�[�h�w��(&CRLF)") ); // ���͉��s�R�[�h�w��(CRLF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						_T("���͉��s�R�[�h�w��(&LF)") ); // ���͉��s�R�[�h�w��(LF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						_T("���͉��s�R�[�h�w��(C&R)") ); // ���͉��s�R�[�h�w��(CR)
						
					//	mp->pt�̓X�e�[�^�X�o�[�����̍��W�Ȃ̂ŁC�X�N���[�����W�ւ̕ϊ����K�v
					POINT	po = mp->pt;
					::ClientToScreen( m_hwndStatusBar, &po );
					int nId = ::TrackPopupMenu(
						hMenuPopUp,
						TPM_CENTERALIGN
						| TPM_BOTTOMALIGN
						| TPM_RETURNCMD
						| TPM_LEFTBUTTON
						,
						po.x,
						po.y,
						0,
						m_hWnd,
						NULL
					);
					::DestroyMenu( hMenuPopUp );
					m_cEditDoc.HandleCommand( nId );
				}
			}
			return 0L;
		}
		//	To Here Feb. 15, 2004 genta 

		switch( pnmh->code ){
		case TTN_NEEDTEXT:
			lptip = (LPTOOLTIPTEXT)pnmh;
			{
				/* �c�[���o�[�̃c�[���`�b�v�̃e�L�X�g���Z�b�g */
				CMemory**	ppcAssignedKeyList;
				int			nAssignedKeyNum;
				int			j;
				char*		pszKey;
				char		szLabel[1024];


				// From Here Oct. 15, 2001 genta
				// �@�\������̎擾��Lookup���g���悤�ɕύX
				m_cEditDoc.m_cFuncLookup.Funccode2Name( lptip->hdr.idFrom, szLabel, 1024 );

				// To Here Oct. 15, 2001 genta
				/* �@�\�ɑΉ�����L�[���̎擾(����) */
				nAssignedKeyNum = CKeyBind::GetKeyStrList(
					m_hInstance, m_pShareData->m_nKeyNameArrNum,
					(KEYDATA*)m_pShareData->m_pKeyNameArr, &ppcAssignedKeyList, lptip->hdr.idFrom
				);
				if( 0 < nAssignedKeyNum ){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						strcat( szLabel, "\n        " );
						pszKey = ppcAssignedKeyList[j]->GetStringPtr();
						strcat( szLabel, pszKey );
						delete ppcAssignedKeyList[j];
					}
					delete [] ppcAssignedKeyList;
				}
				lptip->hinst = NULL;
				lptip->lpszText	= szLabel;
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu( pnmh->hwndFrom );
				if( nId != 0 ) OnCommand( (WORD)0 /*���j���[*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if( pnmh->hwndFrom == m_hwndToolBar ){
				//	�c�[���o�[��Owner Draw
				return ToolBarOwnerDraw( (LPNMCUSTOMDRAW)pnmh );
			}
			break;
		//	To Here Jul. 21, 2003 genta
		}
		return 0L;
	case WM_COMMAND:
		OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
		return 0L;
	case WM_INITMENUPOPUP:
		InitMenu( (HMENU)wParam, (UINT)LOWORD( lParam ), (BOOL)HIWORD( lParam ) );
		return 0L;
	case WM_DROPFILES:
		/* �t�@�C�����h���b�v���ꂽ */
		OnDropFiles( (HDROP) wParam );
		return 0L;
	case WM_QUERYENDSESSION:	//OS�̏I��
		if( OnClose() ){
			DestroyWindow( hwnd );
			return TRUE;
		}
		else{
			return FALSE;
		}
	case WM_CLOSE:
		if( OnClose() ){
			DestroyWindow( hwnd );
		}
		return 0L;
	case WM_DESTROY:
		if( m_pShareData->m_bRecordingKeyMacro ){					/* �L�[�{�[�h�}�N���̋L�^�� */
			if( m_pShareData->m_hwndRecordingKeyMacro == m_hWnd ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				m_pShareData->m_bRecordingKeyMacro = FALSE;			/* �L�[�{�[�h�}�N���̋L�^�� */
				m_pShareData->m_hwndRecordingKeyMacro = NULL;		/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
			}
		}

		/* �^�C�}�[���폜 */
		::KillTimer( m_hWnd, IDT_TOOLBAR );

		/* �h���b�v���ꂽ�t�@�C�����󂯓����̂����� */
		::DragAcceptFiles( hwnd, FALSE );
		m_pcDropTarget->Revoke_DropTarget();	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

		/* �ҏW�E�B���h�E���X�g����̍폜 */
		CShareData::getInstance()->DeleteEditWndList( m_hWnd );

		if( m_pShareData->m_hwndDebug == m_hWnd ){
			m_pShareData->m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
		::PostMessage( m_pShareData->m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows �ɃX���b�h�̏I����v�����܂� */
		::PostQuitMessage( 0 );

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// �r�W���A���X�^�C���^�N���V�b�N�X�^�C�����؂�ւ������c�[���o�[���č쐬����
		// �i�r�W���A���X�^�C��: Rebar �L��A�N���V�b�N�X�^�C��: Rebar �����j
		if( m_hwndToolBar ){
			if( IsVisualStyle() == (NULL == m_hwndReBar) ){
				DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		/* �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W */	// 2007.06.07 ryoji
		m_bUIPI = TRUE;	// �g���C����̕Ԏ����󂯎����
		return 0L;

	case MYWM_CLOSE:
		/* �G�f�B�^�ւ̏I���v�� */
		if( FALSE != ( nRet = OnClose()) ){	// Jan. 23, 2002 genta �x���}��
			// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
			if( !(BOOL)wParam ){	// �S�I���v���łȂ��ꍇ
				// �^�u�܂Ƃߕ\����(����)���c���w��̏ꍇ�A�c�E�B���h�E���P�Ȃ�V�K�G�f�B�^���N�����ďI������
				if( m_pShareData->m_Common.m_bDispTabWnd &&
					!m_pShareData->m_Common.m_bDispTabWndMultiWin &&
					m_pShareData->m_Common.m_bTab_RetainEmptyWin
					){
					// ���O���[�v���̎c�E�B���h�E���𒲂ׂ�	// 2007.06.20 ryoji
					int nGroup = CShareData::getInstance()->GetGroupId( m_hWnd );
					if( 1 == CShareData::getInstance()->GetEditorWindowsNum( nGroup ) ){
						EditNode* pEditNode = CShareData::getInstance()->GetEditNode( m_hWnd );
						if( pEditNode )
							pEditNode->m_bClosing = TRUE;	// �����̓^�u�\�����Ă����Ȃ��Ă���
						CEditApp::OpenNewEditor(
							m_hInstance,
							m_hWnd,
							(char*)NULL,
							0,
							FALSE,
							TRUE
						);
					}
				}
			}
			DestroyWindow( hwnd );
		}
		return nRet;


	case MYWM_GETFILEINFO:
		/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
		pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

		/* �ҏW�t�@�C�������i�[ */
		m_cEditDoc.SetFileInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* �ݒ�ύX�̒ʒm */
// Oct 10, 2000 ao
/* �ݒ�ύX���A�c�[���o�[���č쐬����悤�ɂ���i�o�[�̓��e�ύX�����f�j */
		DestroyToolBar();
		LayoutToolBar();
// Oct 10, 2000 ao �����܂�

		// 2008.09.23 nasukoji	��A�N�e�B�u�ȃE�B���h�E�̃c�[���o�[���X�V����
		// �A�N�e�B�u�ȃE�B���h�E�̓^�C�}�ɂ��X�V����邪�A����ȊO�̃E�B���h�E��
		// �^�C�}���~�����Ă���ݒ�ύX����ƑS���L���ƂȂ��Ă��܂����߁A������
		// �c�[���o�[���X�V����
		if( !m_bIsActiveApp )
			UpdateToolbar();

		// �t�@���N�V�����L�[���č쐬����i�o�[�̓��e�A�ʒu�A�O���[�v�{�^�����̕ύX�����f�j	// 2006.12.19 ryoji
		m_CFuncKeyWnd.Close();
		LayoutFuncKey();

		// �^�u�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		LayoutTabBar();

		// �X�e�[�^�X�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		LayoutStatusBar();

		// �����X�N���[���o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		{
			int i;
			bool b1;
			bool b2;
			b1 = (m_pShareData->m_Common.m_bScrollBarHorz == FALSE);
			for( i = 0; i < m_cEditDoc.GetAllViewCount(); i++ )
			{
				b2 = (m_cEditDoc.m_cEditViewArr[i].m_hwndHScrollBar == NULL);
				if( b1 != b2 )		/* �����X�N���[���o�[���g�� */
				{
					m_cEditDoc.m_cEditViewArr[i].DestroyScrollBar();
					m_cEditDoc.m_cEditViewArr[i].CreateScrollBar();
				}
			}
		}

		// �o�[�ύX�ŉ�ʂ�����Ȃ��悤��	// 2006.12.19 ryoji
		EndLayoutBars();

		// 2009.08.15 nasukoji	�A�N�Z�����[�^�e�[�u�����č쐬����(Wine�p)
		if( m_hAccel ){
			::DestroyAcceleratorTable( m_hAccel );		// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u�����J������
			m_hAccel = NULL;
		}

		if( m_pShareData->m_Common.m_bCreateAccelTblEachWin ){		// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
			m_hAccel = CKeyBind::CreateAccerelator(
				m_pShareData->m_nKeyNameArrNum,
				m_pShareData->m_pKeyNameArr
			);

			if( NULL == m_hAccel ){
				::MessageBox(
					NULL,
					_T("CEditWnd::DispatchEvent()\n")
					_T("�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n")
					_T("�V�X�e�����\�[�X���s�����Ă��܂��B"),
					GSTR_APPNAME, MB_OK | MB_ICONSTOP
				);
			}
		}

		if( m_pShareData->m_Common.m_bDispTabWnd )
		{
			// �^�u�\���̂܂܃O���[�v������^���Ȃ����ύX����Ă�����^�u���X�V����K�v������
			m_cTabWnd.Refresh( FALSE );
		}
		if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
		{
			if( CShareData::getInstance()->IsTopEditWnd( m_hWnd ) )
			{
				if( !::IsWindowVisible( m_hWnd ) )
				{
					// ::ShowWindow( m_hWnd, SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos( m_hWnd, NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );

					// ���̃E�B���h�E�� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�ɔ��f����	// 2007.05.18 ryoji
					WindowTopMost( ((DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST)? 1: 2 );
				}
			}
			else
			{
				if( ::IsWindowVisible( m_hWnd ) )
				{
					::ShowWindow( m_hWnd, SW_HIDE );
				}
			}
		}
		else
		{
			if( !::IsWindowVisible( m_hWnd ) )
			{
				// ::ShowWindow( m_hWnd, SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
				::SetWindowPos( m_hWnd, NULL,0,0,0,0,
								SWP_SHOWWINDOW | SWP_NOACTIVATE
								| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
			}
		}

		//	Aug, 21, 2000 genta
		m_cEditDoc.ReloadAutoSaveParam();

		m_cEditDoc.SetDocumentIcon();	// Sep. 10, 2002 genta �����A�C�R���̍Đݒ�
		m_cEditDoc.OnChangeSetting();	/* �r���[�ɐݒ�ύX�𔽉f������ */
		return 0L;
	case MYWM_SETACTIVEPANE:
		if( -1 == (int)wParam ){
			if( 0 == lParam ){
				nPane = m_cEditDoc.m_cSplitterWnd.GetFirstPane();
			}else{
				nPane = m_cEditDoc.m_cSplitterWnd.GetLastPane();
			}
			m_cEditDoc.SetActivePane( nPane );
		}
		return 0L;


	case MYWM_SETCARETPOS:	/* �J�[�\���ʒu�ύX�ʒm */
		{
			//	2006.07.09 genta LPARAM�ɐV���ȈӖ���ǉ�
			//	bit 0 (MASK 1): (bit 1==0�̂Ƃ�) 0/�I���N���A, 1/�I���J�n�E�ύX
			//	bit 1 (MASK 2): 0: bit 0�̐ݒ�ɏ]���D1:���݂̑I�����b�Ns��Ԃ��p��
			//	�����̎����ł� �ǂ����0�Ȃ̂ŋ��������Ɖ��߂����D
			//	�Ăяo������e_PM_SETCARETPOS_SELECTSTATE�̒l���g�����ƁD
			int bSelect = lParam & 1;
			if( lParam & 2 ){
				// ���݂̏�Ԃ�KEEP
				bSelect = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_bSelectingLock;
			}
			
			ppoCaret = (POINT*)m_pShareData->m_szWork;
			//	2006.07.09 genta �����������Ȃ�
			/*
			�J�[�\���ʒu�ϊ�
			 �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			��
			 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			m_cEditDoc.m_cLayoutMgr.LogicToLayout(
				ppoCaret->x,
				ppoCaret->y,
				&nCaretPosX,
				&nCaretPosY
			);
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��	// 2007.08.22 ryoji
			// Note. ���Ƃ����s�P�ʂ̌��ʒu�Ȃ̂Ń��C�A�E�g�܂�Ԃ��̌��ʒu�𒴂��邱�Ƃ͂Ȃ��B
			//       �I���w��(bSelect==TRUE)�̏ꍇ�ɂ͂ǂ�����̂��Ó����悭�킩��Ȃ����A
			//       2007.08.22���݂ł̓A�E�g���C����̓_�C�A���O���猅�ʒu0�ŌĂяo�����
			//       �p�^�[�������Ȃ��̂Ŏ��p����ɖ��͖����B
			if( !bSelect ){
				const CDocLine *pTmpDocLine = m_cEditDoc.m_cDocLineMgr.GetLine( ppoCaret->y );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x ) nCaretPosX--;
				}
			}
			//	2006.07.09 genta �I��͈͂��l�����Ĉړ�
			//	MoveCursor�̈ʒu�����@�\������̂ŁC�ŏI�s�ȍ~�ւ�
			//	�ړ��w���̒�����MoveCursor�ɂ܂�����
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].MoveCursorSelecting( nCaretPosX, nCaretPosY, bSelect, _CARETMARGINRATE / 3 );
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* �J�[�\���ʒu�擾�v�� */
		ppoCaret = (POINT*)m_pShareData->m_szWork;
		/*
		�J�[�\���ʒu�ϊ�
		 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		��
		�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		{
			m_cEditDoc.m_cLayoutMgr.LayoutToLogic(
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX,
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY,
				(int*)&ppoCaret->x,
				(int*)&ppoCaret->y
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	/* �s(���s�P��)�f�[�^�̗v�� */
		pLine = m_cEditDoc.m_cDocLineMgr.GetLineStr( (int)wParam, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > sizeof( m_pShareData->m_szWork ) ){
			memcpy( m_pShareData->m_szWork, pLine, sizeof( m_pShareData->m_szWork ) );
		}else{
			memcpy( m_pShareData->m_szWork, pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_ADDTAIL, TRUE, (LPARAM)m_pShareData->m_szWork, (LPARAM)lstrlen( m_pShareData->m_szWork ), 0, 0 );
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_GOFILEEND, TRUE, 0, 0, 0, 0 );
		return 0L;

	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_cTabWnd.TabWindowNotify( wParam, lParam );
		return 0L;

	//�o�[�̕\���E��\��	//@@@ 2003.06.10 MIK
	case MYWM_BAR_CHANGE_NOTIFY:
		if( m_hWnd != (HWND)lParam )
		{
			switch( wParam )
			{
			case MYBCN_TOOLBAR:
				LayoutToolBar();	// 2006.12.19 ryoji
				break;
			case MYBCN_FUNCKEY:
				LayoutFuncKey();	// 2006.12.19 ryoji
				break;
			case MYBCN_TAB:
				LayoutTabBar();		// 2006.12.19 ryoji
				if( m_pShareData->m_Common.m_bDispTabWnd
					&& !m_pShareData->m_Common.m_bDispTabWndMultiWin )
				{
					::ShowWindow(m_hWnd, SW_HIDE);
				}
				else
				{
					// ::ShowWindow( hwnd, SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos( hwnd, NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
				}
				break;
			case MYBCN_STATUSBAR:
				LayoutStatusBar();		// 2006.12.19 ryoji
				break;
			}
			EndLayoutBars();	// 2006.12.19 ryoji
		}
		return 0L;

	//by �S (2) MYWM_CHECKSYSMENUDBLCLK�͕s�v��, WM_LBUTTONDBLCLK�ǉ�
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			m_cEditDoc.ActiveView().ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_NCPAINT:
		DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_hwndStatusBar ){
			PrintMenubarMessage( NULL );
		}
		return 0;

	case WM_NCACTIVATE:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�P�j	// 2007.04.03 ryoji
		// �O�ʂɂ���̂��ҏW�E�B���h�E�Ȃ�A�N�e�B�u��Ԃ�ێ�����
		if( m_pShareData->m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow()) ){
			wParam = TRUE;	// �A�N�e�B�u
		}
		lRes = DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_hwndStatusBar ){
			PrintMenubarMessage( NULL );
		}
		return lRes;

	case WM_SETTEXT:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�Q�j	// 2007.04.03 ryoji
		// �^�C�}�[���g�p���ă^�C�g���̕ύX��x������
		if( m_pShareData->m_bEditWndChanging ){
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[ ::_tcslen((LPCTSTR)lParam) + 1 ];
			::_tcscpy( m_pszLastCaption, (LPCTSTR)lParam );	// �ύX��̃^�C�g�����L�����Ă���
			::SetTimer( m_hWnd, IDT_CAPTION, 50, NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_TIMER:
		if( !OnTimer(wParam, lParam) )
			return 0L;
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
// >> by aroka
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

/*! �I�����̏���

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
int	CEditWnd::OnClose()
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	int nRet = m_cEditDoc.OnFileClose();
	if( !nRet ) return nRet;

	// 2005.09.01 ryoji �^�u�܂Ƃߕ\���̏ꍇ�͎��̃E�B���h�E��O�ʂɁi�I�����̃E�B���h�E�������}���j
	if( m_pShareData->m_Common.m_bDispTabWnd
		&& !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		int i, j;
		EditNode*	p = NULL;
		int nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, FALSE );
		if( nCount > 1 )
		{
			for( i = 0; i < nCount; i++ )
			{
				if( p[ i ].m_hWnd == m_hWnd )
					break;
			}
			if( i < nCount )
			{
				for( j = i + 1; j < nCount; j++ )
				{
					if( p[ j ].m_nGroup == p[ i ].m_nGroup )
						break;
				}
				if( j >= nCount )
				{
					for( j = 0; j < i; j++ )
					{
						if( p[ j ].m_nGroup == p[ i ].m_nGroup )
							break;
					}
				}
				if( j != i )
				{
					HWND hwnd = p[ j ].m_hWnd;
					{
						// 2006.01.28 ryoji
						// �^�u�܂Ƃߕ\���ł��̉�ʂ���\������\���ɕς���Ă�������ꍇ(�^�u�̒��N���b�N����)�A
						// �ȑO�̃E�B���h�E�������������Ɉ�C�ɂ����܂ŏ������i��ł��܂���
						// ���Ƃŉ�ʂ�������̂ŁA�ȑO�̃E�B���h�E��������̂�������Ƃ����҂�
						int iWait = 0;
						while( ::IsWindowVisible( hwnd ) && iWait++ < 20 )
							::Sleep(1);
					}
					if( !::IsWindowVisible( hwnd ) )
					{
						ActivateFrameWindow( hwnd );
					}
				}
			}
		}
		if( p ) delete []p;
	}

	return nRet;
}






//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s(F_HELP_CONTENTS,F_HELP_SEARCH)���C���E����
void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{

	switch( wNotifyCode ){
	/* ���j���[����̃��b�Z�[�W */
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta �}�E�X����Ăт����ꂽ�ꍇ
		switch( wID ){
		case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
			/* �T�N���G�f�B�^�̑S�I�� */
			CEditApp::TerminateApplication( m_hWnd );	// 2006.12.25 ryoji �����ǉ�
			break;

		case F_HELP_CONTENTS:
			/* �w���v�ڎ� */
			ShowWinHelpContents( m_hWnd, CEditApp::GetHelpFilePath() );	//	�ڎ���\������
			break;

		case F_HELP_SEARCH:
			/* �w���v�L�[���[�h���� */
			MyWinHelp( m_hWnd, CEditApp::GetHelpFilePath(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
			break;

		case F_ABOUT:	//Dec. 25, 2000 JEPRO F_�ɕύX
			/* �o�[�W������� */
			{
				CDlgAbout cDlgAbout;
				cDlgAbout.DoModal( m_hInstance, m_hWnd );
			}
			break;
		default:
			//�E�B���h�E�؂�ւ�
			if( wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_nEditArrNum ){
				ActivateFrameWindow( m_pShareData->m_pEditArr[wID - IDM_SELWINDOW].m_hWnd );
			}
			//�ŋߎg�����t�@�C��
			else if( wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999){
				/* �w��t�@�C�����J����Ă��邩���ׂ� */
				CMRUFile cMRU;
				EditInfo checkEditInfo;
				cMRU.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
				m_cEditDoc.OpenFile( checkEditInfo.m_szPath, (ECodeType)checkEditInfo.m_nCharCode);
			}
			//�ŋߎg�����t�H���_
			else if( wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999){
				//�t�H���_�擾
				CMRUFolder cMRUFolder;
				LPCTSTR pszFolderPath = cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER );

				//Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
				NetConnect( pszFolderPath );

				//�u�t�@�C�����J���v�_�C�A���O
				ECodeType nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
				BOOL bReadOnly = FALSE;
				char		szPath[_MAX_PATH + 3];
				szPath[0] = '\0';
				if( !m_cEditDoc.OpenFileDialog( m_hWnd, cMRUFolder.GetPath(wID - IDM_SELOPENFOLDER), szPath, &nCharCode, &bReadOnly ) ){
					return;
				}
				//	Oct.  9, 2004 genta ���ʊ֐���
				m_cEditDoc.OpenFile( szPath, nCharCode, bReadOnly );
			}
			//���̑��R�}���h
			else{
				//�r���[�Ƀt�H�[�J�X���ړ����Ă���
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX ) {
					::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );
					//�����{�b�N�X���X�V	// 2010/6/6 Uchi
					AcceptSharedSearchKey();
				}

				// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
				//	May 19, 2006 genta ��ʃr�b�g��n��
				//	Jul. 7, 2007 genta ��ʃr�b�g��萔��
				m_cEditDoc.HandleCommand( wID | 0 );
			}
			break;
		}
		break;
	/* �A�N�Z�����[�^����̃��b�Z�[�W */
	case 1:
		{
			//�r���[�Ƀt�H�[�J�X���ړ����Ă���
			if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
				::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

			int nFuncCode = CKeyBind::GetFuncCode(
				wID,
				m_pShareData->m_nKeyNameArrNum,
				m_pShareData->m_pKeyNameArr
			);
			m_cEditDoc.HandleCommand( nFuncCode | FA_FROMKEYBOARD );
		}
		break;

	case CBN_SETFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = F_SEARCH_BOX;
		}
		break;

	case CBN_KILLFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = 0;

			//�t�H�[�J�X���͂��ꂽ�Ƃ��Ɍ����L�[�ɂ��Ă��܂��B
			//�����L�[���[�h���擾
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );
			}

		}
		break;
	}

	return;
}





//	�L�[���[�h�F���j���[�o�[����
//	Sept.14, 2000 Jepro note: ���j���[�o�[�̍��ڂ̃L���v�V�����⏇�Ԑݒ�Ȃǂ͈ȉ��ōs���Ă���炵��
//	Sept.16, 2000 Jepro note: �A�C�R���Ƃ̊֘A�t����CShareData_new2.cpp�t�@�C���ōs���Ă���
void CEditWnd::InitMenu( HMENU hMenu, UINT uPos, BOOL fSystemMenu )
{
	int			cMenuItems;
	int			nPos;
	UINT		fuFlags;
	int			i;
	BOOL		bRet;
	int			nRowNum;
	HMENU		hMenuPopUp;
	HMENU		hMenuPopUp_2;
	const char*	pszLabel;


	if( hMenu == ::GetSubMenu( ::GetMenu( m_hWnd ), uPos )
		&& !fSystemMenu ){
		switch( uPos ){
		case 0:
			/* �u�t�@�C���v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, "�V�K�쐬(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T("�V�K�E�C���h�E���J��(&M)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, "�J��(&O)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, "�㏑���ۑ�(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, "���O��t���ĕۑ�(&A)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL		, "���ׂď㏑���ۑ�(&Z)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//	Feb. 28, 2003 genta �ۑ����ĕ����ǉ��D���郁�j���[���߂��Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVECLOSE	, "�ۑ����ĕ���(&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "����(&C)" );	//Feb. 18, 2001	JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, "����(����) (&R)" );	//Oct. 17, 2000 jepro �L���v�V�������u����v����ύX	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(C��B; Blank�̈Ӗ�)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, "���ĊJ��(&L)..." );

			// �u�����R�[�h�Z�b�g�v�|�b�v�A�b�v���j���[
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, "�J������(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, "SJIS�ŊJ������(&S)" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, "JIS�ŊJ������(&J)" );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, "EUC�ŊJ������(&E)" );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, "Unicode�ŊJ������(&U)" );	//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, "UnicodeBE�ŊJ������(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, "UTF-8�ŊJ������(&8)" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, "UTF-7�ŊJ������(&7)" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp_2 , "�J������(&W)" );//Oct. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(M��H)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, "���(&P)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, "����v���r���[(&V)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, "�y�[�W�ݒ�(&U)..." );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�t�@�C������v�|�b�v�A�b�v���j���[
			//hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, "�t�@�C���̃v���p�e�B(&T)" );		//Nov. 7, 2000 jepro �L���v�V������'�t�@�C����'��ǉ�
			//m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�t�@�C������(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BROWSE						, "�u���E�Y(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
			{
				//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
				CMRUFile cMRU;
				hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	�t�@�C�����j���[
				if ( cMRU.Length() > 0 ){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
				}
			}

			/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
				CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE, "�O���[�v�����(&G)" );	// �O���[�v�����	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, "�ҏW�̑S�I��(&Q)" );	//Feb/ 19, 2001 JEPRO �ǉ�	// 2006.10.21 ryoji �\��������ύX	// 2007.02.13 ryoji ��F_EXITALLEDITORS
			//	Jun. 9, 2001 genta �\�t�g�E�F�A������
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, "�T�N���G�f�B�^�̑S�I��(&X)" );	//Sept. 11, 2000 jepro �L���v�V�������u�A�v���P�[�V�����I���v����ύX	//Dec. 26, 2000 JEPRO F_�ɕύX
			break;

		case 1:
			/* �u�ҏW�v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, "���ɖ߂�(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, "��蒼��(&R)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, "�؂���(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, "�R�s�[(&C)" );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "�S�s���p�R�s�[(&N)" );
			//	Sept. 14, 2000 JEPRO	�L���v�V�����Ɂu�L���t���v��ǉ��A�A�N�Z�X�L�[�ύX(N��.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "�I��͈͓��S�s���p���t���R�s�[(&.)" );
//			Sept. 30, 2000 JEPRO	���p���t���R�s�[�̃A�C�R�����쐬�����̂ŏ�L���j���[�͏d��������āu���x�ȑ���v���ɂ��������ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, "�\��t��(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, "�폜(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, "���ׂđI��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );							/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, "�ĕϊ�(&R)" );		/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, "CRLF���s�ŃR�s�[(&L)" );				//Nov. 9, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, "�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, "��`�\��t��(&X)" );					//Sept. 13, 2000 JEPRO �ړ��ɔ����A�N�Z�X�L�[�t�^	//Oct. 22, 2000 JEPRO �A�N�Z�X�L�[�ύX(P��X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, "�J�[�\���O���폜(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�}���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, "���t(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, "����(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, "�R���g���[���R�[�h(&C)..." );	// 2004.05.06 MIK ...�ǉ�

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�}��(&I)" );

			// �u���x�ȑ���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	"�P��̍��[�܂ō폜(&L)" );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	"�P��̉E�[�܂ō폜(&R)" );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	"���݈ʒu�̒P��I��(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	"�P��؂���(&T)" );				//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	"�P��폜(&D)" );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	"�s���܂Ő؂���(���s�P��) (&U)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	"�s���܂Ő؂���(���s�P��) (&K)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	"�s���܂ō폜(���s�P��) (&H)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	"�s���܂ō폜(���s�P��) (&E)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	"�s�؂���(�܂�Ԃ��P��) (&X)" );	//Jan. 16, 2001 JEPRO �s(���E��)�֌W�̏��������ւ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	"�s�폜(�܂�Ԃ��P��) (&Y)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	"�s�̓�d��(�܂�Ԃ��P��) (&2)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	"TAB�C���f���g(&A)" );				//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	"�tTAB�C���f���g(&B)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	"SPACE�C���f���g(&S)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	"�tSPACE�C���f���g(&P)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, "�I��͈͓��S�s�R�s�[(&@)" );		//Sept. 14, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, "�I��͈͓��S�s���p���t���R�s�[(&.)" );//Sept. 13, 2000 JEPRO �L���v�V��������u�L���t���v��ǉ��A�A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, "�I��͈͓��S�s�s�ԍ��t���R�s�[(&:)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	"���̃t�@�C�������R�s�[(&-)" );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	"���̃t�@�C���̃p�X�����R�s�[(&\\)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	"���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[(&^)" );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, "�L�[���蓖�Ĉꗗ���R�s�[(&Q)" );	//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, "�P�ꃊ�t�@�����X(&W)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "���x�ȑ���(&V)" );

		//From Here Feb. 19, 2001 JEPRO [�ړ�(M)], [�I��(R)]���j���[��[�ҏW]�̃T�u���j���[�Ƃ��Ĉړ�
			// �u�ړ��v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, "�J�[�\����ړ�(�Q�s����) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, "�J�[�\�����ړ�(�Q�s����) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, "�P��̍��[�Ɉړ�(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, "�P��̉E�[�Ɉړ�(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, "�O�̒i���Ɉړ�(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, "���̒i���Ɉړ�(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, "�s���Ɉړ�(�܂�Ԃ��P��) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, "�s���Ɉړ�(�܂�Ԃ��P��) (&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, "�P�y�[�W�A�b�v(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, "�P�y�[�W�_�E��(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, "�t�@�C���̐擪�Ɉړ�(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, "�t�@�C���̍Ō�Ɉړ�(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, "�J�[�\���s���E�B���h�E������(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, "�w��s�փW�����v(&J)..." );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "�����J�n�ʒu�֖߂�(&I)" );	// �����J�n�ʒu�֖߂� 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, "�ړ�����: �O��(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, "�ړ�����: ����(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, "���݈ʒu���ړ������ɓo�^(&S)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ړ�(&O)" );

			// �u�I���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, "���݈ʒu�̒P��I��(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, "���ׂđI��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, "�͈͑I���J�n(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, "(�I��)�J�[�\����ړ�(�Q�s����) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, "(�I��)�J�[�\�����ړ�(�Q�s����) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, "(�I��)�P��̍��[�Ɉړ�(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, "(�I��)�P��̉E�[�Ɉړ�(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, "(�I��)�O�̒i���Ɉړ�(&2)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, "(�I��)���̒i���Ɉړ�(&8)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, "(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, "(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, "(�I��)�P�y�[�W�A�b�v(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, "(�I��)�P�y�[�W�_�E��(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, "(�I��)�t�@�C���̐擪�Ɉړ�(&1)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, "(�I��)�t�@�C���̍Ō�Ɉړ�(&9)" );

			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Mar. 11, 2004 genta ��`�I�����j���[��I�����j���[�ɓ���
			//	���̂Ƃ����1�����Ȃ��̂�
			// �u��`�I���v�|�b�v�A�b�v���j���[
			// hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, "��`�͈͑I���J�n(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�I��(&S)" );

//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "��`�I��(&E)" );

			// �u���`�v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, "��(�擪)�̋󔒂��폜(&L)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, "�E(����)�̋󔒂��폜(&R)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, "�I���s�̏����\�[�g(&A)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, "�I���s�̍~���\�[�g(&D)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, "�A�������d���s�̍폜(uniq)(&U)" );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "���`(&K)" );

			break;
		//Feb. 19, 2001 JEPRO [�ړ�][�ړ�], [�I��]��[�ҏW]�z���Ɉڂ����̂ō폜

//		case 4://case 2: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 2://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�ϊ��v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, "������(&L)" );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, "�啶��(&U)" );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "�S�p�����p" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "���p���S�p�J�^�J�i" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "���p���S�p�Ђ炪��" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "�S�p�����p(&F)" );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
	// From Here 2007.01.24 maru ���j���[�̕��ѕύX�ƃA�N�Z�X�L�[�ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "���p�{�S�Ђ灨�S�p�E�J�^�J�i(&Z)" );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "���p�{�S�J�^���S�p�E�Ђ炪��(&N)" );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, "�S�p�p�������p�p��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, "���p�p�����S�p�p��(&M)" );				//July. 29, 2001 Misaka �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, "�S�p�J�^�J�i�����p�J�^�J�i(&J)" );		//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru ���j���[�̕��ѕύX�ƃA�N�Z�X�L�[�ǉ�
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKATA, "���p�J�^�J�i���S�p�J�^�J�i(&K)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENHIRA, "���p�J�^�J�i���S�p�Ђ炪��(&H)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, "TAB����(&S)" );	//Feb. 19, 2001 JEPRO ������ړ�����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, "�󔒁�TAB(&T)" );	//#### Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//�u�����R�[�h�ϊ��v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, "�������ʁ�SJIS�R�[�h�ϊ�(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, "E-Mail(JIS��SJIS)�R�[�h�ϊ�(&M)" );//Sept. 11, 2000 JEPRO �L���v�V�����ɁuE-Mail�v��ǉ����A�N�Z�X�L�[�ύX(V��M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, "EUC��SJIS�R�[�h�ϊ�(&W)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(E��W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, "Unicode��SJIS�R�[�h�ϊ�(&U)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX����I:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, "UnicodeBE��SJIS�R�[�h�ϊ�(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, "UTF-8��SJIS�R�[�h�ϊ�(&T)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(T:uTF/shifT)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, "UTF-7��SJIS�R�[�h�ϊ�(&F)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(F:utF/shiFt)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, "SJIS��JIS�R�[�h�ϊ�(&J)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, "SJIS��EUC�R�[�h�ϊ�(&E)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, "SJIS��&Unicode�R�[�h�ϊ�" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, "SJIS��UTF-8�R�[�h�ϊ�(&8)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, "SJIS��UTF-7�R�[�h�ϊ�(&7)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, "Base64�f�R�[�h���ĕۑ�(&B)" );	//Oct. 6, 2000 JEPRO �A�N�Z�X�L�[�ύX(6��B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, "uudecode���ĕۑ�(&D)" );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(U��D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�����R�[�h�ϊ�(&C)" );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 3://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, "����(&F)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, "��������(&N)" );				//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, "�O������(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, "�u��(&R)..." );				//Oct. 7, 2000 JEPRO ���̃Z�N�V�������炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, "�����}�[�N�̐ؑւ�(&C)" );	// "�����}�[�N�̃N���A(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "�����J�n�ʒu�֖߂�(&I)" );	// �����J�n�ʒu�֖߂� 02/06/26 ai
		
			// Jan. 10, 2005 genta �C���N�������^���T�[�`
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_NEXT, "�O���C���N�������^���T�[�`(&F)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_PREV, "����C���N�������^���T�[�`(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_NEXT, "���K�\���O���C���N�������^���T�[�`(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_PREV, "���K�\������C���N�������^���T�[�`(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_NEXT, "MIGEMO�O���C���N�������^���T�[�`(&M)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_PREV, "MIGEMO����C���N�������^���T�[�`(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�C���N�������^���T�[�`(&S)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
// From Here 2001.12.03 hor
			// Jan. 10, 2005 genta ��������̂Ńu�b�N�}�[�N���T�u���j���[��
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, "�u�b�N�}�[�N�ݒ�E����(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, "���̃u�b�N�}�[�N��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, "�O�̃u�b�N�}�[�N��(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, "�u�b�N�}�[�N�̑S����(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, "�u�b�N�}�[�N�̈ꗗ(&V)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�u�b�N�}�[�N(&M)" );
// To Here 2001.12.03 hor
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, "Grep(&G)..." );					//Oct. 7, 2000 JEPRO �����炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, "�w��s�փW�����v(&J)..." );	//Sept. 11, 2000 jepro �L���v�V�����Ɂu �W�����v�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, "�A�E�g���C�����(&L)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, "�^�O�W�����v(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, "�^�O�W�����v�o�b�N(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, "�^�O�t�@�C���̍쐬..." );	//@@@ 2003.04.13 MIK // 2004.05.06 MIK ...�ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, "�_�C���N�g�^�O�W�����v" );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP_KEYWORD	, "�L�[���[�h���w�肵�ă^�O�W�����v" ); //@@ 2005.03.31 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC				, "������C/C++�w�b�_(�\�[�X)���J��(&C)" );	//Feb. 7, 2001 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, "�t�@�C�����e��r(&@)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, "DIFF�����\��(&D)..." );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, "���̍�����" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, "�O�̍�����" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, "�����\���̑S����" );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	�Ί��ʂ̌��������j���[�ɒǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, "�Ί��ʂ̌���(&[)" );
//	To Here Sept. 1, 2000

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 4://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�I�v�V�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( !m_pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "�L�[�}�N���̋L�^�J�n(&R)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "�L�[�}�N���̕ۑ�(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "�L�[�}�N���̓ǂݍ���(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "�L�[�}�N���̎��s(&D)" );
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "�L�[�}�N���̋L�^�I��(&R)" );
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "�L�[�}�N���̋L�^�I��&&�ۑ�(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "�L�[�}�N���̋L�^�I��&&�ǂݍ���(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "�L�[�}�N���̋L�^�I��&&���s(&D)" );
			}
			
			//	From Here Sep. 14, 2001 genta
			//�u�o�^�ς݃}�N���v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			
			for( i = 0; i < MAX_CUSTMACRO; ++i ){
				MacroRec *mp = &m_pShareData->m_MacroTable[i];
				if( mp->IsEnabled() ){
					if(  mp->m_szName[0] ){
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szName );
					}
					else {
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szFile );
					}
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�o�^�ς݃}�N��(&B)" );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECEXTMACRO, "���O���w�肵�ă}�N�����s(&E)..." ); // 2008.10.22 syat �ǉ�

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, "�O���R�}���h���s(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECMD_DIALOG, "�O���R�}���h���s(&X)..." );	//Mar. 10, 2001 JEPRO �@�\���Ȃ��̂Ń��j���[����B����	//Mar.21, 2001 JEPRO �W���o�͂��Ȃ��ŕ��� // 2004.05.06 MIK ...�ǉ�
			//To Here Sept. 20, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, "SQL*Plus���A�N�e�B�u�\��(&P)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^	�����́u�A�N�e�B�u���v���u�A�N�e�B�u�\���v�ɓ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, "SQL*Plus�Ŏ��s(&S)" );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, "���͕⊮(&/)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//�u�J�X�^�����j���[�v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	�E�N���b�N���j���[
			if( m_pShareData->m_Common.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_szCustMenuNameArr[0] );
			}
			//	�J�X�^�����j���[
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_szCustMenuNameArr[i] );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�J�X�^�����j���[(&U)" );

//		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

			break;

		case 5://Feb. 28, 2004 genta �u�ݒ�v���j���[
			//	�ݒ荀�ڂ��u�c�[���v����Ɨ�������
			m_CMenuDrawer.ResetContents();
			/* �u�E�B���h�E�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

//	From Here Sept. 17, 2000 JEPRO
//	��͂�Win�W���ɍ��킹�ă`�F�b�N�}�[�N�����ŕ\���^��\���𔻒f����悤�ɂ������������̂ŕύX
			if ( FALSE == m_pShareData->m_Common.m_bMenuIcon ){
				pszLabel = "�c�[���o�[��\��(&T)";				//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				pszLabel = "�t�@���N�V�����L�[��\��(&K)";		//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				pszLabel = "�^�u�o�[��\��";		//����̂ݕ\��	//@@@ 2003.06.10 MIK	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//����̂�	//@@@ 2003.06.10 MIK
				pszLabel = "�X�e�[�^�X�o�[��\��(&S)";			//����̂ݕ\��
//	To Here Sept.17, 2000 JEPRO
//	From Here Oct. 28, 2000 JEPRO
//	3�{�^���̃A�C�R�����ł������Ƃɔ����\���^��\���̃��b�Z�[�W��ς���悤�ɍĂѕύX
			}else{
				if( m_hwndToolBar == NULL ){
					pszLabel = "�c�[���o�[��\��(&T)";			//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃c�[���o�[���B��(&T)";			//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				if( NULL == m_CFuncKeyWnd.m_hWnd ){
					pszLabel = "�t�@���N�V�����L�[��\��(&K)";	//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃t�@���N�V�����L�[���B��(&K)";	//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				//@@@ 2003.06.10 MIK
				if( NULL == m_cTabWnd.m_hWnd ){
					pszLabel = "�^�u�o�[��\��(&M)";	//����̂ݕ\��	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				}else{
					pszLabel = "�\�����̃^�u�o�[���B��(&M)";	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//����̂�
				if( m_hwndStatusBar == NULL ){
					pszLabel = "�X�e�[�^�X�o�[��\��(&S)";		//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃X�e�[�^�X�o�[���B��(&S)";		//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
			}
//	To Here Oct. 28, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWSTATUSBAR, pszLabel );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, "�^�C�v�ʐݒ�ꗗ(&L)..." );	//Sept. 13, 2000 JEPRO �ݒ����Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, "�^�C�v�ʐݒ�(&Y)..." );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(S��Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, "���ʐݒ�(&C)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, "�t�H���g�ݒ�(&F)..." );		//Sept. 17, 2000 jepro �L���v�V�����Ɂu�ݒ�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, "�����̊Ǘ�(&O)..." );	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX�i�ꎞ�ݒ�j��ǉ�
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPNOWRAP, "�܂�Ԃ��Ȃ�(&X)" );		// �܂�Ԃ��Ȃ��i�ꎞ�ݒ�j
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPSETTING, "�w�茅�Ő܂�Ԃ�(&S)" );	// �w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPWINDOW, "�E�[�Ő܂�Ԃ�(&W)" );		// �E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j
			// �܂�Ԃ����@�Ɉꎞ�ݒ��K�p��
			if( m_cEditDoc.m_bTextWrapMethodCurTemp )
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�܂�Ԃ����@�i�ꎞ�ݒ�K�p���j(&X)" );
			else
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�܂�Ԃ����@(&X)" );

//@@@ 2002.01.14 YAZAKI �܂�Ԃ��Ȃ��R�}���h�ǉ�
// 20051022 aroka �^�C�v�ʐݒ�l�ɖ߂��R�}���h�ǉ�
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			//	Jan.  8, 2006 genta ���ʊ֐���
			{
				int width;
				CEditView::TOGGLE_WRAP_ACTION mode = m_cEditDoc.ActiveView().GetWrapMode( width );
				if( mode == CEditView::TGWRAP_NONE ){
					pszLabel = "�܂�Ԃ�����(&W)";
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , pszLabel );
				}
				else {
					char szBuf[60];
					pszLabel = szBuf;
					if( mode == CEditView::TGWRAP_FULL ){
						sprintf(
							szBuf,
							"�܂�Ԃ�����: %d ���i�ő�j(&W)",
							MAXLINEKETAS
						);
					}
					else if( mode == CEditView::TGWRAP_WINDOW ){
						int nActive = m_cEditDoc.GetActivePane();
						sprintf(
							szBuf,
							"�܂�Ԃ�����: %d ���i�E�[�j(&W)",
							m_cEditDoc.m_cEditViewArr[nActive].ViewColNumToWrapColNum(
								m_cEditDoc.m_cEditViewArr[nActive].m_nViewColNum
							)
						);
					}
					else {	// TGWRAP_PROP
						sprintf(
							szBuf,
							"�܂�Ԃ�����: %d ���i�w��j(&W)",
							m_cEditDoc.GetDocumentAttribute().m_nMaxLineKetas
						);
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel );
				}
			}
			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , "���݂̃E�B���h�E���Ő܂�Ԃ�(&W)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			// 2003.06.08 Moca �ǉ�
			// �u���[�h�ύX�v�|�b�v�A�b�v���j���[
			// Feb. 28, 2004 genta �ҏW���j���[����ړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, "�}���^�㏑�����[�h(&I)" );	//Nov. 9, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_READONLY					, "�ǂݎ���p(&R)" );
			if ( FALSE == m_pShareData->m_Common.m_bMenuIcon ){
				pszLabel = "�L�[���[�h�w���v�����\��(&H)";
			}
			else if( IsFuncChecked( &m_cEditDoc, m_pShareData, F_TOGGLE_KEY_SEARCH ) ){
				pszLabel = "�L�[���[�h�w���v�����\�����Ȃ�(&H)";
			}
			else {
				pszLabel = "�L�[���[�h�w���v�����\������(&H)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOGGLE_KEY_SEARCH, pszLabel );
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, "���͉��s�R�[�h�w��(&CRLF)" ); // ���͉��s�R�[�h�w��(CRLF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF, "���͉��s�R�[�h�w��(&LF)" ); // ���͉��s�R�[�h�w��(LF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR, "���͉��s�R�[�h�w��(C&R)" ); // ���͉��s�R�[�h�w��(CR)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "���͉��s�R�[�h�w��(&E)" );


			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
//		case 5://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
		case 6://Feb. 28, 2004 genta �u�ݒ�v�̐V�݂̂��ߔԍ������炵��
			m_CMenuDrawer.ResetContents();
			/* �u�E�B���h�E�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitRows() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "�㉺�ɕ���(&-)";	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(T��-)
			}else{
				pszLabel = "�㉺�����̉���(&-)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_V , pszLabel );

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitCols() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "���E�ɕ���(&I)";	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(Y��I)
			}else{
				pszLabel = "���E�����̉���(&I)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_H , pszLabel );
//	From Here Sept. 17, 2000 JEPRO	�c�������̏ꍇ����Ԃɂ���ă��j���[���b�Z�[�W���ς��悤�ɕύX
			if( (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitRows()) && (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitCols()) ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "�c�������̉���(&S)";	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(Q��S)
			}else{
				pszLabel = "�c���ɕ���(&S)";	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(S��Q)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�����ɖ߂���(Q��S)
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_VH , pszLabel );
//	To Here Sept. 17, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "����(&C)" );			//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(O��C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, "���ׂĕ���(&Q)" );		//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)
			if( m_pShareData->m_Common.m_bDispTabWnd ){
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAB_CLOSEOTHER	, "���̃^�u�ȊO�����(&O)" );	// 2009.12.26 syat �^�u�̑��상�j���[����ړ�
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAB_CLOSEOTHER	, "���̃E�B���h�E�ȊO�����(&O)" );	// 2009.12.26 syat �^�u�̑��상�j���[����ړ�
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, "���̃E�B���h�E(&N)" );	//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, "�O�̃E�B���h�E(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINLIST			, "�E�B���h�E�ꗗ(&W)..." );		// 2006.03.23 fon
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, "�d�˂ĕ\��(&E)" );		//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(C��E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, "�㉺�ɕ��ׂĕ\��(&H)" );	//Sept. 13, 2000 JEPRO �����ɍ��킹�ă��j���[�̍��E�Ə㉺�����ւ��� //Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(V��H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, "���E�ɕ��ׂĕ\��(&T)" );	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(H��T)
			if( (DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST ){
				pszLabel = "��Ɏ�O������(&F)";
			}else{
				pszLabel = "��Ɏ�O�ɕ\��(&F)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOPMOST, pszLabel ); //2004.09.21 Moca	// 2007.06.20 ryoji �A�N�Z�X�L�[�ǉ�

			hMenuPopUp = ::CreatePopupMenu();	// 2007.06.20 ryoji
			if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin ){
				pszLabel = "�O���[�v��������(&B)";
			}else{
				pszLabel = "�O���[�v��(&B)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BIND_WINDOW, pszLabel );		//2004.07.14 Kazika �V�K�ǉ�	// 2007.02.13 ryoji �u�������ĕ\���v���u�ЂƂɂ܂Ƃ߂ĕ\���v	// 2007.06.20 ryoji �u�O���[�v���v
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE		, "�O���[�v�����(&G)" );	// 2007.06.20 ryoji
			//2009.12.26 syat �u���̃^�u�ȊO�����v�́u���̃E�B���h�E�ȊO�����v�ƌ��p�Ƃ��A�E�B���h�E���j���[�����ֈړ��B
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSEOTHER	, "���̃^�u�ȊO�����(&O)" );	// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSELEFT	, "�������ׂĕ���(&H)" );		// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSERIGHT	, "�E�����ׂĕ���(&M)" );		// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_NEXTGROUP		, "���̃O���[�v(&N)" );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PREVGROUP		, "�O�̃O���[�v(&P)" );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVERIGHT	, "�^�u���E�Ɉړ�(&R)" );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVELEFT	, "�^�u�����Ɉړ�(&L)" );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_SEPARATE	, "�V�K�O���[�v(&E)" );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTNEXT	, "���̃O���[�v�Ɉړ�(&X)" );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTPREV	, "�O�̃O���[�v�Ɉړ�(&V)" );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�^�u�̑���(&B)" );		// 2007.06.20 ryoji

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, "�c�����ɍő剻(&X)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, "�������ɍő剻(&Y)" );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, "���ׂčŏ���(&M)" );		//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */				//Oct. 22, 2000 JEPRO ���́u�ĕ`��v�����ɔ����Z�p���[�^��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, "�ĕ`��(&R)" );			//Oct. 22, 2000 JEPRO �R�����g�A�E�g����Ă����̂𕜊�������
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, "�A�E�g�v�b�g(&U)" );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(O��U)
// 2006.03.23 fon CHG-start>>
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );		/* �Z�p���[�^ */
			EditNode*	pEditNodeArr;
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			delete [] pEditNodeArr;
//<< 2006.03.23 fon CHG-end
			break;

//		case 8://case 6: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
//		case 6://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
		case 7://Feb. 28, 2004 genta �u�ݒ�v�̐V�݂̂��ߔԍ������炵��
			m_CMenuDrawer.ResetContents();
			/* �u�w���v�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "�ڎ�(&O)" );				//Sept. 7, 2000 jepro �L���v�V�������u�w���v�ڎ��v����ύX	Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(C��O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 "�L�[���[�h����(&S)..." );	//Sept. 7, 2000 jepro �L���v�V�������u�w���v�g�s�b�N�̌����v����ύX //Nov. 25, 2000 jepro�u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX // 2004.05.06 MIK ...�ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, "�R�}���h�ꗗ(&M)" );		//Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(L��M)
//Sept. 16, 2000 JEPRO �V���[�g�J�b�g�L�[�����܂������Ȃ��̂Ŏ��s�͎E���Č��ɖ߂��Ă���		//Dec. 25, 2000 ����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, "�L�[���蓖�Ĉꗗ���R�s�[(&Q)" );			//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q) IDM_TEST��F�ɕύX			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, "�O���w���v�P(&E)" );		//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��g�b�v���牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, "�O��HTML�w���v(&H)" );	//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��Q�Ԗڂ��牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, "�o�[�W�������(&A)" );	//Dec. 25, 2000 JEPRO F_�ɕύX
			break;
		}
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if (m_pPrintPreview)	return;	//	����v���r���[���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

	/* �@�\�����p�\���ǂ����A�`�F�b�N��Ԃ��ǂ������ꊇ�`�F�b�N */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		UINT id = ::GetMenuItemID(hMenu, nPos);
		/* �@�\�����p�\�����ׂ� */
		//	Jan.  8, 2006 genta �@�\���L���ȏꍇ�ɂ͖����I�ɍĐݒ肵�Ȃ��悤�ɂ���D
		if( ! IsFuncEnable( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		/* �@�\���`�F�b�N��Ԃ����ׂ� */
		if( IsFuncChecked( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
		/* else{
			fuFlags = MF_BYCOMMAND | MF_UNCHECKED;
		}
		*/
	}

	return;
}




STDMETHODIMP CEditWnd::DragEnter(  LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pDataObject == NULL || pdwEffect == NULL ){
		return E_INVALIDARG;
	}

	// �E�{�^���t�@�C���h���b�v�̏ꍇ������������
	if( !((MK_RBUTTON & dwKeyState) && IsDataAvailable(pDataObject, CF_HDROP)) ){
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// ����v���r���[�ł͎󂯕t���Ȃ�
	if( m_pPrintPreview ){
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP CEditWnd::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP CEditWnd::DragLeave( void )
{
	return S_OK;
}

STDMETHODIMP CEditWnd::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	// �t�@�C���h���b�v���A�N�e�B�u�r���[�ŏ�������
	*pdwEffect &= DROPEFFECT_LINK;
	return m_cEditDoc.m_cEditViewArr[m_cEditDoc.GetActivePane()].PostMyDropFiles( pDataObject );
}

/* �t�@�C�����h���b�v���ꂽ */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
	POINT		pt;
	WORD		cFiles, i;
	BOOL		bOpened;
	EditInfo*	pfi;
	HWND		hWndOwner;

	::DragQueryPoint( hDrop, &pt );
	cFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0);
	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	if( m_pShareData->m_Common.m_bDropFileAndClose ){
		cFiles = 1;
	}
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	if( cFiles > m_pShareData->m_Common.m_nDropFileNumMax ){
		cFiles = m_pShareData->m_Common.m_nDropFileNumMax;
	}

	for( i = 0; i < cFiles; i++ ) {
		//�t�@�C���p�X�擾�A�����B
		TCHAR		szFile[_MAX_PATH + 1];
		// 2008.07.28 nasukoji	��������p�X���h���b�v�����ƈُ�I�����邱�Ƃւ̑΍�
		if( ::DragQueryFile( hDrop, i, NULL, 0 ) >= _MAX_PATH ){
			ErrorBeep();
			MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				_T("�t�@�C���p�X���������܂��B ANSI �łł� %d �o�C�g�ȏ�̐�΃p�X�������܂���B"),
				_MAX_PATH );
			continue;
		}

		::DragQueryFile( hDrop, i, szFile, _countof(szFile) );
		ResolvePath(szFile);

		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
			::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( hWndOwner );
			/* MRU���X�g�ւ̓o�^ */
			CMRUFile cMRU;
			cMRU.Add( pfi );
		}
		else{
			/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
			//	2005.06.24 Moca
			if( m_cEditDoc.IsFileOpenInThisWindow() ){
				/* �t�@�C���ǂݍ��� */
				BOOL bRet = m_cEditDoc.FileRead(
					szFile,
					&bOpened,
					CODE_AUTODETECT,	/* �����R�[�h�������� */
					FALSE,				/* �ǂݎ���p�� */
					//	Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
					m_pShareData->m_Common.m_bQueryIfCodeChange
				);
				hWndOwner = m_hWnd;
				/* �A�N�e�B�u�ɂ��� */
				// 2007.06.17 maru ���łɊJ���Ă��邩�`�F�b�N�ς݂���
				// �h���b�v���ꂽ�̂̓t�H���_��������Ȃ��̂ōă`�F�b�N
				if(FALSE==bOpened) ActivateFrameWindow( hWndOwner );

				// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
				// 2007.06.27 maru ���łɕҏW�E�B���h�E�͊J���Ă���̂�FileRead���L�����Z�����ꂽ�Ƃ��͊J���}�N�������s����K�v�Ȃ�
				if(TRUE==bRet) m_cEditDoc.RunAutoMacro( m_pShareData->m_nMacroOnOpened );
			}
			else{
				/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
				if( m_pShareData->m_Common.m_bDropFileAndClose ){
					/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
					if( m_cEditDoc.OnFileClose() ){
						/* �����f�[�^�̃N���A */
						m_cEditDoc.InitDoc();

						/* �S�r���[�̏����� */
						m_cEditDoc.InitAllView();

						/* �e�E�B���h�E�̃^�C�g�����X�V */
						m_cEditDoc.UpdateCaption();

						/* �t�@�C���ǂݍ��� */
							m_cEditDoc.FileRead(
							szFile,
							&bOpened,
							CODE_AUTODETECT,	/* �����R�[�h�������� */
							FALSE,				/* �ǂݎ���p�� */
							//	Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
							m_pShareData->m_Common.m_bQueryIfCodeChange
							//TRUE
						);
						hWndOwner = m_hWnd;
						/* �A�N�e�B�u�ɂ��� */
						ActivateFrameWindow( hWndOwner );

						// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
						m_cEditDoc.RunAutoMacro( m_pShareData->m_nMacroOnOpened );
					}
					goto end_of_drop_query;
				}
				else{
					/* �ҏW�E�B���h�E�̏���`�F�b�N */
					if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
						::MYMESSAGEBOX( NULL, MB_OK, GSTR_APPNAME, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
						::DragFinish( hDrop );
						return;
					}
					char	szFile2[_MAX_PATH + 3];
					if( strchr( szFile, ' ' ) ){
						wsprintf( szFile2, "\"%s\"", szFile );
						strcpy( szFile, szFile2 );
					}
					/* �V���ȕҏW�E�B���h�E���N�� */
					CEditApp::OpenNewEditor(
						m_hInstance,
						m_hWnd,
						szFile,
						CODE_AUTODETECT,	/* �����R�[�h�������� */
						FALSE				/* �ǂݎ���p�� */
					);
				}
			}
		}
	}
end_of_drop_query:;
	::DragFinish( hDrop );
	return;
}

/*! WM_TIMER ���� 
	@date 2007.04.03 ryoji �V�K
	@date 2008.04.19 ryoji IDT_FIRST_IDLE �ł� MYWM_FIRST_IDLE �|�X�g������ǉ�
*/
LRESULT CEditWnd::OnTimer( WPARAM wParam, LPARAM lParam )
{
	// �^�C�}�[ ID �ŏ�����U�蕪����
	switch( wParam )
	{
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		CShareData::getInstance()->PostMessageToAllEditors( MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL );	// �v���Z�X�̏���A�C�h�����O�ʒm	// 2008.04.19 ryoji
		::KillTimer( m_hWnd, wParam );
		break;
	default:
		return 1L;
	}

	return 0L;
}

/*! �^�C�}�[�̏���
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka �c�[���o�[�X�V�� OnToolbarTimer�Ɉړ�����
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	@date 2012.11.29 aroka �Ăяo���Ԋu�̃o�O�C��
*/
void CEditWnd::OnEditTimer( void )
{
	//static	int	nLoopCount = 0; // wmlhq m_nTimerCount�Ɉڍs
	// �^�C�}�[�̌Ăяo���Ԋu�� 500ms�ɕύX�B300*10��500*6�ɂ���B 20060128 aroka
	m_nTimerCount++;
	if( 6 <= m_nTimerCount ){	// 2012.11.29 aroka �Ăяo���Ԋu�̃o�O�C��
		m_nTimerCount = 0;
	}

	// 2006.01.28 aroka �c�[���o�[�X�V�֘A�� OnToolbarTimer�Ɉړ������B
	
	//	Aug. 29, 2003 wmlhq, ryoji
	if( m_nTimerCount == 0 && GetCapture() == NULL ){ 
		/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
		m_cEditDoc.CheckFileTimeStamp() ;
	}

	m_cEditDoc.CheckAutoSave();
	return;
}

/*! �c�[���o�[�X�V�p�^�C�}�[�̏���
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka OnTimer���番��
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	@date 2008.09.23 nasukoji �c�[���o�[�X�V�������O�ɏo����
	@date 2012.11.29 aroka OnTimer���番�������Ƃ��̃o�O�C��
*/
void CEditWnd::OnToolbarTimer( void )
{
	// 2012.11.29 aroka �����ł̓J�E���g�A�b�v�s�v
	//m_nTimerCount++;
	//if( 10 < m_nTimerCount ){
	//	m_nTimerCount = 0;
	//}

	UpdateToolbar();	// 2008.09.23 nasukoji	�c�[���o�[�̕\�����X�V����

	return;
}

/*!
	@brief �c�[���o�[�̕\�����X�V����
	
	@note ������Ăׂ�悤��OnToolbarTimer()���؂�o����
	
	@date 2008.09.23 nasukoji
*/
void CEditWnd::UpdateToolbar( void )
{
	//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[�Ȃ�A�������Ȃ��B�����łȂ���΁A�c�[���o�[�̏�ԍX�V */
	if( !m_pPrintPreview && NULL != m_hwndToolBar ){
		for( int i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			TBBUTTON tbb = m_CMenuDrawer.getButton(m_pShareData->m_Common.m_nToolBarButtonIdxArr[i]);

			/* �@�\�����p�\�����ׂ� */
			::PostMessage(
				m_hwndToolBar, TB_ENABLEBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( (IsFuncEnable( &m_cEditDoc, m_pShareData, tbb.idCommand ) ) , 0 )
			);

			/* �@�\���`�F�b�N��Ԃ����ׂ� */
			::PostMessage(
				m_hwndToolBar, TB_CHECKBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( IsFuncChecked( &m_cEditDoc, m_pShareData, tbb.idCommand ), 0 )
			);
		}
	}
}

//�����{�b�N�X���X�V
void CEditWnd::AcceptSharedSearchKey()
{
	if( m_hwndSearchBox )
	{
		int	i;
		::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; i++ )
		{
			::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
		}
		::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
	}
}

/*! �L���v�V�����X�V�p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �V�K
*/
void CEditWnd::OnCaptionTimer( void )
{
	// �ҏW��ʂ̐ؑցi�^�u�܂Ƃߎ��j���I����Ă�����^�C�}�[���I�����ă^�C�g���o�[���X�V����
	// �܂��ؑ֒��Ȃ�^�C�}�[�p��
	if( !m_pShareData->m_bEditWndChanging ){
		::KillTimer( m_hWnd, IDT_CAPTION );
		::SetWindowText( m_hWnd, m_pszLastCaption );
	}
}

/*! �V�X�e�����j���[�\���p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	                       �ȑO�̓R�[���o�b�N�֐��ł���Ă���KillTimer()�������ōs���悤�ɂ���
*/
void CEditWnd::OnSysMenuTimer( void ) //by �S(2)
{
	::KillTimer( m_hWnd, IDT_SYSMENU );	// 2007.04.03 ryoji

	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//�V�X�e�����j���[�\��
		// 2006.04.21 ryoji �}���`���j�^�Ή��̏C��
		// 2007.05.13 ryoji 0x0313���b�Z�[�W���|�X�g��������ɕύX�iTrackPopupMenu���ƃ��j���[���ڂ̗L���^������Ԃ��s���ɂȂ���΍�j
		RECT R;
		GetWindowRect(m_hWnd, &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect( pt, &R );
		::PostMessage(
			m_hWnd,
			0x0313, //�E�N���b�N�ŃV�X�e�����j���[��\������ۂɑ��M���郂�m�炵��
			0,
			MAKELPARAM( (pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom )
		);
	}
	m_IconClicked = icNone;
}

/* �f�o�b�O���j�^���[�h�ɐݒ� */
void CEditWnd::SetDebugModeON()
{
	if( NULL != m_pShareData->m_hwndDebug ){
		if( IsSakuraMainWindow( m_pShareData->m_hwndDebug ) ){
			return;
		}
	}
	m_pShareData->m_hwndDebug = m_hWnd;
	m_cEditDoc.m_bDebugMode = TRUE;

// 2001/06/23 N.Nakatani �A�E�g�v�b�g���ւ̏o�̓e�L�X�g�̒ǉ�F_ADDTAIL���}�~�����̂łƂ肠�����ǂݎ���p���[�h�͎��߂܂���
	m_cEditDoc.m_bReadOnly = FALSE;		/* �ǂݎ���p���[�h */
	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_cEditDoc.UpdateCaption();
}

// 2005.06.24 Moca
//! �f�o�b�N���j�^���[�h�̉���
void CEditWnd::SetDebugModeOFF()
{
	if( m_pShareData->m_hwndDebug == m_hWnd ){
		m_pShareData->m_hwndDebug = NULL;
		m_cEditDoc.m_bDebugMode = FALSE;
		m_cEditDoc.UpdateCaption();
	}
}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX

/* ����v���r���[���[�h�̃I��/�I�t */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;
	HWND	hwndToolBar;

	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;

	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	����v���r���[���[�h���������܂��B	*/
		delete m_pPrintPreview;	//	�폜�B
		m_pPrintPreview = NULL;	//	NULL���ۂ��ŁA�v�����g�v���r���[���[�h�����f���邽�߁B

		/*	�ʏ탂�[�h�ɖ߂�	*/
		::ShowWindow( m_cEditDoc.m_hWnd, SW_SHOW );
		::ShowWindow( hwndToolBar, SW_SHOW );	// 2006.06.17 ryoji
		::ShowWindow( m_hwndStatusBar, SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_SHOW );
		::ShowWindow( m_cTabWnd.m_hWnd, SW_SHOW );	//@@@ 2003.06.25 MIK

		::SetFocus( m_hWnd );

		hMenu = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( m_hWnd, hMenu );
		::DrawMenuBar( m_hWnd );

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	�ʏ탂�[�h���B��	*/
		hMenu = ::GetMenu( m_hWnd );
		//	Jun. 18, 2001 genta Print Preview�ł̓��j���[���폜
		::SetMenu( m_hWnd, NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( m_hWnd );

		::ShowWindow( m_cEditDoc.m_hWnd, SW_HIDE );
		::ShowWindow( hwndToolBar, SW_HIDE );	// 2006.06.17 ryoji
		::ShowWindow( m_hwndStatusBar, SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_HIDE );
		::ShowWindow( m_cTabWnd.m_hWnd, SW_HIDE );	//@@@ 2003.06.25 MIK

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview = new CPrintPreview( this );
		/* ���݂̈���ݒ� */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	�v�����^�̏����擾�B

		/* ���݂̃f�t�H���g�v�����^�̏����擾 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( !bRes ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME, _T("����v���r���[�����s����O�ɁA�v�����^���C���X�g�[�����Ă��������B\n") );
			return;
		}

		/* ����ݒ�̔��f */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( m_hWnd, NULL, TRUE );
		::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}




/* WM_SIZE ���� */
LRESULT CEditWnd::OnSize( WPARAM wParam, LPARAM lParam )
{
	HWND		hwndToolBar;
	int			cx;
	int			cy;
	int			nToolBarHeight;
	int			nStatusBarHeight;
	int			nFuncKeyWndHeight;
	int			nTabWndHeight;	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	RECT		rc, rcClient;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��
//	�ϐ��폜

	RECT		rcWin;



	cx = LOWORD( lParam );
	cy = HIWORD( lParam );

	/* �E�B���h�E�T�C�Y�p�� */
	if( wParam != SIZE_MINIMIZED ){						/* �ŏ����͌p�����Ȃ� */
		//	2004.05.13 Moca m_eSaveWindowSize�̉��ߒǉ��̂���
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_eSaveWindowSize ){		/* �E�B���h�E�T�C�Y�p�������邩 */
			if( wParam == SIZE_MAXIMIZED ){					/* �ő剻�̓T�C�Y���L�^���Ȃ� */
				if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ){
					m_pShareData->m_Common.m_nWinSizeType = wParam;
				}
			}else{
				::GetWindowRect( m_hWnd, &rcWin );
				/* �E�B���h�E�T�C�Y�Ɋւ���f�[�^���ύX���ꂽ�� */
				if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ||
					m_pShareData->m_Common.m_nWinSizeCX != rcWin.right - rcWin.left ||
					m_pShareData->m_Common.m_nWinSizeCY != rcWin.bottom - rcWin.top
				){
					m_pShareData->m_Common.m_nWinSizeType = wParam;
					m_pShareData->m_Common.m_nWinSizeCX = rcWin.right - rcWin.left;
					m_pShareData->m_Common.m_nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// ���ɖ߂��Ƃ��̃T�C�Y��ʂ��L��	// 2007.06.20 ryoji
		EditNode *p = CShareData::getInstance()->GetEditNode( m_hWnd );
		if( p != NULL ){
			p->m_showCmdRestore = ::IsZoomed( p->m_hWnd )? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	/* �T�C�Y�ύX�̃^�C�v */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;
	nToolBarHeight = 0;
	if( NULL != hwndToolBar ){
		::SendMessage( hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		::SendMessage( m_CFuncKeyWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.m_hWnd, &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//�^�u�E�C���h�E
	nTabWndHeight = 0;
	if( NULL != m_cTabWnd.m_hWnd )
	{
		::SendMessage( m_cTabWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_cTabWnd.m_hWnd, &rc );
		nTabWndHeight = rc.bottom - rc.top;
	}
	//@@@ To Here 2003.05.31 MIK
	nStatusBarHeight = 0;
	if( NULL != m_hwndStatusBar ){
		::SendMessage( m_hwndStatusBar, WM_SIZE, wParam, lParam );
		::GetClientRect( m_hwndStatusBar, &rc );
		//	May 12, 2000 genta
		//	2�J�����ڂɉ��s�R�[�h�̕\����}��
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0�p�~�ɏ]���A�K���ɒ���
		// 2004-02-28 yasu ��������o�͎��̏����ɍ��킹��
		// ����ς����ꍇ�ɂ�CEditView::DrawCaretPosInfo()�ł̕\�����@���������K�v����D
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 �s 9999 ��"), _T("CRLF"), _T("0000"), _T("Unicode"), _T("REC"), _T("�㏑") };	//Oct. 30, 2000 JEPRO �疜�s���v���
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth = rc.right - rc.left;
		int			nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // �T�C�Y�{�b�N�X�̕�
		int			nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // ���E�̕�
		SIZE		sz;
		HDC			hdc;
		int			i;
		// 2004-02-28 yasu
		// ���m�ȕ����v�Z���邽�߂ɁA�\���t�H���g���擾����hdc�ɑI��������B
		hdc = ::GetDC( m_hwndStatusBar );
		HFONT hFont = (HFONT)::SendMessage(m_hwndStatusBar, WM_GETFONT, 0, 0);
		if (hFont != NULL)
		{
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if( wParam != SIZE_MAXIMIZED ){
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for( i = nStArrNum - 1; i > 0; i-- ){
			::GetTextExtentPoint32( hdc, pszLabel[i], _tcslen( pszLabel[i] ), &sz );
			nStArr[i - 1] = nStArr[i] - ( sz.cx + nBdrWidth );
		}

		//	Nov. 8, 2003 genta
		//	������Ԃł͂��ׂĂ̕������u�g����v�����C���b�Z�[�W�G���A�͘g��`�悵�Ȃ��悤�ɂ��Ă���
		//	���߁C���������̘g���ςȕ��Ɏc���Ă��܂��D������ԂŘg��`�悳���Ȃ����邽�߁C
		//	�ŏ��Ɂu�g�����v��Ԃ�ݒ肵����Ńo�[�̕������s���D
		::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)_T(""));

		::SendMessage( m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
		if (hFont != NULL)
		{
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC( m_hwndStatusBar, hdc );

		::UpdateWindow( m_hwndStatusBar );	// 2006.06.17 ryoji �����`��ł���������炷
		::GetWindowRect( m_hwndStatusBar, &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_hWnd, &rcClient );

	//@@@ From 2003.05.31 MIK
	//�^�u�E�C���h�E�ǉ��ɔ����C�t�@���N�V�����L�[�\���ʒu������

	//�^�u�E�C���h�E
	if( m_cTabWnd.m_hWnd )
	{
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 )
		{
			::MoveWindow( m_cTabWnd.m_hWnd, 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
		}
		else
		{
			::MoveWindow( m_cTabWnd.m_hWnd, 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
		}
	}

	//	2005.04.23 genta �t�@���N�V�����L�[��\���̎��͈ړ����Ȃ�
	if( m_CFuncKeyWnd.m_hWnd != NULL ){
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 )
		{	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			::MoveWindow(
				m_CFuncKeyWnd.m_hWnd,
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE );
		}
		else if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 1 )
		{	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			::MoveWindow(
				m_CFuncKeyWnd.m_hWnd,
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			BOOL	bSizeBox = TRUE;
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
			if( wParam == SIZE_MAXIMIZED ){
				bSizeBox = FALSE;
			}
			m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
		}
		::UpdateWindow( m_CFuncKeyWnd.m_hWnd );	// 2006.06.17 ryoji �����`��ł���������炷
	}

	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 )
	{
		m_cEditDoc.OnMove(
			0,
			nToolBarHeight + nFuncKeyWndHeight + nTabWndHeight,	//@@@ 2003.05.31 MIK
			cx,
			cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nStatusBarHeight	//@@@ 2003.05.31 MIK
		);
	}
	else
	{
		m_cEditDoc.OnMove(
			0,
			nToolBarHeight + nTabWndHeight,
			cx,
			cy - nToolBarHeight - nTabWndHeight - nFuncKeyWndHeight - nStatusBarHeight	//@@@ 2003.05.31 MIK
		);
	}
	//@@@ To 2003.05.31 MIK

	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if( !m_pPrintPreview ){
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}




/* WM_PAINT �`�揈�� */
LRESULT CEditWnd::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	HDC				hdc;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		PAINTSTRUCT		ps;
		hdc = ::BeginPaint( hwnd, &ps );
		::EndPaint( hwnd, &ps );
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

/* ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL */
LRESULT CEditWnd::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		return 0;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}




/* ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� */
LRESULT CEditWnd::OnHScroll( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		return 0;
	}
	return m_pPrintPreview->OnHScroll( wParam, lParam );
}

LRESULT CEditWnd::OnLButtonDown( WPARAM wParam, LPARAM lParam )
{
	//by �S(2) �L���v�`���[���ĉ����ꂽ���N���C�A���g�ł��������ɗ���
	if(m_IconClicked != icNone)
		return 0;

	WPARAM		fwKeys;
	int			xPos;
	int			yPos;
	fwKeys = wParam;		// key flags
	xPos = LOWORD(lParam);	// horizontal position of cursor
	yPos = HIWORD(lParam);	// vertical position of cursor
	m_nDragPosOrgX = xPos;
	m_nDragPosOrgY = yPos;
	SetCapture( m_hWnd );
	m_bDragMode = TRUE;

	return 0;
}

LRESULT CEditWnd::OnLButtonUp( WPARAM wParam, LPARAM lParam )
{
	//by �S 2002/04/18
	if(m_IconClicked != icNone)
	{
		if(m_IconClicked == icDown)
		{
			m_IconClicked = icClicked;
			//by �S(2) �^�C�}�[(ID�͓K���ł�)
			SetTimer(m_hWnd, IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	m_bDragMode = FALSE;
	ReleaseCapture();
	::InvalidateRect( m_hWnd, NULL, TRUE );
	return 0;
}


/*!	WM_MOUSEMOVE����
	@date 2008.05.05 novice ���������[�N�C��
*/
LRESULT CEditWnd::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	//by �S
	if(m_IconClicked != icNone)
	{
		//by �S(2) ��񉟂��ꂽ������
		if(m_IconClicked == icDown)
		{
			POINT P;
			GetCursorPos(&P); //�X�N���[�����W
			if(SendMessage(m_hWnd, WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
			{
				ReleaseCapture();
				m_IconClicked = icNone;

				if(m_cEditDoc.IsFilePathAvailable())
				{
					const char *PathEnd = m_cEditDoc.GetFilePath();
					for(const char* I = m_cEditDoc.GetFilePath(); *I != 0; ++I)
					{
						//by �S(2): DBCS����
						if(IsDBCSLeadByte(*I))
							++I;
						else if(*I == '\\')
							PathEnd = I;
					}

					wchar_t WPath[MAX_PATH];
					int c = MultiByteToWideChar(CP_ACP, 0, m_cEditDoc.GetFilePath(), PathEnd - m_cEditDoc.GetFilePath(), WPath, MAX_PATH);
					WPath[c] = 0;
					wchar_t WFile[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, PathEnd + 1, -1, WFile, MAX_PATH);

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if(SUCCEEDED(Desktop->ParseDisplayName(0, NULL, WPath, &Eaten, &PathID, &Attribs)))
					{
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if(SUCCEEDED(Folder->ParseDisplayName(0, NULL, WFile, &Eaten, &ItemID, &Attribs)))
						{
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							//�e�L�X�g�ł���������c�֗�
							{
								FORMATETC F;
								F.cfFormat = CF_TEXT;
								F.ptd      = NULL;
								F.dwAspect = DVASPECT_CONTENT;
								F.lindex   = -1;
								F.tymed    = TYMED_HGLOBAL;

								STGMEDIUM M;
								const TCHAR* pFilePath = m_cEditDoc.GetFilePath();
								int Len = _tcslen(pFilePath);
								M.tymed          = TYMED_HGLOBAL;
								M.pUnkForRelease = NULL;
								M.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len+1)*sizeof(TCHAR));
								void* p = GlobalLock(M.hGlobal);
								CopyMemory(p, pFilePath, (Len+1)*sizeof(TCHAR));
								GlobalUnlock(M.hGlobal);

								DataObject->SetData(&F, &M, TRUE);
							}
#endif
							//�ړ��͋֎~
							DWORD R;
							CDropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
							DataObject->Release();
						}
						Folder->Release();
					}
					Desktop->Release();
					Malloc->Release();
				}
			}
		}
		return 0;
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if (!m_pPrintPreview){
		return 0;
	}
	else {
		return m_pPrintPreview->OnMouseMove( wParam, lParam );
	}
}




LRESULT CEditWnd::OnMouseWheel( WPARAM wParam, LPARAM lParam )
{
	if( m_pPrintPreview ){
		return m_pPrintPreview->OnMouseWheel( wParam, lParam );
	}
	return m_cEditDoc.DispatchEvent( m_hWnd, WM_MOUSEWHEEL, wParam, lParam );
}

/** �}�E�X�z�C�[������

	@date 2007.10.16 ryoji OnMouseWheel()���珈�������o��
*/
BOOL CEditWnd::DoMouseWheel( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		// 2006.03.26 ryoji by assitance with John �^�u��Ȃ�E�B���h�E�؂�ւ�
		if( m_pShareData->m_Common.m_bChgWndByWheel && NULL != m_cTabWnd.m_hwndTab )
		{
			POINT pt;
			pt.x = (short)LOWORD( lParam );
			pt.y = (short)HIWORD( lParam );
			int nDelta = (short)HIWORD( wParam );
			HWND hwnd = ::WindowFromPoint( pt );
			if( (hwnd == m_cTabWnd.m_hwndTab || hwnd == m_cTabWnd.m_hWnd) )
			{
				// ���݊J���Ă���ҏW���̃��X�g�𓾂�
				EditNode* pEditNodeArr;
				int nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
				if(  nRowNum > 0 )
				{
					// �����̃E�B���h�E�𒲂ׂ�
					int i, j;
					int nGroup = 0;
					for( i = 0; i < nRowNum; ++i )
					{
						if( m_hWnd == pEditNodeArr[i].m_hWnd )
						{
							nGroup = pEditNodeArr[i].m_nGroup;
							break;
						}
					}
					if( i < nRowNum )
					{
						if( nDelta < 0 )
						{
							// ���̃E�B���h�E
							for( j = i + 1; j < nRowNum; ++j )
							{
								if( nGroup == pEditNodeArr[j].m_nGroup )
									break;
							}
							if( j >= nRowNum )
							{
								for( j = 0; j < i; ++j )
								{
									if( nGroup == pEditNodeArr[j].m_nGroup )
										break;
								}
							}
						}
						else
						{
							// �O�̃E�B���h�E
							for( j = i - 1; j >= 0; --j )
							{
								if( nGroup == pEditNodeArr[j].m_nGroup )
									break;
							}
							if( j < 0 )
							{
								for( j = nRowNum - 1; j > i; --j )
								{
									if( nGroup == pEditNodeArr[j].m_nGroup )
										break;
								}
							}
						}

						/* ���́ior �O�́j�E�B���h�E���A�N�e�B�u�ɂ��� */
						if( i != j )
							ActivateFrameWindow( pEditNodeArr[j].m_hWnd );
					}

					delete []pEditNodeArr;
				}
				return TRUE;	// ��������
			}
		}
		return FALSE;	// �������Ȃ�����
	}
	return FALSE;	// �������Ȃ�����
}

/* ����y�[�W�ݒ�
	����v���r���[���ɂ��A�����łȂ��Ƃ��ł��Ă΂��\��������B
*/
BOOL CEditWnd::OnPrintPageSetting( void )
{
	/* ����ݒ�iCANCEL�������Ƃ��ɔj�����邽�߂̗̈�j */
	CDlgPrintSetting	cDlgPrintSetting;
	BOOL				bRes;
	PRINTSETTING		PrintSettingArr[MAX_PRINTSETTINGARR];
	int					i;
	int					nCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[i];
	}

//	cDlgPrintSetting.Create( m_hInstance, m_hWnd );
	nCurrentPrintSetting = m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		m_hInstance,
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_hWnd,
		&nCurrentPrintSetting, /* ���ݑI�����Ă������ݒ� */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* ���ݑI������Ă���y�[�W�ݒ�̔ԍ����ύX���ꂽ�� */
		if( nCurrentPrintSetting !=
			m_pShareData->m_Types[m_cEditDoc.GetDocumentType()].m_nCurrentPrintSetting
		){
//			/* �ύX�t���O(�^�C�v�ʐݒ�) */
			m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
		}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		//	����v���r���[���̂݁B
		if ( m_pPrintPreview ){
			/* ���݂̈���ݒ� */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* ����v���r���[ �X�N���[���o�[������ */
			m_pPrintPreview->InitPreviewScrollBar();

			/* ����ݒ�̔��f */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( m_hWnd, NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by �S

LRESULT CEditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(wp == HTSYSMENU)
	{
		SetCapture(m_hWnd);
		m_IconClicked = icDown;
		Result = 0;
	}
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT CEditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		//�O�̂���
		ReleaseCapture();
		m_IconClicked = icNone;
		Result = 0;
	}
	else if(wp == HTSYSMENU)
		Result = 0;
	else{
		//	2004.05.23 Moca ���b�Z�[�W�~�X�C��
		//	�t���[���̃_�u���N���b�N����ɃE�B���h�E�T�C�Y
		//	�ύX���[�h�Ȃ��Ă���
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONUP, wp, lp);
	}

	return Result;
}

LRESULT CEditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) //by �S(2)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		ReleaseCapture();
		m_IconClicked = icDoubleClicked;

		SendMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}
	else {
		//	2004.05.23 Moca ���b�Z�[�W�~�X�C��
		Result = DefWindowProc(m_hWnd, WM_LBUTTONDBLCLK, wp, lp);
	}

	return Result;
}

/*! �h���b�v�_�E�����j���[(�J��) */	//@@@ 2002.06.15 MIK
int	CEditWnd::CreateFileDropDownMenu( HWND hwnd )
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;
	int			nIndex;

	// ���j���[�\���ʒu�����߂�	// 2007.03.25 ryoji
	// �� TBN_DROPDOWN ���� NMTOOLBAR::iItem �� NMTOOLBAR::rcButton �ɂ̓h���b�v�_�E�����j���[(�J��)�{�^����
	//    ��������Ƃ��͂ǂ�������������P�ڂ̃{�^����񂪓���悤�Ȃ̂Ń}�E�X�ʒu����{�^���ʒu�����߂�
	::GetCursorPos( &po );
	::ScreenToClient( hwnd, &po );
	nIndex = ::SendMessage( hwnd, TB_HITTEST, (WPARAM)0, (LPARAM)&po );
	if( nIndex < 0 ){
		return 0;
	}
	::SendMessage( hwnd, TB_GETITEMRECT, (WPARAM)nIndex, (LPARAM)&rc );
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen( hwnd, &po );
	GetMonitorWorkRect( po, &rc );
	if( po.x < rc.left )
		po.x = rc.left;
	if( po.y < rc.top )
		po.y = rc.top;


	m_CMenuDrawer.ResetContents();

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
	CMRUFile cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_(&D)") );
	}
	else 
	{
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_(&D)") );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T("�V�K�쐬(&N)"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T("�V�K�E�C���h�E���J��(&M)"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T("�J��(&O)..."), FALSE );

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		m_hWnd,	// 2009.02.03 ryoji �A�N�Z�X�L�[�L�����̂��� hwnd -> m_hWnd �ɕύX
		NULL
	);

	::DestroyMenu( hMenu );

	return nId;
}

/*! �����{�b�N�X�ł̏��� */
void CEditWnd::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( (TCHAR)msg->wParam == VK_RETURN )  //���^�[���L�[
		{
			//�����L�[���[�h���擾
			char	szText[_MAX_PATH];
			memset( szText, 0, _countof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );

				//�����{�b�N�X���X�V	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus( m_hWnd );	//��Ƀt�H�[�J�X���ړ����Ă����Ȃ��ƃL�����b�g��������
				::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

				// �����J�n���̃J�[�\���ʒu�o�^������ύX 02/07/28 ai start
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosX_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX_PHY;
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosY_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY_PHY;
				// 02/07/28 ai end

				//��������
				OnCommand( (WORD)0 /*���j���[*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( (TCHAR)msg->wParam == VK_TAB )	//�^�u�L�[
		{
			//�t�H�[�J�X���ړ�
			//	2004.10.27 MIK IME�\���ʒu�̂���C��
			::SetFocus( m_hWnd  );
		}
	}
}

/*!
	@brief �E�B���h�E�̃A�C�R���ݒ�

	�w�肳�ꂽ�A�C�R�����E�B���h�E�ɐݒ肷��D
	�ȑO�̃A�C�R���͔j������D

	@param hIcon [in] �ݒ肷��A�C�R��
	@param flag [in] �A�C�R����ʁDICON_BIG�܂���ICON_SMALL.
	@author genta
	@date 2002.09.10
*/
void CEditWnd::SetWindowIcon(HICON hIcon, int flag)
{
	HICON hOld = (HICON)::SendMessage( m_hWnd, WM_SETICON, flag, (LPARAM)hIcon );
	if( hOld != NULL ){
		::DestroyIcon( hOld );
	}
}

/*!
	�W���A�C�R���̎擾

	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta �V�݂������ʊ֐����g���悤��
*/
void CEditWnd::GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const
{
	*hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	*hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
}

/*!
	�A�C�R���̎擾
	
	�w�肳�ꂽ�t�@�C�����ɑΉ�����A�C�R��(��E��)���擾���ĕԂ��D
	
	@param szFile     [in] �t�@�C����
	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��
	
	@retval true �֘A�Â���ꂽ�A�C�R������������
	@retval false �֘A�Â���ꂽ�A�C�R����������Ȃ�����
	
	@author genta
	@date 2002.09.10
*/
bool CEditWnd::GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const
{
	if( NULL != szFile && szFile[0] != _T('\0') ){
		TCHAR szExt[_MAX_EXT];
		TCHAR FileType[1024];

		// (.�Ŏn�܂�)�g���q�̎擾
		_tsplitpath( szFile, NULL, NULL, NULL, szExt );
		
		if( ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, _countof(FileType) - 13)){
			_tcscat( FileType, _T("\\DefaultIcon") );
			if( ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)){
				// �֘A�Â���ꂽ�A�C�R�����擾����
				SHFILEINFO shfi;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON );
				*hIconBig = shfi.hIcon;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON );
				*hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	//	�W���̃A�C�R����Ԃ�
	GetDefaultIcon( hIconBig, hIconSmall );
	return false;
}

/*
	@brief ���j���[�o�[�\���p�t�H���g�̏�����
	
	���j���[�o�[�\���p�t�H���g�̏��������s���D
	
	@date 2002.12.04 CEditView�̃R���X�g���N�^����ړ�
*/
void CEditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	/* LOGFONT�̏����� */
	memset( &lf, 0, sizeof( lf ) );
	lf.lfHeight			= -12;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	_tcscpy( lf.lfFaceName, _T("�l�r �S�V�b�N") );
	m_hFontCaretPosInfo = ::CreateFontIndirect( &lf );

	hdc = ::GetDC( ::GetDesktopWindow() );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	::GetTextMetrics( hdc, &tm );
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	for( i = 0; i < ( sizeof( m_pnCaretPosInfoDx ) / sizeof( m_pnCaretPosInfoDx[0] ) ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( ::GetDesktopWindow(), hdc );
}

/*
	@brief ���j���[�o�[�Ƀ��b�Z�[�W��\������
	
	���O�Ƀ��j���[�o�[�\���p�t�H���g������������Ă��Ȃ��Ă͂Ȃ�Ȃ��D
	�w��ł��镶�����͍ő�30�o�C�g�D����ȏ�̏ꍇ�͂����؂��ĕ\������D
	
	@author genta
	@date 2002.12.04
*/
void CEditWnd::PrintMenubarMessage( const TCHAR* msg )
{
	if( NULL == ::GetMenu( m_hWnd ) )	// 2007.03.08 ryoji �ǉ�
		return;

	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL �̂Ƃ��͈ȑO�� m_pszMenubarMessage �ōĕ`��
	if( msg ){
		int len = _tcslen( msg );
		_tcsncpy( m_pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN );
		if( len < MENUBAR_MESSAGE_MAX_LEN ){
			memset( m_pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len );	//  null�I�[�͕s�v
		}
	}

	HDC		hdc;
	hdc = ::GetWindowDC( m_hWnd );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( m_hWnd, &poFrame );
	::GetWindowRect( m_hWnd, &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - ( ::GetSystemMetrics( SM_CXSIZEFRAME ) + 2 );
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	//	Sep. 6, 2003 genta Windows XP(Luna)�̏ꍇ�ɂ�COLOR_MENUBAR���g��Ȃ��Ă͂Ȃ�Ȃ�
	COLORREF bkColor =
		::GetSysColor( COsVersionInfo().IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU );
	::SetBkColor( hdc, bkColor );
	::ExtTextOut( hdc,rc.left,rc.top,ETO_OPAQUE,&rc,m_pszMenubarMessage,nStrLen,m_pnCaretPosInfoDx);
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	�X�e�[�^�X�o�[����\���̏ꍇ�̓��j���[�o�[�̉E�[�ɕ\������D
	
	@param msg [in] �\�����郁�b�Z�[�W
	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta CEditView���ړ�
*/
void CEditWnd::SendStatusMessage( const TCHAR* msg )
{
	if( NULL == m_hwndStatusBar ){
		// ���j���[�o�[��
		PrintMenubarMessage( msg );
	}
	else{
		// �X�e�[�^�X�o�[��
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	���j���[�o�[�E�[�ɓ���Ȃ����̂�C���ʒu�\�����B�������Ȃ����̂Ɏg��
	
	�Ăяo���O��SendStatusMessage2IsEffective()�ŏ����̗L����
	�m�F���邱�ƂŖ��ʂȏ������Ȃ����Ƃ��o����D

	@param msg [in] �\�����郁�b�Z�[�W
	@date 2005.07.09 genta �V�K�쐬
	
	@sa SendStatusMessage2IsEffective
*/
void CEditWnd::SendStatusMessage2( const char* msg )
{
	if( NULL != m_hwndStatusBar ){
		// �X�e�[�^�X�o�[��
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}

/*! �t�@�C�����ύX�ʒm

	@author MIK
	@date 2003.05.31 �V�K�쐬
	@date 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
*/
void CEditWnd::ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, BOOL bIsGrep )
{
	CRecent	cRecentEditNode;
	EditNode	*p;
	int		nIndex;

	if( NULL == pszTabCaption ) pszTabCaption = _T("");	//�K�[�h
	if( NULL == pszFilePath )pszFilePath = "";		//�K�[�h 2006.01.28 ryoji

	cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
	nIndex = cRecentEditNode.FindItem( (const char*)&m_hWnd );
	if( -1 != nIndex )
	{
		p = (EditNode*)cRecentEditNode.GetItem( nIndex );
		if( p )
		{
			int	size = _countof( p->m_szTabCaption ) - 1;
			_tcsncpy( p->m_szTabCaption, pszTabCaption, size );
			p->m_szTabCaption[ size ] = _T('\0');

			// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�ǉ�
			size = sizeof( p->m_szFilePath ) - 1;
			_tcsncpy( p->m_szFilePath, pszFilePath, size );
			p->m_szFilePath[ size ] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	cRecentEditNode.Terminate();

	//�t�@�C�����ύX�ʒm���u���[�h�L���X�g����B
	int nGroup = CShareData::getInstance()->GetGroupId( m_hWnd );
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TWNT_FILE,
		(LPARAM)m_hWnd,
		m_hWnd,
		nGroup
	);

	return;
}
/*!	@brief ToolBar��OwnerDraw

	@param pnmh [in] Owner Draw���

	@note Common Control V4.71�ȍ~��NMTBCUSTOMDRAW�𑗂��Ă��邪�C
	Common Control V4.70��LPNMCUSTOMDRAW���������Ă��Ȃ��̂�
	���S�̂��ߏ��������ɍ��킹�ď������s���D
	
	@author genta
	@date 2003.07.21 �쐬

*/
LPARAM CEditWnd::ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh )
{
	switch( pnmh->dwDrawStage ){
	case CDDS_PREPAINT:
		//	�`��J�n�O
		//	�A�C�e�������O�ŕ`�悷��|��ʒm����
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		//	�ʓ|�������̂ŁC�g��Toolbar�ɕ`���Ă��炤
		//	�A�C�R�����o�^����Ă��Ȃ��̂Œ��g�͉����`����Ȃ�
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			//	�`��
			// �R�}���h�ԍ��ipnmh->dwItemSpec�j����A�C�R���ԍ����擾����	// 2007.11.02 ryoji
			int nIconId = ::SendMessage( pnmh->hdr.hwndFrom, TB_GETBITMAP, (WPARAM)pnmh->dwItemSpec, 0 );

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - m_cIcons.cy()) / 2;		// �A�C�e����`����̉摜�̃I�t�Z�b�g	// 2007.03.25 ryoji
			int shift = pnmh->uItemState & ( CDIS_SELECTED | CDIS_CHECKED ) ? 1 : 0;	//	Aug. 30, 2003 genta �{�^���������ꂽ�炿����Ɖ摜�����炷
			int color = pnmh->uItemState & CDIS_CHECKED ? COLOR_3DHILIGHT : COLOR_3DFACE;

			//	Sep. 6, 2003 genta �������͉E�����łȂ����ɂ����炷
			m_cIcons.Draw( nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED ) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}

/*! ��Ɏ�O�ɕ\��
	@param top  0:�g�O������ 1:�őO�� 2:�őO�ʉ��� ���̑�:�Ȃɂ����Ȃ�
	@date 2004.09.21 Moca
*/
void CEditWnd::WindowTopMost( int top )
{
	if( 0 == top ){
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE );
		if( dwExstyle & WS_EX_TOPMOST ){
			top = 2; // �őO�ʂł��� -> ����
		}else{
			top = 1;
		}
	}

	HWND hwndInsertAfter;
	switch( top ){
	case 1:
		hwndInsertAfter = HWND_TOPMOST;
		break;
	case 2:
		hwndInsertAfter = HWND_NOTOPMOST;
		break;
	default:
		return;
	}

	::SetWindowPos( m_hWnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	// �^�u�܂Ƃߎ��� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�œ�������	// 2007.05.18 ryoji
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin ){
		HWND hwnd;
		int i;
		for( i = 0, hwndInsertAfter = m_hWnd; i < m_pShareData->m_nEditArrNum; i++ ){
			hwnd = m_pShareData->m_pEditArr[i].m_hWnd;
			if( hwnd != m_hWnd && IsSakuraMainWindow( hwnd ) ){
				if( !CShareData::getInstance()->IsSameGroup( m_hWnd, hwnd ) )
					continue;
				::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
				hwndInsertAfter = hwnd;
			}
		}
	}
}

/*!
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void CEditWnd::SetFocusSearchBox( void ) const
{
	if( m_hwndSearchBox ){
		::SetFocus(m_hwndSearchBox);
	}
}


// �^�C�}�[�̍X�V���J�n�^��~����B 20060128 aroka
// �c�[���o�[�\���̓^�C�}�[�ɂ��X�V���Ă��邪�A
// �A�v���̃t�H�[�J�X���O�ꂽ�Ƃ��ɃE�B���h�E����ON/OFF��
//	�Ăяo���Ă��炤���Ƃɂ��A�]�v�ȕ��ׂ��~�������B
void CEditWnd::Timer_ONOFF( BOOL bStart )
{
	if( NULL != m_hWnd ){
		if( bStart ){
			/* �^�C�}�[���N�� */
			if( 0 == ::SetTimer( m_hWnd, IDT_TOOLBAR, 300, NULL ) ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("CEditWnd::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
			}
		} else {
			/* �^C�}�[���폜 */
			::KillTimer( m_hWnd, IDT_TOOLBAR );
		}
	}
	return;
}

/*!	@brief �E�B���h�E�ꗗ���|�b�v�A�b�v�\��

	@param[in] bMousePos true: �}�E�X�ʒu�Ƀ|�b�v�A�b�v�\������

	@date 2006.03.23 fon OnListBtnClick���x�[�X�ɐV�K�쐬
	@date 2006.05.10 ryoji �|�b�v�A�b�v�ʒu�ύX�A���̑����C��
	@date 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
	@date 2009.06.02 ryoji m_CMenuDrawer�̏������R��C��
*/
LRESULT CEditWnd::PopupWinList( bool bMousePos )
{
	POINT pt;

	// �|�b�v�A�b�v�ʒu���A�N�e�B�u�r���[�̏�ӂɐݒ�
	RECT rc;
	
	if( bMousePos ){
		::GetCursorPos( &pt );	// �}�E�X�J�[�\���ʒu�ɕύX
	}
	else {
		::GetWindowRect( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd, &rc );
		pt.x = rc.right - 150;
		if( pt.x < rc.left )
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// �E�B���h�E�ꗗ���j���[���|�b�v�A�b�v�\������
	if( NULL != m_cTabWnd.m_hWnd ){
		m_cTabWnd.TabListMenu( pt );
	}
	else{
		m_CMenuDrawer.ResetContents();	// 2009.06.02 ryoji �ǉ�
		EditNode*	pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();	// 2006.03.23 fon
		int nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
		WinListMenu( hMenu, pEditNodeArr, nRowNum, TRUE );
		// ���j���[��\������
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
		int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
									0, m_hWnd, NULL);
		delete [] pEditNodeArr;
		::DestroyMenu( hMenu );
		::SendMessage( m_hWnd, WM_COMMAND, (WPARAM)nId, (LPARAM)NULL );
	}

	return 0L;
}

/*! @brief ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� 
	@date  2006.03.23 fon CEditWnd::InitMenu����ړ��B////�������炠��R�����g�B//>�͒ǉ��R�����g�A�E�g�B
	@date 2009.06.02 ryoji �A�C�e�����������Ƃ��̓A�N�Z�X�L�[�� 1-9,A-Z �͈̔͂ōĎg�p����i�]����36����������j
*/
LRESULT CEditWnd::WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull )
{
	int			i;
	TCHAR		szMemu[280];
//>	EditNode*	pEditNodeArr;
	EditInfo*	pfi;

//>	int	nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
//>		/* �Z�p���[�^ */
//>		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		CShareData::getInstance()->TransformFileName_MakeCache();
		for( i = 0; i < nRowNum; ++i ){
			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
////	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
			TCHAR c = ((1 + i%35) <= 9)?(_T('1') + i%35):(_T('A') + i%35 - 9);	// 2009.06.02 ryoji �A�N�Z�X�L�[�� 1-9,A-Z �͈̔͂ōĎg�p
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
			if( pfi->m_bIsGrep ){
				/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
				CMemory		cmemDes;
				int			nDesLen;
				const char*	pszDes;
				LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
				pszDes = cmemDes.GetStringPtr();
				nDesLen = lstrlen( pszDes );
				wsprintf( szMemu, _T("&%c �yGrep�z\"%s%s\""),
					c,
					pszDes,
					( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? _T("�E�E�E"):_T("")
				);
			}
			else if( pEditNodeArr[i].m_hWnd == m_pShareData->m_hwndDebug ){
				wsprintf( szMemu, _T("&%c �A�E�g�v�b�g"), c );

			}
			else{
////		From Here Jan. 23, 2001 JEPRO
////		�t�@�C������p�X����'&'���g���Ă���Ƃ��ɗ��𓙂ŃL�`���ƕ\������Ȃ������C��(&��&&�ɒu�����邾��)
////<----- From Here Added
				TCHAR	szFile2[_MAX_PATH * 2];
				if( _T('\0') == pfi->m_szPath[0] ){
					_tcscpy( szFile2, _T("(����)") );
				}else{
					TCHAR buf[_MAX_PATH];
					CShareData::getInstance()->GetTransformFileNameFast( pfi->m_szPath, buf, _MAX_PATH );

					dupamp( buf, szFile2 );
				}
				wsprintf(
					szMemu,
					_T("&%c %s %s"),
					c,
					szFile2,
					pfi->m_bIsModified ? _T("*"):_T(" ")
				);
////-----> To Here Added
////		To Here Jan. 23, 2001

////	To Here Oct. 4, 2000
				// SJIS�ȊO�̕����R�[�h�̎�ʂ�\������
				// gm_pszCodeNameArr_3 ����R�s�[����悤�ɕύX
				if( IsValidCodeTypeExceptSJIS(pfi->m_nCharCode) ){
					_tcscat( szMemu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMemu );
			if( m_hWnd == pEditNodeArr[i].m_hWnd ){
				::CheckMenuItem( hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED );
			}
		}
//>		delete [] pEditNodeArr;
	}
	return 0L;
}


/*!
	@brief �}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j

	@note �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[���Ή��̂��߂ɒǉ��B
		  �y�[�W�X�N���[���E���X�N���[������t���O��OFF����B

	@date 2009.01.12 nasukoji	�V�K�쐬
*/
void CEditWnd::ClearMouseState( void )
{
	SetPageScrollByWheel( FALSE );		// �z�C�[������ɂ��y�[�W�X�N���[���L��
	SetHScrollByWheel( FALSE );			// �z�C�[������ɂ�鉡�X�N���[���L��
}


/*[EOF]*/
