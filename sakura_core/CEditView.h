/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji, Moca, maru
	Copyright (C) 2007, ryoji, maru, genta, Moca, nasukoji
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, nasukoji, ryoji, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_

#define _CARETMARGINRATE 20
#define SCROLLMARGIN_LEFT 4
#define SCROLLMARGIN_RIGHT 4

#include <windows.h>
#include "CShareData.h"
#include "CViewFont.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEol.h"

class CEditWnd;
class CEditView;

class CDropTarget; /// 2002/2/3 aroka �w�b�_�y�ʉ�
class CMemory;///
class COpe;///
class COpeBlk;///
class CSplitBoxWnd;///
class CDlgCancel;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka �w�b�_�y�ʉ� to here
class CEditDoc;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CLayout;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class CDocLine;
class CMigemo;	// 2004.09.14 isearch
class CSMacroMgr;

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

#if defined(__GNUC__)
#else
#if !defined(RECONVERTSTRING) && (WINVER < 0x040A)
typedef struct tagRECONVERTSTRING {
    DWORD dwSize;
    DWORD dwVersion;
    DWORD dwStrLen;
    DWORD dwStrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwTargetStrLen;
    DWORD dwTargetStrOffset;
} RECONVERTSTRING, *PRECONVERTSTRING;
#endif // RECONVERTSTRING
#endif // defined(__GNUC__)

///	�}�E�X����R�}���h�����s���ꂽ�ꍇ�̏�ʃr�b�g
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CCaretUnderLine
{
public:
	CCaretUnderLine()
	{
		m_nLockCounter = 0;
	}
	//	�\����\����؂�ւ����Ȃ��悤�ɂ���
	void Lock()
	{
		m_nLockCounter++;
	}
	//	�\����\����؂�ւ�����悤�ɂ���
	void UnLock()
	{
		m_nLockCounter--;
		if (m_nLockCounter < 0){
			m_nLockCounter = 0;
		}
	}
	void CaretUnderLineON( bool );								// �J�[�\���s�A���_�[���C����ON
	void CaretUnderLineOFF( bool );								// �J�[�\���s�A���_�[���C����OFF
	void SetView( CEditView* pcEditView ){
		m_pcEditView = pcEditView;
	}
protected:
	/* ���b�N�J�E���^�B0�̂Ƃ��́A���b�N����Ă��Ȃ��BUnLock���Ă΂ꂷ���Ă����ɂ͂Ȃ�Ȃ� */
	int m_nLockCounter;
	CEditView* m_pcEditView;
};

/*!
	@brief �����E�B���h�E�̊Ǘ�
	
	1�̕����E�B���h�E�ɂ�1��CEditDoc�I�u�W�F�N�g�����蓖�Ă��A
	1��CEditDoc�I�u�W�F�N�g�ɂ��A4��CEditViwe�I�u�W�F�N�g�����蓖�Ă���B
	�E�B���h�E���b�Z�[�W�̏����A�R�}���h���b�Z�[�W�̏����A
	��ʕ\���Ȃǂ��s���B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class CEditView
{
public:
	/* Constructors */
	CEditView( CEditWnd* pcEditWnd );
	~CEditView();
	/* �������n�����o�֐� */
	BOOL Create(
		HINSTANCE	hInstance,
		HWND		hwndParent,	//!< �e
		CEditDoc*	pcEditDoc,	//!< �Q�Ƃ���h�L�������g
		int			nMyIndex,	//!< �r���[�̃C���f�b�N�X
		BOOL		bShow		//!< �쐬���ɕ\�����邩�ǂ���
	);
	BOOL CreateScrollBar();		/* �X�N���[���o�[�쐬 */	// 2006.12.19 ryoji
	void DestroyScrollBar();	/* �X�N���[���o�[�j�� */	// 2006.12.19 ryoji
	/* ��� */
	//<< 2002/03/29 Azumaiya �C�����C���֐���
	BOOL IsTextSelected( void ) const		/* �e�L�X�g���I������Ă��邩 */
	{
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		// ���ׂĂ� or ���Z������ɁA�r�b�g���]���čŏ�ʃr�b�g(�����t���O)��Ԃ��B
		/* ���Ȃ킿�A�����ꂩ���ЂƂł�-1(0xFFFF)�Ȃ�AFALSE��Ԃ��H */
		return ~((DWORD)(m_sSelect.m_ptFrom.y|m_sSelect.m_ptTo.y|m_sSelect.m_ptFrom.x|m_sSelect.m_ptTo.x)) >> 31;
	}
	BOOL IsTextSelecting( void )	/* �e�L�X�g�̑I�𒆂� */
	{
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		return m_bSelectingLock||IsTextSelected();
	}
	//>> 2002/03/29 Azumaiya
	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�E�擾
	bool IsInsMode() const;
	void SetInsMode(bool);

	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	//
	void OnChangeSetting();										/* �ݒ�ύX�𔽉f������ */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* �ʏ�̕`�揈�� */
	void OnTimer( HWND, UINT, UINT_PTR, DWORD );
	//�E�B���h�E
	void OnSize( int, int );							/* �E�B���h�E�T�C�Y�̕ύX���� */
	void OnMove( int, int, int, int );
	//�t�H�[�J�X
	void OnSetFocus( void );
	void OnKillFocus( void );
	//�X�N���[��
	CLayoutInt  OnVScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	CLayoutInt  OnHScroll( int, int );							/* �����X�N���[���o�[���b�Z�[�W���� */
	//�}�E�X
	void OnLBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
	void OnMOUSEMOVE( WPARAM, int, int );				/* �}�E�X�ړ��̃��b�Z�[�W���� */
	void OnLBUTTONUP( WPARAM, int, int );				/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
	void OnLBUTTONDBLCLK( WPARAM, int , int );			/* �}�E�X���{�^���_�u���N���b�N */
	void OnRBUTTONDOWN( WPARAM, int, int );				/* �}�E�X�E�{�^������ */
	void OnRBUTTONUP( WPARAM, int, int );				/* �}�E�X�E�{�^���J�� */
	void OnMBUTTONDOWN( WPARAM, int, int );				/* �}�E�X���{�^������ */
	void OnMBUTTONUP( WPARAM, int, int );				/* �}�E�X���{�^���J�� */			// 2009.01.12 nasukoji
	void OnXLBUTTONDOWN( WPARAM, int, int );			/* �}�E�X�T�C�h�{�^��1���� */
	void OnXLBUTTONUP( WPARAM, int, int );				/* �}�E�X�T�C�h�{�^��1�J�� */		// 2009.01.12 nasukoji
	void OnXRBUTTONDOWN( WPARAM, int, int );			/* �}�E�X�T�C�h�{�^��2���� */
	void OnXRBUTTONUP( WPARAM, int, int );				/* �}�E�X�T�C�h�{�^��2�J�� */		// 2009.01.12 nasukoji
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );				/* �}�E�X�z�C�[���̃��b�Z�[�W���� */
	bool IsSpecialScrollMode( int );					/* �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷�� */		// 2009.01.12 nasukoji

	BOOL HandleCommand( int, bool, LPARAM, LPARAM, LPARAM, LPARAM );
	/* �R�}���h���� */
	void CaretUnderLineON( bool );								/* �J�[�\���s�A���_�[���C����ON */
	void CaretUnderLineOFF( bool );								/* �J�[�\���s�A���_�[���C����OFF */
	void AdjustScrollBars( void );								/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	int GetWrapOverhang( void ) const;							/* �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z */	// 2008.06.08 ryoji
	int ViewColNumToWrapColNum( int nViewColNum ) const;		/* �u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z���� */	// 2008.06.08 ryoji
	int  MoveCursor( int, int, bool, int = _CARETMARGINRATE );	/* �s���w��ɂ��J�[�\���ړ� */
	int MoveCursorProperly( int, int, bool, int = _CARETMARGINRATE, int = 0 );	/* �s���w��ɂ��J�[�\���ړ��i���W�����t���j */
	// 2006.07.09 genta �s���w��ɂ��J�[�\���ړ�(�I��̈���l��)
	void MoveCursorSelecting( CLayoutPoint ptWk_CaretPos, bool, int = _CARETMARGINRATE );
	BOOL GetAdjustCursorPos( CLayoutPoint* pptPosXY );	// �������J�[�\���ʒu���Z�o����
	BOOL DetectWidthOfLineNumberArea( BOOL );					/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	int DetectWidthOfLineNumberArea_calculate( void );			/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
	void DisableSelectArea( bool );								/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	void SetFont( void );										/* �t�H���g�̕ύX */
	void RedrawAll( void );										/* �t�H�[�J�X�ړ����̍ĕ`�� */
	void Redraw( void );										// 2001/06/21 asa-o �ĕ`��
	void RedrawLineNumber( void );								/* �s�ԍ��ĕ`�� */	// 2009.03.26 ryoji
	void CopyViewStatus( CEditView* );							/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
	DWORD DoGrep( const CMemory*, const CMemory*, const CMemory*, BOOL, const SSearchOption&, ECodeType, BOOL, int );	/* Grep���s */
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int DoGrepTree( CDlgCancel*, HWND, const char*, int*, const TCHAR*, const TCHAR*, BOOL, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int, int* );
	/* Grep���s */	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	Mar. 28, 2004 genta �s�v�Ȉ������폜
	int DoGrepFile( CDlgCancel*, HWND, const char*, int*, const char*, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int*, const TCHAR*, CMemory& );
	/* Grep���ʂ�pszWork�Ɋi�[ */
	void SetGrepResult(
		/* �f�[�^�i�[�� */
		char*		pWork,
		int*		pnWorkLen,
		/* �}�b�`�����t�@�C���̏�� */
		const TCHAR*		pszFullPath,	//	�t���p�X
		const TCHAR*		pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		/* �}�b�`�����s�̏�� */
		int			nLine,			//	�}�b�`�����s�ԍ�
		int			nColumn,		//	�}�b�`�������ԍ�
		const char*		pCompareData,	//	�s�̕�����
		int			nLineLen,		//	�s�̕�����̒���
		int			nEolCodeLen,	//	EOL�̒���
		/* �}�b�`����������̏�� */
		const char*		pMatchData,		//	�}�b�`����������
		int			nMatchLen,		//	�}�b�`����������̒���
		/* �I�v�V���� */
		BOOL		bGrepOutputLine,
		int			nGrepOutputStyle
	);
	
	//	Jun. 16, 2000 genta
	bool  SearchBracket( const CLayoutPoint& ptLayout, CLayoutPoint* pptLayoutNew, int* mode );	// �Ί��ʂ̌���		// mode�̒ǉ� 02/09/18 ai
	bool  SearchBracketForward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const char* upChar, const char* dnChar, int* mode, int size );	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const char* dnChar, const char* upChar, int* mode, int size );	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
	void SetUndoBuffer( bool bPaintLineNumber = false );					// �A���h�D�o�b�t�@�̏���

// 2002/01/19 novice public�����ɕύX
	BOOL GetSelectedData( CMemory*, BOOL, const char*, BOOL, bool bAddCRLFWhenCopy, EEolType neweol = EOL_UNKNOWN);/* �I��͈͂̃f�[�^���擾 */
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON( void );	// IME ON��	// 2006.12.04 ryoji
	void CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight );		/* �L�����b�g�̍쐬 */	// 2006.12.07 ryoji
	void ShowEditCaret( void );									/* �L�����b�g�̕\���E�X�V */
	int HokanSearchByFile( const char*, bool, CMemory**, int, int ); // 2003.06.25 Moca

	/*!	CEditView::KeyWordHelpSearchDict�̃R�[�����w��p���[�J��ID
		@date 2006.04.10 fon �V�K�쐬
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon
	BOOL KeySearchCore( const CMemory* pcmemCurText );	// 2006.04.10 fon
	
	//	Jan. 10, 2005 �C���N�������^���T�[�`
	bool IsISearchEnabled(int nCommand) const;

	//	Jan.  8, 2006 genta �܂�Ԃ��g�O�����씻��
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( int& );

	// 2006.05.14 Moca �݊�BMP�ɂ���ʃo�b�t�@
	bool CreateOrUpdateCompatibleBitmap( int, int );	/*!< ������BMP���쐬�܂��͍X�V */
	void DeleteCompatibleBitmap();			/*!< ������BMP���폜 */
	//	2007.09.30 genta CompatibleDC����֐�
	void UseCompatibleDC(BOOL fCache);

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	CDropTarget*	m_pcDropTarget;
	bool			m_bDrawSWITCH;
	BOOL			m_bDragMode;	/* �I���e�L�X�g�̃h���b�O���� */
	CLIPFORMAT		m_cfDragData;	/* �h���b�O�f�[�^�̃N���b�v�`�� */	// 2008.06.20 ryoji
	BOOL			m_bDragBoxData;	/* �h���b�O�f�[�^�͋�`�� */
	CLayoutPoint	m_ptCaretPos_DragEnter;	/* �h���b�O�J�n���̃J�[�\���ʒu */	// 2007.12.09 ryoji
	int		m_nCaretPosX_Prev_DragEnter;	/* �h���b�O�J�n����X���W�L�� */		// 2007.12.09 ryoji

	/* �P�ꌟ���̏�� */

	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	m_CurRegexp;			/*!< �R���p�C���f�[�^ */
	bool	m_bCurSrchKeyMark;			/* ����������̃}�[�N */
	char	m_szCurSrchKey[_MAX_PATH];	/* ���������� */
	SSearchOption		m_sCurSearchOption;			// �����^�u��  �I�v�V����

	bool	m_bExecutingKeyMacro;		/* �L�[�{�[�h�}�N���̎��s�� */
	HWND	m_hWnd;				/* �ҏW�E�B���h�E�n���h�� */
	int		m_nViewTopLine;		/* �\����̈�ԏ�̍s(0�J�n) */
	int		m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */
	/*!	@brief ���O��X���W�L���p

		�t���[�J�[�\�����[�h�łȂ��ꍇ�ɃJ�[�\�����㉺�Ɉړ��������ꍇ
		�J�[�\���ʒu���Z���s�ł͍s���ɃJ�[�\�����ړ����邪�C
		����Ɉړ��𑱂����ꍇ�ɒ����s�ňړ��N�_��X�ʒu�𕜌��ł���悤��
		���邽�߂̕ϐ��D
		
		@par �g����
		�ǂݏo����CEditView::Cursor_UPDOWN()�݂̂ōs���D
		�J�[�\���㉺�ړ��ȊO�ŃJ�[�\���ړ����s�����ꍇ�ɂ�
		������m_ptCaretPos.x�̒l��ݒ肷��D�������Ȃ���
		���̒���̃J�[�\���㉺�ړ��ňړ��O��X���W�ɖ߂��Ă��܂��D
	
		�r���[���[����̃J�[�\�����ʒu(�O�J�n)
		
		@date 2004.04.09 genta �������ǉ�
	*/
	int		m_nCaretPosX_Prev;
	CLayoutPoint	m_ptCaretPos;		/* �J�[�\�����ʒu */
	CLogicPoint	m_ptCaretPos_PHY;		/* �J�[�\���ʒu */
	CLogicPoint	m_ptSrchStartPos_PHY;	/* ����/�u���J�n���̃J�[�\���ʒu */
	BOOL	m_bSearch;				/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	int		m_nCharSize;			/* �Ί��ʂ̕����T�C�Y */	// 02/09/18 ai 
	CLogicPoint	m_ptBracketCaretPos_PHY;/* �O�J�[�\���ʒu�̊��ʂ̈ʒu */
	CLogicPoint	m_ptBracketPairPos_PHY;	/* �Ί��ʂ̈ʒu */
	BOOL	m_bDrawBracketPairFlag;	/* �Ί��ʂ̋����\�����s�Ȃ��� */						// 03/02/18 ai
	bool	m_bDrawSelectArea;		/* �I��͈͂�`�悵���� */	// 02/12/13 ai

	/*
	||  �����o�ϐ�
	*/
	HINSTANCE		m_hInstance;	/* �C���X�^���X�n���h�� */
	HWND			m_hwndParent;	/* �e�E�B���h�E�n���h�� */

	DLLSHAREDATA*	m_pShareData;

	CEditWnd* 		m_pcEditWnd;
	CEditDoc*		m_pcEditDoc;		/* �h�L�������g */
	COpeBlk*		m_pcOpeBlk;			/* ����u���b�N */
	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	int				m_nVScrollRate;		/* �����X�N���[���o�[�̏k�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* ���������{�b�N�X */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* ���������{�b�N�X */
	HDC				m_hdcCompatDC;		/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	int				m_nCompatBMPWidth;  /* �č��p�������a�l�o�̕� */
	int				m_nCompatBMPHeight; /* �č��p�������a�l�o�̍��� */
	// To Here 2007.09.09 Moca

//@@@2002.01.14 YAZAKI static�ɂ��ă������̐ߖ�i(10240+10) * 3 �o�C�g�j
	static int		m_pnDx[MAXLINEKETAS + 10];	/* ������`��p�������z�� */
	CViewFont*		m_pcViewFont;
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* �R�}���h�̎��s�� */
	/* ������� */
	int				m_nMyIndex;
	/* �L�[���s�[�g��� */
	int				m_bPrevCommand;
	/* ���͏�� */
	int		m_nCaretWidth;			/* �L�����b�g�̕� */
	int		m_nCaretHeight;			/* �L�����b�g�̍��� */
	COLORREF	m_crCaret;			/* �L�����b�g�̐F */			// 2006.12.07 ryoji
	COLORREF	m_crBack;			/* �e�L�X�g�̔w�i�F */			// 2006.12.07 ryoji
	HBITMAP	m_hbmpCaret;			/* �L�����b�g�̃r�b�g�}�b�v */	// 2006.11.28 ryoji
	CCaretUnderLine m_cUnderLine;	/* �A���_�[���C�� */
	int		m_nOldUnderLineY;	// �O���悵���J�[�\���A���_�[���C���̈ʒu 0����=��\��
	int		m_nOldCursorLineX;	/* �O���悵���J�[�\���ʒu�c���̈ʒu */ // 2007.09.09 Moca

	int		m_nOldCaretPosX;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	int		m_nOldCaretWidth;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
	bool	m_bRedrawRuler;		// ���[���[�S�̂�`�������� = true      2002.02.25 Add By KK

	/* �I����� */
	bool	m_bSelectingLock;		// �I����Ԃ̃��b�N
	bool	m_bBeginSelect;			// �͈͑I��
	bool	m_bBeginBoxSelect;		// ��`�͈͑I��
	bool	m_bBeginLineSelect;		// �s�P�ʑI��
	bool	m_bBeginWordSelect;		// �P��P�ʑI��
	// �I��͈͂�ێ����邽�߂̕ϐ��Q
	// �����͂��ׂĐ܂�Ԃ��s�ƁA�܂�Ԃ�����ێ����Ă���B
	CLayoutRange	m_sSelectBgn; //�͈͑I��(���_)
	CLayoutRange	m_sSelect;    //�͈͑I��
	CLayoutRange	m_sSelectOld; //�͈͑I��Old
	/*
	   m_sSelectOld�ɂ���
	   DrawSelectArea()�Ɍ��݂̑I��͈͂������č����̂ݕ`�悷�邽�߂̂���
	   ���݂̑I��͈͂�Old�փR�s�[������ŐV�����I��͈͂�Select�ɐݒ肵��
	   DrawSelectArea()���Ăт������ƂŐV�����͈͂��`�����D
	*/
	int		m_nMouseRollPosXOld;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
	int		m_nMouseRollPosYOld;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	/* ��ʏ�� */
	int		m_nViewAlignLeft;		/* �\����̍��[���W */
	int		m_nViewAlignLeftCols;	/* �s�ԍ���̌��� */
	int		m_nViewAlignTop;		/* �\����̏�[���W */
	int		m_nTopYohaku;
	int		m_nViewCx;				/* �\����̕� */
	int		m_nViewCy;				/* �\����̍��� */
	int		m_nViewColNum;			/* �\����̌��� */
	int		m_nViewRowNum;			/* �\����̍s�� */
	/* �\�����@ */
	int		m_nCharWidth;			/* ���p�����̕� */
	int		m_nCharHeight;			/* �����̍��� */
	/* ����Tip�֘A */
	DWORD	m_dwTipTimer;			/* Tip�N���^�C�}�[ */
	CTipWnd	m_cTipWnd;				/* Tip�\���E�B���h�E */
	POINT	m_poTipCurPos;			/* Tip�N�����̃}�E�X�J�[�\���ʒu */
	BOOL	m_bInMenuLoop;			/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
	CDicMgr	m_cDicMgr;				/* �����}�l�[�W�� */
	/* ���͕⊮ */
//	CHokanMgr	m_cHokanMgr;
	BOOL		m_bHokan;			//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH
	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump����
	CRegexKeyword	*m_cRegexKeyword;	//@@@ 2001.11.17 add MIK

	CSMacroMgr*		m_pcSMacroMgr;

	/*
	||  �����w���p�֐�
	*/
	void GetCurrentTextForSearch( CMemory& );			/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	void GetCurrentTextForSearchDlg( CMemory& );		/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j 2006.08.23 ryoji */
	void DestroyCaret( void ){	/* �L�����b�g��j������i�����I�ɂ��j���j*/
		::DestroyCaret();
		m_nCaretWidth = 0;
	}
	bool MyGetClipboardData( CMemory&, bool*, bool* = NULL );			/* �N���b�v�{�[�h����f�[�^���擾 */	// 2007.10.04 ryoji MSDEVLineSelect�Ή��p�����[�^��ǉ�
	bool MySetClipboardData( const char*, int, bool, bool = false );	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */	// 2007.10.04 ryoji MSDEVLineSelect�Ή��p�����[�^��ǉ�
protected:
	int GetLeftWord( CMemory*, int );					/* �J�[�\�����O�̒P����擾 */
	bool GetCurrentWord( CMemory* );					/* �L�����b�g�ʒu�̒P����擾 */	// 2006.03.24 fon
	bool DispLineNew( HDC, const CLayout*, int&, int, int&, int, BOOL );	/* �s�̃e�L�X�g�^�I����Ԃ̕`�� */
	void DispLineNumber( HDC, const CLayout*, int, int );		/* �s�ԍ��\�� */
	void SetCurrentColor( HDC, EColorIndexType );				/* ���݂̐F���w�� */
	void DispRuler( HDC );										/* ���[���[�`�� */
	void DrawRulerCaret( HDC hdc );								// ���[���[�̃L�����b�g�̂ݕ`�� 2002.02.25 Add By KK
	int	DispText( HDC, int, int, const char*, int );	/* �e�L�X�g�\�� */	//@@@ 2002.09.22 YAZAKI
	void DispTextSelected( HDC, int, int, int, int );			/* �e�L�X�g���] */
//	int	DispCtrlCode( HDC, int, int, const unsigned char*, int );	/* �R���g���[���R�[�h�\�� */
	void TwoPointToRect( RECT*, int, int, int, int );			/* 2�_��Ίp�Ƃ����`�����߂� */
	void DrawSelectArea( void );								/* �w��s�̑I��̈�̕`�� */
	void DrawSelectAreaLine( HDC, int, int, int, int, int );	/* �w��s�̑I��̈�̕`�� */
	int  LineColumnToIndex( const CLayout* pcLayout, int nColumn );		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColumnToIndex( const CDocLine* pcDocLine, int nColumn );		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColumnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen );	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */		// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColumn( const CLayout* pcLayout, int nIndex );		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */	// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColumn( const CDocLine* pcLayout, int nIndex );		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */	// @@@ 2002.09.28 YAZAKI
	void CopySelectedAllLines( const char*, BOOL );				/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	void ConvSelectedArea( int );								/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	void ConvMemory( CMemory*, int, int nStartColumn );							/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
	/*! �I��͈͂��w�肷��(���_���I��)

		@date 2005.06.24 Moca
	*/
	void SetSelectArea( const CLayoutRange& sRange ){
		m_sSelectBgn.m_ptFrom = sRange.m_ptFrom;
		m_sSelectBgn.m_ptTo = sRange.m_ptFrom;
		m_sSelect = sRange;
	}
	void BeginSelectArea( void );								/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	void ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos );			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
	void ChangeSelectAreaByCurrentCursorTEST( const CLayoutPoint& ptCaretPos, CLayoutRange* pSelect );// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
	void PrintSelectionInfoMsg(void);		// �I�����f�[�^�̍쐬	2005.07.09 genta
	int  MoveCursorToPoint( int, int );							/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ� */
	int  GetRightEdgeForScrollBar( void );						/* �X�N���[���o�[����p�ɉE�[���W���擾���� */		// 2009.08.28 nasukoji
	int  ScrollAtV( int );										/* �w���[�s�ʒu�փX�N���[�� */
	int  ScrollAtH( int );										/* �w�荶�[���ʒu�փX�N���[�� */
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	int  ScrollByV( int vl ){	return ScrollAtV( m_nViewTopLine + vl );}			/* �w��s�X�N���[��*/
	int  ScrollByH( int hl ){	return ScrollAtH( m_nViewLeftCol + hl );}					/* �w�茅�X�N���[�� */
	void SyncScrollV( int );									/* ���������X�N���[�� */
	void SyncScrollH( int );									/* ���������X�N���[�� */
	//	To Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	int Cursor_UPDOWN( int, int );								/* �J�[�\���㉺�ړ����� */
	void DrawBracketPair( bool );								/* �Ί��ʂ̋����\�� 02/09/18 ai */
	void SetBracketPairPos( bool );								/* �Ί��ʂ̋����\���ʒu�ݒ� 03/02/18 ai */
	EColorIndexType GetColorIndex( HDC, const CLayout* const, int );		/* �w��ʒu��ColorIndex�̎擾 02/12/13 ai */
	bool IsBracket( const char*, int, int );					/* ���ʔ��� 03/01/09 ai */
public:
	void SetIMECompFormPos( void );								/* IME�ҏW�G���A�̈ʒu��ύX */
protected:
	void SetIMECompFormFont( void );							/* IME�ҏW�G���A�̕\���t�H���g��ύX */
public:
	void ShowCaretPosInfo( void );								/* �L�����b�g�̍s���ʒu��\������ */
protected:
	bool IsCurrentPositionURL( const CLayoutPoint& ptCaretPos, CLogicRange* pUrlRange, std::tstring* ptstrURL );/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	BOOL CheckTripleClick( int, int);							/* �g���v���N���b�N���`�F�b�N���� */	// 2007.10.02 nasukoji
	int IsCurrentPositionSelected( CLayoutPoint ptCaretPos );					/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	int IsCurrentPositionSelectedTEST( const CLayoutPoint& ptCaretPos, const CLayoutRange& sSelect ) const;/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	BOOL IsSearchString( const char*, int, int, int*, int* );	/* ���݈ʒu������������ɊY�����邩 */	//2002.02.08 hor �����ǉ�
	HFONT ChooseFontHandle( bool bBold, bool bUnderLine );		/* �t�H���g��I�� */
	void ExecCmd(const char*, const int);						// �q�v���Z�X�̕W���o�͂����_�C���N�g����	//2006.12.03 maru �����̊g��
	void AddToCmdArr( const TCHAR* );
	BOOL ChangeCurRegexp(bool bRedrawIfChanged= true);			// 2002.01.16 hor ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�)
	void SendStatusMessage( const TCHAR* msg );					// 2002.01.26 hor �����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������
	void SendStatusMessage2( const TCHAR* msg );					// Jul. 9, 2005 genta
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* �ĕϊ��p�\���̂�ݒ肷�� 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				/* �ĕϊ��p�\���̂̏������ɑI��͈͂�ύX���� 2002.04.09 minfu */

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* IDropTarget���� */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP PostMyDropFiles( LPDATAOBJECT pDataObject );		/* �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g���� */	// 2008.06.20 ryoji
	void OnMyDropFiles( HDROP hDrop );								/* �Ǝ��h���b�v�t�@�C�����b�Z�[�W���� */	// 2008.06.20 ryoji
	CLIPFORMAT GetAvailableClipFormat( LPDATAOBJECT pDataObject );
	DWORD TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect );
	bool IsDragSource( void );

	/* �t�@�C������n */
	void Command_FILENEW( void );				/* �V�K�쐬 */
	void Command_FILENEW_NEWWINDOW( void );		/* �V�K�쐬�i�^�u�ŊJ���Łj */
	/* �t�@�C�����J�� */
	// Oct. 2, 2001 genta �}�N���p�ɋ@�\�g��
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILEOPEN(
		const char *filename = NULL,
		ECodeType nCharCode = CODE_AUTODETECT,
		bool bReadOnly = false
	);
	
	/* �㏑���ۑ� */ // Feb. 28, 2004 genta �����ǉ�, Jan. 24, 2005 genta �����ǉ�
	BOOL Command_FILESAVE( bool warnbeep = true, bool askname = true );
	BOOL Command_FILESAVEAS_DIALOG();							/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEAS( const char* filename );			/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEALL( void );	/* �S�ď㏑���ۑ� */ // Jan. 23, 2005 genta
	void Command_FILECLOSE( void );				/* �J����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	/* ���ĊJ��*/
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILECLOSE_OPEN( const char *filename = NULL,
		ECodeType nCharCode = CODE_AUTODETECT, bool bReadOnly = false );
	
	void Command_FILE_REOPEN( ECodeType nCharCode, bool bNoConfirm );	/* �ăI�[�v�� */	//Dec. 4, 2002 genta �����ǉ�

	void Command_PRINT( void );					/* ���*/
	void Command_PRINT_PREVIEW( void );			/* ����v���r���[*/
	void Command_PRINT_PAGESETUP( void );		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	BOOL Command_OPEN_HfromtoC( BOOL );			/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
	BOOL Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	BOOL Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plus�Ŏ��s */
	void Command_BROWSE( void );				/* �u���E�Y */
	void Command_READONLY( void );				/* �ǂݎ���p */
	void Command_PROPERTY_FILE( void );			/* �t�@�C���̃v���p�e�B */
	void Command_EXITALLEDITORS( void );		/* �ҏW�̑S�I�� */	// 2007.02.13 ryoji �ǉ�
	void Command_EXITALL( void );				/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�
	BOOL Command_PUTFILE( const char*, ECodeType, int );	/* ��ƒ��t�@�C���̈ꎞ�o�� maru 2006.12.10 */
	BOOL Command_INSFILE( const char*, ECodeType, int );	/* �L�����b�g�ʒu�Ƀt�@�C���}�� maru 2006.12.10 */
	void Command_TEXTWRAPMETHOD( int );			/* �e�L�X�g�̐܂�Ԃ����@��ύX���� */		// 2008.05.30 nasukoji

	/* �ҏW�n */
	void Command_CHAR( char );				/* �������� */
	void Command_IME_CHAR( WORD );			/* �S�p�������� */
	void Command_UNDO( void );				/* ���ɖ߂�(Undo) */
	void Command_REDO( void );				/* ��蒼��(Redo) */
	void Command_DELETE( void );			/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	void Command_DELETE_BACK( void );		/* �J�[�\���O���폜 */
	void Command_WordDeleteToStart( void );	/* �P��̍��[�܂ō폜 */
	void Command_WordDeleteToEnd( void );	/* �P��̉E�[�܂ō폜 */
	void Command_WordCut( void );			/* �P��؂��� */
	void Command_WordDelete( void );		/* �P��폜 */
	void Command_LineCutToStart( void );	//�s���܂Ő؂���(���s�P��)
	void Command_LineCutToEnd( void );		//�s���܂Ő؂���(���s�P��)
	void Command_LineDeleteToStart( void );	/* �s���܂ō폜(���s�P��) */
	void Command_LineDeleteToEnd( void );  	//�s���܂ō폜(���s�P��)
	void Command_CUT_LINE( void );			/* �s�؂���(�܂�Ԃ��P��) */
	void Command_DELETE_LINE( void );		/* �s�폜(�܂�Ԃ��P��) */
	void Command_DUPLICATELINE( void );		/* �s�̓�d��(�܂�Ԃ��P��) */
	void Command_INDENT( char cChar );		/* �C���f���g ver 1 */
// From Here 2001.12.03 hor
//	void Command_INDENT( const char*, int );/* �C���f���g ver0 */
	void Command_INDENT( const char*, int ,BOOL=FALSE );/* �C���f���g ver0 */
	void Command_INDENT_TAB( void );		/* �C���f���g ver 2 */
// To Here 2001.12.03 hor
	void Command_UNINDENT( char cChar );	/* �t�C���f���g */
//	void Command_WORDSREFERENCE( void );	/* �P�ꃊ�t�@�����X */
	void Command_TRIM(BOOL);				// 2001.12.03 hor
	void Command_TRIM2(CMemory*,BOOL);		// 2001.12.03 hor
	void Command_SORT(BOOL);				// 2001.12.06 hor
	void Command_MERGE(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
	void Command_CtrlCode_Dialog(void);		/* �R���g���[���R�[�h�̓���(�_�C�A���O) */	//@@@ 2002.06.02 MIK

	/* �w��ʒu�̎w�蒷�f�[�^�폜 */
	void DeleteData2(
		int			nCaretX,
		int			nCaretY,
		int			nDelLen,
		CMemory*	pcMem,
		COpe*		pcOpe		/* �ҏW����v�f COpe */
	);

	/* ���݈ʒu�̃f�[�^�폜 */
	void DeleteData( bool bRedraw );

	/* ���݈ʒu�Ƀf�[�^��}�� */
	void InsertData_CEditView(
		CLayoutPoint	ptInsertPos,
		const char*		pData,
		int				nDataLen,
		CLayoutPoint*	pptNewPos,	//�}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		COpe*			pcOpe,		/* �ҏW����v�f COpe */
		bool			bRedraw
	);
	void SmartIndent_CPP( char );	/* C/C++�X�}�[�g�C���f���g���� */
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */

	/* �J�[�\���ړ��n */
	//	Oct. 24, 2001 genta �@�\�g���̂��߈����ǉ�
	int Command_UP( bool bSelect, bool bRepeat, int line = 0 );			/* �J�[�\����ړ� */
	int Command_DOWN( bool bSelect, bool bRepeat );						/* �J�[�\�����ړ� */
	int  Command_LEFT( bool, bool );			/* �J�[�\�����ړ� */
	void Command_RIGHT( bool, bool, bool );	/* �J�[�\���E�ړ� */
	void Command_UP2( bool bSelect );				/* �J�[�\����ړ��i�Q�s�Âj */
	void Command_DOWN2( bool bSelect );				/* �J�[�\�����ړ��i�Q�s�Âj */
	void Command_WORDLEFT( bool bSelect );			/* �P��̍��[�Ɉړ� */
	void Command_WORDRIGHT( bool bSelect );			/* �P��̉E�[�Ɉړ� */
	//	Oct. 29, 2001 genta �}�N�������@�\�g��
	void Command_GOLINETOP( bool bSelect, int lparam );	/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
	void Command_GOLINEEND( bool bSelect, int );		/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
//	void Command_ROLLDOWN( int );			/* �X�N���[���_�E�� */
//	void Command_ROLLUP( int );				/* �X�N���[���A�b�v */
	void Command_HalfPageUp( bool bSelect );			//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_HalfPageDown( bool bSelect );		//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_1PageUp( bool bSelect );			//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	void Command_1PageDown( bool bSelect );			//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	void Command_GOFILETOP( bool bSelect );			/* �t�@�C���̐擪�Ɉړ� */
	void Command_GOFILEEND( bool bSelect );			/* �t�@�C���̍Ō�Ɉړ� */
	void Command_CURLINECENTER( void );		/* �J�[�\���s���E�B���h�E������ */
	void Command_JUMPHIST_PREV(void);		// �ړ�����: �O��
	void Command_JUMPHIST_NEXT(void);		// �ړ�����: ����
	void Command_JUMPHIST_SET(void);		// ���݈ʒu���ړ������ɓo�^
	void Command_WndScrollDown(void);		// �e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// �e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	void Command_GONEXTPARAGRAPH( bool bSelect );	// ���̒i���֐i��
	void Command_GOPREVPARAGRAPH( bool bSelect );	// �O�̒i���֖߂�

	/* �I���n */
	bool Command_SELECTWORD( void );		/* ���݈ʒu�̒P��I�� */
	void Command_SELECTALL( void );			/* ���ׂđI�� */
	void Command_SELECTLINE( int lparam );	/* 1�s�I�� */	// 2007.10.13 nasukoji
	void Command_BEGIN_SELECT( void );		/* �͈͑I���J�n */

	/* ��`�I���n */
//	void Command_BOXSELECTALL( void );		/* ��`�ł��ׂđI�� */
	void Command_BEGIN_BOXSELECT( void );	/* ��`�͈͑I���J�n */
	int Command_UP_BOX( BOOL );				/* (��`�I��)�J�[�\����ړ� */

	/* �N���b�v�{�[�h�n */
	void CopyCurLine( bool bAddCRLFWhenCopy, EEolType neweol, bool bEnableLineModePaste );	/* �J�[�\���s���N���b�v�{�[�h�ɃR�s�[���� */	// 2007.10.08 ryoji
	void Command_CUT( void );						/* �؂���i�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜�j*/
	void Command_COPY( bool, bool bAddCRLFWhenCopy, EEolType neweol = EOL_UNKNOWN );/* �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[) */
	void Command_PASTE( int option );				/* �\��t���i�N���b�v�{�[�h����\��t���j*/
	void Command_PASTEBOX( int option );			/* ��`�\��t���i�N���b�v�{�[�h�����`�\��t���j*/
	//<< 2002/03/29 Azumaiya
	/* ��`�\��t���i�����n���ł̒���t���j*/
	void Command_PASTEBOX( const char *szPaste, int nPasteSize );
	//>> 2002/03/29 Azumaiya
	void Command_INSTEXT( bool, const char*, int, BOOL, BOOL bLinePaste = FALSE ); // 2004.05.14 Moca �e�L�X�g��\��t�� '\0'�Ή�
	void Command_ADDTAIL( const char* pszData, int nDataLen );		/* �Ō�Ƀe�L�X�g��ǉ� */
	void Command_COPYFILENAME( void );				/* ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ */ //2002/2/3 aroka
	void Command_COPYPATH( void );					/* ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
	void Command_COPYTAG( void );					/* ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[ */
	void Command_COPYLINES( void );					/* �I��͈͓��S�s�R�s�[ */
	void Command_COPYLINESASPASSAGE( void );		/* �I��͈͓��S�s���p���t���R�s�[ */
	void Command_COPYLINESWITHLINENUMBER( void );	/* �I��͈͓��S�s�s�ԍ��t���R�s�[ */
	void Command_CREATEKEYBINDLIST( void );			// �L�[���蓖�Ĉꗗ���R�s�[ //Sept. 15, 2000 JEPRO	Command_�̍������킩��Ȃ��̂ŎE���Ă���


	/* �f�[�^�u�� �폜&�}���ɂ��g���� */
	void ReplaceData_CEditView(
		CLayoutRange	sDelRange,			// �폜�͈̓��C�A�E�g�P��
		CMemory*		pcmemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
		const char*		pInsData,			// �}������f�[�^
		int				nInsDataLen,		// �}������f�[�^�̒���
		bool			bRedraw
	);

	/* �}���n */
	void Command_INS_DATE( void );	//���t�}��
	void Command_INS_TIME( void );	//�����}��

	/* �ϊ��n */
	void Command_TOLOWER( void );				/* ������ */
	void Command_TOUPPER( void );				/* �啶�� */
	void Command_TOZENKAKUKATA( void );			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	void Command_TOZENKAKUHIRA( void );			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	void Command_TOHANKAKU( void );				/* �S�p�����p */
	void Command_TOHANKATA( void );				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	void Command_TOZENEI( void );				/* ���p�p�����S�p�p�� */ //July. 30, 2001 Misaka
	void Command_TOHANEI( void );				/* �S�p�p�������p�p�� */ //@@@ 2002.2.11 YAZAKI
	void Command_HANKATATOZENKAKUKATA( void );	/* ���p�J�^�J�i���S�p�J�^�J�i */
	void Command_HANKATATOZENKAKUHIRA( void );	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	void Command_TABTOSPACE( void );			/* TAB���� */
	void Command_SPACETOTAB( void );			/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* �������ʁ�SJIS�R�[�h�ϊ� */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODEBE2SJIS( void );	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS��JIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS��EUC�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS��UTF-8�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS��UTF-7�R�[�h�ϊ� */
	void Command_BASE64DECODE( void );			/* Base64�f�R�[�h���ĕۑ� */
	void Command_UUDECODE( void );				/* uudecode���ĕۑ� */	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
	void Command_SEARCH_BOX( void );					/* ����(�{�b�N�X) */	// 2006.06.04 yukihane
	void Command_SEARCH_DIALOG( void );					/* ����(�P�ꌟ���_�C�A���O) */
	void Command_SEARCH_NEXT( bool, bool, HWND, const char* );/* �������� */
	void Command_SEARCH_PREV( bool bReDraw, HWND );		/* �O������ */
	void Command_REPLACE_DIALOG( void );				/* �u��(�u���_�C�A���O) */
	void Command_REPLACE( HWND hwndParent );			/* �u��(���s) 2002/04/08 YAZAKI �e�E�B���h�E���w�肷��悤�ɕύX */
	void Command_REPLACE_ALL();							/* ���ׂĒu��(���s) */
	void Command_SEARCH_CLEARMARK( void );				/* �����}�[�N�̃N���A */
	void Command_JUMP_SRCHSTARTPOS( void );				/* �����J�n�ʒu�֖߂� */	// 02/06/26 ai

	//	Jan. 10, 2005 genta HandleCommand����grep�֘A�����𕪗�
	void TranslateCommand_grep( int&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	void Command_GREP_DIALOG( void );					/* Grep�_�C�A���O�̕\�� */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP_DIALOG( void );					/* �w��s�w�W�����v�_�C�A���O�̕\�� */
	void Command_JUMP( void );							/* �w��s�w�W�����v */
// From Here 2001.12.03 hor
	BOOL Command_FUNCLIST( int nAction ,int nOutlineType = OUTLINE_DEFAULT );	/* �A�E�g���C����� */ // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta �����ǉ�
	bool Command_TAGJUMP( bool bClose = false );		/* �^�O�W�����v�@�\ */
	void Command_TAGJUMPBACK( void );					/* �^�O�W�����v�o�b�N�@�\ */
	bool Command_TagJumpByTagsFile( void );				//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.13 MIK
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose�ǉ�
	//	Feb. 17, 2007 genta ���΃p�X�̊�f�B���N�g���w����ǉ�
	bool TagJumpSub( const char *pszJumpToFile, int nLine, int nColumn, bool bClose = false, bool bRelFromIni = false );
	bool Command_TagsMake( void );						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword( const char* keyword );	//	@@ 2005.03.31 MIK
	void Command_COMPARE( void );						/* �t�@�C�����e��r */
	void Command_Diff_Dialog( void );					/* DIFF�����\���_�C�A���O */	//@@@ 2002.05.25 MIK
	void Command_Diff( const char*, int );	/* DIFF�����\�� */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next( void );						/* ���̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev( void );						/* �O�̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset( void );					/* �����̑S���� */	//@@@ 2002.05.25 MIK
	void Command_BRACKETPAIR( void );					/* �Ί��ʂ̌��� */
// From Here 2001.12.03 hor
	void Command_BOOKMARK_SET( void );					/* �u�b�N�}�[�N�ݒ�E���� */
	void Command_BOOKMARK_NEXT( void );					/* ���̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_PREV( void );					/* �O�̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_RESET( void );				/* �u�b�N�}�[�N�̑S���� */
// To Here 2001.12.03 hor
	void Command_BOOKMARK_PATTERN( void );				// 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N

//2004.10.13 �C���N�������^���T�[�`�֌W
	void TranslateCommand_isearch( int&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );

	/* ���[�h�؂�ւ��n */
	void Command_CHGMOD_INS( void );	/* �}���^�㏑�����[�h�؂�ւ� */
	void Command_CHGMOD_EOL( EEolType );	/* ���͂�����s�R�[�h��ݒ� 2003.06.23 moca */
	void Command_CANCEL_MODE( void );	/* �e�탂�[�h�̎����� */

	/* �ݒ�n */
	void Command_SHOWTOOLBAR( void );		/* �c�[���o�[�̕\��/��\�� */
	void Command_SHOWFUNCKEY( void );		/* �t�@���N�V�����L�[�̕\��/��\�� */
	void Command_SHOWTAB( void );			/* �^�u�̕\��/��\�� */	//@@@ 2003.06.10 MIK
	void Command_SHOWSTATUSBAR( void );		/* �X�e�[�^�X�o�[�̕\��/��\�� */
	void Command_TYPE_LIST( void );			/* �^�C�v�ʐݒ�ꗗ */
	void Command_CHANGETYPE( int nTypePlusOne );	// �^�C�v�ʐݒ�ꎞ�K�p
	void Command_OPTION_TYPE( void );		/* �^�C�v�ʐݒ� */
	void Command_OPTION( void );			/* ���ʐݒ� */
	void Command_FONT( void );				/* �t�H���g�ݒ� */
	void Command_SETFONTSIZE( int, int );	/* �t�H���g�T�C�Y�ݒ� */
	void Command_WRAPWINDOWWIDTH( void );	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	void Command_Favorite( void );	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
	void Command_SET_QUOTESTRING( const char* );	//	Jan. 29, 2005 genta ���p���̐ݒ�

	/* �}�N���n */
	void Command_RECKEYMACRO( void );	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	void Command_SAVEKEYMACRO( void );	/* �L�[�}�N���̕ۑ� */
	void Command_LOADKEYMACRO( void );	/* �L�[�}�N���̓ǂݍ��� */
	void Command_EXECKEYMACRO( void );	/* �L�[�}�N���̎��s */
	void Command_EXECEXTMACRO( const char* path, const char* type );	/* ���O���w�肵�ă}�N�����s */
//	From Here 2006.12.03 maru �����̊g���D
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void Command_EXECCMMAND( void );	/* �O���R�}���h���s */
	//	Oct. 9, 2001 genta �}�N���Ή��̂��ߋ@�\�g��
//	void Command_EXECCOMMAND_DIALOG( const char* cmd );	/* �O���R�}���h���s�_�C�A���O�\�� */
//	void Command_EXECCOMMAND( const char* cmd );	/* �O���R�}���h���s */
	void Command_EXECCOMMAND_DIALOG( void );	/* �O���R�}���h���s�_�C�A���O�\�� */	//	�����g���ĂȂ��݂����Ȃ̂�
	//	�}�N������̌Ăяo���ł̓I�v�V������ۑ������Ȃ����߁ACommand_EXECCOMMAND_DIALOG���ŏ������Ă����D
	void Command_EXECCOMMAND( const char* cmd, const int nFlgOpt );	/* �O���R�}���h���s */
//	To Here Sept. 20, 2000
//	To Here 2006.12.03 maru �����̊g��

	/* �J�X�^�����j���[ */
	void Command_MENU_RBUTTON( void );	/* �E�N���b�N���j���[ */
	int	CreatePopUpMenu_R( void );		/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
	int Command_CUSTMENU( int );		/* �J�X�^�����j���[�\�� */

	/* �E�B���h�E�n */
	void Command_SPLIT_V( void );		/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	void Command_SPLIT_H( void );		/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	void Command_SPLIT_VH( void );		/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	void Command_WINCLOSE( void );		/* �E�B���h�E����� */
	void Command_FILECLOSEALL( void );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	void Command_BIND_WINDOW( void );	/* �������ĕ\�� */	//2004.07.14 Kazika �V�K�ǉ�
	void Command_CASCADE( void );		/* �d�˂ĕ\�� */
	void Command_TILE_V( void );		/* �㉺�ɕ��ׂĕ\�� */
	void Command_TILE_H( void );		/* ���E�ɕ��ׂĕ\�� */
	void Command_MAXIMIZE_V( void );	/* �c�����ɍő剻 */
	void Command_MAXIMIZE_H( void );	/* �������ɍő剻 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* ���ׂčŏ��� */
	void Command_REDRAW( void );		/* �ĕ`�� */
	void Command_WIN_OUTPUT( void );	//�A�E�g�v�b�g�E�B���h�E�\��
	void Command_TRACEOUT( const char* outputstr , int );	//�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	void Command_WINTOPMOST( LPARAM );		// ��Ɏ�O�ɕ\�� 2004.09.21 Moca
	void Command_WINLIST( int nCommandFrom );		/* �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�
	void Command_GROUPCLOSE( void );	/* �O���[�v����� */		// 2007.06.20 ryoji
	void Command_NEXTGROUP( void );		/* ���̃O���[�v */			// 2007.06.20 ryoji
	void Command_PREVGROUP( void );		/* �O�̃O���[�v */			// 2007.06.20 ryoji
	void Command_TAB_MOVERIGHT( void );	/* �^�u���E�Ɉړ� */		// 2007.06.20 ryoji
	void Command_TAB_MOVELEFT( void );	/* �^�u�����Ɉړ� */		// 2007.06.20 ryoji
	void Command_TAB_SEPARATE( void );	/* �V�K�O���[�v */			// 2007.06.20 ryoji
	void Command_TAB_JOINTNEXT( void );	/* ���̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
	void Command_TAB_JOINTPREV( void );	/* �O�̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
	void Command_TAB_CLOSEOTHER( void );/* ���̃^�u�ȊO����� */	// 2009.07.20 syat
	void Command_TAB_CLOSELEFT( void );	/* �������ׂĕ��� */		// 2009.07.20 syat
	void Command_TAB_CLOSERIGHT( void );/* �E�����ׂĕ��� */		// 2009.07.20 syat

	/* �x�� */
	//	Jan. 10, 2005 genta HandleCommand����⊮�֘A�����𕪗�
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);
	void Command_ToggleKeySearch( int );	/* �L�����b�g�ʒu�̒P���������������@�\ON-OFF */	// 2006.03.24 fon

	void ShowHokanMgr( CMemory& cmemData, BOOL bAutoDecided );	//	�⊮�E�B���h�E��\������BCtrl+Space��A�����̓���/�폜���ɌĂяo����܂��B YAZAKI 2002/03/11
	void Command_HOKAN( void );			/* ���͕⊮ */
	void Command_HELP_CONTENTS( void );	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* �R�}���h�ꗗ */
	void Command_EXTHELP1( void );		/* �O���w���v�P */
	//	Jul. 5, 2002 genta
	void Command_EXTHTMLHELP( const char* helpfile = NULL, const char* kwd = NULL );	/* �O��HTML�w���v */
	void Command_ABOUT( void );			/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */

	//	May 23, 2000 genta
	//	��ʕ`��⏕�֐�
//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
	void DrawEOL(HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight,
		CEol cEol, int bBold, COLORREF pColor );
	//�^�u���`��֐�	//@@@ 2003.03.26 MIK
	void DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor );
	//	EOF�`��֐�	//	2004.05.29 genta
	int  DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo );
	//!	�w�茅�c���`��֐�	// 2005.11.08 Moca
	void DispVerticalLines( HDC, int, int, int, int );

	void AnalyzeDiffInfo( const char*, int );	/* DIFF���̉�� */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( TCHAR*, HWND );	/* DIFF�ꎞ�t�@�C���쐬 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const TCHAR*, const TCHAR*, int );		/* DIFF�����\�� */		//2005.10.29 maru

	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//���ݍs�𗚗��ɒǉ�����

	
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const TCHAR* [], const TCHAR* [], int, int, const TCHAR* ); // �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐� // 2003.08.12 Moca

// 2002/07/22 novice
	void ShowCaret_( HWND hwnd );
	void HideCaret_( HWND hwnd );

	// �w��ʒu�܂��͎w��͈͂��e�L�X�g�̑��݂��Ȃ��G���A���`�F�b�N����		// 2008.08.03 nasukoji
	bool IsEmptyArea( CLayoutPoint ptFrom, CLayoutPoint ptTo = CLayoutPoint( -1, -1 ), bool bSelect = false, bool bBoxSelect = false );

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;
	UINT	m_uWM_MSIME_RECONVERTREQUEST;
	
	int		m_nLastReconvLine;             //2002.04.09 minfu �ĕϊ����ۑ��p;
	int		m_nLastReconvIndex;            //2002.04.09 minfu �ĕϊ����ۑ��p;

	//ATOK��p�ĕϊ���API
	HMODULE m_hAtokModule;
	BOOL (WINAPI *m_AT_ImmSetReconvertString)( HIMC , int ,PRECONVERTSTRING , DWORD  );
	
	bool	m_bUnderLineON;
	bool	m_bCaretShowFlag;

	/* �C���N�������^���T�[�` */ 
	//2004.10.24 isearch migemo
	CMigemo* m_pcmigemo;
	void ISearchEnter( int mode, ESearchDirection direction);
	void ISearchExit();
	void ISearchExec(WORD wChar);
	void ISearchExec(const char* pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CMemory* msg) const;
	ESearchDirection m_nISearchDirection;
	int m_nISearchMode;
	bool m_bISearchWrap;
	bool m_bISearchFlagHistory[256];
	int m_nISearchHistoryCount;
	bool m_bISearchFirst;
	CLayoutRange m_sISearchHistory[256];

	// 2007.10.02 nasukoji
	bool	m_bActivateByMouse;		//!< �}�E�X�ɂ��A�N�e�B�x�[�g

	// 2007.10.02 nasukoji
	DWORD	m_dwTripleClickCheck;	//!< �g���v���N���b�N�`�F�b�N�p����

private:
	void AlertNotFound(HWND hwnd, LPCTSTR format, ...);
	void DelCharForOverwrite( void );	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
	int ConvertEol(const char* pszText, int nTextLen, char* pszConvertedText);
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
