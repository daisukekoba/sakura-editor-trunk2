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
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <process.h> // _beginthreadex
#include "CEditView.h"
#include "Debug.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CDlgCancel.h"
#include "etc_uty.h"
#include "os.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpe.h"///
#include "COpeBlk.h"///
#include "CDropTarget.h"///
#include "CSplitBoxWnd.h"///
#include "CRegexKeyword.h"///	//@@@ 2001.11.17 add MIK
#include "CMarkMgr.h"///
#include "COsVersionInfo.h"
#include "CFileLoad.h" // 2002/08/30 Moca
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include <vector> // 2008/02/16 bosagami add
#include <algorithm> // 2008/02/16 bosagami add
#include <assert.h>

const int STRNCMP_MAX = 100;	/* MAX�L�[���[�h���Fstrnicmp�������r�ő�l(CEditView::KeySearchCore) */	// 2006.04.10 fon

CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT_PTR, DWORD );

#define IDT_ROLLMOUSE	1

/* ���\�[�X�w�b�_�[ */
#define	 BFT_BITMAP		0x4d42	  /* 'BM' */

/* ���\�[�X��DIB���ǂ����𔻒f����}�N�� */
#define	 ISDIB(bft)		((bft) == BFT_BITMAP)

/* �w�肳�ꂽ�l���ł��߂��o�C�g���E�ɐ��񂳂���}�N�� */
#define	 WIDTHBYTES(i)	((i+31)/32*4)


//@@@2002.01.14 YAZAKI static�ɂ��ă������̐ߖ�i(10240+10) * 3 �o�C�g�j
int CEditView::m_pnDx[MAXLINEKETAS + 10];



/*
|| �E�B���h�E�v���V�[�W��
||
*/

LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
)
{
	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessage( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
			}
			else{
				return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
			//	To Here
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


/*
||  �^�C�}�[���b�Z�[�W�̃R�[���o�b�N�֐�
||
||	���݂́A�}�E�X�ɂ��̈�I�����̃X�N���[�������̂��߂Ƀ^�C�}�[���g�p���Ă��܂��B
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}



//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditView::CEditView()
: m_cHistory( new CAutoMarkMgr )
, m_bActivateByMouse( FALSE )	// 2007.10.02 nasukoji
, m_cRegexKeyword( NULL )				// 2007.04.08 ryoji
{
}


// 2007.10.23 kobake �R���X�g���N�^���̏��������ׂ�Create�Ɉڂ��܂����B(�������������s�K�v�ɕ��U���Ă�������)
BOOL CEditView::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,		//!< �e
	CEditDoc*	pcEditDoc,		//!< �Q�Ƃ���h�L�������g
	int			nMyIndex,		//!< �r���[�̃C���f�b�N�X
	BOOL		bShow			//!< �쐬���ɕ\�����邩�ǂ���
)
{
	m_pcViewFont = pcEditDoc->m_pcEditWnd->m_pcViewFont;

	m_bDrawSWITCH = true;
	m_pcDropTarget = new CDropTarget( this );
	m_bDragMode = FALSE;					/* �I���e�L�X�g�̃h���b�O���� */
	m_bCurSrchKeyMark = false;				/* ���������� */
	//	Jun. 27, 2001 genta
	m_szCurSrchKey[0] = '\0';
	m_sCurSearchOption.Reset();				// �����^�u�� �I�v�V����

	m_bPrevCommand = 0;
	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��CEditWnd�ֈڊ�

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_bCommandRunning = FALSE;	/* �R�}���h�̎��s�� */
	m_pcOpeBlk = NULL;			/* ����u���b�N */
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */
	m_pcsbwVSplitBox = NULL;	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = NULL;	/* ���������{�b�N�X */
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* �����X�N���[���o�[�̏k�� */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;
	m_ptCaretPos.x = 0;			/* �r���[���[����̃J�[�\�����ʒu(�O�I���W��) */
	m_nCaretPosX_Prev = 0;		/* �r���[���[����̃J�[�\�������O�̈ʒu(�O�I���W��) */
	m_ptCaretPos.y = 0;			/* �r���[��[����̃J�[�\���s�ʒu(�O�I���W��) */

	m_ptCaretPos_PHY.x = 0;		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g��(�O�J�n) */
	m_ptCaretPos_PHY.y = 0;		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ�(�O�J�n) */

	m_ptSrchStartPos_PHY.x = -1;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/06/26 ai
	m_ptSrchStartPos_PHY.y = -1;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ�(0�J�n) */				// 02/06/26 ai
	m_bSearch = FALSE;			/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	m_ptBracketPairPos_PHY.x = -1;/* �Ί��ʂ̈ʒu ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/12/13 ai
	m_ptBracketPairPos_PHY.y = -1;/* �Ί��ʂ̈ʒu ���s�P�ʍs�̍s�ԍ�(0�J�n) */			// 02/12/13 ai
	m_ptBracketCaretPos_PHY.x = -1;	/* 03/02/18 ai */
	m_ptBracketCaretPos_PHY.y = -1;	/* 03/02/18 ai */
	m_bDrawBracketPairFlag = FALSE;	/* 03/02/18 ai */
	m_bDrawSelectArea = false;	/* �I��͈͂�`�悵���� */	// 02/12/13 ai

	m_nCaretWidth = 0;			/* �L�����b�g�̕� */
	m_nCaretHeight = 0;			/* �L�����b�g�̍��� */
	m_crCaret = -1;				/* �L�����b�g�̐F */			// 2006.12.16 ryoji
	m_crBack = -1;				/* �e�L�X�g�̔w�i�F */			// 2006.12.16 ryoji
	m_hbmpCaret = NULL;			/* �L�����b�g�p�r�b�g�}�b�v */	// 2006.11.28 ryoji

	m_bSelectingLock = false;	/* �I����Ԃ̃��b�N */
	m_bBeginSelect = false;		/* �͈͑I�� */
	m_bBeginBoxSelect = false;	/* ��`�͈͑I�� */
	m_bBeginLineSelect = false;	/* �s�P�ʑI�� */
	m_bBeginWordSelect = false;	/* �P��P�ʑI�� */

	m_sSelectBgn.m_ptFrom.y = -1;	/* �͈͑I���J�n�s(���_) */
	m_sSelectBgn.m_ptFrom.x = -1;	/* �͈͑I���J�n��(���_) */
	m_sSelectBgn.m_ptTo.y = -1;	/* �͈͑I���J�n�s(���_) */
	m_sSelectBgn.m_ptTo.x = -1;	/* �͈͑I���J�n��(���_) */

	m_sSelect.m_ptFrom.y = -1;		/* �͈͑I���J�n�s */
	m_sSelect.m_ptFrom.x = -1;		/* �͈͑I���J�n�� */
	m_sSelect.m_ptTo.y = -1;		/* �͈͑I���I���s */
	m_sSelect.m_ptTo.x = -1;		/* �͈͑I���I���� */

	m_sSelectOld.m_ptFrom.y = 0;	/* �͈͑I���J�n�s */
	m_sSelectOld.m_ptFrom.x = 0;	/* �͈͑I���J�n�� */
	m_sSelectOld.m_ptTo.y = 0;		/* �͈͑I���I���s */
	m_sSelectOld.m_ptTo.x = 0;		/* �͈͑I���I���� */
	m_nViewAlignLeft = 0;		/* �\����̍��[���W */
	m_nViewAlignLeftCols = 0;	/* �s�ԍ���̌��� */
	m_nTopYohaku = m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace; 	/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;		/* �\����̏�[���W */

	/* ���[���[�\�� */
	m_nViewAlignTop += m_pShareData->m_Common.m_sWindow.m_nRulerHeight;	/* ���[���[���� */
	m_nOldCaretPosX = 0;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	m_nOldCaretWidth = 0;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
	m_bRedrawRuler = true;	// ���[���[�S�̂�`��������=true   2002.02.25 Add By KK
	m_nViewCx = 0;				/* �\����̕� */
	m_nViewCy = 0;				/* �\����̍��� */
	m_nViewColNum = 0;			/* �\����̌��� */
	m_nViewRowNum = 0;			/* �\����̍s�� */
	m_nViewTopLine = 0;			/* �\����̈�ԏ�̍s */
	m_nViewLeftCol = 0;			/* �\����̈�ԍ��̌� */
	m_hdcCompatDC = NULL;		/* �ĕ`��p�R���p�`�u���c�b */
	m_hbmpCompatBMP = NULL;		/* �ĕ`��p�������a�l�o */
	m_hbmpCompatBMPOld = NULL;	/* �ĕ`��p�������a�l�o(OLD) */
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	m_nCompatBMPWidth = -1;
	m_nCompatBMPHeight = -1;
	// To Here 2007.09.09 Moca
	m_nCharWidth = 10;			/* ���p�����̕� */
	m_nCharHeight = 18;			/* �����̍��� */

	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	2007.08.12 genta ��������ShareData�̒l���K�v�ɂȂ���
	m_CurRegexp.Init(m_pShareData->m_Common.m_sSearch.m_szRegexpLib );

	// 2004.02.08 m_hFont_ZEN�͖��g�p�ɂ��폜
	m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	m_bInMenuLoop = FALSE;				/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
//	MYTRACE( _T("CEditView::CEditView()�����\n") );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	// from here  2002.04.09 minfu OS�ɂ���čĕϊ��̕�����ς���
	//	YAZAKI COsVersionInfo�̃J�v�Z�����͎��܂���B
	if( !OsSupportReconvert() ){
		// 95 or NT�Ȃ��
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage( RWM_RECONVERT );
		m_uATOKReconvertMsg = ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		m_hAtokModule = LoadLibraryExedir(_T("ATOK10WC.DLL"));
		m_AT_ImmSetReconvertString = NULL;
		if ( NULL != m_hAtokModule ) {
			m_AT_ImmSetReconvertString =(BOOL (WINAPI *)( HIMC , int ,PRECONVERTSTRING , DWORD  ) ) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}
	else{ 
		// ����ȊO��OS�̂Ƃ���OS�W�����g�p����
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
	//2004.10.23 isearch
	m_nISearchMode = 0;
	m_pcmigemo = NULL;

	// 2007.10.02 nasukoji
	m_dwTripleClickCheck = 0;		// �g���v���N���b�N�`�F�b�N�p����������



	//�����܂ŃR���X�g���N�^�ł���Ă�����
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�����܂�Create�ł���Ă�����

	WNDCLASS	wc;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;
	
	m_dwTipTimer = ::GetTickCount();

	//	2007.08.18 genta ��������ShareData�̒l���K�v�ɂȂ���
	m_cRegexKeyword = new CRegexKeyword( m_pShareData->m_Common.m_sSearch.m_szRegexpLib );	//@@@ 2001.11.17 add MIK
	m_cRegexKeyword->RegexKeySetTypes(&(m_pcEditDoc->GetDocumentAttribute()));	//@@@ 2001.11.17 add MIK

	m_nTopYohaku = m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace; 	/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;								/* �\����̏�[���W */
	/* ���[���[�\�� */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_sWindow.m_nRulerHeight;	/* ���[���[���� */
	}


	/* �E�B���h�E�N���X�̓o�^ */
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG_PTR );
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_IBEAM )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= GSTR_VIEWNAME;
	if( 0 == ::RegisterClass( &wc ) ){
	}

	/* �G�f�B�^�E�B���h�E�̍쐬 */
	g_m_pcEditView = this;
	m_hWnd = ::CreateWindowEx(
		WS_EX_STATICEDGE,	// extended window style
		GSTR_VIEWNAME,			// pointer to registered class name
		GSTR_VIEWNAME,			// pointer to window name
		0						// window style
		| WS_VISIBLE
		| WS_CHILD
		| WS_CLIPCHILDREN
		, 
		CW_USEDEFAULT,			// horizontal position of window
		0,						// vertical position of window
		CW_USEDEFAULT,			// window width
		0,						// window height
		hwndParent,				// handle to parent or owner window
		NULL,					// handle to menu or child-window identifier
		m_hInstance,			// handle to application instance
		(LPVOID)this			// pointer to window-creation data
	);
	if( NULL == m_hWnd ){
		return FALSE;
	}

	m_pcDropTarget->Register_DropTarget( m_hWnd );

	/* ����Tip�\���E�B���h�E�쐬 */
	m_cTipWnd.Create( m_hInstance, m_hWnd/*m_pShareData->m_hwndTray*/ );

	/* �ĕ`��p�R���p�`�u���c�b */
	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// 2007.09.30 genta �֐���
	UseCompatibleDC( m_pShareData->m_Common.m_sWindow.m_bUseCompatibleBMP );

	/* ���������{�b�N�X */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );

	/* �X�N���[���o�[�쐬 */
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();

	if( bShow ){
		ShowWindow( m_hWnd, SW_SHOW );
	}

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditDoc->UpdateCaption();

	/* �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );

	/* �^�C�}�[�N�� */
	if( 0 == ::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, EditViewTimerProc ) ){
		WarningMessage( m_hWnd, _T("CEditView::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
	}

	/* �A���_�[���C�� */
	m_cUnderLine.SetView( this );
	return TRUE;
}


CEditView::~CEditView()
{
	// �L�����b�g�p�r�b�g�}�b�v	// 2006.11.28 ryoji
	if( m_hbmpCaret != NULL )
		DeleteObject( m_hbmpCaret );

	if( m_hWnd != NULL ){
		::DestroyWindow( m_hWnd );
	}

	/* �ĕ`��p�R���p�`�u���c�b */
	//	2007.09.30 genta �֐���
	//	m_hbmpCompatBMP�������ō폜�����D
	UseCompatibleDC(FALSE);

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;

	delete m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	
	//�ĕϊ� 2002.04.10 minfu
	if(m_hAtokModule)
		FreeLibrary(m_hAtokModule);
}

/** ��ʃL���b�V���pCompatibleDC��p�ӂ���

	@param[in] TRUE: ��ʃL���b�V��ON

	@date 2007.09.30 genta �֐���
*/
void CEditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( fCache ){
		if( m_hdcCompatDC == NULL ){
			HDC			hdc;
			hdc = ::GetDC( m_hWnd );
			m_hdcCompatDC = ::CreateCompatibleDC( hdc );
			::ReleaseDC( m_hWnd, hdc );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Created\n"), fCache);
		}
		else {
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}
	else {
		//	CompatibleBitmap���c���Ă��邩������Ȃ��̂ōŏ��ɍ폜
		DeleteCompatibleBitmap();
		if( m_hdcCompatDC != NULL ){
			::DeleteDC( m_hdcCompatDC );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Deleted.\n"));
			m_hdcCompatDC = NULL;
		}
	}
}

/*! �X�N���[���o�[�쐬
	@date 2006.12.19 ryoji �V�K�쐬�iCEditView::Create���番���j
*/
BOOL CEditView::CreateScrollBar()
{
	SCROLLINFO	si;

	/* �X�N���[���o�[�̍쐬 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		_T("SCROLLBAR"),					/* scroll bar control class */
		(LPSTR) NULL,						/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		m_hWnd,								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		m_hInstance,						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* �X�N���[���o�[�̍쐬 */
	m_hwndHScrollBar = NULL;
	if( m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz ){	/* �����X�N���[���o�[���g�� */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			_T("SCROLLBAR"),					/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd,								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* �T�C�Y�{�b�N�X */
	if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			_T("SCROLLBAR"),					/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			_T("STATIC"),						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}
	return TRUE;
}



/*! �X�N���[���o�[�j��
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditView::DestroyScrollBar()
{
	if( m_hwndVScrollBar )
	{
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}

	if( m_hwndHScrollBar )
	{
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}

	if( m_hwndSizeBox )
	{
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}

/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT CEditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	HDC			hdc;
	int			nPosX;
	int			nPosY;

	switch ( uMsg ){
	case WM_MOUSEWHEEL:
		if( m_pcEditDoc->m_pcEditWnd->DoMouseWheel( wParam, lParam ) ){
			return 0L;
		}
		return OnMOUSEWHEEL( wParam, lParam );

	case WM_CREATE:
		::SetWindowLongPtr( hwnd, 0, (LONG_PTR) this );

		return 0L;

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		// �E�B���h�E��\���̍ĂɌ݊�BMP��p�����ă������[��ߖ񂷂�
		if( hwnd == m_hWnd && (BOOL)wParam == FALSE ){
			DeleteCompatibleBitmap();
		}
		return 0L;
	// To Here 2007.09.09 Moca

	case WM_SIZE:
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;

	case WM_SETFOCUS:
		OnSetFocus();

		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_pcEditDoc->UpdateCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

		// 2009.01.12 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
		m_pcEditDoc->m_pcEditWnd->ClearMouseState();

		return 0L;
	case WM_CHAR:
		HandleCommand( F_CHAR, true, wParam, 0, 0, 0 );
		return 0L;

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_IME_COMPOSITION:
		if( IsInsMode() && (lParam & GCS_RESULTSTR)){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//	maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			LPTSTR lptstr;
			lptstr = (LPTSTR)GlobalLock( hstr );
			if( lptstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lptstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lptstr, dwSize);

			/* �e�L�X�g��\��t�� */
			HandleCommand( F_INSTEXT, true, (LPARAM)lptstr, TRUE, 0, 0 );

			ImmReleaseContext( hwnd, hIMC );

			// add this string into text buffer of application

			GlobalUnlock( hstr );
			GlobalFree( hstr );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_IME_CHAR:
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){ /* �㏑�����[�h���H */
			HandleCommand( F_IME_CHAR, true, wParam, 0, 0, 0 );
		}
		return 0L;

	// From Here 2008.03.24 Moca ATOK���̗v���ɂ�������
	case WM_PASTE:
		return HandleCommand( F_PASTE, true, 0, 0, 0, 0 );

	case WM_COPY:
		return HandleCommand( F_COPY, true, 0, 0, 0, 0 );
	// To Here 2008.03.24 Moca

	case WM_KEYUP:
		/* �L�[���s�[�g��� */
		m_bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+x��ALT���������܂܂��ƃL�[���s�[�g��OFF�ɂȂ�Ȃ��΍�
	case WM_SYSKEYUP:
		m_bPrevCommand = 0;
		// �O�̂��ߌĂ�
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	// 2004.04.27 To Here

	case WM_LBUTTONDBLCLK:

		// 2007.10.02 nasukoji	��A�N�e�B�u�E�B���h�E�̃_�u���N���b�N���͂����ŃJ�[�\�����ړ�����
		// 2007.10.12 genta �t�H�[�J�X�ړ��̂��߁COnLBUTTONDBLCLK���ړ�
		if(m_bActivateByMouse){
			/* �A�N�e�B�u�ȃy�C����ݒ� */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
			// �J�[�\�����N���b�N�ʒu�ֈړ�����
			OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );	
			// 2007.10.02 nasukoji
			m_bActivateByMouse = FALSE;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
		}
		//		MYTRACE( _T(" WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/11 �}�E�X���{�^���Ή�
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

		return 0L;

	case WM_MBUTTONUP:
		// 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
		OnMBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		m_bActivateByMouse = FALSE;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
//		MYTRACE( _T(" WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_LBUTTONUP:

//		MYTRACE( _T(" WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
//		MYTRACE( _T(" WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE( _T(" WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
//			/* �A�N�e�B�u�ȃy�C����ݒ� */
//			m_pcEditDoc->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE( _T(" WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
	case WM_XBUTTONDOWN:
		switch ( HIWORD(wParam) ){
		case XBUTTON1:
			OnXLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		case XBUTTON2:
			OnXRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		}

		return TRUE;

	case WM_XBUTTONUP:
		// 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
		switch ( HIWORD(wParam) ){
		case XBUTTON1:
			OnXLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		case XBUTTON2:
			OnXRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		}

		return TRUE;

	case WM_VSCROLL:
//		MYTRACE( _T("	WM_VSCROLL nPos=%d\n"), GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			int Scroll = OnVScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) * m_nVScrollRate );

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if(!GetKeyState_Shift()){
				SyncScrollV( Scroll );
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE( _T("	WM_HSCROLL nPos=%d\n"), GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			int Scroll = OnHScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) );

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if(!GetKeyState_Shift()){
				SyncScrollH( Scroll );
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */

		/* ����Tip���N������Ă��� */
		if( 0 == m_dwTipTimer ){
			/* ����Tip������ */
			m_cTipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
		}
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
		return 0L;


	case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			hdc = ::BeginPaint( hwnd, &ps );
			OnPaint( hdc, &ps, FALSE );
			::EndPaint(hwnd, &ps);
		}
		return 0L;

	case WM_CLOSE:
//		MYTRACE( _T("	WM_CLOSE\n") );
		::DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
		m_pcDropTarget->Revoke_DropTarget();

		/* �^�C�}�[�I�� */
		::KillTimer( m_hWnd, IDT_ROLLMOUSE );


//		MYTRACE( _T("	WM_DESTROY\n") );
		/*
		||�q�E�B���h�E�̔j��
		*/
		if( NULL != m_hwndVScrollBar ){	// Aug. 20, 2005 Aroka
			::DestroyWindow( m_hwndVScrollBar );
			m_hwndVScrollBar = NULL;
		}
		if( NULL != m_hwndHScrollBar ){
			::DestroyWindow( m_hwndHScrollBar );
			m_hwndHScrollBar = NULL;
		}
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		delete m_pcsbwVSplitBox;	/* ���������{�b�N�X */
		m_pcsbwVSplitBox = NULL;
		delete m_pcsbwHSplitBox;	/* ���������{�b�N�X */
		m_pcsbwHSplitBox = NULL;

		m_hWnd = NULL;
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( _T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY );
		::SendMessage( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->SetActivePane( m_nMyIndex );
		::PostMessage( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;

	case MYWM_IME_REQUEST:  /* �ĕϊ�  by minfu 2002.03.27 */ // 20020331 aroka
		
		// 2002.04.09 switch case �ɕύX  minfu 
		switch ( wParam ){
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, false);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, false);
			
		}
		
		return 0L;

	case MYWM_DROPFILES:	// �Ǝ��̃h���b�v�t�@�C���ʒm	// 2008.06.20 ryoji
		OnMyDropFiles( (HDROP)wParam );
		return 0L;

	// 2007.10.02 nasukoji	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ�
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc( hwnd, uMsg, wParam, lParam );	// �e�ɐ�ɏ���������
		if( nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT ){
			return nRes;
		}

		// �}�E�X�N���b�N�ɂ��o�b�N�O���E���h�E�B���h�E���A�N�e�B�x�[�g���ꂽ
		//	2007.10.08 genta �I�v�V�����ǉ�
		if( m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation &&
		   (! m_pcEditDoc->m_pcEditWnd->IsActiveApp()))
		{
			m_bActivateByMouse = TRUE;		// �}�E�X�ɂ��A�N�e�B�x�[�g
			return MA_ACTIVATEANDEAT;		// �A�N�e�B�x�[�g��C�x���g��j��
		}

		/* �A�N�e�B�u�ȃy�C����ݒ� */
		if( ::GetFocus() != m_hWnd ){
			POINT ptCursor;
			::GetCursorPos( &ptCursor );
			HWND hwndCursorPos = ::WindowFromPoint( ptCursor );
			if( hwndCursorPos == m_hWnd ){
				// �r���[��Ƀ}�E�X������̂� SetActivePane() �𒼐ڌĂяo��
				// �i�ʂ̃}�E�X���b�Z�[�W���͂��O�ɃA�N�e�B�u�y�C����ݒ肵�Ă����j
				m_pcEditDoc->SetActivePane( m_nMyIndex );
			}else if( (m_pcsbwVSplitBox && hwndCursorPos == m_pcsbwVSplitBox->m_hWnd)
						|| (m_pcsbwHSplitBox && hwndCursorPos == m_pcsbwHSplitBox->m_hWnd) ){
				// 2010.01.19 ryoji
				// �����{�b�N�X��Ƀ}�E�X������Ƃ��̓A�N�e�B�u�y�C����؂�ւ��Ȃ�
				// �i������ MYWM_SETACTIVEPANE �̃|�X�g�ɂ�蕪�����̃S�~���c���Ă��������C���j
			}else{
				// 2008.05.24 ryoji
				// �X�N���[���o�[��Ƀ}�E�X�����邩������Ȃ��̂� MYWM_SETACTIVEPANE ���|�X�g����
				// SetActivePane() �ɂ̓X�N���[���o�[�̃X�N���[���͈͒����������܂܂�Ă��邪�A
				// ���̃^�C�~���O�iWM_MOUSEACTIVATE�j�ŃX�N���[���͈͂�ύX����̂͂܂����B
				// �Ⴆ�� Win XP/Vista ���ƃX�N���[���͈͂��������Ȃ��ăX�N���[���o�[���L������
				// �����ɐ؂�ւ��Ƃ���Ȍ�X�N���[���o�[���@�\���Ȃ��Ȃ�B
				::PostMessage( m_hWnd, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
			}
		}

		return nRes;
	
	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))){
		// 2002.04.08 switch case �ɕύX minfu 
			switch ( wParam ){
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
				
			}
			return 0L;
		}
// >> by aroka

		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( m_hWnd, x, y, nWidth, nHeight, TRUE );
	return;
}


/* �E�B���h�E�T�C�Y�̕ύX���� */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == m_hWnd 
		|| ( cx == 0 && cy == 0 ) ){
		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		// �E�B���h�E�������ɂ��݊�BMP��j������
		DeleteCompatibleBitmap();
		// To Here 2007.09.09 Moca
		return;
	}

	int nVSplitHeight = 0;	/* ���������{�b�N�X�̍��� */
	int nHSplitWidth = 0;	/* ���������{�b�N�X�̕� */

	//�X�N���[���o�[�̃T�C�Y��l���擾
	int nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	/* ���������{�b�N�X */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->m_hWnd, cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* ���������{�b�N�X */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->m_hWnd,0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE );
	}

	/* �T�C�Y�{�b�N�X */
	if( NULL != m_hwndSizeBox ){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}

	m_nViewCx = cx - nCxVScroll - m_nViewAlignLeft;														/* �\����̕� */
	m_nViewCy = cy - ((NULL != m_hwndHScrollBar)?nCyHScroll:0) - m_nViewAlignTop;						/* �\����̍��� */
	m_nViewColNum = (m_nViewCx - 1) / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* �\����̌��� */
	m_nViewRowNum = (m_nViewCy - 1) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* �\����̍s�� */

	// 2008.06.06 nasukoji	�T�C�Y�ύX���̐܂�Ԃ��ʒu�Čv�Z
	BOOL wrapChanged = FALSE;
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH ){
		if( m_nMyIndex == 0 ){	// ������̃r���[�̃T�C�Y�ύX���̂ݏ�������
			// �E�[�Ő܂�Ԃ����[�h�Ȃ�E�[�Ő܂�Ԃ�	// 2008.06.08 ryoji
			wrapChanged = m_pcEditDoc->WrapWindowWidth( 0 );
		}
	}

	if( !wrapChanged )	// �܂�Ԃ��ʒu���ύX����Ă��Ȃ�
		AdjustScrollBars();				// �X�N���[���o�[�̏�Ԃ��X�V����

	/* �ĕ`��p�������a�l�o */
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( m_hdcCompatDC != NULL ){
		CreateOrUpdateCompatibleBitmap( cx, cy );
 	}
	// To Here 2007.09.09 Moca

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditDoc->UpdateCaption();	//	[Q] genta �{���ɕK�v�H

	return;
}

/*!
	��ʂ̌݊��r�b�g�}�b�v���쐬�܂��͍X�V����B
		�K�v�̖����Ƃ��͉������Ȃ��B
	
	@param cx �E�B���h�E�̍���
	@param cy �E�B���h�E�̕�
	@return true: �r�b�g�}�b�v�𗘗p�\ / false: �r�b�g�}�b�v�̍쐬�E�X�V�Ɏ��s

	@date 2007.09.09 Moca CEditView::OnSize���番���B
		�P���ɐ������邾�����������̂��A�d�l�ύX�ɏ]�����e�R�s�[��ǉ��B
		�T�C�Y�������Ƃ��͉������Ȃ��悤�ɕύX

	@par �݊�BMP�ɂ̓L�����b�g�E�J�[�\���ʒu���c���E�Ί��ʈȊO�̏���S�ď������ށB
		�I��͈͕ύX���̔��]�����́A��ʂƌ݊�BMP�̗�����ʁX�ɕύX����B
		�J�[�\���ʒu���c���ύX���ɂ́A�݊�BMP�����ʂɌ��̏��𕜋A�����Ă���B

*/
bool CEditView::CreateOrUpdateCompatibleBitmap( int cx, int cy )
{
	if( NULL == m_hdcCompatDC ){
		return false;
	}
	// �T�C�Y��64�̔{���Ő���
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if( nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight ){
		DEBUG_TRACE( _T("CEditView::CreateOrUpdateCompatibleBitmap( %d, %d ): resized\n"), cx, cy );
		HDC	hdc = ::GetDC( m_hWnd );
		HBITMAP hBitmapNew = NULL;
		if( m_hbmpCompatBMP ){
			// BMP�̍X�V
			HDC hdcTemp = ::CreateCompatibleDC( hdc );
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
			if( hBitmapNew ){
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject( hdcTemp, hBitmapNew );
				// �O�̉�ʓ��e���R�s�[����
				::BitBlt( hdcTemp, 0, 0,
					t_min( nBmpWidthNew,m_nCompatBMPWidth ),
					t_min( nBmpHeightNew, m_nCompatBMPHeight ),
					m_hdcCompatDC, 0, 0, SRCCOPY );
				::SelectObject( hdcTemp, hBitmapOld );
				::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
				::DeleteObject( m_hbmpCompatBMP );
			}
			::DeleteDC( hdcTemp );
		}else{
			// BMP�̐V�K�쐬
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
		}
		if( hBitmapNew ){
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
		}else{
			// �݊�BMP�̍쐬�Ɏ��s
			// ��������s���J��Ԃ��\���������̂�
			// m_hdcCompatDC��NULL�ɂ��邱�Ƃŉ�ʃo�b�t�@�@�\�����̃E�B���h�E�̂ݖ����ɂ���B
			//	2007.09.29 genta �֐����D������BMP�����
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC( m_hWnd, hdc );
	}
	return NULL != m_hbmpCompatBMP;
}


/*!
	�݊�������BMP���폜

	@note �����r���[����\���ɂȂ����ꍇ��
		�e�E�B���h�E����\���E�ŏ������ꂽ�ꍇ�ɍ폜�����B
	@date 2007.09.09 Moca �V�K�쐬 
*/
void CEditView::DeleteCompatibleBitmap()
{
	if( m_hbmpCompatBMP ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}



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


/*!	�L�����b�g�̍쐬

	@param nCaretColor [in]	�L�����b�g�̐F��� (0:�ʏ�, 1:IME ON)
	@param nWidth [in]		�L�����b�g��
	@param nHeight [in]		�L�����b�g��

	@date 2006.12.07 ryoji �V�K�쐬
*/
void CEditView::CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight )
{
	//
	// �L�����b�g�p�̃r�b�g�}�b�v���쐬����
	//
	// Note: �E�B���h�E�݊��̃����� DC ��� PatBlt ��p���ăL�����b�g�F�Ɣw�i�F�� XOR ����
	//       ���邱�ƂŁC�ړI�̃r�b�g�}�b�v�𓾂�D
	//       �� 256 �F���ł� RGB �l��P���ɒ��ډ��Z���Ă��L�����b�g�F���o�����߂̐�����
	//          �r�b�g�}�b�v�F�͓����Ȃ��D
	//       �Q�l: [HOWTO] �L�����b�g�̐F�𐧌䂷����@
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// �L�����b�g�p�̃r�b�g�}�b�v

	HDC hdc = ::GetDC( m_hWnd );

	hbmpCaret = ::CreateCompatibleBitmap( hdc, nWidth, nHeight );
	HDC hdcMem = ::CreateCompatibleDC( hdc );
	HBITMAP hbmpOld = (HBITMAP)::SelectObject( hdcMem, hbmpCaret );
	HBRUSH hbrCaret = ::CreateSolidBrush( crCaret );
	HBRUSH hbrBack = ::CreateSolidBrush( crBack );
	HBRUSH hbrOld = (HBRUSH)::SelectObject( hdcMem, hbrCaret );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATCOPY );
	::SelectObject( hdcMem, hbrBack );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATINVERT );
	::SelectObject( hdcMem, hbrOld );
	::SelectObject( hdcMem, hbmpOld );
	::DeleteObject( hbrCaret );
	::DeleteObject( hbrBack );
	::DeleteDC( hdcMem );

	::ReleaseDC( m_hWnd, hdc );

	// �ȑO�̃r�b�g�}�b�v��j������
	if( m_hbmpCaret != NULL )
		::DeleteObject( m_hbmpCaret );
	m_hbmpCaret = hbmpCaret;

	// �L�����b�g���쐬����
	::CreateCaret( m_hWnd, hbmpCaret, nWidth, nHeight );
	return;
}


// 2002/07/22 novice
/*!
	�L�����b�g�̕\��
*/
void CEditView::ShowCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == false ){
		::ShowCaret( hwnd );
		m_bCaretShowFlag = true;
	}
}


/*!
	�L�����b�g�̔�\��
*/
void CEditView::HideCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == true ){
		::HideCaret( hwnd );
		m_bCaretShowFlag = false;
	}
}

/* �L�����b�g�̕\���E�X�V */
void CEditView::ShowEditCaret( void )
{
	const char*		pLine;
	int				nLineLen;
	int				nCaretWidth = 0;
	int				nCaretHeight = 0;
	int				nIdxFrom;
	int				nCharChars;

/*
	�t�H�[�J�X�������Ƃ��ɓ����I�ɃL�����b�g�쐬����ƈÖٓI�ɃL�����b�g�j���i���j����Ă�
	�L�����b�g������im_nCaretWidth != 0�j�Ƃ������ƂɂȂ��Ă��܂��A�t�H�[�J�X���擾���Ă�
	�L�����b�g���o�Ă��Ȃ��Ȃ�ꍇ������
	�t�H�[�J�X�������Ƃ��̓L�����b�g���쐬�^�\�����Ȃ��悤�ɂ���

	���L�����b�g�̓X���b�h�ɂЂƂ����Ȃ̂ŗႦ�΃G�f�B�b�g�{�b�N�X���t�H�[�J�X�擾�����
	�@�ʌ`��̃L�����b�g�ɈÖٓI�ɍ����ւ����邵�t�H�[�J�X�������ΈÖٓI�ɔj�������

	2007.12.11 ryoji
	�h���b�O�A���h�h���b�v�ҏW���̓L�����b�g���K�v�ňÖٔj���̗v���������̂ŗ�O�I�ɕ\������
*/
	if( ::GetFocus() != m_hWnd && !m_bDragMode ){
		m_nCaretWidth = 0;
		return;
	}

	/* �L�����b�g�̕��A���������� */
	if( 0 == m_pShareData->m_Common.m_sGeneral.GetCaretType() ){	/* �J�[�\���̃^�C�v 0=win 1=dos */
		nCaretHeight = m_nCharHeight;					/* �L�����b�g�̍��� */
		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretWidth = 2;
		}else{
			const CLayout* pcLayout;
			nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				}else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nIdxFrom );
					if( 0 < nCharChars ){
						nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
					}
				}
			}
		}
	}else
	if( 1 == m_pShareData->m_Common.m_sGeneral.GetCaretType() ){	/* �J�[�\���̃^�C�v 0=win 1=dos */
		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretHeight = m_nCharHeight / 2;			/* �L�����b�g�̍��� */
		}else{
			nCaretHeight = m_nCharHeight;				/* �L�����b�g�̍��� */
		}
		const CLayout* pcLayout;
		nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nIdxFrom );
				if( 0 < nCharChars ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
				}
			}
		}

	}

#if 0
	hdc = ::GetDC( m_hWnd );
#endif
	//	�L�����b�g�F�̎擾
	ColorInfo* ColorInfoArr = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr;
	int nCaretColor = ( ColorInfoArr[COLORIDX_CARET_IME].m_bDisp && IsImeON() )? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = ColorInfoArr[nCaretColor].m_colTEXT;
	COLORREF crBack = ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	if( m_nCaretWidth == 0 ){
		/* �L�����b�g���Ȃ������ꍇ */
		/* �L�����b�g�̍쐬 */
		CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}else{
		if( m_nCaretWidth != nCaretWidth || m_nCaretHeight != nCaretHeight ||
			m_crCaret != crCaret || m_crBack != crBack ){
			/* �L�����b�g�͂��邪�A�傫����F���ς�����ꍇ */
			/* ���݂̃L�����b�g���폜 */
			::DestroyCaret();

			/* �L�����b�g�̍쐬 */
			CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}else{
			/* �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ */
			/* �L�����b�g���B�� */
			HideCaret_( m_hWnd ); // 2002/07/22 novice
		}
	}
	/* �L�����b�g�̈ʒu�𒲐� */
	int nPosX = m_nViewAlignLeft + (m_ptCaretPos.x - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nPosY = m_nViewAlignTop  + (m_ptCaretPos.y - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight;
	::SetCaretPos( nPosX, nPosY );
	if ( m_nViewAlignLeft <= nPosX && m_nViewAlignTop <= nPosY ){
		/* �L�����b�g�̕\�� */
		ShowCaret_( m_hWnd ); // 2002/07/22 novice
	}

	m_nCaretWidth = nCaretWidth;
	m_nCaretHeight = nCaretHeight;	/* �L�����b�g�̍��� */
	m_crCaret = crCaret;	//	2006.12.07 ryoji
	m_crBack = crBack;		//	2006.12.07 ryoji
	SetIMECompFormPos();
}





/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
void CEditView::OnSetFocus( void )
{
	// 2004.04.02 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	{
		int nPosX = m_ptCaretPos.x;
		int nPosY = m_ptCaretPos.y;
		if( GetAdjustCursorPos( &nPosX, &nPosY ) ){
			MoveCursor( nPosX, nPosY, false );
			m_nCaretPosX_Prev = m_ptCaretPos.x;
		}
	}

	ShowEditCaret();

//	SetIMECompFormPos();	YAZAKI ShowEditCaret�ō�ƍς�
	SetIMECompFormFont();

	/* ���[���̃J�[�\�����O���[���獕�ɕύX���� */
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	// 03/02/18 �Ί��ʂ̋����\��(�`��) ai
	m_bDrawBracketPairFlag = TRUE;
	DrawBracketPair( true );
}


/* ���̓t�H�[�J�X���������Ƃ��̏��� */
void CEditView::OnKillFocus( void )
{
	// 03/02/18 �Ί��ʂ̋����\��(����) ai
	DrawBracketPair( false );
	m_bDrawBracketPairFlag = FALSE;

	DestroyCaret();

	/* ���[���[�`�� */
	/* ���[���̃J�[�\����������O���[�ɕύX���� */
	HDC	hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	return;
}





/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[�������s��

	@date 2004.09.11 genta �X�N���[���s����Ԃ��悤�ɁD
		���g�p��hwndScrollBar�����폜�D
*/
int CEditView::OnVScroll( int nScrollCode, int nPos )
{
	int nScrollVal = 0;

	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine + 1 );
//		}
		nScrollVal = ScrollAtV( m_nViewTopLine + m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine - 1 );
//		}
		nScrollVal = ScrollAtV( m_nViewTopLine - m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV( m_nViewTopLine + m_nViewRowNum );
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV( m_nViewTopLine - m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV( nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV( nPos );
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV( 0 );
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - m_nViewRowNum );
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[����������

	@date 2004.09.11 genta �X�N���[��������Ԃ��悤�ɁD
		���g�p��hwndScrollBar�����폜�D
*/
int CEditView::OnHScroll( int nScrollCode, int nPos )
{
	int nScrollVal = 0;

	m_bRedrawRuler = true; // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		nScrollVal = ScrollAtH( m_nViewLeftCol - 4 );
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH( m_nViewLeftCol + 4 );
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH( m_nViewLeftCol - m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH( m_nViewLeftCol + m_nViewColNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH( nPos );
//		MYTRACE( _T("nPos=%d\n"), nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH( nPos );
//		MYTRACE( _T("nPos=%d\n"), nPos );
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH( 0 );
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		nScrollVal = ScrollAtH( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - m_nViewColNum );
		break;
	}
	return nScrollVal;
}

/* 2�_��Ίp�Ƃ����`�����߂� */
void CEditView::TwoPointToRect(
		RECT*	prcRect,
	int		nLineFrom,
	int		nColmFrom,
	int		nLineTo,
	int		nColmTo
)
{
	if( nLineFrom < nLineTo ){
		prcRect->top	= nLineFrom;
		prcRect->bottom	= nLineTo;
	}else{
		prcRect->top	= nLineTo;
		prcRect->bottom	= nLineFrom;
	}
	if( nColmFrom < nColmTo ){
		prcRect->left	= nColmFrom;
		prcRect->right	= nColmTo;
	}else{
		prcRect->left	= nColmTo;
		prcRect->right	= nColmFrom;
	}
	return;

}

/*! �I��̈�̕`��

	@date 2006.10.01 Moca �d���R�[�h�폜�D��`�����P�D
	@date 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		��ʃo�b�t�@���L�����A��ʂƌ݊�BMP�̗����̔��]�������s���B
*/
void CEditView::DrawSelectArea( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	int			nFromLine;
	int			nFromCol;
	int			nToLine;
	int			nToCol;
	int			nLineNum;

	m_bDrawSelectArea = true;	// 2002/12/13 ai

	// 2006.10.01 Moca �d���R�[�h����
	HDC         hdc = ::GetDC( m_hWnd );
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	HBRUSH		hBrushCompatOld;
	int			nROPCompatOld;
	if( m_hbmpCompatBMP ){
		hBrushCompatOld = (HBRUSH)::SelectObject( m_hdcCompatDC, hBrush );
		nROPCompatOld = ::SetROP2( m_hdcCompatDC, SELECTEDAREA_ROP2 );
	}
	// To Here 2007.09.09 Moca

//	MYTRACE( _(T"DrawSelectArea()  m_bBeginBoxSelect=%s\n"), m_bBeginBoxSelect?"true":"false" );
	if( m_bBeginBoxSelect ){		/* ��`�͈͑I�� */
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if((m_nViewTopLine+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_sSelect.m_ptTo.y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_sSelectOld.m_ptTo.y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
		const int nCharHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;

		RECT  rcOld;
		RECT  rcNew;
		HRGN  hrgnOld = NULL;
		HRGN  hrgnNew = NULL;
		HRGN  hrgnDraw = NULL;

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcOld,
			m_sSelectOld.m_ptFrom.y,	/* �͈͑I���J�n�s */
			m_sSelectOld.m_ptFrom.x,	/* �͈͑I���J�n�� */
			m_sSelectOld.m_ptTo.y,		/* �͈͑I���I���s */
			m_sSelectOld.m_ptTo.x		/* �͈͑I���I���� */
		);
		if( rcOld.left	< m_nViewLeftCol ){
			rcOld.left = m_nViewLeftCol;
		}
		if( rcOld.right	< m_nViewLeftCol ){
			rcOld.right = m_nViewLeftCol;
		}
		if( rcOld.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcOld.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcOld.top < m_nViewTopLine ){
			rcOld.top = m_nViewTopLine;
		}
		if( rcOld.bottom < m_nViewTopLine - 1 ){	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
			rcOld.bottom = m_nViewTopLine - 1;
		}
		if( rcOld.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcOld.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcOld.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcOld.left  * nCharWidth;
		rcOld.right		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcOld.right * nCharWidth;
		rcOld.top		= ( rcOld.top - m_nViewTopLine ) * nCharHeight + m_nViewAlignTop;
		rcOld.bottom	= ( rcOld.bottom + 1 - m_nViewTopLine ) * nCharHeight + m_nViewAlignTop;
		hrgnOld = ::CreateRectRgnIndirect( &rcOld );

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcNew,
			m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
			m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
			m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
			m_sSelect.m_ptTo.x			/* �͈͑I���I���� */
		);
		if( rcNew.left	< m_nViewLeftCol ){
			rcNew.left = m_nViewLeftCol;
		}
		if( rcNew.right	< m_nViewLeftCol ){
			rcNew.right = m_nViewLeftCol;
		}
		if( rcNew.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcNew.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcNew.top < m_nViewTopLine ){
			rcNew.top = m_nViewTopLine;
		}
		if( rcNew.bottom < m_nViewTopLine - 1 ){	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
			rcNew.bottom = m_nViewTopLine - 1;
		}
		if( rcNew.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcNew.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcNew.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcNew.left  * nCharWidth;
		rcNew.right		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcNew.right * nCharWidth;
		rcNew.top		= (rcNew.top - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
		rcNew.bottom	= (rcNew.bottom + 1 - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
		hrgnNew = ::CreateRectRgnIndirect( &rcNew );

		// ��`���B
		{
			/* ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W��������� */
			hrgnDraw = ::CreateRectRgnIndirect( &rcNew );

			/* ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂� */
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){

				// 2002.02.16 hor
				// ������̃G���A��EOF���܂܂��ꍇ��EOF�ȍ~�̕������������܂�
				// 2006.10.01 Moca ���[�\�[�X���[�N���C��������A�`�����悤�ɂȂ������߁A
				// �}���邽�߂� EOF�ȍ~�����[�W��������폜����1�x�̍��ɂ���

				// 2006.10.01 Moca Start EOF�ʒu�v�Z��GetEndLayoutPos�ɏ��������B
				int  nLastLen;
				int  nLastLine;
				m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( nLastLen, nLastLine );
				// 2006.10.01 Moca End
				if(m_sSelect.m_ptFrom.y>=nLastLine || m_sSelect.m_ptTo.y>=nLastLine ||
					m_sSelectOld.m_ptFrom.y>=nLastLine || m_sSelectOld.m_ptTo.y>=nLastLine){
					//	Jan. 24, 2004 genta nLastLen�͕������Ȃ̂ŕϊ��K�v
					//	�ŏI�s��TAB�������Ă���Ɣ��]�͈͂��s������D
					//	2006.10.01 Moca GetEndLayoutPos�ŏ������邽��ColumnToIndex�͕s�v�ɁB
					rcNew.left = m_nViewAlignLeft + (m_nViewLeftCol + nLastLen) * nCharWidth;
					rcNew.right = m_nViewAlignLeft + m_nViewCx;
					rcNew.top = (nLastLine - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
					rcNew.bottom = rcNew.top + nCharHeight;
					// 2006.10.01 Moca GDI(���[�W����)���\�[�X���[�N�C��
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
				// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
				if( m_hbmpCompatBMP ){
					::PaintRgn( m_hdcCompatDC, hrgnDraw );
				}
				// To Here 2007.09.09 Moca
			}
		}

		//////////////////////////////////////////
		/* �f�o�b�O�p ���[�W������`�̃_���v */
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
	}else{

		/* ���ݕ`�悳��Ă���͈͂Ǝn�_������ */
		if( m_sSelect.m_ptFrom.y == m_sSelectOld.m_ptFrom.y &&
			m_sSelect.m_ptFrom.x  == m_sSelectOld.m_ptFrom.x ){
			/* �͈͂�����Ɋg�傳�ꂽ */
			if( m_sSelect.m_ptTo.y > m_sSelectOld.m_ptTo.y ||
			   (m_sSelect.m_ptTo.y == m_sSelectOld.m_ptTo.y &&
				m_sSelect.m_ptTo.x > m_sSelectOld.m_ptTo.x ) ){
				nFromLine	= m_sSelectOld.m_ptTo.y;
				nFromCol	= m_sSelectOld.m_ptTo.x;
				nToLine		= m_sSelect.m_ptTo.y;
				nToCol		= m_sSelect.m_ptTo.x;
			}else{
				nFromLine	= m_sSelect.m_ptTo.y;
				nFromCol	= m_sSelect.m_ptTo.x;
				nToLine		= m_sSelectOld.m_ptTo.y;
				nToCol		= m_sSelectOld.m_ptTo.x;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else
		if( m_sSelect.m_ptTo.y == m_sSelectOld.m_ptTo.y &&
			m_sSelect.m_ptTo.x  == m_sSelectOld.m_ptTo.x ){
			/* �͈͂��O���Ɋg�傳�ꂽ */
			if( m_sSelect.m_ptFrom.y < m_sSelectOld.m_ptFrom.y ||
			   (m_sSelect.m_ptFrom.y == m_sSelectOld.m_ptFrom.y &&
				m_sSelect.m_ptFrom.x < m_sSelectOld.m_ptFrom.x ) ){
				nFromLine	= m_sSelect.m_ptFrom.y;
				nFromCol	= m_sSelect.m_ptFrom.x;
				nToLine		= m_sSelectOld.m_ptFrom.y;
				nToCol		= m_sSelectOld.m_ptFrom.x;
			}else{
				nFromLine	= m_sSelectOld.m_ptFrom.y;
				nFromCol	= m_sSelectOld.m_ptFrom.x;
				nToLine		= m_sSelect.m_ptFrom.y;
				nToCol		= m_sSelect.m_ptFrom.x;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else{
			nFromLine		= m_sSelectOld.m_ptFrom.y;
			nFromCol		= m_sSelectOld.m_ptFrom.x;
			nToLine			= m_sSelectOld.m_ptTo.y;
			nToCol			= m_sSelectOld.m_ptTo.x;
			for( nLineNum	= nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
			nFromLine	= m_sSelect.m_ptFrom.y;
			nFromCol	= m_sSelect.m_ptFrom.x;
			nToLine		= m_sSelect.m_ptTo.y;
			nToCol		= m_sSelect.m_ptTo.x;
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}
	}
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( m_hbmpCompatBMP ){
		::SetROP2( m_hdcCompatDC, nROPCompatOld );
		::SelectObject( m_hdcCompatDC, hBrushCompatOld );
	}
	// To Here 2007.09.09 Moca
	// 2006.10.01 Moca �d���R�[�h����
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( m_hWnd, hdc );
	//	Jul. 9, 2005 genta �I��̈�̏���\��
	PrintSelectionInfoMsg();
	return;
}




/*! �I��̈�̒��̎w��s�̕`��

	@param[in] hdc �`��̈��Device Context Handle
	@param[in] nLineNum �`��Ώۍs(���C�A�E�g�s)
	@param[in] nFromLine �I���J�n�s(���C�A�E�g���W)
	@param[in] nFromCol  �I���J�n��(���C�A�E�g���W)
	@param[in] nToLine   �I���I���s(���C�A�E�g���W)
	@param[in] nToCol    �I���I����(���C�A�E�g���W)

	�����s�ɓn��I��͈͂̂����CnLineNum�Ŏw�肳�ꂽ1�s��������`�悷��D
	�I��͈͂͌Œ肳�ꂽ�܂�nLineNum�݂̂��K�v�s���ω����Ȃ���Ăт������D

	@date 2006.03.29 Moca 3000��������P�p�D

*/
void CEditView::DrawSelectAreaLine(
		HDC hdc, int nLineNum, int nFromLine, int nFromCol, int nToLine, int nToCol
)
{
//	MYTRACE( _T("CEditView::DrawSelectAreaLine()\n") );
	RECT			rcClip;
	int				nSelectFrom;	// �`��s�̑I���J�n���ʒu
	int				nSelectTo;		// �`��s�̑I���J�n�I���ʒu

	if( nFromLine == nToLine ){
		nSelectFrom = nFromCol;
		nSelectTo	= nToCol;
	}else{
		// 2006.03.29 Moca �s���܂ł̒��������߂�ʒu���ォ�炱���Ɉړ�
		int nPosX = 0;
		const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca ��ʊO�܂ŋ��߂���ł��؂�
			if( it.getColumn() > m_nViewLeftCol + m_nViewColNum ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		
		if( nLineNum == nFromLine ){
			nSelectFrom = nFromCol;
			nSelectTo	= nPosX;
		}else
		if( nLineNum == nToLine ){
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nToCol;
		}else{
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nPosX;
		}
		// 2006.05.24 Moca�t���[�J�[�\���I��(�I���J�n/�I���s)��
		// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
		if( nSelectTo < nSelectFrom ){
			int t = nSelectFrom;
			nSelectFrom = nSelectTo;
			nSelectTo = t;
		}
	}
	
	// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
	if( nSelectFrom < m_nViewLeftCol ){
		nSelectFrom = m_nViewLeftCol;
	}
	int		nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int		nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	rcClip.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + nSelectFrom * nCharWidth;
	rcClip.right	= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + nSelectTo   * nCharWidth;
	rcClip.top		= (nLineNum - m_nViewTopLine) * nLineHeight + m_nViewAlignTop;
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > m_nViewAlignLeft + m_nViewCx ){
		rcClip.right = m_nViewAlignLeft + m_nViewCx;
	}
	//	�K�v�ȂƂ������B
	if ( rcClip.right != rcClip.left ){
		m_cUnderLine.CaretUnderLineOFF( true );	//	YAZAKI
		
		// 2006.03.28 Moca �\������̂ݏ�������
		if( nSelectFrom <= m_nViewLeftCol + m_nViewColNum && m_nViewLeftCol < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			if( m_hbmpCompatBMP ){
				::PaintRgn( m_hdcCompatDC, hrgnDraw );
			}
			// To Here 2007.09.09 Moca
			::DeleteObject( hrgnDraw );
		}
	}

//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1);
//	::FillRect( hdc, &rcClip, hBrushTextCol );

//	//	/* �f�o�b�O���j�^�ɏo�� */
//	m_cShareData.TraceOut( "DrawSelectAreaLine() rcClip.left=%d, rcClip.top=%d, rcClip.right=%d, rcClip.bottom=%d\n", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

	return;
}


/* �t�H���g�̕ύX */
void CEditView::SetFont( void )
{
	HDC			hdc;
	HFONT		hFontOld;
	TEXTMETRIC	tm;
	int			i;
	SIZE		sz;

	hdc = ::GetDC( m_hWnd );
	hFontOld = (HFONT)::SelectObject( hdc, m_pcViewFont->GetFontHan() );
//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_BOLD );
	::GetTextMetrics( hdc, &tm );


// 1999.12.9
//	m_nCharWidth = tm.tmAveCharWidth - 1;
//	m_nCharHeight = tm.tmHeight + tm.tmExternalLeading;
	/* �����̑傫���𒲂ׂ� */
// 2000.2.8
//	::GetTextExtentPoint32( hdc, "X", 1, &sz );
//	m_nCharHeight = sz.cy;
//	m_nCharWidth = sz.cx;
	::GetTextExtentPoint32( hdc, "��", 2, &sz );
	m_nCharHeight = sz.cy;
	m_nCharWidth = sz.cx / 2;


// �s�̍�����2�̔{���ɂ���
//	if( ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) % 2 ){
//		++m_nCharHeight;
//	}

	m_nViewColNum = (m_nViewCx - 1) / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* �\����̌��� */
	m_nViewRowNum = (m_nViewCy - 1) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* �\����̍s�� */
	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	DetectWidthOfLineNumberArea( FALSE );
	/* ������`��p�������z�� */
	for( i = 0; i < ( sizeof(m_pnDx) / sizeof(m_pnDx[0]) ); ++i ){
		m_pnDx[i] = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
	::InvalidateRect( m_hWnd, NULL, TRUE );
//	2002/05/12 YAZAKI �s�v�Ǝv��ꂽ�̂ŁB
//	if( m_nCaretWidth == 0 ){	/* �L�����b�g���Ȃ������ꍇ */
//	}else{
//		OnKillFocus();
//		OnSetFocus();
//	}
	//	Oct. 11, 2002 genta IME�̃t�H���g���ύX
	SetIMECompFormFont();
	return;
}



/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
int CEditView::DetectWidthOfLineNumberArea_calculate( void )
{
	int			i;
	int			nAllLines;
	int			nWork;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		nAllLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}else{
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	}
	if( 0 < nAllLines ){
		nWork = 100;
		for( i = 3; i < 12; ++i ){
			if( nWork > nAllLines ){	// Oct. 18, 2003 genta ���𐮗�
				break;
			}
			nWork *= 10;
		}
	}else{
		//	2003.09.11 wmlhq �s�ԍ���1���̂Ƃ��ƕ������킹��
		i = 3;
	}
	return i;

}


/*
�s�ԍ��\���ɕK�v�ȕ���ݒ蕝���ύX���ꂽ�ꍇ��TRUE��Ԃ�
*/
BOOL CEditView::DetectWidthOfLineNumberArea( BOOL bRedraw )
{
	int				i;
	PAINTSTRUCT		ps;
	HDC				hdc;
//	int				nAllLines;
//	int				nWork;
	int				m_nViewAlignLeftNew;
	int				nCxVScroll;
	RECT			rc;

	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		i = DetectWidthOfLineNumberArea_calculate();
		m_nViewAlignLeftNew = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * (i + 1);	/* �\����̍��[���W */
		m_nViewAlignLeftCols = i + 1;
	}else{
		m_nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}
	//	Sep 18, 2002 genta
	m_nViewAlignLeftNew += m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace;
	if( m_nViewAlignLeftNew != m_nViewAlignLeft ){
		m_nViewAlignLeft = m_nViewAlignLeftNew;
		::GetClientRect( m_hWnd, &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		m_nViewCx = (rc.right - rc.left) - nCxVScroll - m_nViewAlignLeft;	/* �\����̕� */
		// 2008.05.23 nasukoji	�\����̌������Z�o����i�E�[�J�[�\���ړ����̕\���ꏊ����ւ̑Ώ��j
		m_nViewColNum = (m_nViewCx - 1) / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* �\����̌��� */


		if( bRedraw ){
			/* �ĕ`�� */
			hdc = ::GetDC( m_hWnd );
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = 0;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//			OnKillFocus();
			m_cUnderLine.Lock();
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			OnPaint( hdc, &ps, FALSE );
			// To Here 2007.09.09 Moca
			m_cUnderLine.UnLock();
//			OnSetFocus();
//			DispRuler( hdc );
			ShowEditCaret();
			::ReleaseDC( m_hWnd, hdc );
		}
		m_bRedrawRuler = true;
		return TRUE;
	}else{
		return FALSE;
	}
}





/** �X�N���[���o�[�̏�Ԃ��X�V����

	�^�u�o�[�̃^�u�ؑ֎��� SIF_DISABLENOSCROLL �t���O�ł̗L�����^������������ɓ��삵�Ȃ�
	�i�s���ŃT�C�Y�ύX���Ă��邱�Ƃɂ��e�����H�j�̂� SIF_DISABLENOSCROLL �ŗL���^����
	�̐ؑւɎ��s�����ꍇ�ɂ͋����ؑւ���

	@date 2008.05.24 ryoji �L���^�����̋����ؑւ�ǉ�
	@date 2008.06.08 ryoji �����X�N���[���͈͂ɂԂ牺���]����ǉ�
	@date 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̃X�N���[���o�[����
*/
void CEditView::AdjustScrollBars( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}


	int			nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;
	bool		bEnable;

	if( NULL != m_hwndVScrollBar ){
		/* �����X�N���[���o�[ */
		/* nAllLines / nVScrollRate < 65535 �ƂȂ鐮��nVScrollRate�����߂� */
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		nAllLines+=2;
		nVScrollRate = 1;
		while( nAllLines / nVScrollRate > 65535 ){
			++nVScrollRate;
		}
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = nAllLines / nVScrollRate - 1;	/* �S�s�� */
		si.nPage = m_nViewRowNum / nVScrollRate;	/* �\����̍s�� */
		si.nPos  = m_nViewTopLine / nVScrollRate;	/* �\����̈�ԏ�̍s(0�J�n) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* �����X�N���[���o�[�̏k�� */

		//	Nov. 16, 2002 genta
		//	�c�X�N���[���o�[��Disable�ɂȂ����Ƃ��͕K���S�̂���ʓ��Ɏ��܂�悤��
		//	�X�N���[��������
		//	2005.11.01 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
		bEnable = ( m_nViewRowNum < nAllLines );
		if( bEnable != (::IsWindowEnabled( m_hwndVScrollBar ) != 0) ){
			::EnableWindow( m_hwndVScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if( !bEnable ){
			ScrollAtV( 0 );
		}
	}
	if( NULL != m_hwndHScrollBar ){
		/* �����X�N���[���o�[ */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		si.nMax  = GetRightEdgeForScrollBar() - 1;
		si.nPage = m_nViewColNum;		/* �\����̌��� */
		si.nPos  = m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );

		//	2006.1.28 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
		bEnable = ( m_nViewColNum < GetRightEdgeForScrollBar() );
		if( bEnable != (::IsWindowEnabled( m_hwndHScrollBar ) != 0) ){
			::EnableWindow( m_hwndHScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if( !bEnable ){
			ScrollAtH( 0 );
		}
	}

	return;
}

/** �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z
	@date 2008.06.08 ryoji �V�K�쐬
*/
int CEditView::GetWrapOverhang( void ) const
{
	int nMargin = 0;
	if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet )
		nMargin += 2;	// ���s�Ԃ牺��
	if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )
		nMargin += 2;	// ��Ǔ_�Ԃ牺��
	return nMargin;
}

/** �u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z����
	@param nViewColNum	[in] �r���[�̌���
	@retval �܂�Ԃ�����
	@date 2008.06.08 ryoji �V�K�쐬
*/
int CEditView::ViewColNumToWrapColNum( int nViewColNum ) const
{
	// �Ԃ牺���]������������
	int nWidth = nViewColNum - GetWrapOverhang();

	// MINLINEKETAS�����̎���MINLINEKETAS�Ő܂�Ԃ��Ƃ���
	if( nWidth < MINLINEKETAS )
		nWidth = MINLINEKETAS;		// �܂�Ԃ����̍ŏ������ɐݒ�

	return nWidth;
}

/*!
	@brief  �X�N���[���o�[����p�ɉE�[���W���擾����

	�u�܂�Ԃ��Ȃ��v
		�t���[�J�[�\����Ԃ̎��̓e�L�X�g�̕������E���փJ�[�\�����ړ��ł���
		�̂ŁA������l�������X�N���[���o�[�̐��䂪�K�v�B
		�{�֐��́A���L�̓��ōł��傫�Ȓl�i�E�[�̍��W�j��Ԃ��B
		�@�E�e�L�X�g�̉E�[
		�@�E�L�����b�g�ʒu
		�@�E�I��͈͂̉E�[
	
	�u�w�茅�Ő܂�Ԃ��v
	�u�E�[�Ő܂�Ԃ��v
		��L�̏ꍇ�܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z

	@return     �E�[�̃��C�A�E�g���W��Ԃ�

	@note   �u�܂�Ԃ��Ȃ��v�I�����́A�X�N���[����ɃL�����b�g�������Ȃ�
	        �Ȃ�Ȃ��l�ɂ��邽�߂ɉE�}�[�W���Ƃ��Ĕ��p3���Œ�ŉ��Z����B

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
int CEditView::GetRightEdgeForScrollBar( void )
{
	// �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z
	int nWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() + GetWrapOverhang();
	
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		int nRightEdge = m_pcEditDoc->m_cLayoutMgr.GetMaxTextWidth();	// �e�L�X�g�̍ő啝

		// �I��͈͂��� ���� �͈͂̉E�[���e�L�X�g�̕����E��
		if( IsTextSelected() ){
			// �J�n�ʒu�E�I���ʒu�̂��E���ɂ�����Ŕ�r
			if( m_sSelect.m_ptFrom.x < m_sSelect.m_ptTo.x ){
				if( nRightEdge < m_sSelect.m_ptTo.x )
					nRightEdge = m_sSelect.m_ptTo.x;
			}else{
				if( nRightEdge < m_sSelect.m_ptFrom.x )
					nRightEdge = m_sSelect.m_ptFrom.x;
			}
		}

		// �t���[�J�[�\�����[�h ���� �L�����b�g�ʒu���e�L�X�g�̕����E��
		if( m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode && nRightEdge < m_ptCaretPos.x )
			nRightEdge = m_ptCaretPos.x;

		// �E�}�[�W�����i3���j���l������nWidth�𒴂��Ȃ��悤�ɂ���
		nWidth = ( nRightEdge + 3 < nWidth ) ? nRightEdge + 3 : nWidth;
	}

	return nWidth;
}

/*!	@brief �I�����l�������s���w��ɂ��J�[�\���ړ�

	�I����ԃ`�F�b�N���J�[�\���ړ����I��̈�X�V�Ƃ���������
	���������̃R�}���h�ɂ���̂ł܂Ƃ߂邱�Ƃɂ����D
	�܂��C�߂�l�͂قƂ�ǎg���Ă��Ȃ��̂�void�ɂ����D

	�I����Ԃ��l�����ăJ�[�\�����ړ�����D
	��I�����w�肳�ꂽ�ꍇ�ɂ͊����I��͈͂��������Ĉړ�����D
	�I�����w�肳�ꂽ�ꍇ�ɂ͑I��͈͂̊J�n�E�ύX�𕹂��čs���D
	�C���^���N�e�B�u�����O��Ƃ��邽�߁C�K�v�ɉ������X�N���[�����s���D
	�J�[�\���ړ���͏㉺�ړ��ł��J�����ʒu��ۂ悤�C
	m_nCaretPosX_Prev�̍X�V�������čs���D

	@param nWk_CaretPosX	[in] �ړ��挅�ʒu(0�`)
	@param nWk_CaretPosY	[in] �ړ���s�ʒu(0�`)
	@param bSelect			[in] TRUE: �I������/ FALSE: �I������
	@param nCaretMarginRate	[in] �c�X�N���[���J�n�ʒu�����߂�l


	@date 2006.07.09 genta �V�K�쐬
*/
void CEditView::MoveCursorSelecting( int nWk_CaretPosX, int nWk_CaretPosY, bool bSelect, int nCaretMarginRate )
{
	if( bSelect ){
		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );
		}
	}
	MoveCursor( nWk_CaretPosX, nWk_CaretPosY, true, nCaretMarginRate );	// 2007.08.22 ryoji nCaretMarginRate���g���Ă��Ȃ�����
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	if( bSelect ){
		/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
		
			2004.04.02 Moca 
			�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
			�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
			nPosX, nPosY�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
		*/
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
	}
	
}




/*!	@brief �s���w��ɂ��J�[�\���ړ�

	�K�v�ɉ����ďc/���X�N���[��������D
	�����X�N���[���������ꍇ�͂��̍s����Ԃ��i���^���j�D
	
	@param nWk_CaretPosX	[in] �ړ��挅�ʒu(0�`)
	@param nWk_CaretPosY	[in] �ړ���s�ʒu(0�`)
	@param bScroll			[in] true: ��ʈʒu�����L��/ false: ��ʈʒu�����L�薳��
	@param nCaretMarginRate	[in] �c�X�N���[���J�n�ʒu�����߂�l
	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �s���Ȉʒu���w�肳�ꂽ�ꍇ�ɂ͓K�؂ȍ��W�l��
		�ړ����邽�߁C�����ŗ^�������W�ƈړ���̍��W��
		�K��������v���Ȃ��D
	
	@note bScroll��false�̏ꍇ�ɂ̓J�[�\���ʒu�݈̂ړ�����D
		true�̏ꍇ�ɂ̓X�N���[���ʒu�����킹�ĕύX�����

	@date 2001.10.20 deleted by novice AdjustScrollBar()���ĂԈʒu��ύX
	@date 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	@date 2004.09.11 genta bDraw�X�C�b�`�͓���Ɩ��̂���v���Ă��Ȃ��̂�
		�ĕ`��X�C�b�`����ʈʒu�����X�C�b�`�Ɩ��̕ύX
*/
int CEditView::MoveCursor( int nWk_CaretPosX, int nWk_CaretPosY, bool bScroll, int nCaretMarginRate )
{

	/* �X�N���[������ */
	int		nScrollRowNum = 0;
	int		nScrollColNum = 0;
	RECT	rcScrol;
	RECT	rcClip;
	RECT	rcClip2;
//	int		nIndextY = 8;
	int		nCaretMarginY;
	HDC		hdc;
//	HPEN	hPen, hPenOld;
	int		nScrollMarginRight;
	int		nScrollMarginLeft;

	if( 0 >= m_nViewColNum ){
		return 0;
	}
	hdc = ::GetDC( m_hWnd );

	/* �J�[�\���s�A���_�[���C����OFF */
//	if (IsTextSelected()) { //2002.02.27 Add By KK �A���_�[���C���̂������ጸ - �����ł̓e�L�X�g�I�����̂݃A���_�[���C���������B
		m_cUnderLine.CaretUnderLineOFF( bScroll );	//	YAZAKI
//	}	2002/04/04 YAZAKI ���y�[�W�X�N���[�����ɃA���_�[���C�����c�����܂܃X�N���[�����Ă��܂����ɑΏ��B

	if( m_bBeginSelect ){	/* �͈͑I�� */
		nCaretMarginY = 0;
	}else{
		//	2001/10/20 novice
		nCaretMarginY = m_nViewRowNum / nCaretMarginRate;
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	GetAdjustCursorPos( &nWk_CaretPosX, &nWk_CaretPosY );
	
	
	/* �����X�N���[���ʁi�������j�̎Z�o */
	nScrollColNum = 0;
	nScrollMarginRight = SCROLLMARGIN_RIGHT;
	nScrollMarginLeft = SCROLLMARGIN_LEFT;
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() > m_nViewColNum &&
		nWk_CaretPosX > m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ) - nWk_CaretPosX;
	}else
	if( 0 < m_nViewLeftCol &&
		nWk_CaretPosX < m_nViewLeftCol + nScrollMarginLeft
	){
		nScrollColNum = m_nViewLeftCol + nScrollMarginLeft - nWk_CaretPosX;
		if( 0 > m_nViewLeftCol - nScrollColNum ){
			nScrollColNum = m_nViewLeftCol;
		}

	}

	m_nViewLeftCol -= nScrollColNum;

	//	From Here 2007.07.28 ���イ�� : �\���s����3�s�ȉ��̏ꍇ�̓�����P
	/* �����X�N���[���ʁi�s���j�̎Z�o */
										// ��ʂ��R�s�ȉ�
	if( m_nViewRowNum <= 3 ){
							// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
		if( nWk_CaretPosY - m_nViewTopLine < nCaretMarginY ){
			if( nWk_CaretPosY < nCaretMarginY ){	//�P�s�ڂɈړ�
				nScrollRowNum = m_nViewTopLine;
			}else
			if( m_nViewRowNum <= 1 ){	// ��ʂ��P�s
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else
#if !(0)	// COMMENT�ɂ���ƁA�㉺�̋󂫂����炵�Ȃ��ׁA�c�ړ���good�����A���ړ��̏ꍇ�㉺�ɂԂ��
			if( m_nViewRowNum <= 2 ){	// ��ʂ��Q�s
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else
#endif
			{						// ��ʂ��R�s
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY + 1;
			}
		}else
							// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
		if( nWk_CaretPosY - m_nViewTopLine >= (m_nViewRowNum - nCaretMarginY - 2) ){
			int ii = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( ii - nWk_CaretPosY < nCaretMarginY + 1 &&
				ii - m_nViewTopLine < m_nViewRowNum ) {
			} else
			if( m_nViewRowNum <= 2 ){	// ��ʂ��Q�s�A�P�s
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else{						// ��ʂ��R�s
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY + 1;
			}
		}
	}else							// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
	if( nWk_CaretPosY - m_nViewTopLine < nCaretMarginY ){
		if( nWk_CaretPosY < nCaretMarginY ){	//�P�s�ڂɈړ�
			nScrollRowNum = m_nViewTopLine;
		}else{
			nScrollRowNum = -(nWk_CaretPosY - m_nViewTopLine) + nCaretMarginY;
		}
	} else
							// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
	if( nWk_CaretPosY - m_nViewTopLine >= m_nViewRowNum - nCaretMarginY - 2 ){
		int ii = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		if( ii - nWk_CaretPosY < nCaretMarginY + 1 &&
			ii - m_nViewTopLine < m_nViewRowNum ) {
		} else
		{
			nScrollRowNum =
				-(nWk_CaretPosY - m_nViewTopLine) + (m_nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 ���イ��
	if( bScroll ){
		/* �X�N���[�� */
		if( abs( nScrollColNum ) >= m_nViewColNum ||
			abs( nScrollRowNum ) >= m_nViewRowNum ){
			m_nViewTopLine -= nScrollRowNum;
			::InvalidateRect( m_hWnd, NULL, TRUE );
		}else
		if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			rcScrol.left = 0;
			rcScrol.right = m_nViewCx + m_nViewAlignLeft;
			rcScrol.top = m_nViewAlignTop;
			rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
			if( nScrollRowNum > 0 ){
				rcScrol.bottom =
					m_nViewCy + m_nViewAlignTop -
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top = m_nViewAlignTop;
				rcClip.bottom =
					m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			}else
			if( nScrollRowNum < 0 ){
				rcScrol.top =
					m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top =
					m_nViewCy + m_nViewAlignTop +
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				rcClip.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( nScrollColNum > 0 ){
				rcScrol.left = m_nViewAlignLeft;
				rcScrol.right =
					m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left = m_nViewAlignLeft;
				rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}else
			if( nScrollColNum < 0 ){
				rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left =
					m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.right = m_nViewCx + m_nViewAlignLeft;
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( m_bDrawSWITCH ){
				::ScrollWindowEx(
					m_hWnd,
					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* �����X�N���[���� */
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* �����X�N���[���� */
					&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);
				// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
				if( m_hbmpCompatBMP ){
					// �݊�BMP���X�N���[�������̂��߂�BitBlt�ňړ�������
					::BitBlt(
						m_hdcCompatDC,
						rcScrol.left + nScrollColNum * ( m_nCharWidth +  m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
						rcScrol.top  + nScrollRowNum * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ),
						rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
						m_hdcCompatDC, rcScrol.left, rcScrol.top, SRCCOPY
					);
				}
				// �J�[�\���̏c�����e�L�X�g�ƍs�ԍ��̌��Ԃɂ���Ƃ��A�X�N���[�����ɏc���̈���X�V
				if( nScrollColNum != 0 && m_nOldCursorLineX == m_nViewAlignLeft - 1 ){
					RECT rcClip3;
					rcClip3.left = m_nOldCursorLineX;
					rcClip3.right = m_nOldCursorLineX + 1;
					rcClip3.top  = m_nViewAlignTop;
					rcClip3.bottom = m_nViewCy + m_nViewAlignTop;
					::InvalidateRect( m_hWnd, &rcClip3, TRUE );
				}
				// To Here 2007.09.09 Moca
				if( nScrollRowNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip, TRUE );
					if( nScrollColNum != 0 ){
						rcClip.left = 0;
						rcClip.right = m_nViewAlignLeft;
						rcClip.top = 0;
						rcClip.bottom = m_nViewCy + m_nViewAlignTop;
						::InvalidateRect( m_hWnd, &rcClip, TRUE );
					}
				}
				if( nScrollColNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip2, TRUE );
				}
			}
		}

		// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�i�X�N���[���o�[���e�L�X�g���ɍ��킹��j
		if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
			// AdjustScrollBars()�ňړ���̃L�����b�g�ʒu���K�v�Ȃ��߁A�����ŃR�s�[
			if( IsTextSelected() || m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode ){
				m_ptCaretPos.x = nWk_CaretPosX;
				m_ptCaretPos.y = nWk_CaretPosY;
			}
		}

		/* �X�N���[���o�[�̏�Ԃ��X�V���� */
		AdjustScrollBars(); // 2001/10/20 novice
	}

	/* �L�����b�g�ړ� */
	m_ptCaretPos.x = nWk_CaretPosX;
	m_ptCaretPos.y = nWk_CaretPosY;

	/* �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	||  �������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_ptCaretPos.x,
		m_ptCaretPos.y,
		&m_ptCaretPos_PHY.x,	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g��(�O�J�n) */
		&m_ptCaretPos_PHY.y	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ�(�O�J�n) */
	);
	// ���X�N���[��������������A���[���[�S�̂��ĕ`�� 2002.02.25 Add By KK
	if (nScrollColNum != 0 ){
		//����DispRuler�Ăяo�����ɍĕ`��B�ibDraw=false�̃P�[�X���l�������B�j
		m_bRedrawRuler = true;
	}

	/* �J�[�\���s�A���_�[���C����ON */
	//CaretUnderLineON( bDraw ); //2002.02.27 Del By KK �A���_�[���C���̂������ጸ
	if( bScroll ){
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();

		/* ���[���̍ĕ`�� */
		DispRuler( hdc );

		/* �A���_�[���C���̍ĕ`�� */
		m_cUnderLine.CaretUnderLineON( true );

		/* �L�����b�g�̍s���ʒu��\������ */
		DrawCaretPosInfo();

		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		//	bScroll == FALSE�̎��ɂ̓X�N���[�����Ȃ��̂ŁC���s���Ȃ�
		SyncScrollV( -nScrollRowNum );	//	�������t�Ȃ̂ŕ������]���K�v
		SyncScrollH( -nScrollColNum );	//	�������t�Ȃ̂ŕ������]���K�v

	}
	::ReleaseDC( m_hWnd, hdc );


// 02/09/18 �Ί��ʂ̋����\�� ai Start	03/02/18 ai mod S
	DrawBracketPair( false );
	SetBracketPairPos( true );
	DrawBracketPair( true );
// 02/09/18 �Ί��ʂ̋����\�� ai End		03/02/18 ai mod E

	return nScrollRowNum;


}

/*! �������J�[�\���ʒu���Z�o����(EOF�ȍ~�̂�)
	@param pnPosX [in/out] �J�[�\���̃��C�A�E�g���WX
	@param pnPosY [in/out] �J�[�\���̃��C�A�E�g���WY
	@retval	TRUE ���W���C������
	@retval	FALSE ���W�͏C������Ȃ�����
	@note	EOF�̒��O�����s�łȂ��ꍇ�́A���̍s�Ɍ���EOF�ȍ~�ɂ��ړ��\
			EOF�����̍s�́A�擪�ʒu�̂ݐ������B
	@date 2004.04.02 Moca �֐���
*/
BOOL CEditView::GetAdjustCursorPos( int* pnPosX, int* pnPosY ){
	// 2004.03.28 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	int nLayoutLineCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	int nPosX2 = *pnPosX;
	int nPosY2 = *pnPosY;
	BOOL ret = FALSE;
	if( nPosY2 >= nLayoutLineCount ){
		if( 0 < nLayoutLineCount ){
			nPosY2 = nLayoutLineCount - 1;
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nPosY2 );
			if( pcLayout->m_cEol == EOL_NONE ){
				nPosX2 = LineIndexToColmn( pcLayout, pcLayout->GetLength() );
				// EOF�����܂�Ԃ���Ă��邩
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				if( nPosX2 >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
					nPosY2++;
					nPosX2 = 0;
				}
			}else{
				// EOF�����̍s
				nPosY2++;
				nPosX2 = 0;
			}
		}else{
			// ��̃t�@�C��
			nPosX2 = 0;
			nPosY2 = 0;
		}
		if( *pnPosX != nPosX2 || *pnPosY != nPosY2 ){
			*pnPosX = nPosX2;
			*pnPosY = nPosY2;
			ret = TRUE;
		}
	}
	return ret;
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
	return;
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
	return;
}





/** �s���w��ɂ��J�[�\���ړ��i���W�����t���j
	@param nNewX[in/out] �J�[�\���̃��C�A�E�g���WX
	@param nNewY[in/out] �J�[�\���̃��C�A�E�g���WY
	@param bScroll[in] true: ��ʈʒu�����L��/ false: ��ʈʒu�����L�薳��
	@param nCaretMarginRate[in] �c�X�N���[���J�n�ʒu�����߂�l
	@param dx[in] nNewX�ƃ}�E�X�J�[�\���ʒu�Ƃ̌덷(�J�����������̃h�b�g��)
	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �}�E�X���ɂ��ړ��ŕs�K�؂Ȉʒu�ɍs���Ȃ��悤���W�������ăJ�[�\���ړ�����

	@date 2007.08.23 ryoji �֐����iMoveCursorToPoint()���珈���𔲂��o���j
	@date 2007.09.26 ryoji ���p�����ł������ō��E�ɃJ�[�\����U�蕪����
*/
int CEditView::MoveCursorProperly( int nNewX, int nNewY, bool bScroll, int nCaretMarginRate, int dx )
{
	const char*		pLine;
	int				nLineLen;
	const CLayout*	pcLayout;

	if( 0 > nNewY ){
		nNewY = 0;
	}
	/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
	if( nNewY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		// 2004.04.03 Moca EOF�����̍��W�����́AMoveCursor���ł���Ă��炤�̂ŁA�폜
	}else
	/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
	if( nNewY < 0 ){
		nNewX = 0;
		nNewY = 0;
	}else{
		/* �ړ���̍s�̃f�[�^���擾 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nNewY, &nLineLen, &pcLayout );

		int nColWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
		int nPosX = 0;
		int i = 0;
		CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				i = nLineLen;
				break;
			}
			if( it.getColumn() + it.getColumnDelta() > nNewX ){
				if (nNewX >= (pcLayout ? pcLayout->GetIndent() : 0) && ((nNewX - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth){
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if ( it.end() ){
			i = it.getIndex();
			nPosX -= it.getColumnDelta();
		}

		if( i >= nLineLen ){
// From 2001.12.21 hor �t���[�J�[�\��OFF��EOF�̂���s�̒��O���}�E�X�őI���ł��Ȃ��o�O�̏C��
			if( nNewY +1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				EOL_NONE == pcLayout->m_cEol.GetLen() ){
				nPosX = LineIndexToColmn( pcLayout, nLineLen );
			}else
// To 2001.12.21 hor
			/* �t���[�J�[�\�����[�h�� */
			if( m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode
			  || ( m_bBeginSelect && m_bBeginBoxSelect )	/* �}�E�X�͈͑I�� && ��`�͈͑I�� */
//			  || m_bDragMode /* OLE DropTarget */
			  || ( m_bDragMode && m_bDragBoxData ) /* OLE DropTarget && ��`�f�[�^ */
			){
// From 2001.12.21 hor
//				if( nNewY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
//					pLine[ nLineLen - 1 ] != '\n' && pLine[ nLineLen - 1 ] != '\r'
//				){
//					nPosX = LineIndexToColmn( pLine, nLineLen, nLineLen );
//				}else{
// To 2001.12.21 hor
					nPosX = nNewX;
					//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
					if( nPosX < 0 ){
						nPosX = 0;
					}else
					if( nPosX > m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){	/* �܂�Ԃ������� */
						nPosX = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
					}
//				}
			}
		}
		nNewX = nPosX;
	}

	return MoveCursor( nNewX, nNewY, bScroll, nCaretMarginRate );
}



/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ�(���^��)
*/
int CEditView::MoveCursorToPoint( int xPos, int yPos )
{
	int				nScrollRowNum;
	int				nNewX;
	int				nNewY;
	int				dx;

	nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	dx = (xPos - m_nViewAlignLeft) % ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );

	nScrollRowNum = MoveCursorProperly( nNewX, nNewY, true, 1000, dx );
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/* �}�E�X���{�^������ */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearch���L�����Z������
	if (m_nISearchMode > 0 ){
		ISearchExit();
	}

	int			nCaretPosY_Old;
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	�g���v���N���b�N�ł��邱�Ƃ�����
	int			nFuncID = 0;				// 2007.11.30 nasukoji	�}�E�X���N���b�N�ɑΉ�����@�\�R�[�h

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //�t�H�[�J�X���Ȃ��Ƃ�
		return;
	}
	nCaretPosY_Old = m_ptCaretPos.y;

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}
	else{
		m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
	}

	// 2007.11.30 nasukoji	�g���v���N���b�N���`�F�b�N
	tripleClickMode = CheckTripleClick(xPos, yPos);

	if(tripleClickMode){
		// �}�E�X���g���v���N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[5]�ɓ����Ă���
		nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// ���蓖�ċ@�\�����̎��̓g���v���N���b�N OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}

	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );

	// OLE�ɂ��h���b�O & �h���b�v���g��
	// 2007.11.30 nasukoji	�g���v���N���b�N���̓h���b�O���J�n���Ȃ�
	if( !tripleClickMode && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){
		if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource ){		/* OLE�ɂ��h���b�O���ɂ��邩 */
			/* �s�I���G���A���h���b�O���� */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// �J�[�\���ʒuX
				nNewY		// �J�[�\���ʒuY
				)
			){
				POINT ptWk = {xPos, yPos};
				::ClientToScreen(m_hWnd, &ptWk);
				if( !::DragDetect(m_hWnd, ptWk) ){
					// �h���b�O�J�n�����𖞂����Ȃ������̂ŃN���b�N�ʒu�ɃJ�[�\���ړ�����
					if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						DisableSelectArea( true );
					}
//@@@ 2002.01.08 YAZAKI �t���[�J�[�\��OFF�ŕ����s�I�����A�s�̌����N���b�N����Ƃ����ɃL�����b�g���u����Ă��܂��o�O�C��
					/* �J�[�\���ړ��B */
					if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
						if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
							MoveCursorToPoint( xPos, yPos );
						}else
						if( xPos < m_nViewAlignLeft ){
							MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
						}
					}
					return;
				}
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					DWORD dwEffectsSrc = (
							m_pcEditDoc->IsReadOnly()	// �ǂݎ���p
							|| ( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )	// �㏑���֎~
						)? DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pcEditDoc->m_cOpeBuf.GetCurrentPointer();
					m_pcEditDoc->SetDragSourceView( this );
					CDataObject data( cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), m_bBeginBoxSelect );	// 2008.03.26 ryoji �e�L�X�g���A��`�̎w���ǉ�
					dwEffects = data.DragDrop( TRUE, dwEffectsSrc );
					m_pcEditDoc->SetDragSourceView( NULL );
//					MYTRACE( _T("dwEffects=%d\n"), dwEffects );
					if( m_pcEditDoc->m_cOpeBuf.GetCurrentPointer() == nOpe ){	// �h�L�������g�ύX�Ȃ����H	// 2007.12.09 ryoji
						m_pcEditDoc->SetActivePane( m_nMyIndex );
						if( DROPEFFECT_MOVE == (dwEffectsSrc & dwEffects) ){
							// �ړ��͈͂��폜����
							// �h���b�v�悪�ړ����������������h�L�������g�ɂ����܂ŕύX������
							// ���h���b�v��͊O���̃E�B���h�E�ł���
							if( NULL == m_pcOpeBlk ){
								m_pcOpeBlk = new COpeBlk;
							}

							// �I��͈͂��폜
							DeleteData( true );

							// �A���h�D�o�b�t�@�̏���
							SetUndoBuffer();
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALT�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.10.10 nasukoji	�g���v���N���b�N�Ή�
	if( GetKeyState_Alt() &&( ! tripleClickMode)){
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		// �}�E�X�͈͑I��O��ʒu(X���W)
		m_nMouseRollPosYOld = yPos;		// �}�E�X�͈͑I��O��ʒu(Y���W)
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = true;			/* �͈͑I�� */
		m_bBeginBoxSelect = true;		/* ��`�͈͑I�� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */

		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
		m_cUnderLine.CaretUnderLineOFF( true );
		m_cUnderLine.Lock();
		if( xPos < m_nViewAlignLeft ){
			/* �J�[�\�����ړ� */
			Command_DOWN( true, false );
		}
	}
	else{
		/* �J�[�\���ړ� */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}
		else if( yPos < m_nViewAlignTop ){
			//	���[���N���b�N
			return;
		}
		else {
			return;
		}

		/* �}�E�X�̃L���v�`���Ȃ� */
		m_nMouseRollPosXOld = xPos;		/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;		/* �}�E�X�͈͑I��O��ʒu(Y���W) */
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = true;			/* �͈͑I�� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */
		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice


		if(tripleClickMode){		// 2007.10.10 nasukoji	�g���v���N���b�N����������
			// 1�s�I���łȂ��ꍇ�͑I�𕶎��������
			// 2007.11.05 nasukoji	�g���v���N���b�N��1�s�I���łȂ��Ă��N�A�h���v���N���b�N��L���Ƃ���
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, xPos, yPos );	// �����ō��{�^���A�b�v�������Ƃɂ���

				if( IsTextSelected() )		// �e�L�X�g���I������Ă��邩
					DisableSelectArea( true );		// ���݂̑I��͈͂��I����Ԃɖ߂�
			}

			// 2007.10.10 nasukoji	�P��̓r���Ő܂�Ԃ���Ă���Ɖ��̍s���I������Ă��܂����Ƃւ̑Ώ�
			MoveCursorToPoint( xPos, yPos );	// �J�[�\���ړ�

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1�s�I���łȂ��ꍇ�͂����Ŕ�����i���̑I���R�}���h�̎����ƂȂ邩���j
			if(F_SELECTLINE != nFuncID)
				return;

			// 2007.10.13 nasukoji	�I��������̂������i[EOF]�݂̂̍s�j���͒ʏ�N���b�N�Ɠ�������
			if(( ! IsTextSelected() )&&( m_ptCaretPos_PHY.y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() )){
				BeginSelectArea();				// ���݂̃J�[�\���ʒu����I�����J�n����
				m_bBeginLineSelect = false;		// �s�P�ʑI�� OFF
			}
		}else
		/* �I���J�n���� */
		/* SHIFT�L�[��������Ă����� */
		if(GetKeyState_Shift()){
			if( IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
				if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( true );
					/* ���݂̃J�[�\���ʒu����I�����J�n���� */
					BeginSelectArea( );
				}
				else{
				}
			}
			else{
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				BeginSelectArea( );
			}

			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}
		else{
			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( true );
			}
			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea( );
		}


		/******* ���̎��_�ŕK�� true == IsTextSelected() �̏�ԂɂȂ� ****:*/
		if( !IsTextSelected() ){
			WarningMessage( m_hWnd, _T("�o�O���Ă�") );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_ptCaretPos.x,	// �J�[�\���ʒuX
			m_ptCaretPos.y	// �J�[�\���ʒuY
		);


		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );


		// CTRL�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.10.10 nasukoji	�g���v���N���b�N�Ή�
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			m_bBeginWordSelect = true;		/* �P��P�ʑI�� */
			if( !IsTextSelected() ){
				/* ���݈ʒu�̒P��I�� */
				if ( Command_SELECTWORD() ){
					m_sSelectBgn.m_ptFrom.y = m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s(���_) */
					m_sSelectBgn.m_ptFrom.x = m_sSelect.m_ptFrom.x;	/* �͈͑I���J�n��(���_) */
					m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;		/* �͈͑I���J�n�s(���_) */
					m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;		/* �͈͑I���J�n��(���_) */
				}
			}else{

				/* �I��̈�`�� */
				DrawSelectArea();


				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptFrom.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptFrom.y, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );


						nWork = IsCurrentPositionSelected(
							nColmFrom,	// �J�[�\���ʒuX
							nLineFrom	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptFrom.y = nLineFrom;
							m_sSelect.m_ptFrom.x = nColmFrom;
							if( 1 == nWorkRel ){
								m_sSelectBgn.m_ptFrom.y = nLineFrom;	/* �͈͑I���J�n�s(���_) */
								m_sSelectBgn.m_ptFrom.x = nColmFrom;	/* �͈͑I���J�n��(���_) */
								m_sSelectBgn.m_ptTo.y = nLineTo;		/* �͈͑I���J�n�s(���_) */
								m_sSelectBgn.m_ptTo.x = nColmTo;		/* �͈͑I���J�n��(���_) */
							}
						}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptTo.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptTo.y, nIdx,
						&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );

						nWork = IsCurrentPositionSelected(
							nColmFrom,	// �J�[�\���ʒuX
							nLineFrom	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptTo.y = nLineFrom;
							m_sSelect.m_ptTo.x = nColmFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							nColmTo,	// �J�[�\���ʒuX
							nLineTo		// �J�[�\���ʒuY
						) ){
							m_sSelect.m_ptTo.y = nLineTo;
							m_sSelect.m_ptTo.x = nColmTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_sSelectBgn.m_ptFrom.y = nLineFrom;	/* �͈͑I���J�n�s(���_) */
							m_sSelectBgn.m_ptFrom.x = nColmFrom;	/* �͈͑I���J�n��(���_) */
							m_sSelectBgn.m_ptTo.y = nLineTo;		/* �͈͑I���J�n�s(���_) */
							m_sSelectBgn.m_ptTo.x = nColmTo;		/* �͈͑I���J�n��(���_) */
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* �I��̈�`�� */
				DrawSelectArea();
			}
		}
		// �s�ԍ��G���A���N���b�N����
		// 2007.12.08 nasukoji	�V�t�g�L�[�������Ă���ꍇ�͍s���N���b�N�Ƃ��Ĉ���
		if(( xPos < m_nViewAlignLeft )&& !GetKeyState_Shift() ){
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_bBeginLineSelect = true;

			// 2002.10.07 YAZAKI �܂�Ԃ��s���C���f���g���Ă���Ƃ��ɑI�������������o�O�̑΍�
			// �P�s����ʕ����������ƍ��E�ɃX�N���[�����Ă�������������Ȃ�̂Ō�őS�̂��ĕ`��	// 2008.05.20 ryoji
			bool bDrawSwitchOld = m_bDrawSWITCH;
			BOOL bDrawAfter = FALSE;
			if( bDrawSwitchOld ){
				const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_ptCaretPos.y );
				if( pcLayout ){
					int nColumn = LineIndexToColmn( pcLayout, pcLayout->GetLengthWithoutEOL() );
					bDrawAfter = (nColumn + SCROLLMARGIN_RIGHT >= m_nViewColNum);
					if( bDrawAfter ){
						m_bDrawSWITCH = false;
					}
				}
			}
			Command_GOLINEEND( true, 0 );
			Command_RIGHT( true, false, false );
			if( bDrawSwitchOld && bDrawAfter ){
				m_bDrawSWITCH = true;
				Redraw();
			}

			//	Apr. 14, 2003 genta
			//	�s�ԍ��̉����N���b�N���ăh���b�O���J�n����Ƃ��������Ȃ�̂��C��
			//	�s�ԍ����N���b�N�����ꍇ�ɂ�ChangeSelectAreaByCurrentCursor()�ɂ�
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.y��-1���ݒ肳��邪�A���
			//	Command_GOLINEEND(), Command_RIGHT()�ɂ���čs�I�����s����B
			//	�������L�����b�g�������ɂ���ꍇ�ɂ̓L�����b�g���ړ����Ȃ��̂�
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.y��-1�̂܂܎c���Ă��܂��A���ꂪ
			//	���_�ɐݒ肳��邽�߂ɂ��������Ȃ��Ă����B
			//	�Ȃ̂ŁA�͈͑I�����s���Ă��Ȃ��ꍇ�͋N�_�����̐ݒ���s��Ȃ��悤�ɂ���
			if( IsTextSelected() ){
				m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;	/* �͈͑I���J�n�s(���_) */
				m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;	/* �͈͑I���J�n��(���_) */
			}
		}
		else{
			/* URL���N���b�N���ꂽ��I�����邩 */
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL ){

				int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
				int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				int			nUrlLen;	// URL�̒���(�o�C�g��)
				// �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
				bool bIsUrl = IsCurrentPositionURL(
					m_ptCaretPos.x,	// �J�[�\���ʒuX
					m_ptCaretPos.y,	// �J�[�\���ʒuY
					&nUrlLine,		// URL�̍s(���s�P��)
					&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
					&nUrlLen,		// URL�̒���(�o�C�g��)
					NULL			// URL�󂯎���
				);
				if( bIsUrl ){
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( true );

					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						2002/04/08 YAZAKI �����ł��킩��₷���B
					*/
					CLayoutRange sRangeB;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn          , nUrlLine, &sRangeB.m_ptFrom.x, &sRangeB.m_ptFrom.y );
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn + nUrlLen, nUrlLine, &sRangeB.m_ptTo.x,   &sRangeB.m_ptTo.y );

					m_sSelectBgn = sRangeB;
					m_sSelect = sRangeB;

					/* �I��̈�`�� */
					DrawSelectArea();
				}
			}
		}
	}
}

/*	�w��J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
	�߂�l��TRUE�̏ꍇ�A*ppszURL�͌Ăяo������delete���邱��
	2009.05.27 ryoji URL�F�w��̐��K�\���L�[���[�h�Ƀ}�b�`���镶�����URL�Ƃ݂Ȃ�
	                 URL�̋����\��OFF�̃`�F�b�N�͂��̊֐����ōs���悤�ɕύX
 */
bool CEditView::IsCurrentPositionURL(
		int		nCaretPosX,		// �J�[�\���ʒuX
		int		nCaretPosY,		// �J�[�\���ʒuY
		int*	pnUrlLine,		// URL�̍s(���s�P��)
		int*	pnUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		int*	pnUrlLen,		// URL�̒���(�o�C�g��)
		char**	ppszURL			// URL�󂯎���(�֐�����new����)
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::IsCurrentPositionURL" );

	const char*	pLine;
//	const char*	pLineWork;
	int			nLineLen;
	int			nX;
	int			nY;
//	char*		pURL;
	int			i;
//	BOOL		bFindURL;
	int			nCharChars;
	int			nUrlLen;

	// URL�������\�����邩�ǂ����`�F�b�N����	// 2009.05.27 ryoji
	bool bDispUrl = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp;
	BOOL bUseRegexKeyword = FALSE;
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());
	if( TypeDataPtr->m_bUseRegexKeyword ){
		for( int i = 0; i < MAX_REGEX_KEYWORD; i++ ){
			if( TypeDataPtr->m_RegexKeywordArr[i].m_szKeyword[0] == L'\0' )
				break;
			if( TypeDataPtr->m_RegexKeywordArr[i].m_nColorIndex == COLORIDX_URL ){
				bUseRegexKeyword = TRUE;	// URL�F�w��̐��K�\���L�[���[�h������
				break;
			}
		}
	}
	if( !bDispUrl && !bUseRegexKeyword ){
		return false;	// URL�����\�����Ȃ��̂�URL�ł͂Ȃ�
	}

	// ���K�\���L�[���[�h�iURL�F�w��j�s�����J�n����	// 2009.05.27 ryoji
	if( bUseRegexKeyword ){
		m_cRegexKeyword->RegexKeyLineStart();
	}

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		nCaretPosX,
		nCaretPosY,
		(int*)&nX,
		(int*)&nY
	);
	bool		bMatch;
	int			nMatchColor;
	*pnUrlLine = nY;
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );

	i = t_max(0, nX - _MAX_PATH);	// 2009.05.22 ryoji 200->_MAX_PATH�i�������s�͐��x�ቺ���Ă����\�D��ōs���ȊO����J�n�j
	// nLineLen = t_min(nLineLen, nX + _MAX_PATH);
	bool bKeyWordTop = (i == 0);
	while( i <= nX && i < nLineLen ){
		bMatch = ( bUseRegexKeyword
					&& m_cRegexKeyword->RegexIsKeyword( pLine, i, nLineLen, &nUrlLen, &nMatchColor )
					&& nMatchColor == COLORIDX_URL );
		if( !bMatch ){
			bMatch = ( bDispUrl && bKeyWordTop
						&& IsURL(&pLine[i], (int)(nLineLen - i), &nUrlLen) );	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
		}
		if( bMatch ){
			if( i <= nX && nX < i + nUrlLen ){
				/* URL��Ԃ��ꍇ */
				if( NULL != ppszURL ){
					*ppszURL = new char[nUrlLen + 1];
					memcpy( *ppszURL, &pLine[i], nUrlLen );
					(*ppszURL)[nUrlLen] = '\0';
					/* *ppszURL�͌Ăяo������delete���邱�� */
				}
				*pnUrlLen = nUrlLen;
				*pnUrlLine = nY;
				*pnUrlIdxBgn = i;
				return true;
			}else{
				i += nUrlLen;
				continue;
			}
		}
		nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
		bKeyWordTop = (nCharChars == 2 || !IS_KEYWORD_CHAR(pLine[i]));
		i += nCharChars;
	}
	return false;
}


/*!	�g���v���N���b�N�̃`�F�b�N
	@brief �g���v���N���b�N�𔻒肷��
	
	2��ڂ̃N���b�N����3��ڂ̃N���b�N�܂ł̎��Ԃ��_�u���N���b�N���Ԉȓ��ŁA
	�����̎��̃N���b�N�ʒu�̂��ꂪ�V�X�e�����g���b�N�iX:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK�j�̒l�i�s�N�Z���j�ȉ��̎��g���v���N���b�N�Ƃ���B
	
	@param[in] xPos		�}�E�X�N���b�NX���W
	@param[in] yPos		�}�E�X�N���b�NY���W
	@return		�g���v���N���b�N�̎���TRUE��Ԃ�
	�g���v���N���b�N�łȂ�����FALSE��Ԃ�

	@note	m_dwTripleClickCheck��0�łȂ����Ƀ`�F�b�N���[�h�Ɣ��肷�邪�APC��
			�A���ғ����Ă���ꍇ49.7�����ɃJ�E���^��0�ɂȂ�ׁA�킸���ȉ\��
			�ł��邪�g���v���N���b�N������ł��Ȃ���������B
	
	@date 2007.10.02 nasukoji	�V�K�쐬
	@date 2007.10.11 nasukoji	���s������荶���łȂ��ƃg���v���N���b�N���F������Ȃ��s����C��
	@date 2007.10.13 nasukoji	�s�ԍ��\���G���A�̃g���v���N���b�N�͒ʏ�N���b�N�Ƃ��Ĉ���
*/
BOOL CEditView::CheckTripleClick( int xPos, int yPos )
{

	// �g���v���N���b�N�`�F�b�N�L���łȂ��i�������Z�b�g����Ă��Ȃ��j
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// �O��N���b�N�Ƃ̃N���b�N�ʒu�̂�����Z�o
	int dxpos = m_nMouseRollPosXOld - xPos;
	int dypos = m_nMouseRollPosYOld - yPos;

	if(dxpos < 0)
		dxpos = -dxpos;		// ��Βl��

	if(dypos < 0)
		dypos = -dypos;		// ��Βl��

	// �s�ԍ��\���G���A�łȂ��A���N���b�N�v���X����_�u���N���b�N���Ԉȓ��A
	// ���_�u���N���b�N�̋��e����s�N�Z���ȉ��̂���̎��g���v���N���b�N�Ƃ���
	//	2007.10.12 genta/dskoba �V�X�e���̃_�u���N���b�N���x�C���ꋖ�e�ʂ��擾
	if( (xPos >= m_nViewAlignLeft)&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dxpos <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dypos <= GetSystemMetrics(SM_CYDOUBLECLK)))
	{
		result = TRUE;
	}else{
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}
	
	return result;
}


/* �}�E�X�E�{�^������ */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// �J�[�\���ʒuX
		nNewY		// �J�[�\���ʒuY
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;
}

/* �}�E�X�E�{�^���J�� */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[1]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* �E�N���b�N���j���[ */
//	Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 �}�E�X���{�^���Ή�
/*!
	@brief �}�E�X���{�^�����������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2004.10.11 novice �V�K�쐬
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X���{�^�����J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	
	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnMBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[������
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X���{�^���ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[2]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
}


// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	@brief �}�E�X�T�C�h�{�^��1���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X�T�C�h�{�^��1���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnXLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�T�C�h�{�^��1�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[3]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnXRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		// �z�C�[������ɂ��y�[�W�X�N���[�������OFF
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		// �z�C�[������ɂ�鉡�X�N���[�������OFF
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�T�C�h�{�^��2�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[4]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


VOID CEditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime 	// current system time
	)
{
	POINT		po;
	RECT		rc;

	if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		if( IsDragSource() ){
			return;
		}
	}
	/* �͈͑I�𒆂łȂ��ꍇ */
	if(!m_bBeginSelect){
		if(TRUE == KeyWordHelpSearchDict( LID_SKH_ONTIMER, &po, &rc ) ){	// 2006.04.10 fon
			/* ����Tip��\�� */
			m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
		}
	}else{
		::GetCursorPos( &po );
		::GetWindowRect(m_hWnd, &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

		// 1999.12.18 �N���C�A���g�̈���ł̓^�C�}�[�����h���b�O+���[�����Ȃ�
		return;

// 2001.12.21 hor �ȉ��A���s����Ȃ��̂ŃR�����g�A�E�g���܂� (�s��////�͂��Ƃ��ƃR�����g�s�ł�)
////		rc.top += m_nViewAlignTop;
//		RECT rc2;
//		rc2 = rc;
//		rc2.bottom = rc.top + m_nViewAlignTop + ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//		 && 0 < m_nViewTopLine
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//		rc2 = rc;
//		rc2.top = rc.bottom - ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//			&& m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nViewTopLine + m_nViewRowNum
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//
////		rc.top += 48;
////		rc.bottom -= 48;
////		if( !PtInRect( &rc, po ) ){
////			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
////		}
	}
	return;
}

/*! �L�[���[�h���������̑O������`�F�b�N�ƁA����

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_R���番��
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CMemory		cmemCurText;
	int			i;

	/* �L�[���[�h�w���v���g�p���邩�H */
	if( !m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp )	/* �L�[���[�h�w���v�@�\���g�p���� */	// 2006.04.10 fon
		goto end_of_search;
	/* �t�H�[�J�X�����邩�H */
	if( !(m_nCaretWidth > 0) ) 
		goto end_of_search;
	/* �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H */
	GetCursorPos( po );
	GetWindowRect( m_hWnd, rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* �E�R�����g�̂P�`�R�łȂ��ꍇ */
		if(!( m_bInMenuLoop == FALSE	&&			/* �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
			0 != m_dwTipTimer			&&			/* �Q�D����Tip��\�����Ă��Ȃ� */
			300 < ::GetTickCount() - m_dwTipTimer	/* �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
		//	0 != m_dwTipTimer			&&			/* �Q�D����Tip��\�����Ă��Ȃ� */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
		) )	goto end_of_search;
		break;
	default:
		PleaseReportToAuthor( NULL, _T("CEditView::KeyWordHelpSearchDict\nnID=%d"), (int)nID  );
	}
	/* �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		char* pszWork = cmemCurText.GetStringPtr();
		int nWorkLength	= lstrlen( pszWork );
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == '\0' ||
				pszWork[i] == CR ||
				pszWork[i] == LF ){
				break;
			}
		}
		cmemCurText._SetStringLength( i );
	}
	/* �L�����b�g�ʒu�̒P����擾���鏈�� */	// 2006.03.24 fon
	else if(m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}
	else
		goto end_of_search;

	if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* ���Ɍ����ς݂� */
		(!m_cTipWnd.m_KeyWasHit) )								/* �Y������L�[���Ȃ����� */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* �������s */
	if( FALSE == KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* ����Tip��\�����Ă��� */
	m_poTipCurPos = *po;	/* ���݂̃}�E�X�J�[�\���ʒu */
	return TRUE;			/* �����܂ŗ��Ă���΃q�b�g�E���[�h */

	/* �L�[���[�h�w���v�\�������I�� */
	end_of_search:
	return FALSE;
}

/*! �L�[���[�h���������������C��

	@date 2006.04.10 fon KeyWordHelpSearchDict���番��
*/
BOOL CEditView::KeySearchCore( const CMemory* pcmemCurText )
{
	CMemory*	pcmemRefKey;
	CMemory*	pcmemRefText;
	LPSTR		pszWork;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	int nTypeNo = m_pcEditDoc->GetDocumentType();
	m_cTipWnd.m_cInfo.SetString( "" );	/* tooltip�o�b�t�@������ */
	/* 1�s�ڂɃL�[���[�h�\���̏ꍇ */
	if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpKeyDisp){	/* �L�[���[�h���\������ */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendString( "[ " );
		m_cTipWnd.m_cInfo.AppendString( pcmemCurText->GetStringPtr() );
		m_cTipWnd.m_cInfo.AppendString( " ]" );
	}
	/* �r���܂ň�v���g���ꍇ */
	if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix)
		nCmpLen = lstrlen( pcmemCurText->GetStringPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i=0;i<m_pShareData->m_Types[nTypeNo].m_nKeyHelpNum;i++){	//�ő吔�FMAX_KEYHELP_FILE
		if( 1 == m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_nUse ){
			if(m_cDicMgr.Search( pcmemCurText->GetStringPtr(), nCmpLen, &pcmemRefKey, &pcmemRefText, m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_szPath, &nLine )){	// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)������ǉ�
				/* �Y������L�[������ */
				pszWork = pcmemRefText->GetStringPtr();
				/* �L���ɂȂ��Ă��鎫����S���Ȃ߂āA�q�b�g�̓s�x�����̌p������ */
				if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpAllSearch){	/* �q�b�g�������̎��������� */	// 2006.04.10 fon
					/* �o�b�t�@�ɑO�̃f�[�^���l�܂��Ă�����separator�}�� */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( "\n--------------------\n��" );
					else
						m_cTipWnd.m_cInfo.AppendString( "��" );	/* �擪�̏ꍇ */
					/* �����̃p�X�}�� */
					m_cTipWnd.m_cInfo.AppendString( m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_szPath );
					m_cTipWnd.m_cInfo.AppendString( "\n" );
					/* �O����v�Ńq�b�g�����P���}�� */
					if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* �I��͈͂őO����v���� */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringPtr() );
						m_cTipWnd.m_cInfo.AppendString( " >>\n" );
					}/* ���������u�Ӗ��v��}�� */
					m_cTipWnd.m_cInfo.AppendString( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* �^�O�W�����v�p�̏����c�� */
					if(FALSE == m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* �������J���Ƃ��ŏ��Ƀq�b�g�����������J�� */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}else{	/* �ŏ��̃q�b�g���ڂ̂ݕԂ��ꍇ */
					/* �L�[���[�h�������Ă�����separator�}�� */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( "\n--------------------\n" );
					/* �O����v�Ńq�b�g�����P���}�� */
					if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* �I��͈͂őO����v���� */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringPtr() );
						m_cTipWnd.m_cInfo.AppendString( " >>\n" );
					}/* ���������u�Ӗ��v��}�� */
					m_cTipWnd.m_cInfo.AppendString( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* �^�O�W�����v�p�̏����c�� */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if(m_cTipWnd.m_KeyWasHit == TRUE){
			return TRUE;
	}
	/* �Y������L�[���Ȃ������ꍇ */
	return FALSE;
}


/* �}�E�X�ړ��̃��b�Z�[�W���� */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	int			nSelectLineFrom_Old;
	int			nSelectColmFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColmTo_Old;
	int			nSelectLineFrom;
	int			nSelectColmFrom;
	int			nSelectLineTo;
	int			nSelectColmTo;

	nSelectLineFrom_Old		= m_sSelect.m_ptFrom.y;
	nSelectColmFrom_Old		= m_sSelect.m_ptFrom.x;
	nSelectLineTo_Old		= m_sSelect.m_ptTo.y;
	nSelectColmTo_Old		= m_sSelect.m_ptTo.x;

	if( !m_bBeginSelect ){	/* �͈͑I�� */
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: �⊮�E�B���h�E���\������Ă��Ȃ�
		if(!m_bHokan){
			/* ����Tip���N������Ă��� */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* ����Tip������ */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
			}
		}
		/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
		int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		int			nUrlLen;	// URL�̒���(�o�C�g��)


		/* �I���e�L�X�g�̃h���b�O���� */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
				/* ���W�w��ɂ��J�[�\���ړ� */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}
		else{
			/* �s�I���G���A? */
			if( xPos < m_nViewAlignLeft || yPos < m_nViewAlignTop ){	//	2002/2/10 aroka
				/* ���J�[�\�� */
				if( yPos >= m_nViewAlignTop )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
			 && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource /* OLE�ɂ��h���b�O���ɂ��邩 */
			 && 0 == IsCurrentPositionSelected(						/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
				nNewX,	// �J�[�\���ʒuX
				nNewY	// �J�[�\���ʒuY
				)
			){
				/* ���J�[�\�� */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			/* �J�[�\���ʒu��URL���L��ꍇ */
			else if(
				IsCurrentPositionURL(
				nNewX,			// �J�[�\���ʒuX
				nNewY,			// �J�[�\���ʒuY
				&nUrlLine,		// URL�̍s(���s�P��)
				&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				&nUrlLen,		// URL�̒���(�o�C�g��)
				NULL			// URL�󂯎���
			) ){
				/* ��J�[�\�� */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* �A�C�r�[�� */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
		/* ���W�w��ɂ��J�[�\���ړ� */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	}
	else{
		/* ���W�w��ɂ��J�[�\���ړ� */
		if(( xPos < m_nViewAlignLeft || m_dwTripleClickCheck )&& m_bBeginLineSelect ){		// 2007.10.02 nasukoji	�s�P�ʑI��
			// 2007.10.13 nasukoji	������̍s�I�������}�E�X�J�[�\���̈ʒu�̍s���I�������悤�ɂ���
			int nNewY = yPos;
			int nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;		// 1�s�̍���

			// �I���J�n�s�ȉ��ւ̃h���b�O����1�s���ɃJ�[�\�����ړ�����
			if( m_nViewTopLine + (yPos - m_nViewAlignTop) / nLineHeight >= m_sSelectBgn.m_ptTo.y )
				nNewY += nLineHeight;

			// �J�[�\�����ړ�
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , nNewY );

			// 2007.10.13 nasukoji	2.5�N���b�N�ɂ��s�P�ʂ̃h���b�O
			if( m_dwTripleClickCheck ){
				int nSelectStartLine = m_sSelectBgn.m_ptTo.y;

				// �I���J�n�s�ȏ�Ƀh���b�O����
				if( m_ptCaretPos.y <= nSelectStartLine ){
					Command_GOLINETOP( true, 0x09 );	// ���s�P�ʂ̍s���ֈړ�
				}else{
					int nCaretPosX;
					int nCaretPosY;

					int nCaretPrevPosX_PHY;
					int nCaretPrevPosY_PHY = m_ptCaretPos_PHY.y;

					// �I���J�n�s��艺�ɃJ�[�\�������鎞��1�s�O�ƕ����s�ԍ��̈Ⴂ���`�F�b�N����
					// �I���J�n�s�ɃJ�[�\�������鎞�̓`�F�b�N�s�v
					if(m_ptCaretPos.y > nSelectStartLine){
						// 1�s�O�̕����s���擾����
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							0, m_ptCaretPos.y - 1, &nCaretPrevPosX_PHY, &nCaretPrevPosY_PHY
						);
					}

					// �O�̍s�Ɠ��������s
					if( nCaretPrevPosY_PHY == m_ptCaretPos_PHY.y ){
						// 1�s��̕����s���烌�C�A�E�g�s�����߂�
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
							0, m_ptCaretPos_PHY.y + 1, &nCaretPosX, &nCaretPosY
						);

						// �J�[�\�������̕����s���ֈړ�����
						nScrollRowNum = MoveCursor( nCaretPosX, nCaretPosY, true );
					}
				}
			}
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */

		if( !m_bBeginWordSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		}else{
//			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
//			ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_ptCaretPos.x,
				(int)m_ptCaretPos.y,
				(int&)nSelectLineFrom,
				(int&)nSelectColmFrom,
				(int&)nSelectLineTo,
				(int&)nSelectColmTo
			);
			/* �I��͈͂ɕύX�Ȃ� */
			if( nSelectLineFrom_Old == nSelectLineFrom
			 && nSelectColmFrom_Old == nSelectColmFrom
			 && nSelectLineTo_Old == nSelectLineTo
			 && nSelectColmTo_Old == nSelectColmTo
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_ptCaretPos.x,
					(int)m_ptCaretPos.y
				);
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
				/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_ptCaretPos.y, nIdx,
					&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
				){
					/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
					nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
					nColmTo = LineIndexToColmn( pcLayout, nColmTo );

					nWorkF = IsCurrentPositionSelectedTEST(
						nColmFrom,	// �J�[�\���ʒuX
						nLineFrom,	// �J�[�\���ʒuY
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						nColmTo,	// �J�[�\���ʒuX
						nLineTo,	// �J�[�\���ʒuY
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* �n�_���O���Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}
					else if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* �I�_������Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}
					else if( nSelectLineFrom_Old == nSelectLineFrom
					 && nSelectColmFrom_Old == nSelectColmFrom
					){
						/* �n�_�����ύX���O���ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}
					else if( nSelectLineTo_Old == nSelectLineTo
					 && nSelectColmTo_Old == nSelectColmTo
					){
						/* �I�_�����ύX������ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}
				}else{
					/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
					ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
				}
			}else{
				/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
				ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* �}�E�X�z�C�[���̃��b�Z�[�W����
*/
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE( _T("CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos );

	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	// 2009.01.12 nasukoji	�z�C�[���X�N���[���𗘗p�����y�[�W�X�N���[���E���X�N���[���Ή�
	if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel ) ){				// �y�[�W�X�N���[���H
		if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel ) ){		// ���X�N���[���H
			int line = m_nViewLeftCol + (( nScrollCode == SB_LINEUP ) ? -m_nViewColNum : m_nViewColNum );
			SyncScrollH( ScrollAtH( line ) );

			// �z�C�[������ɂ�鉡�X�N���[������
			m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
		}else{
			int line = m_nViewTopLine + (( nScrollCode == SB_LINEUP ) ? -m_nViewRowNum : m_nViewRowNum );
			SyncScrollV( ScrollAtV( line ) );
		}
		// �z�C�[������ɂ��y�[�W�X�N���[������
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( TRUE );
	}else{
		/* �}�E�X�z�C�[���ɂ��X�N���[���s�������W�X�g������擾 */
		nRollLineNum = 6;
		/* ���W�X�g���̑��݃`�F�b�N */
		// 2006.06.03 Moca ReadRegistry �ɏ�������
		unsigned int uDataLen;	// size of value data
		char szValStr[256];
		uDataLen = sizeof(szValStr) - 1;
		if( ReadRegistry( HKEY_CURRENT_USER, "Control Panel\\desktop", "WheelScrollLines", szValStr, uDataLen ) ){
			nRollLineNum = ::atoi( szValStr );
		}
		if( -1 == nRollLineNum ){/* �u1��ʕ��X�N���[������v */
			nRollLineNum = m_nViewRowNum;	// �\����̍s��
		}else{
			if( nRollLineNum < 1 ){
				nRollLineNum = 1;
			}
			if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10��30�ցB
				nRollLineNum = 30;
			}
		}

		// 2009.01.12 nasukoji	�L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������
		bool bHorizontal = IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel );
		int nCount = ( nScrollCode == SB_LINEUP ) ? -1 : 1;		// �X�N���[����

		for( i = 0; i < nRollLineNum; ++i ){

			//	Sep. 11, 2004 genta �����X�N���[���s��
			if( bHorizontal ){
				SyncScrollH( ScrollAtH( m_nViewLeftCol + nCount ) );

				// �z�C�[������ɂ�鉡�X�N���[������
				m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
			}else{
				SyncScrollV( ScrollAtV( m_nViewTopLine + nCount ) );
			}
		}
	}
	return 0;
}


/*!
	@brief �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷��

	�}�E�X�z�C�[�����A�s�X�N���[�����ׂ����y�[�W�X�N���[���E���X�N���[��
	���ׂ����𔻒肷��B
	���݂̃L�[�܂��̓}�E�X��Ԃ������Ŏw�肳�ꂽ�g�ݍ��킹�ɍ��v����ꍇ
	true��Ԃ��B

	@param nSelect	[in] �L�[�E�}�E�X�{�^���̑g�ݍ��킹�w��ԍ�

	@return �y�[�W�X�N���[���܂��͉��X�N���[�����ׂ���Ԃ̎�true��Ԃ�
	        �ʏ�̍s�X�N���[�����ׂ���Ԃ̎�false��Ԃ�

	@date 2009.01.12 nasukoji	�V�K�쐬
*/
bool CEditView::IsSpecialScrollMode( int nSelect )
{
	bool bSpecialScrollMode;

	switch( nSelect ){
	case 0:		// �w��̑g�ݍ��킹�Ȃ�
		bSpecialScrollMode = false;
		break;

	case MOUSEFUNCTION_CENTER:		// �}�E�X���{�^��
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_MBUTTON ) ) );
		break;

	case MOUSEFUNCTION_LEFTSIDE:	// �}�E�X�T�C�h�{�^��1
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON1 ) ) );
		break;

	case MOUSEFUNCTION_RIGHTSIDE:	// �}�E�X�T�C�h�{�^��2
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON2 ) ) );
		break;

	case VK_CONTROL:	// Control�L�[
		bSpecialScrollMode = GetKeyState_Control();
		break;

	case VK_SHIFT:		// Shift�L�[
		bSpecialScrollMode = GetKeyState_Shift();
		break;

	default:	// ��L�ȊO�i�����ɂ͗��Ȃ��j
		bSpecialScrollMode = false;
		break;
	}

	return bSpecialScrollMode;
}





/* ���݂̃J�[�\���ʒu����I�����J�n���� */
void CEditView::BeginSelectArea( void )
{
	m_sSelectBgn.m_ptFrom.y = m_ptCaretPos.y;/* �͈͑I���J�n�s(���_) */
	m_sSelectBgn.m_ptFrom.x = m_ptCaretPos.x;/* �͈͑I���J�n��(���_) */
	m_sSelectBgn.m_ptTo.y = m_ptCaretPos.y;	/* �͈͑I���J�n�s(���_) */
	m_sSelectBgn.m_ptTo.x = m_ptCaretPos.x;	/* �͈͑I���J�n��(���_) */

	m_sSelect.m_ptFrom.y = m_ptCaretPos.y;	/* �͈͑I���J�n�s */
	m_sSelect.m_ptFrom.x = m_ptCaretPos.x;	/* �͈͑I���J�n�� */
	m_sSelect.m_ptTo.y = m_ptCaretPos.y;		/* �͈͑I���I���s */
	m_sSelect.m_ptTo.x = m_ptCaretPos.x;		/* �͈͑I���I���� */
	return;
}





/* ���݂̑I��͈͂��I����Ԃɖ߂� */
void CEditView::DisableSelectArea( bool bDraw )
{
	m_sSelectOld.m_ptFrom.y = m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s */
	m_sSelectOld.m_ptFrom.x = m_sSelect.m_ptFrom.x;	/* �͈͑I���J�n�� */
	m_sSelectOld.m_ptTo.y = m_sSelect.m_ptTo.y;		/* �͈͑I���I���s */
	m_sSelectOld.m_ptTo.x = m_sSelect.m_ptTo.x;		/* �͈͑I���I���� */
//	m_sSelect.m_ptFrom.y = 0;
//	m_sSelect.m_ptFrom.x = 0;
//	m_sSelect.m_ptTo.y = 0;
//	m_sSelect.m_ptTo.x = 0;

	m_sSelect.m_ptFrom.y	= -1;
	m_sSelect.m_ptFrom.x	= -1;
	m_sSelect.m_ptTo.y		= -1;
	m_sSelect.m_ptTo.x		= -1;

	if( bDraw ){
		DrawSelectArea();
		m_bDrawSelectArea = false;	// 02/12/13 ai
	}
	m_bSelectingLock	 = false;	/* �I����Ԃ̃��b�N */
	m_sSelectOld.m_ptFrom.y = 0;		/* �͈͑I���J�n�s */
	m_sSelectOld.m_ptFrom.x = 0; 		/* �͈͑I���J�n�� */
	m_sSelectOld.m_ptTo.y = 0;			/* �͈͑I���I���s */
	m_sSelectOld.m_ptTo.x = 0;			/* �͈͑I���I���� */
	m_bBeginBoxSelect = false;		/* ��`�͈͑I�� */
	m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
	m_bBeginWordSelect = false;		/* �P��P�ʑI�� */

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	m_nCaretPosX_Prev=m_ptCaretPos.x;

	//	From Here Dec. 6, 2000 genta
	//	To Here Dec. 6, 2000 genta

	/* �J�[�\���s�A���_�[���C����ON */
	m_cUnderLine.CaretUnderLineON( bDraw );
	return;
}





/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
void CEditView::ChangeSelectAreaByCurrentCursor( int nCaretPosX, int nCaretPosY )
{
	m_sSelectOld.m_ptFrom.y = m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s */
	m_sSelectOld.m_ptFrom.x = m_sSelect.m_ptFrom.x; 	/* �͈͑I���J�n�� */
	m_sSelectOld.m_ptTo.y = m_sSelect.m_ptTo.y;		/* �͈͑I���I���s */
	m_sSelectOld.m_ptTo.x = m_sSelect.m_ptTo.x;		/* �͈͑I���I���� */

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		nCaretPosX,
		nCaretPosY, 
		m_sSelect.m_ptFrom.y,
		m_sSelect.m_ptFrom.x,
		m_sSelect.m_ptTo.y,
		m_sSelect.m_ptTo.x
	);
	/* �I��̈�̕`�� */
	DrawSelectArea();
	return;
}

/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�) */
void CEditView::ChangeSelectAreaByCurrentCursorTEST(
	int		nCaretPosX,
	int		nCaretPosY,
	int&	nSelectLineFrom,
	int&	nSelectColmFrom,
	int&	nSelectLineTo,
	int&	nSelectColmTo
)
{
	if( m_sSelectBgn.m_ptFrom.y == m_sSelectBgn.m_ptTo.y /* �͈͑I���J�n�s(���_) */
	 && m_sSelectBgn.m_ptFrom.x == m_sSelectBgn.m_ptTo.x ){
		if( nCaretPosY == m_sSelectBgn.m_ptFrom.y
		 && nCaretPosX == m_sSelectBgn.m_ptFrom.x ){
			/* �I������ */
			nSelectLineFrom = -1;
			nSelectColmFrom  = -1;
			nSelectLineTo = -1;
			nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_sSelectBgn.m_ptFrom.y
		 || ( nCaretPosY == m_sSelectBgn.m_ptFrom.y && nCaretPosX < m_sSelectBgn.m_ptFrom.x ) ){
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom = nCaretPosX;
			nSelectLineTo = m_sSelectBgn.m_ptFrom.y;
			nSelectColmTo = m_sSelectBgn.m_ptFrom.x;
		}else{
			nSelectLineFrom = m_sSelectBgn.m_ptFrom.y;
			nSelectColmFrom = m_sSelectBgn.m_ptFrom.x;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}else{
		/* �펞�I��͈͈͓͂̔� */
		if( ( nCaretPosY > m_sSelectBgn.m_ptFrom.y || ( nCaretPosY == m_sSelectBgn.m_ptFrom.y && nCaretPosX >= m_sSelectBgn.m_ptFrom.x ) )
		 && ( nCaretPosY < m_sSelectBgn.m_ptTo.y || ( nCaretPosY == m_sSelectBgn.m_ptTo.y && nCaretPosX < m_sSelectBgn.m_ptTo.x ) )
		){
			nSelectLineFrom = m_sSelectBgn.m_ptFrom.y;
			nSelectColmFrom = m_sSelectBgn.m_ptFrom.x;
			if ( nCaretPosY == m_sSelectBgn.m_ptFrom.y && nCaretPosX == m_sSelectBgn.m_ptFrom.x ){
				nSelectLineTo = m_sSelectBgn.m_ptTo.y;	//	m_sSelectBgn.m_ptTo.y;
				nSelectColmTo = m_sSelectBgn.m_ptTo.x;	//	m_sSelectBgn.m_ptTo.x;
			}
			else {
				nSelectLineTo = nCaretPosY;	//	m_sSelectBgn.m_ptTo.y;
				nSelectColmTo = nCaretPosX;	//	m_sSelectBgn.m_ptTo.x;
			}
		}else
		if( !( nCaretPosY > m_sSelectBgn.m_ptFrom.y || ( nCaretPosY == m_sSelectBgn.m_ptFrom.y && nCaretPosX >= m_sSelectBgn.m_ptFrom.x ) ) ){
			/* �펞�I��͈͂̑O���� */
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom  = nCaretPosX;
			nSelectLineTo = m_sSelectBgn.m_ptTo.y;
			nSelectColmTo = m_sSelectBgn.m_ptTo.x;
		}else{
			/* �펞�I��͈͂̌����� */
			nSelectLineFrom = m_sSelectBgn.m_ptFrom.y;
			nSelectColmFrom = m_sSelectBgn.m_ptFrom.x;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}
	return;

}


/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( _T("OnLBUTTONUP()\n") );

	/* �͈͑I���I�� & �}�E�X�L���v�`���[����� */
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X �L���v�`������� */
		::ReleaseCapture();
		ShowCaret_( m_hWnd ); // 2002/07/22 novice

		m_bBeginSelect = false;

		if( m_sSelect.m_ptFrom.y == m_sSelect.m_ptTo.y &&
			m_sSelect.m_ptFrom.x == m_sSelect.m_ptTo.x
		){
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );

			// �Ί��ʂ̋����\��	// 2007.10.18 ryoji
			DrawBracketPair( false );
			SetBracketPairPos( true );
			DrawBracketPair( true );
		}

		m_cUnderLine.UnLock();
	}
	return;
}


/* ShellExecute���Ăяo���v���V�[�W�� */
/*   �Ăяo���O�� lpParameter �� new ���Ă������� */
static unsigned __stdcall ShellExecuteProc( LPVOID lpParameter )
{
	LPTSTR pszFile = (LPTSTR)lpParameter;
	::ShellExecute( NULL, _T("open"), pszFile, NULL, NULL, SW_SHOW );
	delete []pszFile;
	return 0;
}



/* �}�E�X���{�^���_�u���N���b�N */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
	int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
	int			nUrlLen;	// URL�̒���(�o�C�g��)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���̓`�F�b�N���Ȃ�
	if(! m_dwTripleClickCheck){
		/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
		if(
			IsCurrentPositionURL(
				m_ptCaretPos.x,	// �J�[�\���ʒuX
				m_ptCaretPos.y,	// �J�[�\���ʒuY
				&nUrlLine,		// URL�̍s(���s�P��)
				&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				&nUrlLen,		// URL�̒���(�o�C�g��)
				&pszURL			// URL�󂯎���
			)
		 ){
			char*		pszWork = NULL;
			char*		pszOPEN;

			/* URL���J�� */
			/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
			if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
				pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
				strcpy( pszWork, pszMailTo );
				strcat( pszWork, pszURL );
				pszOPEN = pszWork;
			}else{
				if( _tcsnicmp( pszURL, _T("ttp://"), 6 ) == 0 ){	//�}�~URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 1 + 1 ];
					_tcscpy( pszWork, _T("h") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else if( _tcsnicmp( pszURL, _T("tp://"), 5 ) == 0 ){	//�}�~URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 2 + 1 ];
					_tcscpy( pszWork, _T("ht") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else{
					pszOPEN = pszURL;
				}
			}
			{
				// URL���J��
				// 2009.05.21 syat UNC�p�X����1���ȏ㖳�����ɂȂ邱�Ƃ�����̂ŃX���b�h��
				CWaitCursor cWaitCursor( m_hWnd );	// �J�[�\���������v�ɂ���

				unsigned int nThreadId;
				LPCTSTR szUrl = pszOPEN;
				LPTSTR szUrlDup = new TCHAR[_tcslen( szUrl ) + 1];
				_tcscpy( szUrlDup, szUrl );
				HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, ShellExecuteProc, (LPVOID)szUrlDup, 0, &nThreadId );
				if( hThread != INVALID_HANDLE_VALUE ){
					// ���[�U�[��URL�N���w���ɔ��������ڈ�Ƃ��Ă�����Ƃ̎��Ԃ��������v�J�[�\����\�����Ă���
					// ShellExecute �͑����ɃG���[�I�����邱�Ƃ����傭���傭����̂� WaitForSingleObject �ł͂Ȃ� Sleep ���g�p�iex.���݂��Ȃ��p�X�̋N���j
					// �y�⑫�z������� API �ł��҂��𒷂߁i2�`3�b�j�ɂ���ƂȂ��� Web �u���E�U���N������̋N�����d���Ȃ�͗l�iPC�^�C�v, XP/Vista, IE/FireFox �Ɋ֌W�Ȃ��j
					::Sleep(200);
					::CloseHandle(hThread);
				}else{
					//�X���b�h�쐬���s
					delete[] szUrlDup;
				}
			}
			delete [] pszURL;
			delete [] pszWork;
			return;
		}

		/* GREP�o�̓��[�h�܂��̓f�o�b�O���[�h ���� �}�E�X���{�^���_�u���N���b�N�Ń^�O�W�����v �̏ꍇ */
		//	2004.09.20 naoh �O���R�}���h�̏o�͂���Tagjump�ł���悤��
		if( (m_pcEditDoc->m_bGrepMode || m_pcEditDoc->m_bDebugMode) && m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* �^�O�W�����v�@�\ */
			Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();

	/* �}�E�X���N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[?]�ɓ����Ă��� 2007.10.06 nasukoji */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// ��I����Ԃɂ����㍶�N���b�N�������Ƃɂ���
		// ���ׂđI���̏ꍇ�́A3.5�N���b�N���̑I����ԕێ��ƃh���b�O�J�n����
		// �͈͕ύX�̂��߁B
		// �N�A�h���v���N���b�N�@�\�����蓖�Ă��Ă��Ȃ��ꍇ�́A�_�u���N���b�N
		// �Ƃ��ď������邽�߁B
		if( IsTextSelected() )		// �e�L�X�g���I������Ă��邩
			DisableSelectArea( true );		// ���݂̑I��͈͂��I����Ԃɖ߂�

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
			nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, xPos , yPos );	// �J�[�\�����N���b�N�ʒu�ֈړ�����
		}
	}

	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���������Ŕ�����
	if(m_dwTripleClickCheck){
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF�i����͒ʏ�N���b�N�j
		return;
	}

	// 2007.11.06 nasukoji	�_�u���N���b�N���P��I���łȂ��Ă��g���v���N���b�N��L���Ƃ���
	// 2007.10.02 nasukoji	�g���v���N���b�N�`�F�b�N�p�Ɏ������擾
	m_dwTripleClickCheck = ::GetTickCount();

	// �_�u���N���b�N�ʒu�Ƃ��ċL��
	m_nMouseRollPosXOld = xPos;			// �}�E�X�͈͑I��O��ʒu(X���W)
	m_nMouseRollPosYOld = yPos;			// �}�E�X�͈͑I��O��ʒu(Y���W)

	/*	2007.07.09 maru �@�\�R�[�h�̔����ǉ�
		�_�u���N���b�N����̃h���b�O�ł͒P��P�ʂ͈̔͑I��(�G�f�B�^�̈�ʓI����)�ɂȂ邪
		���̓���́A�_�u���N���b�N���P��I����O��Ƃ������́B
		�L�[���蓖�Ă̕ύX�ɂ��A�_�u���N���b�N���P��I���̂Ƃ��ɂ� m_bBeginWordSelect = true
		�ɂ���ƁA�����̓��e�ɂ���Ă͕\�������������Ȃ�̂ŁA�����Ŕ�����悤�ɂ���B
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
	m_bBeginSelect = true;				/* �͈͑I�� */
	m_bBeginBoxSelect = false;			/* ��`�͈͑I�𒆂łȂ� */
	m_bBeginLineSelect = false;			/* �s�P�ʑI�� */
	m_bBeginWordSelect = true;			/* �P��P�ʑI�� */

	if( m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		/* ALT�L�[��������Ă����� */
		if(GetKeyState_Alt()){
			m_bBeginBoxSelect = true;	/* ��`�͈͑I�� */
		}
	}
	::SetCapture( m_hWnd );
	HideCaret_( m_hWnd ); // 2002/07/22 novice
	if( IsTextSelected() ){
		/* �펞�I��͈͈͂̔� */
		m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;
		m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;
	}else{
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
	}

	return;
}





/* �J�[�\���㉺�ړ����� */
int CEditView::Cursor_UPDOWN( int nMoveLines, int bSelect )
{
	const char*		pLine;
	int				nLineLen;
	int				nPosX = 0;
	int				nPosY = m_ptCaretPos.y;
	int				i;
	int				nLineCols;
	int				nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = 0;
	if( nMoveLines > 0 ){
		/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
		if( m_ptCaretPos.y + nMoveLines >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			nMoveLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - m_ptCaretPos.y  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = LineIndexToColmn( pcLayout, nLineLen );
				/* ���s�ŏI����Ă��邩 */
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				if( ( EOL_NONE != pcLayout->m_cEol )
//				if( ( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' )
				 || nLineCols >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()
				){
					if( bSelect ){
						if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̃J�[�\���ʒu����I�����J�n���� */
							BeginSelectArea();
						}
					}else{
						if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̑I��͈͂��I����Ԃɖ߂� */
							DisableSelectArea( true );
						}
					}
					nPosX = 0;
					++nPosY;
					nScrollLines = MoveCursor( nPosX, nPosY, m_bDrawSWITCH /* true */ ); // YAZAKI.
					if( bSelect ){
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nPosX, nPosY );
					}
				}
			}
			//	Sep. 11, 2004 genta �����X�N���[���̊֐���
			//	MoveCursor�ŃX�N���[���ʒu�����ς�
			//SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}else{
		/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
		if( m_ptCaretPos.y + nMoveLines < 0 ){
			nMoveLines = - m_ptCaretPos.y;
		}
		if( nMoveLines >= 0 ){
			//	Sep. 11, 2004 genta �����X�N���[���̊֐���
			SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );
		}
	}
	/* ���̍s�̃f�[�^���擾 */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y + nMoveLines, &nLineLen, &pcLayout );
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			i = nLineLen;
			break;
		}
		if( it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev ){
			i = it.getIndex();
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn();
	if ( it.end() ){
		i = it.getIndex();
	}

	if( i >= nLineLen ){
		/* �t���[�J�[�\�����[�h�� */
		if( m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode
		 || IsTextSelected() && m_bBeginBoxSelect	/* ��`�͈͑I�� */
		){
			if( m_ptCaretPos.y + nMoveLines + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
						nPosX = m_nCaretPosX_Prev;
					}
				}
			}else{
				nPosX = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( nPosX, m_ptCaretPos.y + nMoveLines, m_bDrawSWITCH /* true */ ); // YAZAKI.
	if( bSelect ){
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelected() && -1 != m_nOldUnderLineY ){
//			HDC		hdc;
//			HPEN	hPen, hPenOld;
//			hdc = ::GetDC( m_hWnd );
//			/* �J�[�\���s�A���_�[���C���̏��� */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//			m_nOldUnderLineY = -1;
//			::ReleaseDC( m_hWnd, hdc );
//		}
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
//		ChangeSelectAreaByCurrentCursor( nPosX, m_ptCaretPos.y + nMoveLines );
		ChangeSelectAreaByCurrentCursor( nPosX, m_ptCaretPos.y );
	}

	return nScrollLines;
}





/*! �w���[�s�ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[�������s�� (��:������/��:�����)

	@date 2004.09.11 genta �s����߂�l�Ƃ��ĕԂ��悤�ɁD(�����X�N���[���p)
*/
int CEditView::ScrollAtV( int nPos )
{
	int			nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 ) - m_nViewRowNum;
		if( nPos < 0 ){
			nPos = 0;
		}
	}
	if( m_nViewTopLine == nPos ){
		return 0;	//	�X�N���[�������B
	}
	/* �����X�N���[���ʁi�s���j�̎Z�o */
	nScrollRowNum = m_nViewTopLine - nPos;

	/* �X�N���[�� */
	if( abs( nScrollRowNum ) >= m_nViewRowNum ){
		m_nViewTopLine = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				m_nViewCy + m_nViewAlignTop -
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top = m_nViewAlignTop;
			rcClip.bottom =
				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		}else
		if( nScrollRowNum < 0 ){
			rcScrol.top =
				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top =
				m_nViewCy + m_nViewAlignTop +
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
		}
		if( m_bDrawSWITCH ){
			::ScrollWindowEx(
				m_hWnd,
				0,	/* �����X�N���[���� */
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* �����X�N���[���� */
				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			// �݊�BMP�̃X�N���[��
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left,
					rcScrol.top + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),
					rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top, SRCCOPY
				);
			}
			// To Here 2007.09.09 Moca
			::InvalidateRect( m_hWnd, &rcClip, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();

	return -nScrollRowNum;	//�������t�Ȃ̂ŕ������]���K�v
}




/*! �w�荶�[���ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[���������� (��:�E����/��:������)

	@date 2004.09.11 genta ������߂�l�Ƃ��ĕԂ��悤�ɁD(�����X�N���[���p)
	@date 2008.06.08 ryoji �����X�N���[���͈͂ɂԂ牺���]����ǉ�
	@date 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����E�ɍs���߂��Ȃ��悤�ɂ���
*/
int CEditView::ScrollAtH( int nPos )
{
	int			nScrollColNum;
	RECT		rcScrol;
	RECT		rcClip2;
	if( nPos < 0 ){
		nPos = 0;
	}else
	//	Aug. 18, 2003 ryoji �ϐ��̃~�X���C��
	//	�E�B���h�E�̕�������߂ċ��������Ƃ��ɕҏW�̈悪�s�ԍ����痣��Ă��܂����Ƃ��������D
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( GetRightEdgeForScrollBar() - m_nViewColNum < nPos ){
		nPos = GetRightEdgeForScrollBar() - m_nViewColNum ;
		//	May 29, 2004 genta �܂�Ԃ������E�B���h�E�����傫���Ƃ���WM_HSCROLL�������
		//	nPos�����̒l�ɂȂ邱�Ƃ�����C���̏ꍇ�ɃX�N���[���o�[����ҏW�̈悪
		//	����Ă��܂��D
		if( nPos < 0 )
			nPos = 0;
	}
	if( m_nViewLeftCol == nPos ){
		return 0;
	}
	/* �����X�N���[���ʁi�������j�̎Z�o */
	nScrollColNum = m_nViewLeftCol - nPos;

	/* �X�N���[�� */
	if( abs( nScrollColNum ) >= m_nViewColNum /*|| abs( nScrollRowNum ) >= m_nViewRowNum*/ ){
		m_nViewLeftCol = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollColNum > 0 ){
			rcScrol.left = m_nViewAlignLeft;
			rcScrol.right =
				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left = m_nViewAlignLeft;
			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}else
		if( nScrollColNum < 0 ){
			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left =
				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}
		m_nViewLeftCol = nPos;
		if( m_bDrawSWITCH ){
			::ScrollWindowEx(
				m_hWnd,
				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* �����X�N���[���� */
				0,	/* �����X�N���[���� */
				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			// �݊�BMP�̃X�N���[��
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
						rcScrol.top, rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top , SRCCOPY
				);
			}
			// �J�[�\���̏c�����e�L�X�g�ƍs�ԍ��̌��Ԃɂ���Ƃ��A�X�N���[�����ɏc���̈���X�V
			if( m_nOldCursorLineX == m_nViewAlignLeft - 1 ){
				RECT rcClip3;
				rcClip3.left = m_nOldCursorLineX;
				rcClip3.right = m_nOldCursorLineX + 1;
				rcClip3.top  = m_nViewAlignTop;
				rcClip3.bottom = m_nViewCy + m_nViewAlignTop;
				::InvalidateRect( m_hWnd, &rcClip3, TRUE );
			}
			// To Here 2007.09.09 Moca
			::InvalidateRect( m_hWnd, &rcClip2, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}
	//	2006.1.28 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
	// ���AdjustScrollBars���Ă�ł��܂��ƁA��x�ڂ͂����܂ł��Ȃ��̂ŁA
	// DispRuler���Ă΂�Ȃ��B���̂��߁A���������ւ����B
	m_bRedrawRuler = true; // ���[���[���ĕ`�悷��B
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();

	return -nScrollColNum;	//�������t�Ȃ̂ŕ������]���K�v
}

/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������
	
	@param line [in] �X�N���[���s�� (��:������/��:�����/0:�������Ȃ�)
	
	@author asa-o
	@date 2001.06.20 asa-o �V�K�쐬
	@date 2004.09.11 genta �֐���

	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������

*/
void CEditView::SyncScrollV( int line )
{
	if( m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll && line != 0
		&& m_pcEditDoc->IsEnablePane(m_nMyIndex^0x01)
	) {
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditViewArr[m_nMyIndex^0x01];
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		pcEditView -> ScrollByV( line );
#else
		pcEditView -> ScrollAtV( m_nViewTopLine );
#endif
	}
}

/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������D
	
	@param col [in] �X�N���[������ (��:�E����/��:������/0:�������Ȃ�)
	
	@author asa-o
	@date 2001.06.20 asa-o �V�K�쐬
	@date 2004.09.11 genta �֐���

	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������
*/
void CEditView::SyncScrollH( int col )
{
	if( m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll && col != 0
		&& m_pcEditDoc->IsEnablePane(m_nMyIndex^0x02)
	) {
		CEditView*	pcEditView = m_pcEditDoc->m_pcEditViewArr[m_nMyIndex^0x02];
		HDC			hdc = ::GetDC( pcEditView->m_hWnd );
		
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		pcEditView -> ScrollByH( col );
#else
		pcEditView -> ScrollAtH( m_nViewLeftCol );
#endif
		m_bRedrawRuler = true; //2002.02.25 Add By KK �X�N���[�������[���[�S�̂�`���Ȃ����B
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
	}
}

/* �I��͈͂̃f�[�^���擾 */
/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
BOOL CEditView::GetSelectedData(
		CMemory*	cmemBuf,
		BOOL		bLineOnly,
		const char*	pszQuote,			/* �擪�ɕt������p�� */
		BOOL		bWithLineNumber,	/* �s�ԍ���t�^���� */
		bool		bAddCRLFWhenCopy,	/* �܂�Ԃ��ʒu�ŉ��s�L�������� */
//	Jul. 25, 2000 genta
		EEolType	neweol				//	�R�s�[��̉��s�R�[�h EOL_NONE�̓R�[�h�ۑ�
)
{
	const char*		pLine;
	int				nLineLen;
	int				nLineNum;
	int				nIdxFrom;
	int				nIdxTo;
	RECT			rcSel;
	int				nRowNum;
	int				nLineNumCols;
	char*			pszLineNum;
	char*			pszSpaces = "                    ";
	const CLayout*	pcLayout;
	CEol			appendEol( neweol );
	bool			addnl = false;

	/* �͈͑I��������Ă��Ȃ� */
	if( !IsTextSelected() ){
		return FALSE;
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		nLineNumCols = DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new char[nLineNumCols + 1];
	}


	if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
			m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
			m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
			m_sSelect.m_ptTo.x			/* �͈͑I���I���� */
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf->SetString( "" );

		//<< 2002/04/18 Azumaiya
		// �T�C�Y�������v�̂��Ƃ��Ă����B
		// ���\��܂��Ɍ��Ă��܂��B
		int i = rcSel.bottom - rcSel.top;

		// �ŏ��ɍs�����̉��s�ʂ��v�Z���Ă��܂��B
		int nBufSize = strlen(CRLF) * i;

		// ���ۂ̕����ʁB
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
		for(; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext)
		{
			pLine = pcLayout->m_pCDocLine->m_cLine.GetStringPtr() + pcLayout->m_ptLogicPos.x;
			nLineLen = pcLayout->m_nLength;
			if( NULL != pLine )
			{
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				nBufSize += nIdxTo - nIdxFrom;
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ��܂��Ɍ����e�ʂ����ɃT�C�Y�����炩���ߊm�ۂ��Ă����B
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );
				//2002.02.08 hor
				// pLine��NULL�̂Ƃ�(��`�G���A�̒[��EOF�݂̂̍s���܂ނƂ�)�͈ȉ����������Ȃ�
				if( nIdxTo - nIdxFrom > 0 ){
					if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
					}else{
						cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					}
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf->AppendString( CRLF );
				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
					break;
				}
//			}
		}
	}else{
		cmemBuf->SetString( "" );

		//<< 2002/04/18 Azumaiya
		//  ���ꂩ��\��t���Ɏg���̈�̑�܂��ȃT�C�Y���擾����B
		//  ��܂��Ƃ������x���ł��̂ŁA�T�C�Y�v�Z�̌덷���i�e�ʂ𑽂����ς�����Ɂj���\�o��Ǝv���܂����A
		// �܂��A�����D��Ƃ������ƂŊ��ق��Ă��������B
		//  ���ʂȗe�ʊm�ۂ��o�Ă��܂��̂ŁA�����������x���グ�����Ƃ���ł����E�E�E�B
		//  �Ƃ͂����A�t�ɏ��������ς��邱�ƂɂȂ��Ă��܂��ƁA���Ȃ葬�x���Ƃ���v���ɂȂ��Ă��܂��̂�
		// �����Ă��܂��Ƃ���ł����E�E�E�B
		m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptFrom.y, &nLineLen, &pcLayout );
		int nBufSize = 0;
		int i = m_sSelect.m_ptTo.y - m_sSelect.m_ptFrom.y;
		// �擪�Ɉ��p����t����Ƃ��B
		if ( NULL != pszQuote )
		{
			nBufSize += strlen(pszQuote);
		}

		// �s�ԍ���t����B
		if ( bWithLineNumber )
		{
			nBufSize += nLineNumCols;
		}

		// ���s�R�[�h�ɂ��āB
		if ( neweol == EOL_UNKNOWN )
		{
			nBufSize += strlen(CRLF);
		}
		else
		{
			nBufSize += appendEol.GetLen();
		}

		// ���ׂĂ̍s�ɂ��ē��l�̑��������̂ŁA�s���{����B
		nBufSize *= i;

		// ���ۂ̊e�s�̒����B
		for (; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext )
		{
			nBufSize += pcLayout->m_nLength + appendEol.GetLen();
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ���ׂ������������o�b�t�@������Ă����B
		cmemBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for( nLineNum = m_sSelect.m_ptFrom.y; nLineNum <= m_sSelect.m_ptTo.y; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == m_sSelect.m_ptFrom.y ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );
			}else{
				nIdxFrom = 0;
			}
			if( nLineNum == m_sSelect.m_ptTo.y ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxTo = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x );
			}else{
				nIdxTo = nLineLen;
			}
			if( nIdxTo - nIdxFrom == 0 ){
				continue;
			}

#if 0
			//	Jul. 25, 2000	genta
			//	���s�����ύX�̂��ߍ폜
			/* ���s����������񂩂��̂��E�E�E�H */
			if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
				nIdxTo = nLineLen - 1;
			}
#endif

			if( NULL != pszQuote && pszQuote[0] != '\0' ){	/* �擪�ɕt������p�� */
//				cmemBuf->Append( pszQuote, lstrlen( pszQuote ) );
				cmemBuf->AppendString( pszQuote );
			}
			if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
				wsprintf( pszLineNum, " %d:" , nLineNum + 1 );
				cmemBuf->AppendString( pszSpaces, nLineNumCols - lstrlen( pszLineNum ) );
//				cmemBuf->Append( pszLineNum, lstrlen( pszLineNum ) );
				cmemBuf->AppendString( pszLineNum );
			}


			if( EOL_NONE != pcLayout->m_cEol ){
				if( nIdxTo >= nLineLen ){
					cmemBuf->AppendString( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf->AppendString( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->m_cEol).GetValue() :	//	�R�[�h�ۑ�
						appendEol.GetValue() );			//	�V�K���s�R�[�h
				}
				else {
					cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf->AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					//if( nIdxTo - nIdxFrom >= nLineLen ){ // 2010.11.06 ryoji �s���ȊO����̑I������[�܂�Ԃ��ʒu�ɉ��s��t���ăR�s�[]�ōŏ��̐܂�Ԃ��ɉ��s���t���悤��
					if( nIdxTo >= nLineLen ){
					if( bAddCRLFWhenCopy ||  /* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
						NULL != pszQuote || /* �擪�ɕt������p�� */
						bWithLineNumber 	/* �s�ԍ���t�^���� */
					){
//						cmemBuf->Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf->AppendString(( neweol == EOL_UNKNOWN ) ?
							CRLF :						//	�R�[�h�ۑ�
							appendEol.GetValue() );		//	�V�K���s�R�[�h
					}
				}
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		delete [] pszLineNum;
	}
	return TRUE;
}




/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
void CEditView::CopySelectedAllLines(
	const char*	pszQuote,		//!< �擪�ɕt������p��
	BOOL		bWithLineNumber	//!< �s�ԍ���t�^����
)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nSelectLineFrom;	/* �͈͑I���J�n�s */
	int			nSelectColmFrom;	/* �͈͑I���J�n�� */
	int			nSelectLineTo;		/* �͈͑I���I���s */
	int			nSelectColmTo;		/* �͈͑I���I���� */
	CMemory		cmemBuf;

	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	{	// �I��͈͓��̑S�s��I����Ԃɂ���
		nSelectLineFrom = m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s */
		nSelectLineTo = m_sSelect.m_ptTo.y;		/* �͈͑I���I���s */
		const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_sSelect.m_ptFrom.y );
		if( !pcLayout ) return;
		nSelectColmFrom = pcLayout->GetIndent();	/* �͈͑I���J�n�� */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_sSelect.m_ptTo.y );
		if( pcLayout && (m_bBeginBoxSelect || m_sSelect.m_ptTo.x > pcLayout->GetIndent()) ){
			// �I��͈͂����s���܂Ŋg�傷��
			nSelectLineTo++;
			pcLayout = pcLayout->m_pNext;
		}
		nSelectColmTo = pcLayout? pcLayout->GetIndent(): 0;	/* �͈͑I���I���� */
		GetAdjustCursorPos( &nSelectColmTo, &nSelectLineTo );	// EOF�s�𒴂��Ă�������W�C��

		DisableSelectArea( false ); // 2011.06.03 true ��false
		SetSelectArea( nSelectLineFrom, nSelectColmFrom, nSelectLineTo, nSelectColmTo );

		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, false );
		ShowEditCaret();
	}
	/* �ĕ`�� */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left = 0;
	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top = m_nViewAlignTop;
	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	OnPaint( hdc, &ps, FALSE );
	// To Here 2007.09.09 Moca
	::ReleaseDC( m_hWnd, hdc );
	/* �I��͈͂��N���b�v�{�[�h�ɃR�s�[ */
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ�͏I������ */
	if( !GetSelectedData(
		&cmemBuf,
		FALSE,
		pszQuote, /* ���p�� */
		bWithLineNumber, /* �s�ԍ���t�^���� */
		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy /* �܂�Ԃ��ʒu�ɉ��s�L�������� */
	) ){
		ErrorBeep();
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), false );
}

/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
void CEditView::ConvSelectedArea( int nFuncCode )
{
	CMemory		cmemBuf;
	int			nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe = NULL;
//	BOOL		bBoxSelected;
//	HDC			hdc;
//	PAINTSTRUCT	ps;
	RECT		rcSel;

	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			i;
	CMemory*	pcMemDeleted;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nSelectLineFromOld_PHY;			/* �͈͑I���J�n�s(PHY) */
	int			nSelectColFromOld_PHY; 			/* �͈͑I���J�n��(PHY) */

	/* �e�L�X�g���I������Ă��邩 */
	if( !IsTextSelected() ){
		return;
	}

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(	// 2009.07.18 ryoji PHY�ŋL������悤�ɕύX
		m_sSelect.m_ptFrom.x,
		m_sSelect.m_ptFrom.y,
		&nSelectColFromOld_PHY,	/* �͈͑I���J�n��(PHY) */
		&nSelectLineFromOld_PHY	/* �͈͑I���J�n�s(PHY) */
	);


	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_sSelect.m_ptFrom.y,					/* �͈͑I���J�n�s */
			m_sSelect.m_ptFrom.x,					/* �͈͑I���J�n�� */
			m_sSelect.m_ptTo.y,					/* �͈͑I���I���s */
			m_sSelect.m_ptTo.x						/* �͈͑I���I���� */
		);

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( false );	// 2009.07.18 ryoji true -> false �e�s�ɃA���_�[���C�����c����̏C��

		nIdxFrom = 0;
		nIdxTo = 0;
		for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
			const CLayout* pcLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= 0;
				nIdxTo		= 0;
			}
			nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSel.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
				nPosX = LineIndexToColmn( pcLayout, nDelPos );
				nPosY =  nLineNum + 1;
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
						nPosX,
						nPosY,
						&pcOpe->m_ptCaretPos_PHY_Before.x,
						&pcOpe->m_ptCaretPos_PHY_Before.y
					);
				}else{
					pcOpe = NULL;
				}

				pcMemDeleted = new CMemory;
				/* �w��ʒu�̎w�蒷�f�[�^�폜 */
				DeleteData2(
					nPosX/*rcSel.left*/,
					nPosY/*nLineNum + 1*/,
					nDelLen,
					pcMemDeleted,
					pcOpe		/* �ҏW����v�f COpe */
//					FALSE,
//					FALSE
				);
				cmemBuf.SetNativeData( pcMemDeleted );
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
						nPosX,
						nPosY,
						&pcOpe->m_ptCaretPos_PHY_After.x,
						&pcOpe->m_ptCaretPos_PHY_After.y
					);
					/* ����̒ǉ� */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}else{
					delete pcMemDeleted;
					pcMemDeleted = NULL;
				}
				/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
				ConvMemory( &cmemBuf, nFuncCode, rcSel.left );
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
						nPosX,
						nPosY,
						&pcOpe->m_ptCaretPos_PHY_Before.x,
						&pcOpe->m_ptCaretPos_PHY_Before.y
					);
				}
				/* ���݈ʒu�Ƀf�[�^��}�� */
				InsertData_CEditView(
					nPosX,
					nPosY,
					cmemBuf.GetStringPtr(),
					cmemBuf.GetStringLength(),
					&nNewLine,
					&nNewPos,
					pcOpe,
					false	// 2009.07.18 ryoji TRUE -> FALSE �e�s�ɃA���_�[���C�����c����̏C��
				);
				/* �J�[�\�����ړ� */
				MoveCursor( nNewPos, nNewLine, false );
				m_nCaretPosX_Prev = m_ptCaretPos.x;
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* �����̃L�����b�g�ʒu�w */
					pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* �����̃L�����b�g�ʒu�x */
					/* ����̒ǉ� */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}
		}
		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
		MoveCursor( rcSel.left, rcSel.top, true );
		m_nCaretPosX_Prev = m_ptCaretPos.x;

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* ������ */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}else{
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy );

		/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
		ConvMemory( &cmemBuf, nFuncCode, m_sSelect.m_ptFrom.x );

		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
			m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
			m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
			m_sSelect.m_ptTo.x,		/* �͈͑I���I���� */
			NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			cmemBuf.GetStringPtr(),	/* �}������f�[�^ */ // 2002/2/10 aroka CMemory�ύX
			cmemBuf.GetStringLength(),		/* �}������f�[�^�̒��� */ // 2002/2/10 aroka CMemory�ύX
			false
		);

		// From Here 2001.12.03 hor
		//	�I���G���A�̕���
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(	// 2009.07.18 ryoji PHY����߂�
			nSelectColFromOld_PHY,
			nSelectLineFromOld_PHY,
			&m_sSelect.m_ptFrom.x,	/* �͈͑I���J�n�� */
			&m_sSelect.m_ptFrom.y	/* �͈͑I���J�n�s */
		);
		SetSelectArea( m_sSelect.m_ptFrom.y, m_sSelect.m_ptFrom.x, m_ptCaretPos.y, m_ptCaretPos.x );	// 2009.07.25 ryoji
		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, true );
		m_nCaretPosX_Prev = m_ptCaretPos.x;

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* ������ */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		// To Here 2001.12.03 hor
	}
	RedrawAll();	// 2009.07.18 ryoji �Ώۂ���`�������ꍇ���Ō�ɍĕ`�悷��
}


/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
void CEditView::ConvMemory( CMemory* pCMemory, int nFuncCode, int nStartColumn )
{
	switch( nFuncCode ){
	case F_TOLOWER: pCMemory->ToLower(); break;						/* ������ */
	case F_TOUPPER: pCMemory->ToUpper(); break;						/* �啶�� */
	case F_TOHANKAKU: pCMemory->ToHankaku( 0x0 ); break;					/* �S�p�����p */
	case F_TOHANKATA: pCMemory->ToHankaku( 0x01 ); break;					/* �S�p�J�^�J�i�����p�J�^�J�i */	// Aug. 29, 2002 ai
	case F_TOZENEI: pCMemory->ToZenkaku( 2, 0 );				/* 2== �p����p				*/ break;	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOHANEI: pCMemory->ToHankaku( 0x4 );						/* 2== �p����p				*/ break;	/* �S�p�p�������p�p�� */			//July. 30, 2001 Misaka
	case F_TOZENKAKUKATA: pCMemory->ToZenkaku( 0, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKATA: pCMemory->ToZenkaku( 0, 1 );		/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENHIRA: pCMemory->ToZenkaku( 1, 1 );		/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_CODECNV_EMAIL:		pCMemory->JIStoSJIS(); break;		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	case F_CODECNV_EUC2SJIS:	pCMemory->EUCToSJIS(); break;		/* EUC��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODE2SJIS:pCMemory->UnicodeToSJIS(); break;	/* Unicode��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODEBE2SJIS: pCMemory->UnicodeBEToSJIS(); break;	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:	pCMemory->SJIStoJIS();break;		/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC: 	pCMemory->SJISToEUC();break;		/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_UTF82SJIS:	pCMemory->UTF8ToSJIS();break;		/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:	pCMemory->UTF7ToSJIS();break;		/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:	pCMemory->SJISToUTF7();break;		/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:	pCMemory->SJISToUTF8();break;		/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_AUTO2SJIS:	pCMemory->AUTOToSJIS();break;		/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_TABTOSPACE:
		pCMemory->TABToSPACE(
			//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), 
			nStartColumn
		);break;	/* TAB���� */
	case F_SPACETOTAB:	//#### Stonee, 2001/05/27
		pCMemory->SPACEToTAB(
			//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace(),
			nStartColumn
		);
		break;		/* �󔒁�TAB */
	case F_LTRIM:	Command_TRIM2( pCMemory , TRUE  );break;	// 2001.12.03 hor
	case F_RTRIM:	Command_TRIM2( pCMemory , FALSE );break;	// 2001.12.03 hor
	}
	return;

}



/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI CDocLine��
*/
int CEditView::LineColmnToIndex( const CDocLine* pcDocLine, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineColmnToIndex( const CLayout* pcLayout, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}



/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */
/* �w�肳�ꂽ�����A�s���Z���ꍇ��pnLineAllColLen�ɍs�S�̂̕\��������Ԃ� */
/* ����ȊO�̏ꍇ��pnLineAllColLen�ɂO���Z�b�g����
	
	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineColmnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen )
{
	pnLineAllColLen = 0;

	int i2 = 0;
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if( i2 >= pcLayout->GetLength() ){
		nPosX2 += it.getColumn();
		pnLineAllColLen = nPosX2;
	}
	return i2;
}





/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineIndexToColmn( const CLayout* pcLayout, int nIndex )
{
	//	�ȉ��Aiterator��
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}


/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI CDocLine��
*/
int CEditView::LineIndexToColmn( const CDocLine* pcDocLine, int nIndex )
{
	int nPosX2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}



/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
//	HMENU		hMenuTop;
	HMENU		hMenu;
	POINT		po;
//	UINT		fuFlags;
//	int			cMenuItems;
//	int			nPos;
	RECT		rc;
	CMemory		cmemCurText;
	char*		pszWork;
	int			i;
	int			nMenuIdx;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;
//	BOOL		bBool;


	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();

	/* �E�N���b�N���j���[�̒�`�̓J�X�^�����j���[�z���0�Ԗ� */
	nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	//�}�W�b�N�i���o�[�r��	//@@@ 2003.06.13 MIK
//	if( nMenuIdx < 0 || MAX_CUSTOM_MENU	<= nMenuIdx ){
//		return 0;
//	}
//	if( 0 == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
//		return 0;
//	}

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
//			::LoadString( m_hInstance, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( '\0' == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)", szLabel, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] );
			}
			/* �@�\�����p�\�����ׂ� */
			if( IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
//			bBool = ::AppendMenu( hMenu, uFlags, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel2 );
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2, _T("") );

		}
	}

	if( !m_bBeginSelect ){	/* �͈͑I�� */
		if( TRUE == KeyWordHelpSearchDict( LID_SKH_POPUPMENU_R, &po, &rc ) ){	// 2006.04.10 fon
			pszWork = m_cTipWnd.m_cInfo.GetStringPtr();
			// 2002.05.25 Moca &�̍l����ǉ� 
			char*	pszShortOut = new char[160 + 1];
			if( 80 < lstrlen( pszWork ) ){
				char*	pszShort = new char[80 + 1];
				memcpy( pszShort, pszWork, 80 );
				pszShort[80] = '\0';
				dupamp( (const char*)pszShort, pszShortOut );
				delete [] pszShort;
			}else{
				dupamp( (const char*)pszWork, pszShortOut );
			}
			::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, "�L�[���[�h�̐������N���b�v�{�[�h�ɃR�s�[(&K)" );	// 2006.04.10 fon ToolTip���e�𒼐ڕ\������̂���߂�
			delete [] pszShortOut;
			::InsertMenu( hMenu, 1, MF_BYPOSITION, IDM_JUMPDICT, "�L�[���[�h�������J��(&J)" );	// 2006.04.10 fon
			::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		}
	}
	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}

/*! �L�����b�g�̍s���ʒu����уX�e�[�^�X�o�[�̏�ԕ\���̍X�V

	@note �X�e�[�^�X�o�[�̏�Ԃ̕��ѕ��̕ύX�̓��b�Z�[�W����M����
		CEditWnd::DispatchEvent()��WM_NOTIFY�ɂ��e�������邱�Ƃɒ���
	
	@note �X�e�[�^�X�o�[�̏o�͓��e�̕ύX��CEditWnd::OnSize()��
		�J�������v�Z�ɉe�������邱�Ƃɒ���
*/
void CEditView::DrawCaretPosInfo( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	char			szText[64];
	HWND			hwndFrame;
	unsigned char*	pLine;
	int				nLineLen;
	int				nIdxFrom;
	int				nCharChars;
	CEditWnd*		pCEditWnd;
	const CLayout*	pcLayout;
	// 2002.05.26 Moca  gm_pszCodeNameArr_2 ���g��
	LPCTSTR pCodeName = gm_pszCodeNameArr_2[m_pcEditDoc->m_nCharCode];
//	2002/04/08 YAZAKI �R�[�h�̏d�����폜

	hwndFrame = ::GetParent( m_hwndParent );
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	/* �J�[�\���ʒu�̕����R�[�h */
//	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen );
	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );

	//	May 12, 2000 genta
	//	���s�R�[�h�̕\����ǉ�
	//	From Here
	CEol cNlType = m_pcEditDoc->GetNewLineCode();
	const char *nNlTypeName = cNlType.GetName();
	//	To Here

	int nPosX, nPosY;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if (pcLayout && pcLayout->m_ptLogicPos.x){
			char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nLineLen = m_ptCaretPos_PHY.x;
			nPosX = 0;
			int i;
			//	Oct. 4, 2002 genta
			//	�����ʒu�̃J�E���g���@������Ă����̂��C��
			for( i = 0; i < nLineLen; ){
				// 2005-09-02 D.S.Koba GetSizeOfChar
				int nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
				if ( nCharChars == 1 && pLine[i] == TAB ){
					//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
					nPosX += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nPosX );
					++i;
				}
				else {
					nPosX += nCharChars;
					i += nCharChars;
				}
			}
			nPosX ++;	//	�␳
		}
		else {
			nPosX = m_ptCaretPos.x + 1;
		}
		nPosY = m_ptCaretPos_PHY.y + 1;
	}
	else {
		nPosX = m_ptCaretPos.x + 1;
		nPosY = m_ptCaretPos.y + 1;
	}

	/* �X�e�[�^�X���������o�� */
	if( NULL == pCEditWnd->m_hwndStatusBar ){
		/* �E�B���h�E�E��ɏ����o�� */
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ�
		//	From Here
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
			if( nIdxFrom >= nLineLen ){
				/* szText */
				wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );	//Oct. 31, 2000 JEPRO //Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( (char *)pLine, nLineLen, nIdxFrom );
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
				switch( nCharChars ){
				case 1:
					/* szText */
					wsprintf( szText, "%s(%s)   [%02x]%6d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom], nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				case 2:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x]%6d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , nPosY, nPosX);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				case 4:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , nPosY, nPosX);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				default:
					/* szText */
					wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}
			}
		}else{
			/* szText */
			wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
		}
		//	To Here
		//	Dec. 4, 2002 genta ���j���[�o�[�\����CEditWnd���s��
		m_pcEditDoc->m_pcEditWnd->PrintMenubarMessage( szText );
	}else{
		/* �X�e�[�^�X�o�[�ɏ�Ԃ������o�� */
		char	szText_1[64];
		char	szText_3[32]; // szText_2 => szTest_3 �ɕύX 64�o�C�g������Ȃ� 2002.06.05 Moca 
		char	szText_6[16]; // szText_5 => szTest_6 �ɕύX 64�o�C�g������Ȃ� 2002.06.05 Moca
		wsprintf( szText_1, "%5d �s %4d ��", nPosY, nPosX );	//Oct. 30, 2000 JEPRO �疜�s���v���

		nCharChars = 0;
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
			if( nIdxFrom >= nLineLen ){
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( (char *)pLine, nLineLen, nIdxFrom );
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
			}
		}

		if( 1 == nCharChars ){
			wsprintf( szText_3, "%02x  ", pLine[nIdxFrom] );
		}else
		if( 2 == nCharChars ){
			wsprintf( szText_3, "%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1] );
		// 2003.08.26 Moca CR0LF0�p�~�� 4 == nCharChars ���폜
		}else{
			wsprintf( szText_3, "    " );
		}

		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			strcpy( szText_6, "�}��" );
		}else{
			strcpy( szText_6, "�㏑" );
		}
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)_T("") );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 1 | 0, (LPARAM) (LPINT)szText_1 );
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ��D���̔ԍ���1�����炷
		//	From Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 2 | 0, (LPARAM) (LPINT)nNlTypeName );
		//	To Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 3 | 0, (LPARAM) (LPINT)szText_3 );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 4 | 0, (LPARAM) (LPINT)gm_pszCodeNameArr_1[m_pcEditDoc->m_nCharCode] );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 5 | SBT_OWNERDRAW, (LPARAM) (LPINT)_T("") );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 6 | 0, (LPARAM) (LPINT)szText_6 );
	}

	return;
}

/*!	�I��͈͏�񃁃b�Z�[�W�̕\��

	@author genta
	@date 2005.07.09 genta �V�K�쐬
	@date 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍��ǉ�
*/
void CEditView::PrintSelectionInfoMsg(void)
{
	//	�o�͂���Ȃ��Ȃ�v�Z���ȗ�
	if( ! m_pcEditDoc->m_pcEditWnd->SendStatusMessage2IsEffective() )
		return;

	if( ! IsTextSelected() ){
		m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( "" );
		return;
	}

	char msg[128];
	//	From here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	int nLineCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( m_sSelect.m_ptFrom.y >= nLineCount ){	// �擪�s�����݂��Ȃ�
		m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( "" );
		return;
	}
	int select_line;
	if( m_sSelect.m_ptTo.y >= nLineCount ){	// �ŏI�s�����݂��Ȃ�
		select_line = nLineCount - m_sSelect.m_ptFrom.y + 1;
	}
	else {
		select_line = m_sSelect.m_ptTo.y - m_sSelect.m_ptFrom.y + 1;
	}
	//	To here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	if( m_bBeginBoxSelect ){
		//	��`�̏ꍇ�͕��ƍ��������ł��܂���
		int select_col = m_sSelect.m_ptFrom.x - m_sSelect.m_ptTo.x;
		if( select_col < 0 ){
			select_col = -select_col;
		}
		wsprintf( msg, "%d Columns * %d lines selected.",
			select_col, select_line );
			
	}
	else {
		//	�ʏ�̑I���ł͑I��͈͂̒��g�𐔂���
		int select_sum = 0;	//	�o�C�g�����v
		const char *pLine;	//	�f�[�^���󂯎��
		int	nLineLen;		//	�s�̒���
		const CLayout*	pcLayout;

		//	1�s��
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptFrom.y, &nLineLen, &pcLayout );
		if( pLine ){
			//	1�s�����I������Ă���ꍇ
			if( m_sSelect.m_ptFrom.y == m_sSelect.m_ptTo.y ){
				select_sum = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x )
					- LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );
			}
			else {	//	2�s�ȏ�I������Ă���ꍇ
				select_sum = pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen()
					- LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );

				//	GetSelectedData�Ǝ��Ă��邪�C�擪�s�ƍŏI�s�͔r�����Ă���
				//	Aug. 16, 2005 aroka nLineNum��for�ȍ~�ł��g����̂�for�̑O�Ő錾����
				//	VC .NET�ȍ~�ł�Microsoft�g����L���ɂ����W�������VC6�Ɠ������Ƃɒ���
				int nLineNum;
				for( nLineNum = m_sSelect.m_ptFrom.y + 1;
					nLineNum < m_sSelect.m_ptTo.y; ++nLineNum ){
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					//	2006.06.06 ryoji �w��s�̃f�[�^�����݂��Ȃ��ꍇ�̑΍�
					if( NULL == pLine )
						break;
					select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen();
				}

				//	�ŏI�s�̏���
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					int last_line_chars = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x );
					select_sum += last_line_chars;
					if( last_line_chars == 0 ){
						//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}
				else
				{
					//	�ŏI�s����s�Ȃ�
					//	���̍s���s���Ɋ܂߂Ȃ�
					--select_line;
				}
			}
		}

#ifdef _DEBUG
		wsprintf( msg, "%d bytes (%d lines) selected. [%d:%d]-[%d:%d]",
			select_sum, select_line,
			m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y,
			m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y );
#else
		wsprintf( msg, "%d bytes (%d lines) selected.", select_sum, select_line );
#endif
	}
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( msg );
}


/* �ݒ�ύX�𔽉f������ */
void CEditView::OnChangeSetting( void )
{
	RECT		rc;

	m_nTopYohaku = m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace; 		/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;									/* �\����̏�[���W */

	/* ���[���[�\�� */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_sWindow.m_nRulerHeight;	/* ���[���[���� */
	}

	/* �t�H���g�̕ύX */
	SetFont();

	/* �t�H���g���ς���Ă��邩������Ȃ��̂ŁA�J�[�\���ړ� */
	MoveCursor( m_ptCaretPos.x, m_ptCaretPos.y, true );


	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();
	
	//	2007.09.30 genta ��ʃL���b�V���pCompatibleDC��p�ӂ���
	UseCompatibleDC( m_pShareData->m_Common.m_sWindow.m_bUseCompatibleBMP );

	/* �E�B���h�E�T�C�Y�̕ύX���� */
	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	/* �ĕ`�� */
	::InvalidateRect( m_hWnd, NULL, TRUE );

	return;
}




/* �t�H�[�J�X�ړ����̍ĕ`��

	@date 2001/06/21 asa-o �u�X�N���[���o�[�̏�Ԃ��X�V����v�u�J�[�\���ړ��v�폜
*/
void CEditView::RedrawAll( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	/* �ĕ`�� */
	hdc = ::GetDC( m_hWnd );

//	OnKillFocus();

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );
//	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );

	/* �L�����b�g�̕\�� */
	ShowEditCaret();

	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditDoc->UpdateCaption();

	//	Jul. 9, 2005 genta	�I��͈͂̏����X�e�[�^�X�o�[�֕\��
	PrintSelectionInfoMsg();

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	return;
}

// 2001/06/21 Start by asa-o �ĕ`��
void CEditView::Redraw( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( m_hWnd );

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );

	::ReleaseDC( m_hWnd, hdc );

	return;
}
// 2001/06/21 End

/** �s�ԍ��ĕ`��
	@date 2009.03.26 ryoji �V�K�쐬
*/
void CEditView::RedrawLineNumber( void )
{
	//�`��
	PAINTSTRUCT	ps;
	ps.rcPaint.left = 0;
	ps.rcPaint.right = m_nViewAlignLeft;
	ps.rcPaint.top = m_nViewAlignTop;
	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	HDC hdc = GetDC( m_hWnd );
	OnPaint( hdc, &ps, FALSE );
	ReleaseDC( m_hWnd, hdc );
}

/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
void CEditView::CopyViewStatus( CEditView* pView )
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	/* ���͏�� */
	pView->m_ptCaretPos.x 			= m_ptCaretPos.x;			/* �r���[���[����̃J�[�\�����ʒu�i�O�J�n�j*/
	pView->m_nCaretPosX_Prev		= m_nCaretPosX_Prev;	/* �r���[���[����̃J�[�\�����ʒu�i�O�I���W���j*/
	pView->m_ptCaretPos.y				= m_ptCaretPos.y;			/* �r���[��[����̃J�[�\���s�ʒu�i�O�J�n�j*/
	pView->m_ptCaretPos_PHY.x			= m_ptCaretPos_PHY.x;		/* �J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	pView->m_ptCaretPos_PHY.y			= m_ptCaretPos_PHY.y;		/* �J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
//	�L�����b�g�̕��E�����̓R�s�[���Ȃ��B2002/05/12 YAZAKI
//	pView->m_nCaretWidth			= m_nCaretWidth;		/* �L�����b�g�̕� */
//	pView->m_nCaretHeight			= m_nCaretHeight;		/* �L�����b�g�̍��� */

	/* �L�[��� */
	pView->m_bSelectingLock			= m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	pView->m_bBeginSelect			= m_bBeginSelect;		/* �͈͑I�� */
	pView->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* ��`�͈͑I�� */

	pView->m_sSelectBgn.m_ptFrom.y		= m_sSelectBgn.m_ptFrom.y;	/* �͈͑I���J�n�s(���_) */
	pView->m_sSelectBgn.m_ptFrom.x		= m_sSelectBgn.m_ptFrom.x;	/* �͈͑I���J�n��(���_) */
	pView->m_sSelectBgn.m_ptTo.y		= m_sSelectBgn.m_ptTo.y;	/* �͈͑I���J�n�s(���_) */
	pView->m_sSelectBgn.m_ptTo.x		= m_sSelectBgn.m_ptTo.x;	/* �͈͑I���J�n��(���_) */

	pView->m_sSelect.m_ptFrom.y		= m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s */
	pView->m_sSelect.m_ptFrom.x		= m_sSelect.m_ptFrom.x;	/* �͈͑I���J�n�� */
	pView->m_sSelect.m_ptTo.y			= m_sSelect.m_ptTo.y;		/* �͈͑I���I���s */
	pView->m_sSelect.m_ptTo.x			= m_sSelect.m_ptTo.x;		/* �͈͑I���I���� */
	pView->m_sSelectOld.m_ptFrom.y		= m_sSelectOld.m_ptFrom.y;	/* �͈͑I���J�n�s */
	pView->m_sSelectOld.m_ptFrom.x		= m_sSelectOld.m_ptFrom.x;	/* �͈͑I���J�n�� */
	pView->m_sSelectOld.m_ptTo.y		= m_sSelectOld.m_ptTo.y;	/* �͈͑I���I���s */
	pView->m_sSelectOld.m_ptTo.x		= m_sSelectOld.m_ptTo.x;	/* �͈͑I���I���� */
	pView->m_nMouseRollPosXOld		= m_nMouseRollPosXOld;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
	pView->m_nMouseRollPosYOld		= m_nMouseRollPosYOld;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */

	/* ��ʏ�� */
	pView->m_nViewAlignLeft			= m_nViewAlignLeft;		/* �\����̍��[���W */
	pView->m_nViewAlignLeftCols		= m_nViewAlignLeftCols;	/* �s�ԍ���̌��� */
	pView->m_nViewAlignTop			= m_nViewAlignTop;		/* �\����̏�[���W */
//	pView->m_nViewCx				= m_nViewCx;			/* �\����̕� */
//	pView->m_nViewCy				= m_nViewCy;			/* �\����̍��� */
//	pView->m_nViewColNum			= m_nViewColNum;		/* �\����̌��� */
//	pView->m_nViewRowNum			= m_nViewRowNum;		/* �\����̍s�� */
	pView->m_nViewTopLine			= m_nViewTopLine;		/* �\����̈�ԏ�̍s(0�J�n) */
	pView->m_nViewLeftCol			= m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */

	/* �\�����@ */
	pView->m_nCharWidth				= m_nCharWidth;			/* ���p�����̕� */
	pView->m_nCharHeight			= m_nCharHeight;		/* �����̍��� */

	return;
}


/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( NULL != m_pcsbwVSplitBox ){	/* ���������{�b�N�X */
		}else{
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
		}
	}else{
		delete m_pcsbwVSplitBox;	/* ���������{�b�N�X */
		m_pcsbwVSplitBox = NULL;
	}
	if( bHorz ){
		if( NULL != m_pcsbwHSplitBox ){	/* ���������{�b�N�X */
		}else{
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );
		}
	}else{
		delete m_pcsbwHSplitBox;	/* ���������{�b�N�X */
		m_pcsbwHSplitBox = NULL;
	}

	if( bSizeBox ){
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,													/* no extended styles */
			"SCROLLBAR",										/* scroll bar control class */
			(LPSTR) NULL,										/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			m_hWnd,												/* handle of main window */
			(HMENU) NULL,										/* no menu for a scroll bar */
			m_hInstance,										/* instance owning this window */
			(LPVOID) NULL										/* pointer not needed */
		);
	}else{
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,														/* no extended styles */
			"STATIC",												/* scroll bar control class */
			(LPSTR) NULL,											/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			m_hWnd,													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			m_hInstance,											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	return;
}





/*! Grep���s

  @param[in] pcmGrepKey �����p�^�[��
  @param[in] pcmGrepFile �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��))
  @param[in] pcmGrepFolder �����Ώۃt�H���_

  @date 2008.12.07 nasukoji	�t�@�C�����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2008.12.13 genta �����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2012.10.13 novice �����I�v�V�������N���X���Ƒ��
*/
DWORD CEditView::DoGrep(
	const CMemory*			pcmGrepKey,
	const CMemory*			pcmGrepFile,
	const CMemory*			pcmGrepFolder,
	BOOL					bGrepSubFolder,
	const SSearchOption&	sSearchOption,
	ECodeType				nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
	BOOL					bGrepOutputLine,
	int						nGrepOutputStyle
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif

	// �ē��s��
	if( m_pcEditDoc->m_bGrepRunning ){
		assert( false == m_pcEditDoc->m_bGrepRunning );
		return 0xffffffff;
	}

	m_pcEditDoc->m_bGrepRunning = TRUE;

	int			nHitCount = 0;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	HWND		hwndMainFrame;
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	cRegexp;
	CMemory		cmemMessage;
	int			nWork;
	int*		pnKey_CharCharsArr = NULL;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	cmemMessage.AllocStringBuffer( 4000 );

	m_bDoing_UndoRedo		= TRUE;


	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
//@@@2002.2.2 YAZAKI NULL����Ȃ��Ɛi�܂Ȃ��̂ŁA�Ƃ肠�����R�����g�B��NULL�̂Ƃ��́Anew COpeBlk����B
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		m_pcOpeBlk = new COpeBlk;
	}

	m_bCurSrchKeyMark = true;								/* ����������̃}�[�N */
	strcpy( m_szCurSrchKey, pcmGrepKey->GetStringPtr() );	/* ���������� */
	m_sCurSearchOption = sSearchOption;						// �����I�v�V����

	/* ���K�\�� */

	//	From Here Jun. 27 genta
	/*
		Grep���s���ɓ������Č����E��ʐF�����p���K�\���o�b�t�@��
		����������D�����Grep�������ʂ̐F�������s�����߁D

		Note: �����ŋ�������͍̂Ō�̌���������ł�����
		Grep�Ώۃp�^�[���ł͂Ȃ����Ƃɒ���
	*/
	if( m_sCurSearchOption.bRegularExp ){
		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
			m_pcEditDoc->m_bGrepRunning = FALSE;
			m_bDoing_UndoRedo = FALSE;
			return 0;
		}

		/* �����p�^�[���̃R���p�C�� */
		int nFlag = 0x00;
		nFlag |= m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}
	//	To Here Jun. 27 genta

	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, " " );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca

	//	2008.12.13 genta �p�^�[������������ꍇ�͓o�^���Ȃ�
	//	(���K�\�����r���œr�؂��ƍ���̂�)
	if( pcmGrepKey->GetStringLength() < sizeof( m_pcEditDoc->m_szGrepKey )){
		strcpy( m_pcEditDoc->m_szGrepKey, pcmGrepKey->GetStringPtr() );
	}
	m_pcEditDoc->m_bGrepMode = true;

	//	2007.07.22 genta
	//	�o�[�W�����ԍ��擾�̂��߁C������O�̕��ֈړ�����
	if( sSearchOption.bRegularExp ){
		if( !InitRegexp( m_hWnd, cRegexp, true ) ){
			m_pcEditDoc->m_bGrepRunning = FALSE;
			m_bDoing_UndoRedo = FALSE;
			return 0;
		}
		/* �����p�^�[���̃R���p�C�� */
		int nFlag = 0x00;
		nFlag |= sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( !cRegexp.Compile( pcmGrepKey->GetStringPtr(), nFlag ) ){
			m_pcEditDoc->m_bGrepRunning = FALSE;
			m_bDoing_UndoRedo = FALSE;
			return 0;
		}
	}else{
		/* ���������̏�� */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pcmGrepKey->GetStringPtr(),
			pcmGrepKey->GetStringLength(),
			&pnKey_CharCharsArr
		);
	}

//2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWnd�ɐV�݂����֐����g���悤��
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	TCHAR szPath[_MAX_PATH];
	_tcscpy( szPath, pcmGrepFolder->GetStringPtr() );

	/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
	AddLastYenFromDirectoryPath( szPath );

	nWork = pcmGrepKey->GetStringLength(); // 2003.06.10 Moca ���炩���ߒ������v�Z���Ă���

	/* �Ō�Ƀe�L�X�g��ǉ� */
	CMemory		cmemWork;
	cmemMessage.AppendString( "\r\n����������  " );
	if( 0 < nWork ){
		CMemory cmemWork2;
		cmemWork2.SetNativeData( pcmGrepKey );
		if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace_j( "\\", "\\\\" );
			cmemWork2.Replace_j( "\'", "\\\'" );
			cmemWork2.Replace_j( "\"", "\\\"" );
		}else{
			cmemWork2.Replace_j( "\'", "\'\'" );
			cmemWork2.Replace_j( "\"", "\"\"" );
		}
		cmemWork.AppendString( "\"" );
		cmemWork.AppendNativeData( cmemWork2 );
		cmemWork.AppendString( "\"\r\n" );
	}else{
		cmemWork.AppendString( "�u�t�@�C�������v\r\n" );
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendString( "�����Ώ�   " );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += *pcmGrepFile;




	cmemMessage.AppendString( "\r\n" );
	cmemMessage.AppendString( "�t�H���_   " );
	cmemWork.SetString( szPath );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendString( "\r\n" );

	const char*	pszWork;
	if( bGrepSubFolder ){
		pszWork = "    (�T�u�t�H���_������)\r\n";
	}else{
		pszWork = "    (�T�u�t�H���_���������Ȃ�)\r\n";
	}
	cmemMessage.AppendString( pszWork );

	if( 0 < nWork ){ // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if( sSearchOption.bWordOnly ){
		/* �P��P�ʂŒT�� */
			cmemMessage.AppendString( "    (�P��P�ʂŒT��)\r\n" );
		}

		if( sSearchOption.bLoHiCase ){
			pszWork = "    (�p�啶������������ʂ���)\r\n";
		}else{
			pszWork = "    (�p�啶������������ʂ��Ȃ�)\r\n";
		}
		cmemMessage.AppendString( pszWork );

		if( sSearchOption.bRegularExp ){
			//	2007.07.22 genta : ���K�\�����C�u�����̃o�[�W�������o�͂���
			cmemMessage.AppendString( "    (���K�\��:" );
			cmemMessage.AppendString( cRegexp.GetVersionT() );
			cmemMessage.AppendString( ")\r\n" );
		}
	}

	if( CODE_AUTODETECT == nGrepCharSet ){
		cmemMessage.AppendString( "    (�����R�[�h�Z�b�g�̎�������)\r\n" );
	}else if(IsValidCodeType(nGrepCharSet)){
		cmemMessage.AppendString( "    (�����R�[�h�Z�b�g�F" );
		cmemMessage.AppendString( gm_pszCodeNameArr_1[nGrepCharSet] );
		cmemMessage.AppendString( ")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if( bGrepOutputLine ){
		/* �Y���s */
			pszWork = "    (��v�����s���o��)\r\n";
		}else{
			pszWork = "    (��v�����ӏ��̂ݏo��)\r\n";
		}
		cmemMessage.AppendString( pszWork );
	}


	cmemMessage.AppendString( "\r\n\r\n" );
	pszWork = cmemMessage.GetStringPtr( &nWork );
//@@@ 2002.01.03 YAZAKI Grep����̓J�[�\����Grep���O�̈ʒu�ɓ�����
	int tmp_PosY_PHY = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
	}
	cmemMessage.SetString("");
	cmemWork.SetString("");

	//	2007.07.22 genta �o�[�W�������擾���邽�߂ɁC
	//	���K�\���̏���������ֈړ�


	/* �\������ON/OFF */
	// 2003.06.23 Moca ���ʐݒ�ŕύX�ł���悤��
	// 2008.06.08 ryoji �S�r���[�̕\��ON/OFF�𓯊�������
//	m_bDrawSWITCH = false;
	if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
		m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V
	m_pcEditDoc->SetDrawSwitchOfAllViews( 0 != m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView );


	int nGrepTreeResult = DoGrepTree(
		&cDlgCancel,
		hwndCancel,
		pcmGrepKey->GetStringPtr(),
		pnKey_CharCharsArr,
		pcmGrepFile->GetStringPtr(),
		szPath,
		bGrepSubFolder,
		sSearchOption,
		nGrepCharSet,
		bGrepOutputLine,
		nGrepOutputStyle,
		&cRegexp,
		0,
		&nHitCount
	);
	if( -1 == nGrepTreeResult ){
		wsprintf( szPath, "���f���܂����B\r\n", nHitCount );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
	}
	{
		TCHAR  szBuffer[128];
		wsprintf( szBuffer, "%d ����������܂����B\r\n", nHitCount );
		Command_ADDTAIL( szBuffer, lstrlen( szBuffer ) );
#ifdef _DEBUG
		wsprintf( szBuffer, "��������: %d�~���b\r\n", cRunningTimer.Read() );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
#endif
	}
	MoveCursor( 0, tmp_PosY_PHY, true );	//	�J�[�\����Grep���O�̈ʒu�ɖ߂�

	cDlgCancel.CloseDialog( 0 );

	/* �A�N�e�B�u�ɂ��� */
	hwndMainFrame = ::GetParent( m_hwndParent );
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndMainFrame );

	// �A���h�D�o�b�t�@�̏���
	SetUndoBuffer();

	//	Apr. 13, 2001 genta
	//	Grep���s��̓t�@�C����ύX�����̏�Ԃɂ���D
	m_pcEditDoc->SetModified(false,false);

	m_pcEditDoc->m_bGrepRunning = FALSE;
	m_bDoing_UndoRedo = FALSE;

	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}

	/* �\������ON/OFF */
	m_pcEditDoc->SetDrawSwitchOfAllViews( true );

	/* �ĕ`�� */
	if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
		m_pcEditDoc->RedrawAllViews( NULL );

	return nHitCount;
}



/*
 * SORTED_LIST_BSEARCH
 *   ���X�g�̒T����bsearch���g���܂��B
 *   �w�肵�Ȃ��ꍇ�́A���`�T���ɂȂ�܂��B
 * SORTED_LIST
 *   ���X�g��qsort���܂��B
 *
 * �����F
 *   ���`�T���ł�qsort���g���A�������r�̑召�֌W���t�]�����Ƃ���ŒT����
 *   �ł��؂�Ώ����͑�����������܂���B
 */
//#define SORTED_LIST
//#define SORTED_LIST_BSEARCH

#ifdef SORTED_LIST_BSEARCH
#define SORTED_LIST
#endif

#ifdef SORTED_LIST
typedef int (* COMP)(const void *, const void *);

/*!
	qsort�p��r�֐�
	����a,b�͕�����ւ̃|�C���^�̃|�C���^�ł��邱�Ƃɒ��ӁB
	
	@param a [in] ��r������ւ̃|�C���^�̃|�C���^(list)
	@param b [in] ��r������ւ̃|�C���^�̃|�C���^(list)
	@return ��r����
*/
int grep_compare_pp(const void* a, const void* b)
{
	return _tcscmp( *((const TCHAR**)a), *((const TCHAR**)b) );
}

/*!
	bsearch�p��r�֐�
	����b�͕�����ւ̃|�C���^�̃|�C���^�ł��邱�Ƃɒ��ӁB
	
	@param a [in] ��r������ւ̃|�C���^(key)
	@param b [in] ��r������ւ̃|�C���^�̃|�C���^(list)
	@return ��r����
*/
int grep_compare_sp(const void* a, const void* b)
{
	return _tcscmp( (const TCHAR*)a, *((const TCHAR**)b) );
}
#endif

/*! @brief Grep���s

	@date 2001.06.27 genta	���K�\�����C�u�����̍����ւ�
	@date 2003.06.23 Moca   �T�u�t�H���_���t�@�C���������̂��t�@�C�����T�u�t�H���_�̏��ɕύX
	@date 2003.06.23 Moca   �t�@�C��������""����菜���悤��
	@date 2003.03.27 �݂�   ���O�t�@�C���w��̓����Əd�������h�~�̒ǉ��D
		�啔�����ύX���ꂽ���߁C�ʂ̕ύX�_�L���͖����D
*/
int CEditView::DoGrepTree(
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
	HWND					hwndCancel,			//!< [in] Cancel�_�C�A���O�̃E�B���h�E�n���h��
	const char*				pszKey,				//!< [in] �����p�^�[��
	int*					pnKey_CharCharsArr,	//!< [in] ������z��(2byte/1byte)�D�P�������񌟍��Ŏg�p�D
	const TCHAR*			pszFile,			//!< [in] �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��)
	const TCHAR*			pszPath,			//!< [in] �����Ώۃp�X
	BOOL					bGrepSubFolder,		//!< [in] TRUE: �T�u�t�H���_���ċA�I�ɒT������ / FALSE: ���Ȃ�
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	ECodeType				nGrepCharSet,		//!< [in] �����R�[�h�Z�b�g (0:�����F��)�`
	BOOL					bGrepOutputLine,	//!< [in] TRUE: �q�b�g�s���o�� / FALSE: �q�b�g�������o��
	int						nGrepOutputStyle,	//!< [in] �o�͌`�� 1: Normal, 2: WZ��(�t�@�C���P��)
	CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int						nNest,				//!< [in] �l�X�g���x��
	int*					pnHitCount			//!< [i/o] �q�b�g���̍��v
)
{
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );

	const TCHAR EXCEPT_CHAR = _T('!');	//���O���ʎq
	const TCHAR* WILDCARD_DELIMITER = _T(" ;,");	//���X�g�̋�؂�
	const TCHAR* WILDCARD_ANY = _T("*.*");	//�T�u�t�H���_�T���p

	int		nWildCardLen;
	int		nPos;
	BOOL	result;
	int		i;
	WIN32_FIND_DATA w32fd;
	CMemory			cmemMessage;
	int				nHitCountOld;
	int				nWork = 0;
	nHitCountOld = -100;

	//����̑Ώ�
	HANDLE handle      = INVALID_HANDLE_VALUE;


	/*
	 * ���X�g�̏�����(������ւ̃|�C���^�����X�g�Ǘ�����)
	 */
	int checked_list_size = 256;	//�m�ۍς݃T�C�Y
	int checked_list_count = 0;	//�o�^��
	TCHAR** checked_list = (TCHAR**)malloc( sizeof( TCHAR* ) * checked_list_size );
	if( ! checked_list ) return FALSE;	//�������m�ێ��s


	/*
	 * ���O�t�@�C����o�^����B
	 */
	nPos = 0;
	TCHAR* pWildCard = _tcsdup( pszFile );	//���C���h�J�[�h���X�g��Ɨp
	if( ! pWildCard ) goto error_return;	//�������m�ێ��s
	nWildCardLen = _tcslen( pWildCard );
	TCHAR*	token;
	while( NULL != (token = my_strtok( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//�g�[�N�����ɌJ��Ԃ��B
	{
		//���O�t�@�C���w��łȂ����H
		if( EXCEPT_CHAR != token[0] ) continue;

		//�_�u���R�[�e�[�V�����������A��΃p�X�����쐬����B
		TCHAR* p;
		TCHAR* q;
		p = q = ++token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		{
			std::tstring currentPath = pszPath;	//���ݒT�����̃p�X
			currentPath += token;
			//�t�@�C���̗�����J�n����B
			handle = FindFirstFile( currentPath.c_str(), &w32fd );
		}
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//�t�H���_�łȂ��ꍇ
			{
				//�`�F�b�N�ς݃��X�g�ɓo�^����B
				if( checked_list_count >= checked_list_size )
				{
					checked_list_size += 256;
					TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
					if( ! p ) goto error_return;	//�������m�ێ��s
					checked_list = p;
				}
				checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
				checked_list_count++;
			}

			//���̃t�@�C���𗅗񂷂�B
			result = FindNextFile( handle, &w32fd );
		}
		//�n���h�������B
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
	}
	free( pWildCard );
	pWildCard = NULL;

	/*
	 * �J�����g�t�H���_�̃t�@�C����T������B
	 */
	nPos = 0;
	pWildCard = _tcsdup( pszFile );
	if( ! pWildCard ) goto error_return;	//�������m�ێ��s
	nWildCardLen = _tcslen( pWildCard );
	while( NULL != (token = my_strtok( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//�g�[�N�����ɌJ��Ԃ��B
	{
		//���O�t�@�C���w�肩�H
		if( EXCEPT_CHAR == token[0] ) continue;

		//�_�u���R�[�e�[�V�����������A��΃p�X�����쐬����B
		TCHAR* p;
		TCHAR* q;
		p = q = token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		{
			std::tstring currentPath = pszPath;	//���ݒT�����̃p�X
			currentPath += token;
			//�t�@�C���̗�����J�n����B
			handle = FindFirstFile( currentPath.c_str(), &w32fd );
		}
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
#ifdef SORTED_LIST
		//�\�[�g
		qsort( checked_list, checked_list_count, sizeof( TCHAR* ), (COMP)grep_compare_pp );
#endif
		int current_checked_list_count = checked_list_count;	//�O��܂ł̃��X�g�̐�
		while( result )
		{
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}

			/* �\���ݒ���`�F�b�N */
			m_pcEditDoc->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW )
			);

			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//�t�H���_�łȂ��ꍇ
			{
				/*
				 * ���X�g�ɂ��邩���ׂ�B
				 * ����T�����̃t�@�C�����m���d�����邱�Ƃ͂Ȃ��̂ŁA
				 * �O��܂ł̃��X�g(current_checked_list_count)���猟������΂悢�B
				 */
#ifdef SORTED_LIST_BSEARCH
				if( ! bsearch( w32fd.cFileName, checked_list, current_checked_list_count, sizeof( TCHAR* ), (COMP)grep_compare_sp ) )
#else
				bool found = false;
				TCHAR** ptr = checked_list;
				for( i = 0; i < current_checked_list_count; i++, ptr++ )
				{
#ifdef SORTED_LIST
					int n = _tcscmp( *ptr, w32fd.cFileName );
					if( 0 == n )
					{
						found = true; 
						break;
					}
					else if( n > 0 )	//�T���ł��؂�
					{
						break;
					}
#else
					if( 0 == _tcscmp( *ptr, w32fd.cFileName ) )
					{
						found = true; 
						break;
					}
#endif
				}
				if( ! found )
#endif
				{
					//�`�F�b�N�ς݃��X�g�ɓo�^����B
					if( checked_list_count >= checked_list_size )
					{
						checked_list_size += 256;
						TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
						if( ! p ) goto error_return;	//�������m�ێ��s
						checked_list = p;
					}
					checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
					checked_list_count++;


					//GREP���s�I
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, w32fd.cFileName );

					TCHAR* currentFile = new TCHAR[ _tcslen( pszPath ) + _tcslen( w32fd.cFileName ) + 1 ];
					if( ! currentFile ) goto error_return;	//�������m�ێ��s
					_tcscpy( currentFile, pszPath );
					_tcscat( currentFile, w32fd.cFileName );
					/* �t�@�C�����̌��� */
					int nRet = DoGrepFile(
						pcDlgCancel,
						hwndCancel,
						pszKey,
						pnKey_CharCharsArr,
						w32fd.cFileName,
						sSearchOption,
						nGrepCharSet,
						bGrepOutputLine,
						nGrepOutputStyle,
						pRegexp,
						pnHitCount,
						currentFile,
						cmemMessage
					);
					delete [] currentFile;
					currentFile = NULL;

					// 2003.06.23 Moca ���A���^�C���\���̂Ƃ��͑��߂ɕ\��
					if( m_bDrawSWITCH ){
						if( _T('\0') != pszKey[0] ){
							// �f�[�^�����̂Ƃ��t�@�C���̍��v���ő�10MB�𒴂�����\��
							nWork += ( w32fd.nFileSizeLow + 1023 ) / 1024;
						}
						if( *pnHitCount - nHitCountOld && 
							( *pnHitCount < 20 || 10000 < nWork ) ){
							nHitCountOld = -100; // ���\��
						}
					}
					if( *pnHitCount - nHitCountOld  >= 10 ){
						/* ���ʏo�� */
						if( 0 < cmemMessage.GetStringLength() ){
							Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
							Command_GOFILEEND( false );
							if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
								m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V
							cmemMessage.SetString( _T("") );
						}
						nWork = 0;
						nHitCountOld = *pnHitCount;
					}
					if( -1 == nRet ){
						goto cancel_return;
					}
				}
			}

			//���̃t�@�C���𗅗񂷂�B
			result = FindNextFile( handle, &w32fd );
		}
		//�n���h�������B
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
	}
	free( pWildCard );
	pWildCard = NULL;

	for( i = 0; i < checked_list_count; i++ )
	{
		free( checked_list[ i ] );
	}
	free( checked_list );
	checked_list = NULL;
	checked_list_count = 0;
	checked_list_size = 0;

	// 2010.08.25 �t�H���_�ړ��O�Ɏc����ɏo��
	if( 0 < cmemMessage.GetStringLength() ){
		Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
		Command_GOFILEEND( false );
		if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
			m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V
		cmemMessage.SetString( _T("") );
	}

	/*
	 * �T�u�t�H���_����������B
	 */
	if( bGrepSubFolder ){
		// 2010.08.01 �L�����Z���ł̃������[���[�N�C��
		{
			std::tstring subPath = pszPath;
			subPath += WILDCARD_ANY;
			handle = FindFirstFile( subPath.c_str(), &w32fd );
		}
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			//�T�u�t�H���_�̒T�����ċA�Ăяo���B
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			/* �\���ݒ���`�F�b�N */
			m_pcEditDoc->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW )
			);

			if( (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	//�t�H���_�̏ꍇ
			 && 0 != _tcscmp( w32fd.cFileName, _T("."))
			 && 0 != _tcscmp( w32fd.cFileName, _T("..")) )
			{
				//�t�H���_�����쐬����B
				// 2010.08.01 �L�����Z���Ń������[���[�N���Ă܂���
				std::tstring currentPath  = pszPath;
				currentPath += w32fd.cFileName;
				currentPath += _T("\\");

				int nGrepTreeResult = DoGrepTree(
					pcDlgCancel,
					hwndCancel,
					pszKey,
					pnKey_CharCharsArr,
					pszFile,
					currentPath.c_str(),
					bGrepSubFolder,
					sSearchOption,
					nGrepCharSet,
					bGrepOutputLine,
					nGrepOutputStyle,
					pRegexp,
					nNest + 1,
					pnHitCount
				);
				if( -1 == nGrepTreeResult ){
					goto cancel_return;
				}
				::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add �T�u�t�H���_����߂��Ă�����...

			}

			//���̃t�@�C���𗅗񂷂�B
			result = FindNextFile( handle, &w32fd );
		}
		//�n���h�������B
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
	}

	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, _T(" ") );	// 2002/09/09 Moca add

	return 0;


cancel_return:;
error_return:;
	/*
	 * �G���[���͂��ׂĂ̊m�ۍς݃��\�[�X���������B
	 */
	if( INVALID_HANDLE_VALUE != handle ) FindClose( handle );

	if( pWildCard ) free( pWildCard );

	if( checked_list )
	{
		for( i = 0; i < checked_list_count; i++ )
		{
			free( checked_list[ i ] );
		}
		free( checked_list );
	}

	/* ���ʏo�� */
	if( 0 < cmemMessage.GetStringLength() ){
		Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
		Command_GOFILEEND( false );
		if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
			cmemMessage.GetStringPtr( &nWork );	//	���̃y�C���̕\�����X�V
		cmemMessage.SetString( _T("") );
	}

	return -1;
}




/*!	@brief Grep���ʂ��\�z����

	@param pWork [out] Grep�o�͕�����D�[���ȃ������̈��\�ߊm�ۂ��Ă������ƁD
		�Œ��� �{��2000 byte�{�t�@�C���� _MAX_PATH byte�{�s�E���ʒu�\���̒������K�v�D
		�t�@�C���P�ʏo�͂̏ꍇ�͖{��2500 byte + _MAX_PATH + �s�E���ʒu�\���̒������K�v�D
		

	pWork�͏[���ȃ������̈�������Ă���R�g
	@date 2002/08/29 Moca �o�C�i���[�f�[�^�ɑΉ� pnWorkLen �ǉ�
*/
void CEditView::SetGrepResult(
	/* �f�[�^�i�[�� */
	char*		pWork,
	int*		pnWorkLen,			/*!< [out] Grep�o�͕�����̒��� */
	/* �}�b�`�����t�@�C���̏�� */
	const TCHAR*		pszFullPath,	/*!< [in] �t���p�X */
	const TCHAR*		pszCodeName,	/*!< [in] �����R�[�h���D" [SJIS]"�Ƃ� */
	/* �}�b�`�����s�̏�� */
	int			nLine,				/*!< [in] �}�b�`�����s�ԍ�(1�`) */
	int			nColm,				/*!< [in] �}�b�`�������ԍ�(1�`) */
	const char*		pCompareData,	/*!< [in] �s�̕����� */
	int			nLineLen,			/*!< [in] �s�̕�����̒��� */
	int			nEolCodeLen,		/*!< [in] EOL�̒��� */
	/* �}�b�`����������̏�� */
	const char*		pMatchData,		/*!< [in] �}�b�`���������� */
	int			nMatchLen,			/*!< [in] �}�b�`����������̒��� */
	/* �I�v�V���� */
	BOOL		bGrepOutputLine,	/*!< [in] 0: �Y�������̂�, !0: �Y���s */
	int			nGrepOutputStyle	/*!< [in] 1: Normal, 2: WZ��(�t�@�C���P��) */
)
{

	int nWorkLen = 0;
	const char * pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr;

	/* �m�[�}�� */
	if( 1 == nGrepOutputStyle ){
		nWorkLen = ::wsprintf( pWork, "%s(%d,%d)%s: ", pszFullPath, nLine, nColm, pszCodeName );
		nMaxOutStr = 2000; // 2003.06.10 Moca �ő咷�ύX
	}
	/* WZ�� */
	else if( 2 == nGrepOutputStyle ){
		nWorkLen = ::wsprintf( pWork, "�E(%6d,%-5d): ", nLine, nColm );
		nMaxOutStr = 2500; // 2003.06.10 Moca �ő咷�ύX
	}

	/* �Y���s */
	if( bGrepOutputLine ){
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
	}
	/* �Y������ */
	else{
		pDispData = pMatchData;
		k = nMatchLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
		// �Y�������ɉ��s���܂ޏꍇ�͂��̉��s�R�[�h�����̂܂ܗ��p����(���̍s�ɋ�s�����Ȃ�)
		// 2003.06.10 Moca k==0�̂Ƃ��Ƀo�b�t�@�A���_�[�������Ȃ��悤��
		if( 0 < k && (pMatchData[ k - 1 ] == '\r' || pMatchData[ k - 1 ] == '\n') ){
			bEOL = false;
		}
	}

	memcpy( &pWork[nWorkLen], pDispData, k );
	nWorkLen += k;
	if( bEOL ){
		memcpy( &pWork[nWorkLen], "\r\n", 2 );
		nWorkLen = nWorkLen + 2;
	}
	*pnWorkLen = nWorkLen;
}

/*!
	Grep���s (CFileLoad���g�����e�X�g��)

	@retval -1 GREP�̃L�����Z��
	@retval ����ȊO �q�b�g��(�t�@�C���������̓t�@�C����)

	@date 2001/06/27 genta	���K�\�����C�u�����̍����ւ�
	@date 2002/08/30 Moca CFileLoad���g�����e�X�g��
	@date 2004/03/28 genta �s�v�Ȉ���nNest, bGrepSubFolder, pszPath���폜
*/
int CEditView::DoGrepFile(
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
	HWND					hwndCancel,			//!< [in] Cancel�_�C�A���O�̃E�B���h�E�n���h��
	const char*				pszKey,				//!< [in] �����p�^�[��
	int*					pnKey_CharCharsArr,	//!< [in] ������z��(2byte/1byte)�D�P�������񌟍��Ŏg�p�D
	const char*				pszFile,			//!< [in] �����Ώۃt�@�C����(�\���p)
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	ECodeType				nGrepCharSet,		//!< [in] �����R�[�h�Z�b�g (0:�����F��)�`
	BOOL					bGrepOutputLine,	//!< [in] TRUE: �q�b�g�s���o�� / FALSE: �q�b�g�������o��
	int						nGrepOutputStyle,	//!< [in] �o�͌`�� 1: Normal, 2: WZ��(�t�@�C���P��)
	CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int*					pnHitCount,			//!< [i/o] �q�b�g���̍��v�D���X�̒l�Ɍ��������������Z���ĕԂ��D
	const TCHAR*			pszFullPath,		//!< [in] �����Ώۃt�@�C���p�X
	CMemory&				cmemMessage			//!< 
)
{
	int		nHitCount;
//	char	szLine[16000];
	char	szWork[3000]; // ������ SetGrepResult() ���Ԃ���������i�[�ł���T�C�Y���K�v
	char	szWork0[_MAX_PATH + 100];
	int		nLine;
	int		nWorkLen;
	const char*	pszRes; // 2002/08/29 const�t��
	ECodeType	nCharCode;
	const char*	pCompareData; // 2002/08/29 const�t��
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	int		nLineLen;
	const	char*	pLine;
	CEol	cEol;
	int		nEolCodeLen;
	CFileLoad	cfl;
	int		nOldPercent = 0;

	int	nKeyKen = lstrlen( pszKey );

	//	�����ł͐��K�\���R���p�C���f�[�^�̏������͕s�v

	LPCTSTR	pszCodeName; // 2002/08/29 const�t��
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �s���[�v�̑O���炱���Ɉړ�
	if( 0 == nKeyKen ){
		if( CODE_AUTODETECT == nGrepCharSet ){
			// 2003.06.10 Moca �R�[�h���ʏ����������Ɉړ��D
			// ���ʃG���[�ł��t�@�C�����ɃJ�E���g���邽��
			// �t�@�C���̓��{��R�[�h�Z�b�g����
			nCharCode = CMemory::CheckKanjiCodeOfFile( pszFullPath );
			if( CODE_NONE == nCharCode ){
				pszCodeName = "  [(DetectError)]";
			}else{
				pszCodeName = gm_pszCodeNameArr_3[nCharCode];
			}
		}
		if( 1 == nGrepOutputStyle ){
		/* �m�[�}�� */
			wsprintf( szWork0, "%s%s\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ�� */
			wsprintf( szWork0, "��\"%s\"%s\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendString( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}


	try{
	// �t�@�C�����J��
	// FileClose�Ŗ����I�ɕ��邪�A���Ă��Ȃ��Ƃ��̓f�X�g���N�^�ŕ���
	// 2003.06.10 Moca �����R�[�h���菈����FileOpen�ōs��
	nCharCode = cfl.FileOpen( pszFullPath, nGrepCharSet, 0 );
	if( CODE_AUTODETECT == nGrepCharSet ){
		pszCodeName = gm_pszCodeNameArr_3[nCharCode];
	}
	wsprintf( szWork0, "��\"%s\"%s\r\n", pszFullPath, pszCodeName );
//	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
		return -1;
	}
	/* ���f�{�^�������`�F�b�N */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �t�@�C���I�[�v���̎�O�ֈړ�

	// ���� : cfl.ReadLine �� throw ����\��������
	while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLine;
		pCompareData = pLine;

		/* �������̃��[�U�[������\�ɂ��� */
		// 2010.08.31 �Ԋu��1/32�ɂ���
		if( ((0 == nLine % 32)|| 10000 < nLineLen ) && !::BlockingHook( pcDlgCancel->m_hWnd ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			//	2003.06.23 Moca �\���ݒ���`�F�b�N
			m_pcEditDoc->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW )
			);
			// 2002/08/30 Moca �i�s��Ԃ�\������(5MB�ȏ�)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					::wsprintf( szWork, "%s (%3d%%)", pszFile, nPercent );
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, szWork );
				}
			}
		}

		/* ���K�\������ */
		if( sSearchOption.bRegularExp ){
			int nIndex = 0;
#ifdef _DEBUG
			int nIndexPrev = -1;
#endif

			//	Jun. 21, 2003 genta ���[�v����������
			//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
			//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
			//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
			//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
			// From Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
			// 2010.08.25 �s���ȊO��^�Ƀ}�b�`����s��̏C��
			while( nIndex <= nLineLen && pRegexp->Match( pLine, nLineLen, nIndex ) ){

					//	�p�^�[������
					nIndex = pRegexp->GetIndex();
					int matchlen = pRegexp->GetMatchLen();
#ifdef _DEBUG
					if( nIndex <= nIndexPrev ){
						MYTRACE( _T("ERROR: CEditView::DoGrepFile() nIndex <= nIndexPrev break \n") );
						break;
					}
					nIndexPrev = nIndex;
#endif

					/* Grep���ʂ��AszWork�Ɋi�[���� */
					SetGrepResult(
						szWork,
						&nWorkLen,
						pszFullPath,
						pszCodeName,
						nLine,
						nIndex + 1,
						pLine,
						nLineLen,
						nEolCodeLen,
						pLine + nIndex,
						matchlen,
						bGrepOutputLine,
						nGrepOutputStyle
					);
					// To Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						if( !bOutFileName ){
							cmemMessage.AppendString( szWork0 );
							bOutFileName = TRUE;
						}
					}
					cmemMessage.AppendString( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
					if ( bGrepOutputLine ) {
						break;
					}
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					if( matchlen <= 0 ){
						matchlen = CMemory::GetSizeOfChar( pLine, nLineLen, nIndex );
						if( matchlen <= 0 ){
							matchlen = 1;
						}
					}
					nIndex += matchlen;
			}
		}
		/* �P��̂݌��� */
		else if( sSearchOption.bWordOnly ){
			/*
				2002/02/23 Norio Nakatani
				�P��P�ʂ�Grep�������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
				�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B

				2002/03/06 YAZAKI
				Grep�ɂ����������B
				WhereCurrentWord�ŒP��𒊏o���āA���̒P�ꂪ������Ƃ����Ă��邩��r����B
			*/
			int nNextWordFrom = 0;
			int nNextWordFrom2;
			int nNextWordTo2;
			// Jun. 26, 2003 genta ���ʂ�while�͍폜
			while( CDocLineMgr::WhereCurrentWord_2( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
				if( nKeyKen == nNextWordTo2 - nNextWordFrom2 ){
					// const char* pData = pCompareData;	// 2002/2/10 aroka CMemory�ύX , 2002/08/29 Moca pCompareData��const���ɂ��s�v?
					/* 1==�啶���������̋�� */
					if( (!sSearchOption.bLoHiCase && 0 == my_memicmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) ) ||
						(sSearchOption.bLoHiCase && 0 ==	 memcmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) )
					){
						/* Grep���ʂ��AszWork�Ɋi�[���� */
						SetGrepResult(
							szWork, &nWorkLen,
							pszFullPath, pszCodeName,
							//	Jun. 25, 2002 genta
							//	���ʒu��1�n�܂�Ȃ̂�1�𑫂��K�v������
							nLine, nNextWordFrom2 + 1, pCompareData, nLineLen, nEolCodeLen,
							&(pCompareData[nNextWordFrom2]), nKeyKen,
							bGrepOutputLine, nGrepOutputStyle
						);
						if( 2 == nGrepOutputStyle ){
						/* WZ�� */
							if( !bOutFileName ){
								cmemMessage.AppendString( szWork0 );
								bOutFileName = TRUE;
							}
						}

						cmemMessage.AppendString( szWork, nWorkLen );
						++nHitCount;
						++(*pnHitCount);
						//	May 22, 2000 genta
						if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
							::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
						}

						// 2010.10.31 ryoji �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
						if ( bGrepOutputLine ) {
							break;
						}
					}
				}
				/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
				if( !CDocLineMgr::SearchNextWordPosition( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom, FALSE ) ){
					break;	//	���̒P�ꂪ�����B
				}
			}
		}
		else {
			/* �����񌟍� */
			int nColmPrev = 0;
			//	Jun. 21, 2003 genta ���[�v����������
			//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
			//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
			//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
			while(1){
				pszRes = CDocLineMgr::SearchString(
					(const unsigned char *)pCompareData,
					nLineLen,
					0,
					(const unsigned char *)pszKey,
					nKeyKen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if(!pszRes)break;

				nColm = pszRes - pCompareData + 1;

				/* Grep���ʂ��AszWork�Ɋi�[���� */
				SetGrepResult(
					szWork, &nWorkLen,
					pszFullPath, pszCodeName,
					nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
					pszRes, nKeyKen,
					bGrepOutputLine, nGrepOutputStyle
				);
				if( 2 == nGrepOutputStyle ){
				/* WZ�� */
					if( !bOutFileName ){
						cmemMessage.AppendString( szWork0 );
						bOutFileName = TRUE;
					}
				}

				cmemMessage.AppendString( szWork, nWorkLen );
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}
				
				//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
				if ( bGrepOutputLine ) {
					break;
				}
				//	�T���n�߂�ʒu��␳
				//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
				//	nClom : �}�b�`�ʒu
				//	matchlen : �}�b�`����������̒���
				int nPosDiff = nColm += nKeyKen - 1;
				pCompareData += nPosDiff;
				nLineLen -= nPosDiff;
				nColmPrev += nPosDiff;
			}
		}
	}

	// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		wsprintf( szWork, "file open error [%s]\r\n", pszFullPath );
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
		return 0;
	}
	catch( CError_FileRead ){
		wsprintf( szWork, "CEditView::DoGrepFile() �t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B\r\n");
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
	} // ��O�����I���

	return nHitCount;
}


/*
	�J�[�\�����O�̒P����擾 �P��̒�����Ԃ��܂�
	�P���؂�
*/
int CEditView::GetLeftWord( CMemory* pcmemWord, int nMaxWordLen )
{
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nIdxTo;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	CMemory		cmemWord;
	int			nCurLine;
	int			nCharChars;
	const CLayout* pcLayout;

	nCurLine = m_ptCaretPos.y;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen, &pcLayout );
	if( NULL == pLine ){
//		return 0;
		nIdxTo = 0;
	}else{
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
		nIdxTo = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
	}
	if( 0 == nIdxTo || NULL == pLine ){
		if( nCurLine <= 0 ){
			return 0;
		}
		nCurLine--;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( pLine[nLineLen - 1] == '\r' || pLine[nLineLen - 1] == '\n' ){
			return 0;
		}
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
//		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_ptCaretPos.x );

		nCharChars = &pLine[nLineLen] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nLineLen] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - nCharChars;

//		nIdx = nIdxTo = nLineLen - 1;
	}else{
		nCharChars = &pLine[nIdxTo] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxTo] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdx = nIdxTo - nCharChars;
	}
	if( 1 == nCharChars ){
		if( pLine[nIdx] == SPACE || pLine[nIdx] == TAB ){
			return 0;
		}
	}
	if( 2 == nCharChars ){
		if( (unsigned char)pLine[nIdx	 ] == (unsigned char)0x81 &&
			(unsigned char)pLine[nIdx + 1] == (unsigned char)0x40
		){
			return 0;
		}
	}


//	nIdx = nIdxTo - 1;
//	if( 0 > nIdx ){
//		return 0;
//	}
//	if( pLine[nIdx] == SPACE ||
//		pLine[nIdx] == TAB ||
//		(
//			nIdx > 0 &&
//			pLine[nIdx] == 0x40 &&
//			(unsigned char)pLine[nIdx - 1] == (unsigned char)0x81
//		)
//	){
//		return 0;
//	}
//	while( 0 <= nIdx && ( pLine[nIdx] != SPACE && pLine[nIdx] != TAB ) ){
//		nIdx--;
//	}
//	++nIdx;
//	if( nIdxTo - nIdx > 0 ){
//		if( nMaxWordLen < nIdxTo - nIdx ){
//			return 0;
//		}else{
//			pcmemWord->SetData( &pLine[nIdx], nIdxTo - nIdx );
//			return nIdxTo - nIdx;
//		}
//	}else{
//		return 0;
//	}



	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, &cmemWord, pcmemWord )
	){
		pcmemWord->AppendString( &pLine[nIdx], nCharChars );
//		MYTRACE( _T("==========\n") );
//		MYTRACE( _T("cmemWord=[%s]\n"), cmemWord.GetPtr() );
//		MYTRACE( _T("pcmemWord=[%s]\n"), pcmemWord->GetPtr() );

		return pcmemWord->GetStringLength();
	}else{
		return 0;
	}
}
/*!
	�L�����b�g�ʒu�̒P����擾
	�P���؂�

	@param[out] pcmemWord �L�����b�g�ʒu�̒P��
	@return true: �����Cfalse: ���s
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORD�𗬗p)
*/
BOOL CEditView::GetCurrentWord(
		CMemory* pcmemWord
)
{
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	int				nIdx;
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_ptCaretPos.y );
	if( NULL == pcLayout ){
		return false;	/* �P��I���Ɏ��s */
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_ptCaretPos.y, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, pcmemWord, NULL ) ){
		return true;	/* �P��I���ɐ��� */
	}
	else {
		return false;	/* �P��I���Ɏ��s */
	}
}


/* �w��J�[�\���ʒu���I���G���A���ɂ��邩
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int CEditView::IsCurrentPositionSelected(
	int		nCaretPosX,		// �J�[�\���ʒuX
	int		nCaretPosY		// �J�[�\���ʒuY
)
{
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}
	RECT	rcSel;
	POINT	po;


	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
			m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
			m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
			m_sSelect.m_ptTo.x			/* �͈͑I���I���� */
		);
		++rcSel.bottom;
		po.x = nCaretPosX;
		po.y = nCaretPosY;
		if( IsDragSource() ){
			if(GetKeyState_Control()){ /* Ctrl�L�[��������Ă����� */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( PtInRect( &rcSel, po ) ){
			return 0;
		}
		if( rcSel.top > nCaretPosY ){
			return -1;
		}
		if( rcSel.bottom < nCaretPosY ){
			return 1;
		}
		if( rcSel.left > nCaretPosX ){
			return -1;
		}
		if( rcSel.right < nCaretPosX ){
			return 1;
		}
	}else{
		if( m_sSelect.m_ptFrom.y > nCaretPosY ){
			return -1;
		}
		if( m_sSelect.m_ptTo.y < nCaretPosY ){
			return 1;
		}
		if( m_sSelect.m_ptFrom.y == nCaretPosY ){
			if( IsDragSource() ){
				if(GetKeyState_Control()){	/* Ctrl�L�[��������Ă����� */
					if( m_sSelect.m_ptFrom.x >= nCaretPosX ){
						return -1;
					}
				}else{
					if( m_sSelect.m_ptFrom.x > nCaretPosX ){
						return -1;
					}
				}
			}else
			if( m_sSelect.m_ptFrom.x > nCaretPosX ){
				return -1;
			}
		}
		if( m_sSelect.m_ptTo.y == nCaretPosY ){
			if( IsDragSource() ){
				if(GetKeyState_Control()){	/* Ctrl�L�[��������Ă����� */
					if( m_sSelect.m_ptTo.x <= nCaretPosX ){
						return 1;
					}
				}else{
					if( m_sSelect.m_ptTo.x < nCaretPosX ){
						return 1;
					}
				}
			}else
			if( m_sSelect.m_ptTo.x <= nCaretPosX ){
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 (�e�X�g)
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int CEditView::IsCurrentPositionSelectedTEST(
	int		nCaretPosX,		// �J�[�\���ʒuX
	int		nCaretPosY,		// �J�[�\���ʒuY
	int		nSelectLineFrom,
	int		nSelectColmFrom,
	int		nSelectLineTo,
	int		nSelectColmTo
)
{
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}
//	RECT	rcSel;
//	POINT	po;


	if( nSelectLineFrom > nCaretPosY ){
		return -1;
	}
	if( nSelectLineTo < nCaretPosY ){
		return 1;
	}
	if( nSelectLineFrom == nCaretPosY ){
		if( nSelectColmFrom > nCaretPosX ){
			return -1;
		}
	}
	if( nSelectLineTo == nCaretPosY ){
		if( nSelectColmTo <= nCaretPosX ){
			return 1;
		}
	}
	return 0;
}

/*! �N���b�v�{�[�h����f�[�^���擾
	@date 2005.05.29 novice UNICODE TEXT �Ή�������ǉ�
	@date 2007.10.04 ryoji MSDEVLineSelect�Ή�������ǉ�
	@date 2010.11.17 ryoji VS2010�̍s�R�s�[�Ή�������ǉ�
*/
bool CEditView::MyGetClipboardData( CMemory& cmemBuf, bool* pbColmnSelect, bool* pbLineSelect /*= NULL*/ )
{
	HGLOBAL		hglb;
	char*		lptstr;

	if( NULL != pbColmnSelect ){
		*pbColmnSelect = false;
	}
	if( NULL != pbLineSelect ){
		*pbLineSelect = FALSE;
	}


	UINT uFormatSakuraClip;
	UINT uFormat;
	uFormatSakuraClip = ::RegisterClipboardFormat( _T("SAKURAClip") );

	// 2008/02/16 �N���b�v�{�[�h����̃t�@�C���p�X�\��t���Ή�	bosagami	zlib/libpng license
	if( !::IsClipboardFormatAvailable( CF_OEMTEXT )
	 && !::IsClipboardFormatAvailable( CF_HDROP )
	 && !::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return false;
	}
	if ( !::OpenClipboard( m_hWnd ) ){
		return false;
	}

	char	szFormatName[128];

	if( NULL != pbColmnSelect || NULL != pbLineSelect ){
		/* ��`�I����s�I���̃e�L�X�g�f�[�^���N���b�v�{�[�h�ɂ��邩 */
		uFormat = 0;
		while( 0 != ( uFormat = ::EnumClipboardFormats( uFormat ) ) ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			if( ::GetClipboardFormatName( uFormat, szFormatName, sizeof(szFormatName) - 1 ) ){
				if( NULL != pbColmnSelect && 0 == lstrcmpi( _T("MSDEVColumnSelect"), szFormatName ) ){
					*pbColmnSelect = true;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmpi( _T("MSDEVLineSelect"), szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmpi( _T("VisualStudioEditorOperationsLineCutCopyClipboardTag"), szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
			}
		}
	}
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		hglb = ::GetClipboardData( uFormatSakuraClip );
		if (hglb != NULL) {
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetString( lptstr + sizeof(int), *((int*)lptstr) );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return true;
		}
	}else if(::IsClipboardFormatAvailable( CF_HDROP )){
		// 2008/02/16 �N���b�v�{�[�h����̃t�@�C���p�X�\��t���Ή�	bosagami	zlib/libpng license
		HDROP hDrop = (HDROP)::GetClipboardData( CF_HDROP );
		if(hDrop != NULL)
		{
			//�N���b�v�{�[�h����R�s�[�����p�X�̏����擾
			char sTmpPath[_MAX_PATH + 1] = {0};
			const int nMaxCnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

			std::vector<string> pathList;

			const char* sEol = m_pcEditDoc->GetNewLineCode().GetValue();
			for(int nLoop = 0; nLoop < nMaxCnt; nLoop++)
			{
				DragQueryFile(hDrop, nLoop, sTmpPath, sizeof(sTmpPath) - 1);
				pathList.push_back((string)sTmpPath);
			}

			//���בւ��ăo�b�t�@�ɒǉ�
			// 2008.08.06 nasukoji	�E�{�^���ł̃t�@�C���h���b�v�Ǝd�l�����킹�邽�ߍ폜
//			stable_sort(pathList.begin(), pathList.end(), sort_string_nocase);

			std::vector<string>::iterator pathListItr = pathList.begin();
			while(pathListItr != pathList.end())
			{
				cmemBuf.AppendString(pathListItr->c_str());
				if(pathList.size() > 1)
				{
					cmemBuf.AppendString(sEol);
				}
				pathListItr++;
			}
			::CloseClipboard();
			return true;
		}
	}else{
		// From Here 2005/05/29 novice UNICODE TEXT �Ή�������ǉ�
		hglb = ::GetClipboardData( CF_UNICODETEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			//	UnicodeToSJIS�ł͌��ɗ]�v�ȋ󔒂�����̂ŁC
			//	�ꎞ�ϐ���������\0�܂ł����o���D
			CMemory cmemUnicode( lptstr, GlobalSize(lptstr) );
			cmemUnicode.UnicodeToSJIS();
			cmemBuf.SetString( cmemUnicode.GetStringPtr() );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return true;
		}
		//	To Here 2005/05/29 novice

		hglb = ::GetClipboardData( CF_OEMTEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetString( lptstr );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return true;
		}
	}
	::CloseClipboard();
	return false;
}

/* �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	@date 2004.02.17 Moca �G���[�`�F�b�N����悤��
	@date 2007.10.04 ryoji MSDEVLineSelect�Ή�������ǉ�
	@date 2010.11.17 ryoji VS2010�̍s�R�s�[�Ή�������ǉ�
 */
bool CEditView::MySetClipboardData( const char* pszText, int nTextLen, bool bColmnSelect, bool bLineSelect /*= false*/ )
{
	HGLOBAL		hgClipText = NULL;
	HGLOBAL		hgClipSakura = NULL;
	HGLOBAL		hgClipMSDEVColm = NULL;
	HGLOBAL		hgClipMSDEVLine = NULL;
	HGLOBAL		hgClipMSDEVLine2 = NULL;

	char*		pszClip;
	UINT		uFormat;
	/* Windows�N���b�v�{�[�h�ɃR�s�[ */
	if( FALSE == ::OpenClipboard( m_hWnd ) ){
		return false;
	}
	::EmptyClipboard();
	// �k���I�[�܂ł̒���
	int nNullTerminateLen = lstrlen( pszText );

	/* �e�L�X�g�`���̃f�[�^ */
	hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nNullTerminateLen + 1
	);
	if( hgClipText ){
		pszClip = (char*)::GlobalLock( hgClipText );
		memcpy( pszClip, pszText, nNullTerminateLen );
		pszClip[nNullTerminateLen] = '\0';
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_OEMTEXT, hgClipText );
	}

	/* �o�C�i���`���̃f�[�^
		(int) �u�f�[�^�v�̒���
		�u�f�[�^�v
	*/
	UINT	uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( _T("SAKURAClip") );
	if( 0 != uFormatSakuraClip ){
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			nTextLen + sizeof( int ) + 1
		);
		if( hgClipSakura ){
			pszClip = (char*)::GlobalLock( hgClipSakura );
			*((int*)pszClip) = nTextLen;
			memcpy( pszClip + sizeof( int ), pszText, nTextLen );
			::GlobalUnlock( hgClipSakura );
			::SetClipboardData( uFormatSakuraClip, hgClipSakura );
		}
	}

	/* ��`�I���������_�~�[�f�[�^ */
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVColm = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColm ){
				pszClip = (char*)::GlobalLock( hgClipMSDEVColm );
				pszClip[0] = '\0';
				::GlobalUnlock( hgClipMSDEVColm );
				::SetClipboardData( uFormat, hgClipMSDEVColm );
			}
		}
	}

	/* �s�I���������_�~�[�f�[�^ */
	if( bLineSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVLineSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVLine = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine ){
				pszClip = (char*)::GlobalLock( hgClipMSDEVLine );
				pszClip[0] = (char)0x01;
				::GlobalUnlock( hgClipMSDEVLine );
				::SetClipboardData( uFormat, hgClipMSDEVLine );
			}
		}
		uFormat = ::RegisterClipboardFormat( _T("VisualStudioEditorOperationsLineCutCopyClipboardTag") );
		if( 0 != uFormat ){
			hgClipMSDEVLine2 = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine2 ){
				pszClip = (char*)::GlobalLock( hgClipMSDEVLine2 );
				pszClip[0] = (char)0x01;	// �� ClipSpy �Œ��ׂ�ƃf�[�^�͂���Ƃ͈Ⴄ�����e�ɂ͖��֌W�ɓ������ۂ�
				::GlobalUnlock( hgClipMSDEVLine2 );
				::SetClipboardData( uFormat, hgClipMSDEVLine2 );
			}
		}
	}
	::CloseClipboard();

	if( bColmnSelect && !hgClipMSDEVColm ){
		return false;
	}
	if( bLineSelect && !(hgClipMSDEVLine && hgClipMSDEVLine2) ){
		return false;
	}
	if( !(hgClipText && hgClipSakura) ){
		return false;
	}
	return true;
}






/** DragEnter ����
	@date 2008.03.26 ryoji SAKURAClip�t�H�[�}�b�g�iNULL�������܂ރe�L�X�g�j�ւ̑Ή���ǉ�
	@date 2008.06.20 ryoji CF_HDROP�t�H�[�}�b�g�ւ̑Ή���ǉ�
*/
STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragEnter()\n") );

	if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		//	Oct. 22, 2005 genta �㏑���֎~(�t�@�C�������b�N����Ă���)�ꍇ���s��
		 && !( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )
		 && !m_pcEditDoc->IsReadOnly() ){ // Mar. 30, 2003 �ǂݎ���p�̃t�@�C���ɂ̓h���b�v�����Ȃ�
	}else{
		return E_UNEXPECTED;	//	Moca E_INVALIDARG����ύX
	}

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	m_cfDragData = GetAvailableClipFormat( pDataObject );
	if( m_cfDragData == 0 )
		return E_INVALIDARG;
	else if( m_cfDragData == CF_HDROP ){
		// �E�{�^���œ����Ă����Ƃ������t�@�C�����r���[�Ŏ�舵��
		if( !(MK_RBUTTON & dwKeyState) )
			return E_INVALIDARG;
	}

	/* �������A�N�e�B�u�y�C���ɂ��� */
	m_pcEditDoc->SetActivePane( m_nMyIndex );

	// ���݂̃J�[�\���ʒu���L������	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter.x = m_ptCaretPos.x;
	m_ptCaretPos_DragEnter.y = m_ptCaretPos.y;
	m_nCaretPosX_Prev_DragEnter = m_nCaretPosX_Prev;

	// �h���b�O�f�[�^�͋�`��
	m_bDragBoxData = IsDataAvailable( pDataObject, ::RegisterClipboardFormat( _T("MSDEVColumnSelect") ) );

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = TRUE;

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragOver()\n") );

	/* �}�E�X�ړ��̃��b�Z�[�W���� */
	::ScreenToClient( m_hWnd, (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( m_cfDragData, dwKeyState, pt, *pdwEffect );

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

	// �h���b�O�������r���[�ŁA���̃r���[�̃J�[�\�����h���b�O���̑I��͈͓��̏ꍇ�͋֎~�}�[�N�ɂ���
	// �����r���[�̂Ƃ��͋֎~�}�[�N�ɂ��Ȃ��i���A�v���ł������͂����Ȃ��Ă���͗l�j	// 2009.06.09 ryoji
	if( pcDragSourceView && !IsDragSource() &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}



STDMETHODIMP CEditView::DragLeave( void )
{
	DEBUG_TRACE( _T("CEditView::DragLeave()\n") );
	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	// DragEnter���̃J�[�\���ʒu�𕜌�	// 2007.12.09 ryoji
	MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
	m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ���[���[�A�A���_�[���C���A�J�[�\���ʒu�\���X�V

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	return S_OK;
}

/** �h���b�v����
	@date 2008.03.26 ryoji �h���b�v�œ\��t�����͈͂�I����Ԃɂ���
	                       SAKURAClip�t�H�[�}�b�g�iNULL�������܂ރe�L�X�g�j�ւ̑Ή���ǉ�
	@date 2008.06.20 ryoji CF_HDROP�t�H�[�}�b�g�ւ̑Ή���ǉ�
*/
STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::Drop()\n") );
	CMemory		cmemBuf;
	COpe*		pcOpe;
	BOOL		bBoxData;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	bool		bBeginBoxSelect_Old;
	int			nSelectLineBgnFrom_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColBgnFrom_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat( pDataObject );
	if( cf == 0 )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( cf, dwKeyState, pt, *pdwEffect );
	if( *pdwEffect == DROPEFFECT_NONE )
		return E_INVALIDARG;

	// �t�@�C���h���b�v�� PostMyDropFiles() �ŏ�������
	if( cf == CF_HDROP )
		return PostMyDropFiles( pDataObject );

	// �O������̃h���b�v�͈Ȍ�̏����ł̓R�s�[�Ɠ��l�Ɉ���
	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pcDragSourceView;
	bBoxData = m_bDragBoxData;

	// �J�[�\�����I��͈͓��ɂ���Ƃ��̓R�s�[�^�ړ����Ȃ�	// 2009.06.09 ryoji
	if( pcDragSourceView &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		// DragEnter���̃J�[�\���ʒu�𕜌�
		// Note. �h���b�O�������r���[�ł��}�E�X�ړ��������ƋH�ɂ����ɂ���\�������肻��
		*pdwEffect = DROPEFFECT_NONE;
		MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
		m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if( !IsDragSource() )	// �h���b�O���̏ꍇ�͂����ł͍ĕ`��s�v�iDragDrop�㏈����SetActivePane�ōĕ`�悳���j
			RedrawAll();	// ����ɈȌ�̔�A�N�e�B�u���ɔ����A���_�[���C�������̂��߂Ɉ�x�X�V���Đ������Ƃ�
		return S_OK;
	}

	// �h���b�v�f�[�^�̎擾
	HGLOBAL hData = GetGlobalData( pDataObject, cf );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );
	if( cf == ::RegisterClipboardFormat( _T("SAKURAClip") ) ){
		cmemBuf.SetString( (char*)pData + sizeof(int), *(int*)pData );
	}else{
		CMemory cmemTemp;
		cmemTemp.SetString( (char*)pData, nSize );	// ���S�̂��ߖ����� null ������t��
		cmemBuf.SetString( cmemTemp.GetStringPtr() );		// ������I�[�܂ŃR�s�[
	}

	// �A���h�D�o�b�t�@�̏���
	if( NULL == m_pcOpeBlk ){
		m_pcOpeBlk = new COpeBlk;
	}

	/* �ړ��̏ꍇ�A�ʒu�֌W���Z�o */
	if( bMove ){
		if( bBoxData ){
			/* 2�_��Ίp�Ƃ����`�����߂� */
			TwoPointToRect(
				&rcSel,
				pcDragSourceView->m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
				pcDragSourceView->m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
				pcDragSourceView->m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
				pcDragSourceView->m_sSelect.m_ptTo.x			/* �͈͑I���I���� */
			);
			++rcSel.bottom;
			if( m_ptCaretPos.y >= rcSel.bottom ){
				bMoveToPrev = FALSE;
			}else
			if( m_ptCaretPos.y + rcSel.bottom - rcSel.top < rcSel.top ){
				bMoveToPrev = TRUE;
			}else
			if( m_ptCaretPos.x < rcSel.left ){
				bMoveToPrev = TRUE;
			}else{
				bMoveToPrev = FALSE;
			}
		}else{
			if( pcDragSourceView->m_sSelect.m_ptFrom.y > m_ptCaretPos.y ){
				bMoveToPrev = TRUE;
			}else
			if( pcDragSourceView->m_sSelect.m_ptFrom.y == m_ptCaretPos.y ){
				if( pcDragSourceView->m_sSelect.m_ptFrom.x > m_ptCaretPos.x ){
					bMoveToPrev = TRUE;
				}else{
					bMoveToPrev = FALSE;
				}
			}else{
				bMoveToPrev = FALSE;
			}
		}
	}

	nCaretPosX_Old = m_ptCaretPos.x;
	nCaretPosY_Old = m_ptCaretPos.y;
	if( !bMove ){
		/* �R�s�[���[�h */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( true );
	}else{
		// �h���b�O���̑I��͈͂��L��
		bBeginBoxSelect_Old = pcDragSourceView->m_bBeginBoxSelect;
		nSelectLineBgnFrom_Old = pcDragSourceView->m_sSelectBgn.m_ptFrom.y;
		nSelectColBgnFrom_Old = pcDragSourceView->m_sSelectBgn.m_ptFrom.x;
		nSelectLineBgnTo_Old = pcDragSourceView->m_sSelectBgn.m_ptTo.y;
		nSelectColBgnTo_Old = pcDragSourceView->m_sSelectBgn.m_ptTo.x;
		nSelectLineFrom_Old = pcDragSourceView->m_sSelect.m_ptFrom.y;
		nSelectColFrom_Old = pcDragSourceView->m_sSelect.m_ptFrom.x;
		nSelectLineTo_Old = pcDragSourceView->m_sSelect.m_ptTo.y;
		nSelectColTo_Old = pcDragSourceView->m_sSelect.m_ptTo.x;

		if( bMoveToPrev ){
			/* �ړ����[�h & �O�Ɉړ� */
			/* �I���G���A���폜 */
			if( this != pcDragSourceView ){
				// �h���b�O���̑I��͈͂𕜌�
				pcDragSourceView->DisableSelectArea( true );
				DisableSelectArea( true );
				m_bBeginBoxSelect = bBeginBoxSelect_Old;
				m_sSelectBgn.m_ptFrom.y = nSelectLineBgnFrom_Old;
				m_sSelectBgn.m_ptFrom.x = nSelectColBgnFrom_Old;
				m_sSelectBgn.m_ptTo.y = nSelectLineBgnTo_Old;
				m_sSelectBgn.m_ptTo.x = nSelectColBgnTo_Old;
				m_sSelect.m_ptFrom.y = nSelectLineFrom_Old;
				m_sSelect.m_ptFrom.x = nSelectColFrom_Old;
				m_sSelect.m_ptTo.y = nSelectLineTo_Old;
				m_sSelect.m_ptTo.x = nSelectColTo_Old;
			}
			DeleteData( true );
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
		}else{
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcDragSourceView->DisableSelectArea( true );
			if( this != pcDragSourceView )
				DisableSelectArea( true );
		}
	}
	if( !bBoxData ){	/* ��`�f�[�^ */
		//	2004,05.14 Moca �����ɕ����񒷂�ǉ�

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// �s�I�[���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\�������������Ă���
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		Command_INSTEXT( true, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		SetSelectArea( m_sSelect.m_ptFrom.y, m_sSelect.m_ptFrom.x, m_ptCaretPos.y, m_ptCaretPos.x );	// 2009.07.25 ryoji
	}else{
		// 2004.07.12 Moca �N���b�v�{�[�h�����������Ȃ��悤��
		// TRUE == bBoxData
		// false == m_bBeginBoxSelect
		/* �\��t���i�N���b�v�{�[�h����\��t���j*/
		Command_PASTEBOX( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() );
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if( bMove ){
		if( bMoveToPrev ){
		}else{
			/* �ړ����[�h & ���Ɉړ�*/

			// ���݂̑I��͈͂��L������	// 2008.03.26 ryoji
			int nSelectLineFrom_PHY;
			int nSelectColmFrom_PHY;
			int nSelectLineTo_PHY;
			int nSelectColmTo_PHY;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y,
				&nSelectColmFrom_PHY, &nSelectLineFrom_PHY
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y,
				&nSelectColmTo_PHY, &nSelectLineTo_PHY
			);

			// �ȑO�̑I��͈͂��L������	// 2008.03.26 ryoji
			int nSelectLineFrom_PHY_Old;
			int nSelectColmFrom_PHY_Old;
			int nSelectLineTo_PHY_Old;
			int nSelectColmTo_PHY_Old;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				nSelectColFrom_Old, nSelectLineFrom_Old,
				&nSelectColmFrom_PHY_Old, &nSelectLineFrom_PHY_Old
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				nSelectColTo_Old, nSelectLineTo_Old,
				&nSelectColmTo_PHY_Old, &nSelectLineTo_PHY_Old
			);

			// ���݂̍s�����L������	// 2008.03.26 ryoji
			int nLines_Old = m_pcEditDoc->m_cDocLineMgr.GetLineCount();

			// �ȑO�̑I��͈͂�I������
			m_bBeginBoxSelect = bBeginBoxSelect_Old;
			m_sSelectBgn.m_ptFrom.y = nSelectLineBgnFrom_Old;	/* �͈͑I���J�n�s(���_) */
			m_sSelectBgn.m_ptFrom.x = nSelectColBgnFrom_Old;	/* �͈͑I���J�n��(���_) */
			m_sSelectBgn.m_ptTo.y = nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
			m_sSelectBgn.m_ptTo.x = nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */
			m_sSelect.m_ptFrom.y = nSelectLineFrom_Old;
			m_sSelect.m_ptFrom.x = nSelectColFrom_Old;
			m_sSelect.m_ptTo.y = nSelectLineTo_Old;
			m_sSelect.m_ptTo.x = nSelectColTo_Old;

			/* �I���G���A���폜 */
			DeleteData( true );

			// �폜�O�̑I��͈͂𕜌�����	// 2008.03.26 ryoji
			if( !bBoxData ){
				// �폜���ꂽ�͈͂��l�����đI��͈͂𒲐�����
				if( nSelectLineFrom_PHY == nSelectLineTo_PHY_Old ){	// �I���J�n���폜�����Ɠ���s
					nSelectColmFrom_PHY -= (nSelectColmTo_PHY_Old - nSelectColmFrom_PHY_Old);
				}
				if( nSelectLineTo_PHY == nSelectLineTo_PHY_Old ){	// �I���I�����폜�����Ɠ���s
					nSelectColmTo_PHY -= (nSelectColmTo_PHY_Old - nSelectColmFrom_PHY_Old);
				}
				// Note.
				// (nSelectLineTo_PHY_Old - nSelectLineFrom_PHY_Old) �͎��ۂ̍폜�s���Ɠ����ɂȂ�
				// ���Ƃ����邪�A�i�폜�s���|�P�j�ɂȂ邱�Ƃ�����D
				// ��j�t���[�J�[�\���ł̍s�ԍ��N���b�N���̂P�s�I��
				int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				nSelectLineFrom_PHY -= (nLines_Old - nLines);
				nSelectLineTo_PHY -= (nLines_Old - nLines);

				// ������̑I��͈͂�ݒ肷��
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					nSelectColmFrom_PHY, nSelectLineFrom_PHY,
					&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
				);
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					nSelectColmTo_PHY, nSelectLineTo_PHY,
					&m_sSelect.m_ptTo.x, &m_sSelect.m_ptTo.y
				);
				SetSelectArea( m_sSelect.m_ptFrom.y, m_sSelect.m_ptFrom.x, m_sSelect.m_ptTo.y, m_sSelect.m_ptTo.x );	// 2009.07.25 ryoji
				nCaretPosX_Old = m_sSelect.m_ptTo.x;
				nCaretPosY_Old = m_sSelect.m_ptTo.y;
			}

			// �L�����b�g���ړ�����
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
			m_nCaretPosX_Prev = m_ptCaretPos.x;

			// �폜�ʒu����ړ���ւ̃J�[�\���ړ����A���h�D����ɒǉ�����	// 2008.03.26 ryoji
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;
			pcOpe->m_ptCaretPos_PHY_Before.x = nSelectColmFrom_PHY_Old;
			pcOpe->m_ptCaretPos_PHY_Before.y = nSelectLineFrom_PHY_Old;
			pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;
			pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	DrawSelectArea();

	// �A���h�D�o�b�t�@�̏���
	SetUndoBuffer();

	::GlobalUnlock( hData );
	// 2004.07.12 fotomo/���� �������[���[�N�̏C��
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
		::GlobalFree( hData );
	}

	return S_OK;
}

/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g����
	@date 2008.06.20 ryoji �V�K�쐬
*/
STDMETHODIMP CEditView::PostMyDropFiles( LPDATAOBJECT pDataObject )
{
	HGLOBAL hData = GetGlobalData( pDataObject, CF_HDROP );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );

	// �h���b�v�f�[�^���R�s�[���Ă��ƂœƎ��̃h���b�v�t�@�C���������s��
	HGLOBAL hDrop = ::GlobalAlloc( GHND | GMEM_DDESHARE, nSize );
	memcpy( ::GlobalLock( hDrop ), pData, nSize );
	::GlobalUnlock( hDrop );
	::PostMessage(
		m_hWnd,
		MYWM_DROPFILES,
		(WPARAM)hDrop,
		0
	);

	::GlobalUnlock( hData );
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
		::GlobalFree( hData );
	}

	return S_OK;
}

/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W����
	@date 2008.06.20 ryoji �V�K�쐬
*/
void CEditView::OnMyDropFiles( HDROP hDrop )
{
	// ���ʂɃ��j���[���삪�ł���悤�ɓ��͏�Ԃ��t�H�A�O�����h�E�B���h�E�ɃA�^�b�`����
	int nTid2 = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL );
	int nTid1 = ::GetCurrentThreadId();
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, TRUE );

	// �_�~�[�� STATIC ������ăt�H�[�J�X�𓖂Ă�i�G�f�B�^���O�ʂɏo�Ȃ��悤�Ɂj
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, m_hInstance, NULL );
	::SetFocus(hwnd);

	// ���j���[���쐬����
	POINT pt;
	::GetCursorPos( &pt );
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, _T("�p�X���\��t��(&P)") );
	::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, _T("�t�@�C�����\��t��(&F)") );
	::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu( hMenu, 3, MF_BYPOSITION | MF_STRING, 110, _T("�t�@�C�����J��(&O)") );
	::InsertMenu( hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu( hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, _T("�L�����Z��") );
	int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
								0, hwnd, NULL);
	::DestroyMenu( hMenu );

	::DestroyWindow( hwnd );

	// ���͏�Ԃ��f�^�b�`����
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, FALSE );

	// �I�����ꂽ���j���[�ɑΉ����鏈�������s����
	switch( nId ){
	case 110:	// �t�@�C�����J��
		// �ʏ�̃h���b�v�t�@�C���������s��
		::SendMessage( m_pcEditDoc->m_pcEditWnd->m_hWnd, WM_DROPFILES, (WPARAM)hDrop, 0 );
		break;

	case 100:	// �p�X����\��t����
	case 101:	// �t�@�C������\��t����
		CMemory cmemBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, (LPSTR) NULL, 0 );
		for( UINT i = 0; i < nFiles; i++ ){
			::DragQueryFile( hDrop, i, szPath, sizeof(szPath)/sizeof(TCHAR) );
			if( !::GetLongFileName( szPath, szWork ) )
				continue;
			if( nId == 100 ){	// �p�X��
				::lstrcpy( szPath, szWork );
			}else if( nId == 101 ){	// �t�@�C����
				_tsplitpath( szWork, NULL, NULL, szPath, szExt );
				::lstrcat( szPath, szExt );
			}
			cmemBuf.AppendString( szPath );
			if( nFiles > 1 ){
				cmemBuf.AppendString( m_pcEditDoc->GetNewLineCode().GetValue() );
			}
		}
		::DragFinish( hDrop );

		// �I��͈͂̑I������
		if( IsTextSelected() ){
			DisableSelectArea( true );
		}

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// �s�I�[���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\�������������Ă���
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		// �e�L�X�g�}��
		HandleCommand( F_INSTEXT, true, (LPARAM)cmemBuf.GetStringPtr(), TRUE, 0, 0 );

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		SetSelectArea( m_sSelect.m_ptFrom.y, m_sSelect.m_ptFrom.x, m_ptCaretPos.y, m_ptCaretPos.x );	// 2009.07.25 ryoji
		DrawSelectArea();
		break;
	}

	// ���������
	::GlobalFree( hDrop );
}

CLIPFORMAT CEditView::GetAvailableClipFormat( LPDATAOBJECT pDataObject )
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = ::RegisterClipboardFormat( _T("SAKURAClip") );

	if( IsDataAvailable( pDataObject, cfSAKURAClip ) )
		cf = cfSAKURAClip;
	//else if( IsDataAvailable(pDataObject, CF_UNICODETEXT) )
	//	cf = CF_UNICODETEXT;
	else if( IsDataAvailable(pDataObject, CF_TEXT) )
		cf = CF_TEXT;
	else if( IsDataAvailable(pDataObject, CF_HDROP) )	// 2008.06.20 ryoji
		cf = CF_HDROP;

	return cf;
}

DWORD CEditView::TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect )
{
	if( cf == CF_HDROP )	// 2008.06.20 ryoji
		return DROPEFFECT_LINK;

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

	// 2008.06.21 ryoji
	// Win 98/Me ���ł͊O������̃h���b�O���� GetKeyState() �ł̓L�[��Ԃ𐳂����擾�ł��Ȃ����߁A
	// Drag & Drop �C���^�[�t�F�[�X�œn����� dwKeyState ��p���Ĕ��肷��B
#if 1
	// �h���b�O�����O���E�B���h�E���ǂ����ɂ���Ď󂯕���ς���
	// ���ėp�e�L�X�g�G�f�B�^�ł͂����炪�嗬���ۂ�
	if( pcDragSourceView ){
#else
	// �h���b�O�����ړ����������ǂ����ɂ���Ď󂯕���ς���
	// ��MS ���i�iMS Office, Visual Studio�Ȃǁj�ł͂����炪�嗬���ۂ�
	if( dwEffect & DROPEFFECT_MOVE ){
#endif
		dwEffect &= (MK_CONTROL & dwKeyState)? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else{
		dwEffect &= (MK_SHIFT & dwKeyState)? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool CEditView::IsDragSource( void )
{
	return ( this == m_pcEditDoc->GetDragSourceView() );
}





/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
void CEditView::GetCurrentTextForSearch( CMemory& cmemCurText )
{

	int				i;
	char			szTopic[_MAX_PATH];
//	CMemory			cmemCurText;
	const char*		pLine;
	int				nLineLen;
	int				nIdx;
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
//	const CLayout*	pcLayout;

	cmemCurText.SetString( "" );
	szTopic[0] = '\0';
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �I��͈͂̃f�[�^���擾 */
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
			/* ��������������݈ʒu�̒P��ŏ����� */
			strncpy( szTopic, cmemCurText.GetStringPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = '\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );

			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				m_ptCaretPos.y, nIdx,
				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
			){
				/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
				nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
				nColmTo = LineIndexToColmn( pcLayout, nColmTo );
				/* �I��͈͂̕ύX */
				m_sSelectBgn.m_ptFrom.y = nLineFrom;	/* �͈͑I���J�n�s(���_) */
				m_sSelectBgn.m_ptFrom.x = nColmFrom;	/* �͈͑I���J�n��(���_) */
				m_sSelectBgn.m_ptTo.y = nLineTo;		/* �͈͑I���J�n�s(���_) */
				m_sSelectBgn.m_ptTo.x = nColmTo;		/* �͈͑I���J�n��(���_) */

				m_sSelect.m_ptFrom.y = nLineFrom;
				m_sSelect.m_ptFrom.x = nColmFrom;
				m_sSelect.m_ptTo.y = nLineTo;
				m_sSelect.m_ptTo.x = nColmTo;
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					/* ��������������݈ʒu�̒P��ŏ����� */
					strncpy( szTopic, cmemCurText.GetStringPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = '\0';
				}
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( false );
			}
		}
	}

	/* ����������͉��s�܂� */
	int nLen = (int)lstrlen( szTopic );
	for( i = 0; i < nLen; ++i ){
		if( szTopic[i] == CR || szTopic[i] == LF ){
			szTopic[i] = '\0';
			break;
		}
	}
//	cmemCurText.SetData( szTopic, lstrlen( szTopic ) );
	cmemCurText.SetString( szTopic );
	return;

}


/*!	���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j
	@date 2006.08.23 ryoji �V�K�쐬
*/
void CEditView::GetCurrentTextForSearchDlg( CMemory& cmemCurText )
{
	cmemCurText.SetString( "" );

	if( IsTextSelected() ){	// �e�L�X�g���I������Ă���
		GetCurrentTextForSearch( cmemCurText );
	}
	else{	// �e�L�X�g���I������Ă��Ȃ�
		if( m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// �J�[�\���ʒu�P����擾
		}
		else{
			cmemCurText.SetString( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] );	// ��������Ƃ��Ă���
		}
	}
}


/* �J�[�\���s�A���_�[���C����ON */
void CCaretUnderLine::CaretUnderLineON( bool bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineON( bDraw );
}



/* �J�[�\���s�A���_�[���C����OFF */
void CCaretUnderLine::CaretUnderLineOFF( bool bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineOFF( bDraw );
}


/*! �J�[�\���s�A���_�[���C����ON
	@date 2007.09.09 Moca �J�[�\���ʒu�c�������ǉ�
*/
void CEditView::CaretUnderLineON( bool bDraw )
{

	bool bUnderLine = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp;
	bool bCursorVLine = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp;
	if( !bUnderLine && !bCursorVLine ){
		return;
	}

	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( bCursorVLine ){
		// �J�[�\���ʒu�c���B-1���ăL�����b�g�̍��ɗ���悤�ɁB
		m_nOldCursorLineX = m_nViewAlignLeft + (m_ptCaretPos.x - m_nViewLeftCol)
			* (m_pcEditDoc->GetDocumentAttribute().m_nColmSpace + m_nCharWidth ) - 1;
		if( -1 == m_nOldCursorLineX ){
			m_nOldCursorLineX = -2;
		}
	}else{
		m_nOldCursorLineX = -1;
	}

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nViewAlignLeft - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX
	 && m_nOldCursorLineX <= m_nViewAlignLeft + m_nViewCx
	 && m_bDoing_UndoRedo == FALSE
	){
		// �J�[�\���ʒu�c���̕`��
		// �A���_�[���C���Əc���̌�_�ŁA��������ɂȂ�悤�ɐ�ɏc���������B
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, m_nOldCursorLineX, m_nViewAlignTop, NULL );
		::LineTo(   hdc, m_nOldCursorLineX, m_nViewCy + m_nViewAlignTop );
		// �u�����v�̂Ƃ���2dot�̐��ɂ���B���̍ۃJ�[�\���Ɋ|����Ȃ��悤�ɍ����𑾂�����
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bBoldFont &&
			m_nViewAlignLeft - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX - 1 ){
			::MoveToEx( hdc, m_nOldCursorLineX - 1, m_nViewAlignTop, NULL );
			::LineTo(   hdc, m_nOldCursorLineX - 1, m_nViewCy + m_nViewAlignTop );
		}
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
	if( bUnderLine ){
		m_nOldUnderLineY = m_nViewAlignTop + (m_ptCaretPos.y - m_nViewTopLine)
			 * (m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight) + m_nCharHeight;
		if( -1 == m_nOldUnderLineY ){
			m_nOldUnderLineY = -2;
		}
	}else{
		m_nOldUnderLineY = -1;
	}
	// To Here 2007.09.09 Moca

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nOldUnderLineY >=m_nViewAlignTop
	 && m_bDoing_UndoRedo == FALSE	/* �A���h�D�E���h�D�̎��s���� */
	){
//		MYTRACE( _T("���J�[�\���s�A���_�[���C���̕`��\n") );
		/* ���J�[�\���s�A���_�[���C���̕`�� */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			m_nViewAlignLeft,
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			m_nViewCx + m_nViewAlignLeft,
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
}

/*! �J�[�\���s�A���_�[���C����OFF
	@date 2007.09.09 Moca �J�[�\���ʒu�c�������ǉ�
*/
void CEditView::CaretUnderLineOFF( bool bDraw )
{
	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp &&
			!m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp ){
		return;
	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nOldUnderLineY >=m_nViewAlignTop
		 && !m_bDoing_UndoRedo	/* �A���h�D�E���h�D�̎��s���� */
		){
			/* �J�[�\���s�A���_�[���C���̏����i�������j */
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nViewAlignLeft;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = m_nOldUnderLineY;
			ps.rcPaint.bottom = m_nOldUnderLineY + 1; // 2007.09.09 Moca +1 ����悤��
			HDC hdc = ::GetDC( m_hWnd );
			m_cUnderLine.Lock();
			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			int nSelectLineFrom = m_sSelect.m_ptFrom.y;
			int nSelectLineTo = m_sSelect.m_ptTo.y;
			int nSelectColmFrom = m_sSelect.m_ptFrom.x;
			int nSelectColmTo = m_sSelect.m_ptTo.x;
			m_sSelect.m_ptFrom.y = -1;
			m_sSelect.m_ptTo.y = -1;
			m_sSelect.m_ptFrom.x = -1;
			m_sSelect.m_ptTo.x = -1;
			// �\�Ȃ�݊�BMP����R�s�[���čč��
			OnPaint( hdc, &ps, TRUE );
			//	�I�����𕜌�
			m_sSelect.m_ptFrom.y = nSelectLineFrom;
			m_sSelect.m_ptTo.y = nSelectLineTo;
			m_sSelect.m_ptFrom.x = nSelectColmFrom;
			m_sSelect.m_ptTo.x = nSelectColmTo;
			m_cUnderLine.UnLock();
			ReleaseDC( m_hWnd, hdc );
		}
		m_nOldUnderLineY = -1;
	}
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �J�[�\���ʒu�c��
	if( -1 != m_nOldCursorLineX ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nViewAlignLeft - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace < m_nOldCursorLineX
		 && m_nOldCursorLineX <= m_nViewAlignLeft + m_nViewCx
		 && m_bDoing_UndoRedo == FALSE
		){
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nOldCursorLineX;
			ps.rcPaint.right = m_nOldCursorLineX + 1;
			ps.rcPaint.top = m_nViewAlignTop;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bBoldFont ){
				ps.rcPaint.left += -1;
			}
			HDC hdc = ::GetDC( m_hWnd );
			m_cUnderLine.Lock();
			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			int nSelectLineFrom = m_sSelect.m_ptFrom.y;
			int nSelectLineTo = m_sSelect.m_ptTo.y;
			int nSelectColmFrom = m_sSelect.m_ptFrom.x;
			int nSelectColmTo = m_sSelect.m_ptTo.x;
			m_sSelect.m_ptFrom.y = -1;
			m_sSelect.m_ptTo.y = -1;
			m_sSelect.m_ptFrom.x = -1;
			m_sSelect.m_ptTo.x = -1;
			// �\�Ȃ�݊�BMP����R�s�[���čč��
			OnPaint( hdc, &ps, TRUE );
			//	�I�����𕜌�
			m_sSelect.m_ptFrom.y = nSelectLineFrom;
			m_sSelect.m_ptTo.y = nSelectLineTo;
			m_sSelect.m_ptFrom.x = nSelectColmFrom;
			m_sSelect.m_ptTo.x = nSelectColmTo;
			m_cUnderLine.UnLock();
			ReleaseDC( m_hWnd, hdc );
		}
		m_nOldCursorLineX = -1;
	}
	// To Here 2007.09.09 Moca
	return;
}

/*!
	�����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������

	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta ���̂�CEditWnd�ֈړ�
*/
void CEditView::SendStatusMessage( const TCHAR* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
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
	
	if (bUnicode){
		
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

/*!
	@date 2003/02/18 ai
	@param flag [in] ���[�h(true:�o�^, false:����)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	nCol;
	int	nLine;
	int	mode;

	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !m_bDrawSWITCH ){
		return;
	}

	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// �Ί��ʂ̌���&�o�^
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	mode = 2;

	if( ( flag == true ) && !IsTextSelected() && !m_bDrawSelectArea
		&& ( m_bBeginBoxSelect == false ) && SearchBracket( m_ptCaretPos.x, m_ptCaretPos.y, &nCol, &nLine, &mode ) )
	{
		// �o�^�w��(flag=true)			&&
		// �e�L�X�g���I������Ă��Ȃ�	&&
		// �I��͈͂�`�悵�Ă��Ȃ�		&&
		// ��`�͈͑I�𒆂łȂ�			&&
		// �Ή����銇�ʂ���������		�ꍇ
		if ( ( nCol >= m_nViewLeftCol ) && ( nCol <= m_nViewLeftCol + m_nViewColNum )
			&& ( nLine >= m_nViewTopLine ) && ( nLine <= m_nViewTopLine + m_nViewRowNum ) )
		{
			// �\���̈���̏ꍇ

			// ���C�A�E�g�ʒu���畨���ʒu�֕ϊ�(�����\���ʒu��o�^)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( nCol, nLine, &m_ptBracketPairPos_PHY.x, &m_ptBracketPairPos_PHY.y );
			m_ptBracketCaretPos_PHY.y = m_ptCaretPos_PHY.y;
			if( 0 == ( mode & 4 ) ){
				// �J�[�\���̌�������ʒu
				m_ptBracketCaretPos_PHY.x = m_ptCaretPos_PHY.x;
			}else{
				// �J�[�\���̑O�������ʒu
				m_ptBracketCaretPos_PHY.x = m_ptCaretPos_PHY.x - m_nCharSize;
			}
			return;
		}
	}

	// ���ʂ̋����\���ʒu��񏉊���
	m_ptBracketPairPos_PHY.x  = -1;
	m_ptBracketPairPos_PHY.y  = -1;
	m_ptBracketCaretPos_PHY.x = -1;
	m_ptBracketCaretPos_PHY.y = -1;

	return;
}

/*!
	�Ί��ʂ̋����\��
	@date 2002/09/18 ai
	@date 2003/02/18 ai �ĕ`��Ή��̈ב����
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	int			i;
	int			nCol;
	int			nLine;
	COLORREF	crBackOld;
	COLORREF	crTextOld;
	HFONT		hFontOld;

	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !m_bDrawSWITCH ){
		return;
	}

	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// ���ʂ̋����\���ʒu�����o�^�̏ꍇ�͏I��
	if( ( m_ptBracketPairPos_PHY.x  < 0 ) || ( m_ptBracketPairPos_PHY.y  < 0 )
	 || ( m_ptBracketCaretPos_PHY.x < 0 ) || ( m_ptBracketCaretPos_PHY.y < 0 ) ){
		return;
	}

	// �`��w��(bDraw=true)				����
	// ( �e�L�X�g���I������Ă���		����
	//   �I��͈͂�`�悵�Ă���			����
	//   ��`�͈͑I��					����
	//   �t�H�[�J�X�������Ă��Ȃ�		����
	//   �A�N�e�B�u�ȃy�C���ł͂Ȃ� )	�ꍇ�͏I��
	if( bDraw
	 &&( IsTextSelected() || m_bDrawSelectArea || m_bBeginBoxSelect || !m_bDrawBracketPairFlag
	 || ( m_pcEditDoc->m_nActivePaneIndex != m_nMyIndex ) ) ){
		return;
	}

	HDC hdc = ::GetDC( m_hWnd );
	STypeConfig *TypeDataPtr = &( m_pcEditDoc->GetDocumentAttribute() );

	for( i = 0; i < 2; i++ )
	{
		// i=0:�Ί���,i=1:�J�[�\���ʒu�̊���
		// 2011.11.23 ryoji �Ί��� -> �J�[�\���ʒu�̊��� �̏��ɏ���������ύX
		//   �� { �� } ���قȂ�s�ɂ���ꍇ�� { �� BS �ŏ����� } �̋����\������������Ȃ����iWiki BugReport/89�j�̑΍�
		//   �� ���̏����ύX�ɂ��J�[�\���ʒu�����ʂłȂ��Ȃ��Ă��Ă��Ί��ʂ�����ΑΊ��ʑ��̋����\���͉��������

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY.x,  m_ptBracketPairPos_PHY.y,  &nCol, &nLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY.x, m_ptBracketCaretPos_PHY.y, &nCol, &nLine );
		}

		if ( ( nCol >= m_nViewLeftCol ) && ( nCol <= m_nViewLeftCol + m_nViewColNum )
			&& ( nLine >= m_nViewTopLine ) && ( nLine <= m_nViewTopLine + m_nViewRowNum ) )
		{	// �\���̈���̏ꍇ
			if( !bDraw && m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( nCol, nLine ) ) )
			{	// �I��͈͕`��ς݂ŏ����Ώۂ̊��ʂ��I��͈͓��̏ꍇ
				continue;
			}
			const CLayout* pcLayout;
			int			nLineLen;
			const char*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLine, &nLineLen, &pcLayout );
			if( pLine )
			{
				EColorIndexType nColorIndex;
				int	OutputX = LineColmnToIndex( pcLayout, nCol );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, m_nCharSize ) ){
						// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
						//nColorIndex = GetColorIndex( hdc, pcLayout, OutputX );
						if( i == 0 ){
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketPairPos_PHY.x );
						}else{
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketCaretPos_PHY.x );
						}
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}
				hFontOld = (HFONT)::SelectObject( hdc, m_pcViewFont->GetFontHan() );
				m_hFontOld = NULL;
				crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
				crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );
				SetCurrentColor( hdc, nColorIndex );

				int nHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
				int nLeft = (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				int nTop  = ( nLine - m_nViewTopLine ) * nHeight + m_nViewAlignTop;

				// 03/03/03 ai �J�[�\���̍��Ɋ��ʂ����芇�ʂ������\������Ă����Ԃ�Shift+���őI���J�n�����
				//             �I��͈͓��ɔ��]�\������Ȃ�������������̏C��
				if( ( nCol == m_ptCaretPos.x ) && ( m_bCaretShowFlag == true ) ){
					HideCaret_( m_hWnd );	// �L�����b�g����u������̂�h�~
					DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					DispVerticalLines( hdc, nTop, nTop + nHeight, nCol, nCol + m_nCharSize );
					ShowCaret_( m_hWnd );	// �L�����b�g����u������̂�h�~
				}else{
					DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					DispVerticalLines( hdc, nTop, nTop + nHeight, nCol, nCol + m_nCharSize );
				}

				if( NULL != m_hFontOld ){
					::SelectObject( hdc, m_hFontOld );
					m_hFontOld = NULL;
				}
				::SetTextColor( hdc, crTextOld );
				::SetBkColor( hdc, crBackOld );
				::SelectObject( hdc, hFontOld );

				if( ( m_pcEditDoc->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( nLine == m_ptCaretPos.y ) || ( nLine - 1 == m_ptCaretPos.y ) ) ){	// 03/02/27 ai �s�̊Ԋu��"0"�̎��ɃA���_�[���C���������鎖������׏C��
					m_cUnderLine.CaretUnderLineON( true );
				}
			}
		}
	}

	::ReleaseDC( m_hWnd, hdc );

	return;
}

/*! �w��ʒu��ColorIndex�̎擾
	CEditView::DispLineNew�����ɂ�������CEditView::DispLineNew��
	�C�����������ꍇ�́A�������C�����K�v�B
*/
EColorIndexType CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout* const	pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	EColorIndexType			nCOMMENTMODE;
	EColorIndexType			nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	EColorIndexType			nColorIndex;

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->m_pCDocLine->m_cLine.GetStringLength();	// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
		pLine = pcLayout->m_pCDocLine->m_cLine.GetStringPtr();			// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�

		// 2005.11.20 Moca �F���������Ȃ����Ƃ�������ɑΏ�
		const CLayout* pcLayoutLineFirst = pcLayout;
		// �_���s�̍ŏ��̃��C�A�E�g�����擾����
		while( 0 != pcLayoutLineFirst->m_ptLogicPos.x ){
			pcLayoutLineFirst = pcLayoutLineFirst->m_pPrev;
		}
		nCOMMENTMODE = pcLayoutLineFirst->m_nTypePrev;
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
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

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

		while( nPos <= nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
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
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* ���݂̐F���w�� */
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}else
					if( bSearchStringMode
					 && nSearchEnd == nPos
					){
						nBgn = nPos;
						/* ���݂̐F���w�� */
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
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
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = (EColorIndexType)(COLORIDX_REGEX_FIRST + nMatchColor);	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[0].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[0].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[0].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComments[1].Match_CommentFrom(nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComments[1].Match_CommentTo(nPos + (int)lstrlen( TypeDataPtr->m_cBlockComments[1].getBlockCommentFrom() ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (nNumLen = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						nNumLen = nPos + nNumLen;
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = nNumLen;
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* �L�[���[�h������̏I�[��T�� */
						int nKeyEnd;
						for( nKeyEnd = nPos + 1; nKeyEnd <= nLineLen - 1; ++nKeyEnd ){
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
									/* ���݂̐F���w�� */
									nBgn = nPos;
									nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + n);
									nCOMMENTEND = nKeyEnd;
									if( !bSearchStringMode ){
										nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
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
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE & COLORIDX_REGEX_BIT ){	//���K�\���L�[���[�h1�`10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == TAB ){
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					nBgn = nPos + 2;
					nCharChars = 2;
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
				 && !(nCOMMENTMODE & COLORIDX_REGEX_BIT) )
				{
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
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos > nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}

/*!	�}�����[�h�擾

	@date 2005.10.02 genta �Ǘ����@�ύX�̂��ߊ֐���
*/
bool CEditView::IsInsMode(void) const
{
	return m_pcEditDoc->IsInsMode();
}

void CEditView::SetInsMode(bool mode)
{
	m_pcEditDoc->SetInsMode( mode );
}

/*! �A���h�D�o�b�t�@�̏��� */
void CEditView::SetUndoBuffer( bool bPaintLineNumber )
{
	if( NULL != m_pcOpeBlk && m_pcOpeBlk->Release() == 0 ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
			/* ����̒ǉ� */
			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );

			if( bPaintLineNumber
			 && m_pcEditDoc->m_cOpeBuf.GetCurrentPointer() == 1 )	// �SUndo��Ԃ���̕ύX���H	// 2009.03.26 ryoji
				RedrawLineNumber();	// ���y�C���̍s�ԍ��i�ύX�s�j�\�����X�V �� �ύX�s�݂̂̕\���X�V�ōς܂��Ă���ꍇ�����邽��

			if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
				m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V
		}
		else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}
}

/*[EOF]*/
