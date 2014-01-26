/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2002/01/14 YAZAKI PrintPreview�̕���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001-2002, YAZAKI
	Copyright (C) 2002, aroka, genta, MIK
	Copyright (C) 2003, MIK, genta, wmlhq
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji, aroka, fon, yukihane. Moca
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include "CEditDoc.h"
#include "CShareData.h"
#include "CTabWnd.h"	//@@@ 2003.05.31 MIK
#include "CFuncKeyWnd.h"
#include "CMenuDrawer.h"
#include "CImageListMgr.h"

//by �S
#include "CDropTarget.h"
#include "CViewFont.h"

static const int MENUBAR_MESSAGE_MAX_LEN = 30;

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
class CPrintPreview;// 2002/2/10 aroka
class CDropTarget;


//���C���E�B���h�E���R���g���[��ID
#define IDT_EDIT		455  // 20060128 aroka
#define IDT_TOOLBAR		456
#define IDT_CAPTION		457
#define IDT_FIRST_IDLE	458
#define IDT_SYSMENU		1357
#define ID_TOOLBAR		100

struct STabGroupInfo{
	HWND			hwndTop;
	WINDOWPLACEMENT	wpTop;

	STabGroupInfo() : hwndTop(NULL) { }
	bool IsValid() const{ return hwndTop!=NULL; }
};

//! �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class CEditWnd
{
public:
	static CEditWnd* getInstance()
	{
		static CEditWnd instance;

		return &instance;
	}

private:
	CEditWnd();
	CEditWnd(CEditWnd const&);
	void operator=(CEditWnd const&);

public:
	~CEditWnd();
	/*
	|| �����o�֐�
	*/
	//	Mar. 7, 2002 genta �����^�C�v�p�����ǉ�
	// 2007.06.26 ryoji �O���[�v�w������ǉ�
	//! �쐬
	HWND Create(
		HINSTANCE,
		HWND,
		CEditDoc*		pcEditDoc,
		CImageListMgr*	pcIcons,
		int nGroup
	);
	void _GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(int& nWinOX, int& nWinOY, int& nWinCX, int& nWinCY, int nGroup, const STabGroupInfo& sTabGroupInfo);
	HWND _CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo);
	void _AdjustInMonitor(const STabGroupInfo& sTabGroupInfo);

	void OpenDocumentWhenStart(const char*, ECodeType, bool);	//!< [in] �ŏ��ɊJ���t�@�C���̃p�X�DNULL�̂Ƃ��J���t�@�C�������D
	void SetDocumentTypeWhenCreate(ECodeType, bool, int = -1);	//!< [in] �����^�C�v�D-1�̂Ƃ������w�薳���D

	void UpdateCaption();

	void MessageLoop( void );								/* ���b�Z�[�W���[�v */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W���� */


	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE ���� */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	BOOL DoMouseWheel( WPARAM wParam, LPARAM lParam );	// �}�E�X�z�C�[������	// 2007.10.16 ryoji
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );
	int	OnClose();	/* �I�����̏��� */
	void OnDropFiles( HDROP );	/* �t�@�C�����h���b�v���ꂽ */
	BOOL OnPrintPageSetting( void );/* ����y�[�W�ݒ� */
	LRESULT OnTimer( WPARAM, LPARAM );	// WM_TIMER ����	// 2007.04.03 ryoji
	void OnEditTimer( void );	/* �^�C�}�[�̏��� */
	void OnToolbarTimer( void );	/* �^�C�}�[�̏��� 20060128 aroka */
	void UpdateToolbar( void );		// �c�[���o�[�̕\�����X�V����		// 2008.09.23 nasukoji
	void OnCaptionTimer( void );
	void OnSysMenuTimer( void );
	void OnCommand( WORD, WORD , HWND );
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);

	void CreateToolBar( void );			/* �c�[���o�[�쐬 */
	void DestroyToolBar( void );		/* �c�[���o�[�j�� */
	void CreateStatusBar( void );		/* �X�e�[�^�X�o�[�쐬 */
	void DestroyStatusBar( void );		/* �X�e�[�^�X�o�[�j�� */
	void AcceptSharedSearchKey();		/* �����{�b�N�X���X�V */

	//�t�@�C�����ύX�ʒm
	void ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep );	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	void InitMenu( HMENU, UINT, BOOL );
	void InitMenubarMessageFont(void);
	LRESULT WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull );	/*!< �E�B���h�E�ꗗ���j���[�쐬���� */	// 2006.03.23 fon
	LRESULT PopupWinList( bool bMousePos );	/*!< �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon	// 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
	void SetMenuFuncSel( HMENU hMenu, int nFunc, const TCHAR* sKey, bool flag );				// �\���̓��I�I��	2010/5/19 Uchi
	void LayoutToolBar( void );			/* �c�[���o�[�̔z�u���� */			// 2006.12.19 ryoji
	void LayoutFuncKey( void );			/* �t�@���N�V�����L�[�̔z�u���� */	// 2006.12.19 ryoji
	void LayoutTabBar( void );			/* �^�u�o�[�̔z�u���� */			// 2006.12.19 ryoji
	void LayoutStatusBar( void );		/* �X�e�[�^�X�o�[�̔z�u���� */		// 2006.12.19 ryoji
	void EndLayoutBars( BOOL bAdjust = TRUE );	/* �o�[�̔z�u�I������ */	// 2006.12.19 ryoji


	void PrintPreviewModeONOFF( void );	/* ����v���r���[���[�h�̃I��/�I�t */
	void SetWindowIcon( HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel( BOOL bState ) { m_bPageScrollByWheel = bState; }	// �z�C�[������ɂ��y�[�W�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.12 nasukoji
	void SetHScrollByWheel( BOOL bState ) { m_bHorizontalScrollByWheel = bState; }	// �z�C�[������ɂ�鉡�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.12 nasukoji
	void ClearMouseState( void );		// 2009.01.12 nasukoji	�}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j
	//! ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	bool IsActiveApp() const { return m_bIsActiveApp; }



	BOOL IsPageScrollByWheel() const { return m_bPageScrollByWheel; }		// �z�C�[������ɂ��y�[�W�X�N���[���L��	// 2009.01.12 nasukoji
	BOOL IsHScrollByWheel() const { return m_bHorizontalScrollByWheel; }	// �z�C�[������ɂ�鉡�X�N���[���L��		// 2009.01.12 nasukoji

	void PrintMenubarMessage( const TCHAR* msg );
	void SendStatusMessage( const TCHAR* msg );		//	Dec. 4, 2002 genta ���̂�CEditView����ړ�
	//	Jul. 9, 2005 genta ���j���[�o�[�E�[�ɂ͏o�������Ȃ����߂̃��b�Z�[�W���o��
	void SendStatusMessage2( const char* msg );
	/*!	SendStatusMessage2()�������ڂ����邩��\�߃`�F�b�N
		@date 2005.07.09 genta
		@note ����SendStatusMessage2()�ŃX�e�[�^�X�o�[�\���ȊO�̏�����ǉ�
		����ꍇ�ɂ͂�����ύX���Ȃ��ƐV�����ꏊ�ւ̏o�͂��s���Ȃ��D
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective(void) const {
		return NULL != m_hwndStatusBar;
	}


	void WindowTopMost( int ); // 2004.09.21 Moca

	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_Redraw();
	void SetActivePane( int );	/* �A�N�e�B�u�ȃy�C����ݒ� */
	int GetActivePane( void ) const { return m_nActivePaneIndex; }	/* �A�N�e�B�u�ȃy�C�����擾 */ //2007.08.26 kobake const�ǉ�
	void SetDrawSwitchOfAllViews( bool bDraw );					/* ���ׂẴy�C���̕`��X�C�b�`��ݒ肷�� */	// 2008.06.08 ryoji
	void RedrawAllViews( CEditView* pcViewExclude );				/* ���ׂẴy�C����Redraw���� */
	BOOL DetectWidthOfLineNumberAreaAllPane( bool bRedraw );	/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
	BOOL WrapWindowWidth( int nPane );	/* �E�[�Ő܂�Ԃ� */	// 2008.06.08 ryoji
	BOOL UpdateTextWrap( void );		/* �܂�Ԃ����@�֘A�̍X�V */	// 2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB���Ɛ܂�Ԃ��ʒu�̍X�V
	void ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineKetas );
	//	Aug. 14, 2005 genta
	CLogicPoint* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView( CLogicPoint* posary );
	// �݊�BMP�ɂ���ʃo�b�t�@ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); //!< CEditView�̉�ʃo�b�t�@���폜

	void SetFocusSearchBox( void ) const;			/* �c�[���o�[�����{�b�N�X�փt�H�[�J�X���ړ� */	// 2006.06.04 yukihane
	void SetDebugModeON( void );	/* �f�o�b�O���j�^���[�h�ɐݒ� */
	void SetDebugModeOFF( void );

	const CEditView& GetActiveView() const { return *m_pcEditView; }
	CEditView& GetActiveView() { return *m_pcEditView; }
	bool IsEnablePane(int n) const { return 0 <= n && n < m_nEditViewCount; }
	int	GetAllViewCount() const { return m_nEditViewCount; }

	CEditView* GetDragSourceView() const { return m_pcDragSourceView; }
	void SetDragSourceView( CEditView* pcDragSourceView ) { m_pcDragSourceView = pcDragSourceView; }

	enum EIconClickStatus{
		icNone,
		icDown,
		icClicked,
		icDoubleClicked
	};
	int	CreateFileDropDownMenu( HWND );	//�J��(�h���b�v�_�E��)	//@@@ 2002.06.15 MIK
	void Timer_ONOFF( bool ); /* �X�V�̊J�n�^��~ 20060128 aroka */

	//! ��������m_nTimerCount���C���N�������g
	void IncrementTimerCount(int nInterval)
	{
		m_nTimerCount++;
		if( nInterval <= m_nTimerCount ){ // 2012.11.29 aroka �Ăяo���Ԋu�̃o�O�C��
			m_nTimerCount = 0;
		}
	}

	void CreateAccelTbl( void ); // �E�B���h�E���̃A�N�Z�����[�^�e�[�u���쐬(Wine�p)
	void DeleteAccelTbl( void ); // �E�B���h�E���̃A�N�Z�����[�^�e�[�u���j��(Wine�p)

	/* IDropTarget���� */	// 2008.06.20 ryoji
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );

	void	ProcSearchBox( MSG* );	//����(�{�b�N�X)

	//	Jul. 21, 2003 genta ToolBar��Owner Draw
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	/*
	|| �����o�ϐ�
	*/
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	CEditDoc*		m_pcEditDoc;
	HWND			m_hwndParent;
    HWND			m_hwndReBar;	// Rebar �E�B���h�E	//@@@ 2006.06.17 ryoji
    HWND			m_hwndToolBar;
	HWND			m_hwndStatusBar;
	HWND			m_hwndProgressBar;
	DLLSHAREDATA*	m_pShareData;

	CFuncKeyWnd		m_CFuncKeyWnd;
	CTabWnd			m_cTabWnd;		//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	CMenuDrawer		m_CMenuDrawer;
	bool			m_bDragMode;
	POINT			m_ptDragPosOrg;
	CDropTarget*	m_pcDropTarget;
	CPrintPreview*	m_pPrintPreview;	//!< ����v���r���[�\�����B�K�v�ɂȂ����Ƃ��̂݃C���X�^���X�𐶐�����B

	CSplitterWnd	m_cSplitterWnd;		//!< �����t���[��
	CEditView*		m_pcDragSourceView;	//!< �h���b�O���̃r���[
	CViewFont*		m_pcViewFont;		//!< �t�H���g

	CImageListMgr*	m_pcIcons;

	CEditView*		m_pcEditViewArr[4];			//!< �r���[ 
	CEditView*		m_pcEditView;				//!< �L���ȃr���[
	int				m_nActivePaneIndex;			//!< �L���ȃr���[��index
	int				m_nEditViewCount;			//!< �L���ȃr���[�̐�
	const int		m_nEditViewMaxCount;		//!< �r���[�̍ő吔=4

private:
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;

	HWND	m_hwndSearchBox;
	HFONT	m_hFontSearchBox;
	bool	m_bIsActiveApp;	//!< ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	LPTSTR	m_pszLastCaption;
	LPTSTR	m_pszMenubarMessage; //!< ���j���[�o�[�E�[�ɕ\�����郁�b�Z�[�W
	int 	m_nTimerCount; //!< OnTimer�p 2003.08.29 wmlhq
	int		m_nCurrentFocus;	//!< ���݂̃t�H�[�J�X���
	int		m_nWinSizeType;	//!< �T�C�Y�ύX�̃^�C�v
	BOOL	m_bPageScrollByWheel;		//!< �z�C�[������ɂ��y�[�W�X�N���[������	// 2009.01.12 nasukoji
	BOOL	m_bHorizontalScrollByWheel;	//!< �z�C�[������ɂ�鉡�X�N���[������		// 2009.01.12 nasukoji
	HACCEL	m_hAccelWine;		//!< �E�B���h�E���̃A�N�Z�����[�^�e�[�u���̃n���h��(Wine�p)	// 2009.08.15 nasukoji
	HACCEL	m_hAccel;			//!< �A�N�Z�����[�^�e�[�u��(���L or �E�B���h�E��)

	HFONT		m_hFontCaretPosInfo;	//!< �L�����b�g�̍s���ʒu�\���p�t�H���g
	int			m_nCaretPosInfoCharWidth;	//!< �L�����b�g�̍s���ʒu�\���p�t�H���g�̕�
	int			m_nCaretPosInfoCharHeight;	//!< �L�����b�g�̍s���ʒu�\���p�t�H���g�̍���
	int			m_pnCaretPosInfoDx[64];	// ������`��p�������z��

	BOOL				m_bUIPI;		// �G�f�B�^�|�g���C�Ԃł�UI���������m�F�p�t���O	// 2007.06.07 ryoji
	EIconClickStatus	m_IconClicked;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */


/*[EOF]*/
