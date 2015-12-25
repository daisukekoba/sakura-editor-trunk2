/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka, ���イ��
	Copyright (C) 2007, ryoji, maru, genta, kobake
	Copyright (C) 2008, ryoji, nasukoji, bosagami, novice, aroka
	Copyright (C) 2009, nasukoji, syat, aroka
	Copyright (C) 2010, ryoji, Moca
	Copyright (C) 2011, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <dlgs.h>
#include <io.h>
#include <cderr.h> // Nov. 3, 2005 genta
#include "CControlTray.h"
#include "CEditApp.h"
#include "CEditDoc.h"
#include "Debug.h"
#include "Funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CShareData.h"
#include "CEditWnd.h"
#include "etc_uty.h"
#include "format.h"
#include "module.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"
#include "CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include "file.h"
#include "CLayout.h"	// 2007.08.22 ryoji �ǉ�
#include "CMemoryIterator.h"	// 2007.08.22 ryoji �ǉ�
#include "CDlgOpenFile.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

//! �ҏW�֎~�R�}���h
static const int EIsModificationForbidden[] = {
	F_CHAR,
	F_IME_CHAR,
	F_UNDO,		// 2007.10.12 genta
	F_REDO,		// 2007.10.12 genta
	F_DELETE,
	F_DELETE_BACK,
	F_WordDeleteToStart,
	F_WordDeleteToEnd,
	F_WordCut,
	F_WordDelete,
	F_LineCutToStart,
	F_LineCutToEnd,
	F_LineDeleteToStart,
	F_LineDeleteToEnd,
	F_CUT_LINE,
	F_DELETE_LINE,
	F_DUPLICATELINE,
	F_INDENT_TAB,
	F_UNINDENT_TAB,
	F_INDENT_SPACE,
	F_UNINDENT_SPACE,
	F_LTRIM,		// 2001.12.03 hor
	F_RTRIM,		// 2001.12.03 hor
	F_SORT_ASC,	// 2001.12.11 hor
	F_SORT_DESC,	// 2001.12.11 hor
	F_MERGE,		// 2001.12.11 hor
	F_CUT,
	F_PASTE,
	F_PASTEBOX,
	F_INSTEXT,
	F_ADDTAIL,
	F_INS_DATE,
	F_INS_TIME,
	F_CTRL_CODE_DIALOG,	//@@@ 2002.06.02 MIK
	F_TOLOWER,
	F_TOUPPER,
	F_TOHANKAKU,
	F_TOZENKAKUKATA,
	F_TOZENKAKUHIRA,
	F_HANKATATOZENKATA,
	F_HANKATATOZENHIRA,
	F_TOZENEI,					// 2001/07/30 Misaka
	F_TOHANEI,
	F_TOHANKATA,				// 2002/08/29 ai
	F_TABTOSPACE,
	F_SPACETOTAB,  //---- Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS,
	F_CODECNV_EMAIL,
	F_CODECNV_EUC2SJIS,
	F_CODECNV_UNICODE2SJIS,
	F_CODECNV_UTF82SJIS,
	F_CODECNV_UTF72SJIS,
	F_CODECNV_UNICODEBE2SJIS,
	F_CODECNV_SJIS2JIS,
	F_CODECNV_SJIS2EUC,
	F_CODECNV_SJIS2UTF8,
	F_CODECNV_SJIS2UTF7,
	F_REPLACE_DIALOG,
	F_REPLACE,
	F_REPLACE_ALL,
	F_CHGMOD_INS,
	F_HOKAN,
};

/*!
	@note
		m_pcEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD

	@date 2000.05.12 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	@date 2004.06.21 novice �^�O�W�����v�@�\�ǉ�
*/
CEditDoc::CEditDoc()
: m_nSettingType( 0 )	// Sep. 11, 2002 genta
, m_nSettingTypeLocked( false )	//	�ݒ�l�ύX�\�t���O
, m_bIsModified( false )	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
, m_bGrepRunning( FALSE )		/* Grep������ */
, m_nCommandExecNum( 0 )			/* �R�}���h���s�� */
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_cSaveLineCode( EOL_NONE )		//	�ۑ�����Line Type
, m_bReadOnly( false )			/* �ǂݎ���p���[�h */
, m_bDebugMode( false )			/* �f�o�b�O���j�^���[�h */
, m_bGrepMode( false )			/* Grep���[�h�� */
, m_hInstance( NULL )
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )	/* �t�@�C���̔r�����䃂�[�h */
, m_hLockedFile( INVALID_HANDLE_VALUE )	/* ���b�N���Ă���t�@�C���̃n���h�� */
, m_eWatchUpdate( CEditDoc::WU_QUERY )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );

	m_szFilePath[0] = _T('\0');			/* ���ݕҏW���̃t�@�C���̃p�X */
	m_szSaveFilePath[0] = _T('\0');		/* �ۑ����̃t�@�C���̃p�X�i�}�N���p�j */	// 2006.09.04 ryoji
	m_szGrepKey[0] = _T('\0');

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_pTypes = &m_pShareData->m_Types[m_nSettingType];

	// ���C�A�E�g�Ǘ����̏�����
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );

	// ���C�A�E�g���̕ύX
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	STypeConfig ref = GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH ){
		ref.m_nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo( true, NULL, ref );

	//	�����ۑ��̐ݒ�	//	Aug, 21, 2000 genta
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	�}�N��
	m_pcSMacroMgr = CEditApp::getInstance()->m_pcSMacroMgr;
	
	//	m_FileTime�̏�����
	m_FileTime.ClearFILETIME();

	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode );

	// 2008.06.07 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	// �����R�[�h��ʂ�������
	m_nCharCode = m_pShareData->m_Types[0].m_encoding.m_eDefaultCodetype;
	m_bBomExist = m_pShareData->m_Types[0].m_encoding.m_bDefaultBom;
	SetNewLineCode( static_cast<EEolType>(m_pShareData->m_Types[0].m_encoding.m_eDefaultEoltype) );

#ifdef _DEBUG
	{
		// �ҏW�֎~�R�}���h�̕��т��`�F�b�N
		int i;
		for ( i = 0; i < _countof(EIsModificationForbidden) - 1; i++){
			assert( EIsModificationForbidden[i] <  EIsModificationForbidden[i+1] );
		}
	}
#endif
}


CEditDoc::~CEditDoc()
{
	/* �t�@�C���̔r�����b�N���� */
	DoFileUnlock();
}


void CEditDoc::Clear()
{
	// �t�@�C���̔r�����b�N����
	DoFileUnlock();

	// �t�@�C���̔r�����䃂�[�h
	m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;

	// �A���h�D�E���h�D�o�b�t�@�̃N���A
	m_cOpeBuf.ClearAll();

	// �e�L�X�g�f�[�^�̃N���A
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	// �t�@�C���p�X�ƃA�C�R���̃N���A
	SetFilePathAndIcon( _T("") );

	// �t�@�C���̃^�C���X�^���v�̃N���A
	m_FileTime.ClearFILETIME();

	// �u��{�v�̃^�C�v�ʐݒ��K�p
	int doctype = CShareData::getInstance()->GetDocumentTypeOfPath( GetFilePath() );
	SetDocumentType( doctype, true );

	// ���C�A�E�g�Ǘ����̏�����
	STypeConfig& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo( true, NULL, ref );
}

/* �����f�[�^�̃N���A */
void CEditDoc::InitDoc()
{
	m_bReadOnly = false;	// �ǂݎ���p���[�h
	m_szGrepKey[0] = _T('\0');

	m_bGrepMode = false;	/* Grep���[�h */
	m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta �X�V�Ď����@

	// 2005.06.24 Moca �o�O�C��
	//	�A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if( m_bDebugMode ){
		m_pcEditWnd->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	�A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	Clear();

	/* �ύX�t���O */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* �����R�[�h��� */
	m_nCharCode = m_pShareData->m_Types[0].m_encoding.m_eDefaultCodetype;
	m_bBomExist = m_pShareData->m_Types[0].m_encoding.m_bDefaultBom;
	SetNewLineCode( static_cast<EEolType>(m_pShareData->m_Types[0].m_encoding.m_eDefaultEoltype) );

	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode );
}


/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
void CEditDoc::InitAllView( void )
{

	m_nCommandExecNum = 0;	/* �R�}���h���s�� */

	// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	// CEditWnd�Ɉ��z��
	m_pcEditWnd->InitAllViews();

	return;
}



/*! �E�B���h�E�̍쐬��

	@date 2001.09.29 genta �}�N���N���X��n���悤��
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	CEditWnd* pcEditWnd,
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::Create" );

	m_hInstance = hInstance;

	m_pcEditWnd = pcEditWnd;

	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pShareData->m_Common.m_sMacro.m_MacroTable, &m_pShareData->m_Common );

	MY_TRACETIME( cRunningTimer, "End: PropSheet" );

	return TRUE;
}




/*!
	�t�@�C�����̐ݒ�
	
	�t�@�C������ݒ肷��Ɠ����ɁC�E�B���h�E�A�C�R����K�؂ɐݒ肷��D
	
	@param szFile [in] �t�@�C���̃p�X��
	
	@author genta
	@date 2002.09.09
*/
void CEditDoc::SetFilePathAndIcon(const TCHAR* szFile)
{
	_tcscpy( m_szFilePath, szFile );
	SetDocumentIcon();
}


/*! @brief �w��R�}���h�ɂ�鏑���������֎~����Ă��邩�ǂ���

	@retval true  �֎~
	@retval false ����

	@date 2000.08.14 genta �V�K�쐬
	@date 2014.07.27 novice �ҏW�֎~�̏ꍇ�̌������@�ύX
*/
bool CEditDoc::IsModificationForbidden( int nCommand ) const
{
	//	�ҏW�\�̏ꍇ
	if( IsEditable() )
		return false; // ��ɏ�����������

	//	�ҏW�֎~�̏ꍇ(�o�C�i���T�[�`)
	{
		int lbound = 0;
		int ubound = _countof(EIsModificationForbidden) - 1;

		while( lbound <= ubound ){
			int mid = ( lbound + ubound ) / 2;

			if( nCommand < EIsModificationForbidden[mid] ){
				ubound = mid - 1;
			} else if( nCommand > EIsModificationForbidden[mid] ){
				lbound = mid + 1;
			}else{
				return true;
			}
		}
	}

	return false;
}

/*! �R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param[in] nCommand MAKELONG( �R�}���h�R�[�h�C���M�����ʎq )

	@date 2006.05.19 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX
	@date 2007.06.20 ryoji �O���[�v���ŏ��񂷂�悤�ɕύX
*/
BOOL CEditDoc::HandleCommand( int nCommand )
{
	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//�O�̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}else{
				CControlTray::ActiveNextWindow(m_pcEditWnd->m_hWnd);
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//���̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}
			else{
				CControlTray::ActivePrevWindow(m_pcEditWnd->m_hWnd);
			}
		}
		return TRUE;
	default:
		return m_pcEditWnd->GetActiveView().HandleCommand( nCommand, true, 0, 0, 0, 0 );
	}
}

/*!	�^�C�v�ʐݒ�̓K�p��ύX
	@date 2011.12.15 CEditView::Command_TYPE_LIST����ړ�
*/
void CEditDoc::OnChangeType()
{
	// �V�K�Ŗ��ύX�Ȃ�f�t�H���g�����R�[�h��K�p����	// 2011.01.24 ryoji
	if( !IsValidPath() ){
		if( !IsModified()  && m_cDocLineMgr.GetLineCount() == 0 ){
			STypeConfig& types = GetDocumentAttribute();
			m_nCharCode = types.m_encoding.m_eDefaultCodetype;
			m_bBomExist = types.m_encoding.m_bDefaultBom;
			SetNewLineCode( static_cast<EEolType>(types.m_encoding.m_eDefaultEoltype) );
		}
	}
	/* �ݒ�ύX�𔽉f������ */
	m_bTextWrapMethodCurTemp = false;	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p��������	// 2008.06.08 ryoji
	OnChangeSetting();

	// 2006.09.01 ryoji �^�C�v�ύX�㎩�����s�}�N�������s����
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged );
}

/*! �r���[�ɐݒ�ύX�𔽉f������
	@param [in] bDoLayout ���C�A�E�g���̍č쐬

	@date 2004.06.09 Moca ���C�A�E�g�č\�z����Progress Bar��\������D
	@date 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX������ǉ�
	@date 2013.04.22 novice ���C�A�E�g���̍č쐬��ݒ�ł���悤�ɂ���
*/
void CEditDoc::OnChangeSetting(
	bool	bDoLayout
)
{
	int			i;
	HWND		hwndProgress = NULL;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
		//	Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	/* �t�@�C���̔r�����[�h�ύX */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
		/* �t�@�C���̔r�����b�N���� */
		DoFileUnlock();
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	CShareData::getInstance()->TransformFileName_MakeCache();
	int doctype = CShareData::getInstance()->GetDocumentTypeOfPath( GetFilePath() );
	SetDocumentType( doctype, false );

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* ���C�A�E�g���̍쐬 */
	STypeConfig ref = GetDocumentAttribute();
	{
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �܂�Ԃ����@�̈ꎞ�ݒ�ƃ^�C�v�ʐݒ肪��v������ꎞ�ݒ�K�p���͉���
		if( m_nTextWrapMethodCur == ref.m_nTextWrapMethod )
			m_bTextWrapMethodCurTemp = false;		// �ꎞ�ݒ�K�p��������

		// �ꎞ�ݒ�K�p���łȂ���ΐ܂�Ԃ����@�ύX
		if( !m_bTextWrapMethodCurTemp )
			m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// �܂�Ԃ����@

		// �w�茅�Ő܂�Ԃ��F�^�C�v�ʐݒ���g�p
		// �E�[�Ő܂�Ԃ��F���Ɍ��݂̐܂�Ԃ������g�p
		// ��L�ȊO�FMAXLINEKETAS���g�p
		if( m_nTextWrapMethodCur != WRAP_SETTING_WIDTH ){
			if( m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH )
				ref.m_nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
			else
				ref.m_nMaxLineKetas = MAXLINEKETAS;
		}
	}

	m_cLayoutMgr.SetLayoutInfo( bDoLayout, hwndProgress, ref ); /* ���C�A�E�g���̕ύX */

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	/* �r���[�ɐݒ�ύX�𔽉f������ */
	int viewCount = m_pcEditWnd->GetAllViewCount();
	for( i = 0; i < viewCount; ++i ){
		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
	}
	m_pcEditWnd->RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}

/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL CEditDoc::OnFileClose()
{
	int			nRet;
	int			nBool;
	HWND		hwndMainFrame;
	hwndMainFrame = m_pcEditWnd->m_hWnd;

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	if( m_bGrepRunning ){		/* Grep������ */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			hwndMainFrame,
			_T("Grep�̏������ł��B\n")
		);
		return FALSE;
	}


	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( IsModified()
	&& !m_bDebugMode	/* �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ� */
	){
		if( m_bGrepMode ){	/* Grep���[�h�̂Ƃ� */
			/* Grep���[�h�ŕۑ��m�F���邩 */
			if( FALSE == m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm ){
				return TRUE;
			}
		}
		/* �E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			ConfirmBeep();
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H\n\n�ǂݎ���p�ŊJ���Ă���̂ŁA���O��t���ĕۑ�����΂����Ǝv���܂��B\n"),
				IsValidPath() ? GetFilePath() : _T("(����)")
			);
			switch( nRet ){
			case IDYES:
				nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}
		else{
			ConfirmBeep();
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H"),
				IsValidPath() ? GetFilePath() : _T("(����)")
			);
			switch( nRet ){
			case IDYES:
				if( IsValidPath() ){
					nBool = FileSave();	// 2006.12.30 ryoji
				}
				else{
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}
	}else{
		return TRUE;
	}
}

/*!	@brief �}�N���������s

	@param type [in] �������s�}�N���ԍ�
	@return

	@author ryoji
	@date 2006.09.01 ryoji �쐬
	@date 2007.07.20 genta HandleCommand�ɒǉ�����n���D
		�������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
*/
void CEditDoc::RunAutoMacro( int idx, LPCTSTR pszSaveFilePath )
{
	static bool bRunning = false;
	if( bRunning )
		return;	// �ē�����s�͂��Ȃ�

	bRunning = true;
	if( m_pcSMacroMgr->IsEnabled(idx) ){
		if( !( ::GetAsyncKeyState(VK_SHIFT) & 0x8000 ) ){	// Shift �L�[��������Ă��Ȃ���Ύ��s
			if( NULL != pszSaveFilePath )
				_tcscpy( m_szSaveFilePath, pszSaveFilePath );
			//	2007.07.20 genta �������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
			HandleCommand(( F_USERMACRO_0 + idx ) | FA_NONRECORD );
			m_szSaveFilePath[0] = _T('\0');
		}
	}
	bRunning = false;
}

/*! �e�L�X�g���I������Ă��邩 */
BOOL CEditDoc::IsTextSelected( void ) const
{
	return m_pcEditWnd->GetActiveView().IsTextSelected();
}




/*! �t�@�C�����J��

	@return ����: TRUE/pbOpened==FALSE,
			���ɊJ����Ă���: FALSE/pbOpened==TRUE
			���s: FALSE/pbOpened==FALSE

	@note genta �߂������Ɍ����������������ȁD

	@date 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
	@date 2000.05,12 genta ���s�R�[�h�̐ݒ�
	@date 2000.10.25 genta �����R�[�h�ُ̈�Ȓl���`�F�b�N
	@date 2000.11.20 genta IME��Ԃ̐ݒ�
	@date 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	@date 2002.01.16 hor �u�b�N�}�[�N����
	@date 2002.10.19 genta �ǂݎ��s�̃t�@�C���͕����R�[�h���ʂŎ��s����
	@date 2003.03.28 MIK ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��
	@date 2003.07.26 ryoji BOM�����ǉ�
	@date 2002.05.26 Moca gm_pszCodeNameArr_1 ���g���悤�ɕύX
	@date 2004.06.18 moca �t�@�C�����J���Ȃ������ꍇ��pbOpened��FALSE�ɏ���������Ă��Ȃ������D
	@date 2004.10.09 genta ���݂��Ȃ��t�@�C�����J�����Ƃ����Ƃ���
					�t���O�ɉ����Čx�����o���i�ȑO�̓���j�悤��
	@date 2006.12.16 ���イ�� �O��̕����R�[�h��D�悷��
	@date 2007.03.12 maru �t�@�C�������݂��Ȃ��Ă��O��̕����R�[�h���p��
						���d�I�[�v��������CEditDoc::IsPathOpened�Ɉړ�
*/
BOOL CEditDoc::FileRead(
	TCHAR*	pszPath,	//!< [in,out]
	BOOL*	pbOpened,	//!< [out] ���łɊJ����Ă�����
	ECodeType	nCharCode,		/*!< [in] �����R�[�h��� */
	bool	bReadOnly,			/*!< [in] �ǂݎ���p�� */
	bool	bConfirmCodeChange	/*!< [in] �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
)
{
	int				i;
	HWND			hWndOwner;
	BOOL			bRet;
	EditInfo		fi;
//	EditInfo*		pfi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( m_pcEditWnd->m_hWnd );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	*pbOpened = FALSE;	// 2004.06.18 Moca �������~�X
	m_bReadOnly = bReadOnly;	/* �ǂݎ���p���[�h */

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRUFile			cMRU;

	/* �t�@�C���̑��݃`�F�b�N */
	bFileIsExist = FALSE;
	if( !fexist( pszPath ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );
//? 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
//?		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
//?		}else{
			bFileIsExist = TRUE;
//?		}
		/* �t�H���_���w�肳�ꂽ�ꍇ */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* �w��t�H���_�Łu�J���_�C�A���O�v��\�� */
			{
				TCHAR*		pszPathNew = new TCHAR[_MAX_PATH];

				pszPathNew[0] = _T('\0');

				/* �u�t�@�C�����J���v�_�C�A���O */
				nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
				bReadOnly = false;
//				::ShowWindow( m_hWnd, SW_SHOW );
				if( !OpenFileDialog( m_pcEditWnd->m_hWnd, pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					delete [] pszPathNew;
					return FALSE;
				}
				_tcscpy( pszPath, pszPathNew );
				delete [] pszPathNew;
				if( !fexist( pszPath ) ){
					bFileIsExist = FALSE;
				}else{
					bFileIsExist = TRUE;
				}
			}
		}

	}

	//	From Here Oct. 19, 2002 genta
	//	�ǂݍ��݃A�N�Z�X���������ꍇ�ɂ͊����R�[�h����Ńt�@�C����
	//	�J���Ȃ��̂ŕ����R�[�h���ʃG���[�Əo�Ă��܂��D
	//	���K�؂ȃ��b�Z�[�W���o�����߁C�ǂ߂Ȃ��t�@�C����
	//	���O�ɔ���E�r������
	//
	//	_taccess�ł̓��b�N���ꂽ�t�@�C���̏�Ԃ��擾�ł��Ȃ��̂�
	//	���ۂɃt�@�C�����J���Ċm�F����
	if( bFileIsExist){
		HANDLE hTest = 	CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hTest == INVALID_HANDLE_VALUE ){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			ErrorMessage(
				m_pcEditWnd->m_hWnd,
				_T("\'%s\'\n�Ƃ����t�@�C�����J���܂���B\n�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
			return FALSE;
		}
		else {
			CloseHandle( hTest );
		}
	}
	//	To Here Oct. 19, 2002 genta

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	bRet = TRUE;
	if( NULL == pszPath ){
		MYMESSAGEBOX(
			m_pcEditWnd->m_hWnd,
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			_T("�o�O���႟�������I�I�I"),
			_T("CEditDoc::FileRead()\n\nNULL == pszPath\n�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������ˁB")
		);
		return FALSE;
	}
	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pszPath, &hWndOwner, nCharCode) ){	/* 2007.03.12 maru ���d�I�[�v��������IsPathOpened�ɂ܂Ƃ߂� */
		*pbOpened = TRUE;
		bRet = FALSE;
		goto end_of_func;
	}
	for( i = 0; i < m_pcEditWnd->GetAllViewCount(); ++i ){
		if( m_pcEditWnd->m_pcEditViewArr[i]->IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pcEditWnd->m_pcEditViewArr[i]->DisableSelectArea( true );
		}
	}

	//	Sep. 10, 2002 genta
	SetFilePathAndIcon( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */


	/* �w�肳�ꂽ�����R�[�h��ʂɕύX���� */
	//	Oct. 25, 2000 genta
	//	�����R�[�h�Ƃ��Ĉُ�Ȓl���ݒ肳�ꂽ�ꍇ�̑Ή�
	//	-1�ȏ�CODE_MAX�����̂ݎ󂯕t����
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECT�͂��͈̔͂���O��Ă��邩��ʂɃ`�F�b�N
	if( nCharCode == CODE_NONE || IsValidCodeType(nCharCode) || nCharCode == CODE_AUTODETECT )
		m_nCharCode = nCharCode;
	
	/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	if ( cMRU.GetEditInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

		if( CODE_NONE == m_nCharCode ){
			/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
			m_nCharCode = fi.m_nCharCode;
		}
		
		if( !bConfirmCodeChange && ( CODE_AUTODETECT == m_nCharCode ) ){	// �����R�[�h�w��̍ăI�[�v���Ȃ�O��𖳎�
			m_nCharCode = fi.m_nCharCode;
		}
		if( (FALSE == bFileIsExist) && (CODE_AUTODETECT == m_nCharCode) ){
			/* ���݂��Ȃ��t�@�C���̕����R�[�h�w��Ȃ��Ȃ�O����p�� */
			m_nCharCode = fi.m_nCharCode;
		}
	} else {
		bIsExistInMRU = FALSE;
	}

	/* �����R�[�h�������� */
	if( CODE_AUTODETECT == m_nCharCode ) {
		if( FALSE == bFileIsExist ){	/* �t�@�C�������݂��Ȃ� */
			m_nCharCode = CODE_DEFAULT;
		} else {
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( CODE_NONE == m_nCharCode ){
				TopWarningMessage( m_pcEditWnd->m_hWnd,
					_T("%s\n�����R�[�h�̔��ʏ����ŃG���[���������܂����B"),
					pszPath
				);
				//	Sep. 10, 2002 genta
				SetFilePathAndIcon( _T("") );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	/* �����R�[�h���قȂ�Ƃ��Ɋm�F���� */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (m_nCharCode != fi.m_nCharCode ) {	// MRU �̕����R�[�h�Ɣ��ʂ��قȂ�
			LPCTSTR	pszCodeNameOld = NULL;
			LPCTSTR	pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_1 ���g���悤�ɕύX Moca. 2002/05/26
			if( IsValidCodeType(fi.m_nCharCode) ){
				pszCodeNameOld = gm_pszCodeNameArr_1[fi.m_nCharCode];
			}
			if( IsValidCodeType(m_nCharCode) ){
				pszCodeNameNew = gm_pszCodeNameArr_1[m_nCharCode];
			}
			if( pszCodeNameOld != NULL ){
				ConfirmBeep();
				nRet = MYMESSAGEBOX(
					m_pcEditWnd->m_hWnd,
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					_T("�����R�[�h���"),
					_T("%s\n")
					_T("\n")
					_T("���̃t�@�C���́A�O��͕ʂ̕����R�[�h %s �ŊJ����Ă��܂��B\n")
					_T("�O��Ɠ��������R�[�h���g���܂����H\n")
					_T("\n")
					_T("�E[�͂�(Y)]  ��%s\n")
					_T("�E[������(N)]��%s\n")
					_T("�E[�L�����Z��]���J���܂���"),
					GetFilePath(),
					pszCodeNameOld,
					pszCodeNameOld,
					pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
					m_nCharCode = fi.m_nCharCode;
				}else
				if( IDCANCEL == nRet ){
					m_nCharCode = CODE_DEFAULT;
					//	Sep. 10, 2002 genta
					SetFilePathAndIcon( _T("") );
					bRet = FALSE;
					goto end_of_func;
				}
			}else{
				MYMESSAGEBOX(
					m_pcEditWnd->m_hWnd,
					MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
					_T("�o�O���႟�������I�I�I"),
					_T("�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������B")
				);
				//	Sep. 10, 2002 genta
				SetFilePathAndIcon( _T("") );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	if( CODE_NONE == m_nCharCode ){
		m_nCharCode = CODE_DEFAULT;
	}

	//	Nov. 12, 2000 genta �����O�t�@�C�����̎擾��O���Ɉړ�
	char szWork[MAX_PATH];
	/* �����O�t�@�C�������擾���� */
	if( FALSE != ::GetLongFileName( pszPath, szWork ) ){
		//	Sep. 10, 2002 genta
		SetFilePathAndIcon( szWork );
	}

	// �^�C�v�ʐݒ�
	if( bIsExistInMRU && ((fi.m_nTypeId>=0 && fi.m_nTypeId<MAX_TYPES)) ){
		doctype = fi.m_nTypeId;
	}else{
		doctype = CShareData::getInstance()->GetDocumentTypeOfPath( GetFilePath() );
	}
	SetDocumentType( doctype, true );

	//	From Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�
	switch( m_nCharCode ){
	case CODE_UNICODE:
	case CODE_UNICODEBE:
		m_bBomExist = true;
		break;
	case CODE_UTF8:
	default:
		m_bBomExist = false;
		break;
	}
	//	To Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�

	//�t�@�C�������݂���ꍇ�̓t�@�C����ǂ�
	if( bFileIsExist ){
		/* �t�@�C����ǂ� */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		//	Jul. 26, 2003 ryoji BOM�����ǉ�
		if( FALSE == m_cDocLineMgr.ReadFile( GetFilePath(), m_pcEditWnd->m_hWnd, hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.m_sFile.GetAutoMIMEdecode(), &m_bBomExist ) ){
			//	Sep. 10, 2002 genta
			SetFilePathAndIcon( _T("") );
			bRet = FALSE;
			goto end_of_func;
		}
	}else{
		// ���݂��Ȃ��Ƃ����h�L�������g�ɕ����R�[�h�𔽉f����
		const STypeConfig& type = GetDocumentAttribute();
		m_nCharCode = type.m_encoding.m_eDefaultCodetype;
		m_bBomExist = type.m_encoding.m_bDefaultBom;

		// �I�v�V�����F�J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
		if( m_pShareData->m_Common.m_sFile.GetAlertIfFileNotExist() ){
			InfoBeep();

			//	Feb. 15, 2003 genta Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			TopInfoMessage(
				m_pcEditWnd->m_hWnd,
				_T("%s\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B"),	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			);
		}
	}

	/* ���C�A�E�g���̕ύX */
	{
//		STypeConfig& ref = GetDocumentAttribute();
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
		// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
		STypeConfig ref = GetDocumentAttribute();
		if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
			ref.m_nMaxLineKetas = MAXLINEKETAS;

		m_cLayoutMgr.SetLayoutInfo( true, hwndProgress, ref );
	}

	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.m_sFile.GetRestoreCurPosition() ){
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nCaretPosX;
		int		nCaretPosY;
		m_cLayoutMgr.LogicToLayout(
			fi.m_ptCursor.x,
			fi.m_ptCursor.y,
			&nCaretPosX,
			&nCaretPosY
		);
		if( nCaretPosY >= m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
			m_pcEditWnd->GetActiveView().HandleCommand( F_GOFILEEND, false, 0, 0, 0, 0 );
		}else{
			m_pcEditWnd->GetActiveView().m_nViewTopLine = fi.m_nViewTopLine; // 2001/10/20 novice
			m_pcEditWnd->GetActiveView().m_nViewLeftCol = fi.m_nViewLeftCol; // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const CDocLine *pTmpDocLine = m_cDocLineMgr.GetLine( fi.m_ptCursor.y );	// 2008.08.22 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) nCaretPosX--;
			}
			// To Here Mar. 28, 2003 MIK
			m_pcEditWnd->GetActiveView().MoveCursor( nCaretPosX, nCaretPosY, true );
			m_pcEditWnd->GetActiveView().m_nCaretPosX_Prev = m_pcEditWnd->GetActiveView().m_ptCaretPos.x;
		}
	}
	// 2002.01.16 hor �u�b�N�}�[�N����
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.m_sFile.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		fi.m_szMarkLines[0] = '\0';
	}
	GetEditInfo( &fi );

	//	May 12, 2000 genta
	//	���s�R�[�h�̐ݒ�
	{
		const STypeConfig& type = GetDocumentAttribute();
		if ( m_nCharCode == type.m_encoding.m_eDefaultCodetype ){
			SetNewLineCode( static_cast<EEolType>(type.m_encoding.m_eDefaultEoltype) );	// 2011.01.24 ryoji �f�t�H���gEOL
		}
		else{
			SetNewLineCode( EOL_CRLF );
		}
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLine( 0 );
		if( pFirstlineinfo != NULL ){
			EEolType t = (EEolType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );
	
	/* �J�����g�f�B���N�g���̕ύX */
	{
		char	szCurDir[_MAX_PATH];
		char	szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
		_splitpath( GetFilePath(), szDrive, szDir, NULL, NULL );
		strcpy( szCurDir, szDrive);
		strcat( szCurDir, szDir );
		::SetCurrentDirectory( szCurDir );
	}

end_of_func:;
	//	2004.05.13 Moca ���s�R�[�h�̐ݒ�����炱���Ɉړ�
	m_pcEditWnd->GetActiveView().ShowCaretPosInfo();

	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( FALSE != bRet && IsValidPath() ){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	//	From Here Jul. 26, 2003 ryoji �G���[�̎��͋K���BOM�ݒ�Ƃ���
	if( FALSE == bRet ){
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = true;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = false;
			break;
		}
	}
	//	To Here Jul. 26, 2003 ryoji
	return bRet;
}


/*!	@brief �t�@�C���̕ۑ�
	
	@param pszPath [in] �ۑ��t�@�C����
	@param cEolType [in] ���s�R�[�h���
	
	pszPath��NULL�ł����Ă͂Ȃ�Ȃ��B
	
	@date Feb. 9, 2001 genta ���s�R�[�h�p�����ǉ�
*/
BOOL CEditDoc::FileWrite( const char* pszPath, EEolType cEolType )
{
	BOOL		bRet;
	EditInfo	fi;
	HWND		hwndProgress;
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRUFile	cMRU;
	//	Feb. 9, 2001 genta
	CEol	cEol( cEolType );

	//	Jun.  5, 2004 genta ������ReadOnly�`�F�b�N������ƁC�t�@�C������ύX���Ă�
	//	�ۑ��ł��Ȃ��Ȃ��Ă��܂��̂ŁC�`�F�b�N���㏑���ۑ������ֈړ��D

	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	WatchUpdate wuSave = m_eWatchUpdate;
	m_eWatchUpdate = WU_NONE;

	bRet = TRUE;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}


	/* �t�@�C���̔r�����b�N���� */
	DoFileUnlock();

	if( m_pShareData->m_Common.m_sBackup.m_bBackUp ){	/* �o�b�N�A�b�v�̍쐬 */
		//	Jun.  5, 2004 genta �t�@�C������^����悤�ɁD�߂�l�ɉ�����������ǉ��D
		switch( MakeBackUp( pszPath )){
		case 2:	//	���f�w��
			return FALSE;
		case 3: //	�t�@�C���G���[
			if( IDYES != ::MYMESSAGEBOX(
				m_pcEditWnd->m_hWnd,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				_T("�t�@�C���ۑ�"),
				_T("�o�b�N�A�b�v�̍쐬�Ɏ��s���܂����D���t�@�C���ւ̏㏑�����p�����čs���܂����D")
			)){
				return FALSE;
			}
		}
	}

	CWaitCursor cWaitCursor( m_pcEditWnd->m_hWnd );
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	if( FALSE == m_cDocLineMgr.WriteFile( pszPath, m_pcEditWnd->m_hWnd, hwndProgress,
		m_nCharCode, &m_FileTime, cEol , m_bBomExist ) ){
		bRet = FALSE;
		goto end_of_func;
	}
	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	m_cDocLineMgr.ResetAllModifyFlag();

	int	v;
	for( v = 0; v < m_pcEditWnd->GetAllViewCount(); ++v ){
		if( m_pcEditWnd->m_nActivePaneIndex != v ){
			m_pcEditWnd->m_pcEditViewArr[v]->RedrawAll();
		}
	}
	m_pcEditWnd->GetActiveView().RedrawAll();

	//	Sep. 10, 2002 genta
	SetFilePathAndIcon( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */

	SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = false;	/* �ǂݎ���p���[�h */

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption();
end_of_func:;

	if( IsValidPath() &&
		!m_bReadOnly && /* �ǂݎ���p���[�h �ł͂Ȃ� */
		FALSE != bRet
	){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	m_eWatchUpdate = wuSave;


	return bRet;
}

std::tstring CEditDoc::GetDlgInitialDir()
{
	if( IsValidPath() ){
		return GetFilePath();
	}
	else if( m_pShareData->m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_CUR ){
		// 2002.10.25 Moca
		TCHAR pszCurDir[_MAX_PATH];
		int nCurDir = ::GetCurrentDirectory( _countof(pszCurDir), pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			return _T("");
		}
		else{
			return pszCurDir;
		}
	}else if( m_pShareData->m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_MRU ){
		const CMRUFile cMRU;
		std::vector<LPCTSTR> vMRU = cMRU.GetPathList();
		if( !vMRU.empty() ){
			return vMRU[0];
		}else{
			TCHAR pszCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory( _countof(pszCurDir), pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				return _T("");
			}
			else{
				return pszCurDir;
			}
		}
	}else{
		TCHAR selDir[_MAX_PATH];
		CShareData::ExpandMetaToFolder( m_pShareData->m_Common.m_sEdit.m_OpenDialogSelDir , selDir, _countof(selDir) );
		return selDir;
	}
}

/* �u�t�@�C�����J���v�_�C�A���O */
//	Mar. 30, 2003 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
bool CEditDoc::OpenFileDialog(
	HWND		hwndParent,
	const char*	pszOpenFolder,	//!< [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	char*		pszPath,		//!< [out] �J���t�@�C���̃p�X���󂯎��A�h���X
	ECodeType*	pnCharCode,		//!< [out] �w�肳�ꂽ�����R�[�h��ʂ��󂯎��A�h���X
	bool*		pbReadOnly		//!< [out] �ǂݎ���p��
)
{
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndParent );

	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		m_hInstance,
		hwndParent,
		"*.*",
		pszOpenFolder ? pszOpenFolder : GetDlgInitialDir().c_str(),	// �����t�H���_
		CMRUFile().GetPathList(),
		CMRUFolder().GetPathList()
	);
	return cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly );
}



/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O

	@param pszPath [out]	�ۑ��t�@�C����
	@param pnCharCode [out]	�ۑ������R�[�h�Z�b�g
	@param pcEol [out]		�ۑ����s�R�[�h

	@date 2001.02.09 genta	���s�R�[�h�����������ǉ�
	@date 2003.03.30 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
	@date 2003.07.20 ryoji	BOM�̗L�������������ǉ�
	@date 2006.11.10 ryoji	���[�U�[�w��̊g���q���󋵈ˑ��ŕω�������
*/
BOOL CEditDoc::SaveFileDialog( char* pszPath, ECodeType* pnCharCode, CEol* pcEol, bool* pbBomExist )
{
	char	szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	char	szExt[_MAX_EXT];

	/* �t�@�C���ۑ��_�C�A���O�̏����� */
	/* �t�@�C�����̖����t�@�C����������AppszMRU[0]���f�t�H���g�t�@�C�����Ƃ��āHppszOPENFOLDER����Ȃ��H */
	// �t�@�C�����̖����Ƃ��̓J�����g�t�H���_���f�t�H���g�ɂ��܂��BMar. 30, 2003 genta
	// �f���v�] No.2699 (2003/02/05)
	if( !IsValidPath() ){
		// 2002.10.25 Moca ����̃R�[�h�𗬗p Mar. 23, 2003 genta
		strcpy(szDefaultWildCard, "*.txt");
		if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew )
			strcat(szDefaultWildCard, ";*.*");	// �S�t�@�C���\��
	}else{
		_splitpath(GetFilePath(), NULL, NULL, NULL, szExt);
		if( szExt[0] == _T('.') && szExt[1] != _T('\0') ){
			strcpy(szDefaultWildCard, "*");
			strcat(szDefaultWildCard, szExt);
			if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile )
				strcat(szDefaultWildCard, ";*.*");	// �S�t�@�C���\��
		}else{
			strcpy(szDefaultWildCard, "*.*");
		}
	}

	/* �_�C�A���O��\�� */
	CDlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		m_hInstance,
		m_pcEditWnd->m_hWnd,
		szDefaultWildCard,
		GetDlgInitialDir().c_str(),	// �����t�H���_
		CMRUFile().GetPathList(),		//	�ŋ߂̃t�@�C��
		CMRUFolder().GetPathList()	//	�ŋ߂̃t�H���_
	);
	return cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol, pbBomExist );
}

/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
bool CEditDoc::IsEnableUndo( void ) const
{
	return m_cOpeBuf.IsEnableUndo();
}



/*! Redo(��蒼��)�\�ȏ�Ԃ��H */
bool CEditDoc::IsEnableRedo( void ) const
{
	return m_cOpeBuf.IsEnableRedo();
}




/*! �N���b�v�{�[�h����\��t���\���H */
BOOL CEditDoc::IsEnablePaste( void ) const
{
	UINT uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );

	// 2008/02/16 �N���b�v�{�[�h����̃t�@�C���p�X�\��t���Ή�	bosagami	zlib/libpng license
	if( ::IsClipboardFormatAvailable( CF_OEMTEXT )
	 || ::IsClipboardFormatAvailable( CF_HDROP )
	 || ::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return TRUE;
	}
	return FALSE;
}

/*! �o�b�N�A�b�v�̍쐬
	@author genta
	@date 2001.06.12 asa-o
		�t�@�C���̎��������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\
	@date 2001.12.11 MIK �o�b�N�A�b�v�t�@�C�����S�~���ɓ����@�\
	@date 2004.06.05 genta �o�b�N�A�b�v�Ώۃt�@�C���������ŗ^����悤�ɁD
		���O��t���ĕۑ��̎��͎����̃o�b�N�A�b�v������Ă����Ӗ��Ȃ̂ŁD
		�܂��C�o�b�N�A�b�v���ۑ����s��Ȃ��I������ǉ��D
	@date 2005.11.26 aroka �t�@�C����������FormatBackUpPath�ɕ���

	@param target_file [in] �o�b�N�A�b�v���p�X��

	@retval 0 �o�b�N�A�b�v�쐬���s�D
	@retval 1 �o�b�N�A�b�v�쐬�����D
	@retval 2 �o�b�N�A�b�v�쐬���s�D�ۑ����f�w���D
	@retval 3 �t�@�C������G���[�ɂ��o�b�N�A�b�v�쐬���s�D

	@todo Advanced mode�ł̐���Ǘ�
*/
int CEditDoc::MakeBackUp(
	const TCHAR* target_file
)
{
	int		nRet;

	/* �o�b�N�A�b�v�\�[�X�̑��݃`�F�b�N */
	//	Aug. 21, 2005 genta �������݃A�N�Z�X�����Ȃ��ꍇ��
	//	�t�@�C�����Ȃ��ꍇ�Ɠ��l�ɉ������Ȃ�
	if( (_taccess( target_file, 2 )) == -1 ){
		return 0;
	}

	const CommonSetting_Backup& bup_setting = m_pShareData->m_Common.m_sBackup;

	TCHAR	szPath[_MAX_PATH]; // �o�b�N�A�b�v��p�X��
	if( !FormatBackUpPath( szPath, _countof(szPath), target_file ) ){
		int nMsgResult = ::TopConfirmMessage(
			m_pcEditWnd->m_hWnd,
			_T("�o�b�N�A�b�v��̃p�X�쐬���ɃG���[�ɂȂ�܂����B\n")
			_T("�p�X���������邩�s���ȏ����ł��B\n")
			_T("�o�b�N�A�b�v���쐬�����ɏ㏑���ۑ����Ă�낵���ł����H")
		);
		if( nMsgResult == IDYES ){
			return 0;//	�ۑ��p��
		}
		return 2;// �ۑ����f
	}

	//@@@ 2002.03.23 start �l�b�g���[�N�E�����[�o�u���h���C�u�̏ꍇ�͂��ݔ��ɕ��荞�܂Ȃ�
	bool dustflag = false;
	if( bup_setting.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( bup_setting.m_bBackUpDialog ){	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		ConfirmBeep();
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//���ʐݒ�F�o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX(
				m_pcEditWnd->m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				_T("�o�b�N�A�b�v�쐬�̊m�F"),
				_T("�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n")
				_T("��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n")
				_T("\n")
				_T("%s\n")
				_T("    ��\n")
				_T("%s\n")
				_T("\n")
				_T("�쐬�����o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�݂܂��B\n"),
				target_file,
				szPath
			);
		}
		else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				m_pcEditWnd->m_hWnd,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				_T("�o�b�N�A�b�v�쐬�̊m�F"),
				_T("�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n")
				_T("��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n")
				_T("\n")
				_T("%s\n")
				_T("    ��\n")
				_T("%s\n")
				_T("\n"),
				target_file,
				szPath
			);	//Jul. 06, 2001 jepro [���O��t���ĕۑ�] �̏ꍇ������̂Ń��b�Z�[�W���C��
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta �߂�l�ύX
		if( IDNO == nRet ){
			return 0;//	�ۑ��p��
		}else if( IDCANCEL == nRet ){
			return 2;// �ۑ����f
		}
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1�̊g���q���c���ł�ǉ�
	if( bup_setting.GetBackupType() == 3 ||
		bup_setting.GetBackupType() == 6 ){
		//	���ɑ��݂���Backup�����炷����
		int				i;

		//	�t�@�C�������p
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		TCHAR*	pBase = szPath + _tcslen( szPath ) - 2;	//	2: �g���q�̍Ō��2���̈Ӗ�

		//------------------------------------------------------------------
		//	1. �Y���f�B���N�g������backup�t�@�C����1���T��
		for( i = 0; i <= 99; i++ ){	//	�ő�l�Ɋւ�炸�C99�i2���̍ő�l�j�܂ŒT��
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, _T("%02d"), i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
				break;
			}
			::FindClose( hFind );
			//	���������t�@�C���̑������`�F�b�N
			//	�͖ʓ|���������炵�Ȃ��D
			//	�������O�̃f�B���N�g������������ǂ��Ȃ�̂��낤...
		}
		--i;

		//------------------------------------------------------------------
		//	2. �ő�l���琧����-1�Ԃ܂ł��폜
		int boundary = bup_setting.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	�ŏ��l��0

		for( ; i >= boundary; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, _T("%02d"), i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( m_pcEditWnd->m_hWnd, szPath, _T("�폜���s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
				//	���s�����ꍇ
				//	��ōl����
			}
		}

		//	���̈ʒu��i�͑��݂���o�b�N�A�b�v�t�@�C���̍ő�ԍ���\���Ă���D

		//	3. ��������0�Ԃ܂ł̓R�s�[���Ȃ���ړ�
		TCHAR szNewPath[MAX_PATH];
		TCHAR *pNewNrBase;

		_tcscpy( szNewPath, szPath );
		pNewNrBase = szNewPath + _tcslen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, _T("%02d"), i );
			wsprintf( pNewNrBase, _T("%02d"), i + 1 );

			//	�t�@�C���̈ړ�
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	���s�����ꍇ
				//	��ōl����
				::MessageBox( m_pcEditWnd->m_hWnd, szPath, _T("�ړ����s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	/* �o�b�N�A�b�v�̍쐬 */
	//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	TCHAR	szPath2[MAX_PATH];
	wsprintf( szPath2, _T("%s%s"), szDrive, szDir );

	HANDLE			hFind;
	WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* ����I�� */
		//@@@ 2001.12.11 start MIK
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//@@@ 2002.03.23 �l�b�g���[�N�E�����[�o�u���h���C�u�łȂ�
			TCHAR	szDustPath[_MAX_PATH+1];
			_tcscpy(szDustPath, szPath);
			szDustPath[_tcslen(szDustPath) + 1] = _T('\0');
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = m_pcEditWnd->m_hWnd;
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//�_�C�A���O�Ȃ�
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"�o�b�N�A�b�v�t�@�C�������ݔ��Ɉړ����Ă��܂�...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* ����I�� */
			}else{
				/* �G���[�I�� */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* �G���[�I�� */
		//	Jun.  5, 2005 genta �߂�l�ύX
		return 3;
	}
	//	Jun.  5, 2005 genta �߂�l�ύX
	return 1;
}




/*! �o�b�N�A�b�v�p�X�̍쐬

	@author aroka

	@param szNewPath [out] �o�b�N�A�b�v��p�X��
	@param newPathCount [in] szNewPath�̃T�C�Y
	@param target_file [in] �o�b�N�A�b�v���p�X��

	@retval true  ����
	@retval false �o�b�t�@�s��

	@date 2005.11.29 aroka
		MakeBackUp���番���D���������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\�ǉ�
	@date 2008.11.23 nasukoji	�p�X����������ꍇ�ւ̑Ή�
	@date 2009.10.10 aroka	�K�w���󂢂Ƃ��ɗ�����o�O�̑Ή�
	@date 2012.12.26 aroka	�ڍאݒ�̃t�@�C���ۑ������ƌ��ݎ����ŏ��������킹��Ή�
	@date 2013.04.15 novice �w��t�H���_�̃��^������W�J�T�|�[�g

	@todo Advanced mode�ł̐���Ǘ�
*/
bool CEditDoc::FormatBackUpPath(
	TCHAR*			szNewPath,
	size_t 			newPathCount,
	const TCHAR*	target_file
)
{
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	TCHAR*	psNext;
	TCHAR	szTempPath[1024];		// �p�X���쐬�p�̈ꎞ�o�b�t�@�i_MAX_PATH��肠����x�傫�����Ɓj

	bool	bOverflow = false;		// �o�b�t�@�I�[�o�[�t���[

	const CommonSetting_Backup& bup_setting = m_pShareData->m_Common.m_sBackup;

	/* �p�X�̕��� */
	_tsplitpath( target_file, szDrive, szDir, szFname, szExt );

	if( bup_setting.m_bBackUpFolder
	  && (!bup_setting.m_bBackUpFolderRM || !IsLocalDrive( target_file )) ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */	// m_bBackUpFolderRM �ǉ�	2010/5/27 Uchi
		TCHAR selDir[_MAX_PATH];
		CShareData::ExpandMetaToFolder( bup_setting.m_szBackUpFolder, selDir, _countof(selDir) );
		if (GetFullPathName(selDir, _MAX_PATH, szTempPath, &psNext) == 0) {
			// ���܂����Ȃ�����
			_tcscpy( szTempPath, selDir );
		}
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		AddLastYenFromDirectoryPath( szTempPath );
	}
	else{
		wsprintf( szTempPath, _T("%s%s"), szDrive, szDir );
	}

	/* ���΃t�H���_��}�� */
	if( !bup_setting.m_bBackUpPathAdvanced ){
		time_t	ltime;
		struct	tm *today;
		TCHAR	szTime[64];
		TCHAR	szForm[64];

		TCHAR*	pBase;
		pBase = szTempPath + _tcslen( szTempPath );

		/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		switch( bup_setting.GetBackupType() ){
		case 1:
			wsprintf( pBase, _T("%s.bak"), szFname );
			break;
		case 5: //	Jun.  5, 2005 genta 1�̊g���q���c����
			wsprintf( pBase, _T("%s%s.bak"), szFname, szExt );
			break;
		case 2:	//	���t�C����
			_tzset();
			_strdate( szTime );
			time( &ltime );				/* �V�X�e�������𓾂܂� */
			today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

			_tcscpy( szForm, _T("") );
			if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				strcat( szForm, "%Y" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				strcat( szForm, "%m" );
			}
			if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				strcat( szForm, "%d" );
			}
			if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				strcat( szForm, "%H" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				strcat( szForm, "%M" );
			}
			if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				strcat( szForm, "%S" );
			}
			/* YYYYMMDD�����b �`���ɕϊ� */
			strftime( szTime, _countof( szTime ) - 1, szForm, today );
			wsprintf( pBase, _T("%s_%s%s"), szFname, szTime, szExt );
			break;
	//	2001/06/12 Start by asa-o: �t�@�C���ɕt������t��O��̕ۑ���(�X�V����)�ɂ���
		case 4:	//	���t�C����
			{
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );

				_tcscpy( szTime, _T("") );
				if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
					wsprintf(szTime,_T("%d"),ctimeLastWrite->wYear);
				}
				if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
					wsprintf(szTime,_T("%s%02d"),szTime,ctimeLastWrite->wMonth);
				}
				if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
					wsprintf(szTime,_T("%s%02d"),szTime,ctimeLastWrite->wDay);
				}
				if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
					wsprintf(szTime,_T("%s%02d"),szTime,ctimeLastWrite->wHour);
				}
				if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
					wsprintf(szTime,_T("%s%02d"),szTime,ctimeLastWrite->wMinute);
				}
				if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
					wsprintf(szTime,_T("%s%02d"),szTime,ctimeLastWrite->wSecond);
				}
				wsprintf( pBase, _T("%s_%s%s"), szFname, szTime, szExt );
			}
			break;
	// 2001/06/12 End

		case 3: //	?xx : xx = 00~99, ?�͔C�ӂ̕���
		case 6: //	Jun.  5, 2005 genta 3�̊g���q���c����
			//	Aug. 15, 2000 genta
			//	�����ł͍쐬����o�b�N�A�b�v�t�@�C�����̂ݐ�������D
			//	�t�@�C������Rotation�͊m�F�_�C�A���O�̌�ōs���D
			{
				//	Jun.  5, 2005 genta �g���q���c����悤�ɏ����N�_�𑀍삷��
				TCHAR* ptr;
				if( bup_setting.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + _tcslen( szExt );
				}
				*ptr   = _T('.');
				*++ptr = bup_setting.GetBackupExtChar();
				*++ptr = _T('0');
				*++ptr = _T('0');
				*++ptr = _T('\0');
			}
			wsprintf( pBase, _T("%s%s"), szFname, szExt );
			break;
		}

	}else{ // �ڍאݒ�g�p����
		TCHAR szFormat[1024];

		switch( bup_setting.GetBackupTypeAdv() ){
		case 4:	//	�t�@�C���̓��t�C����
			{
				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );
				if( !GetDateTimeFormat( szFormat, _countof(szFormat), bup_setting.m_szBackUpPathAdvanced , ctimeLastWrite.GetSYSTEMTIME() ) ){
					return false;
				}
			}
			break;
		case 2:	//	���݂̓��t�C����
		default:
			{
				// 2012.12.26 aroka	�ڍאݒ�̃t�@�C���ۑ������ƌ��ݎ����ŏ��������킹��
				SYSTEMTIME	SystemTime;
				::GetSystemTime(&SystemTime);			// ���ݎ������擾

				if( !GetDateTimeFormat( szFormat, _countof(szFormat), bup_setting.m_szBackUpPathAdvanced , SystemTime ) ){
					return false;
				}
			}
			break;
		}

		{
			// make keys
			// $0-$9�ɑΉ�����t�H���_����؂�o��
			TCHAR keybuff[1024];
			_tcscpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			TCHAR *folders[10];
			{
				//	Jan. 9, 2006 genta VC6�΍�
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = const_cast<TCHAR*>(_T(""));		// 2009.10.10 aroka	�K�w���󂢂Ƃ��ɗ�����o�O�̑Ή�
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					TCHAR *cp;
					cp = sjis_strrchr2(keybuff, _T('\\'), _T('\\'));
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = _T('\0');
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9��u��
				//_tcscpy( szNewPath, _T("") );
				TCHAR *q= szFormat;
				TCHAR *q2 = szFormat;
				while( *q ){
					if( *q==_T('$') ){
						++q;
						if( isdigit(*q) ){
							q[-1] = _T('\0');

							// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
							if( _tcslen( szTempPath ) + _tcslen( q2 ) + _tcslen( folders[*q-_T('0')] ) >= newPathCount ){
								bOverflow = true;
								break;
							}

							if( _T('\\') == *q2 ){
								// 2010.04.13 Moca \�̏d���`�F�b�N C:\backup\\dir�ɂȂ�̂��ɖh��
								// �������l�b�g���[�N�p�X����菜���ƍ���̂ŁA3�����ȏ�
								int nTempPathLen = strlen( szTempPath );
								if( 3 <= nTempPathLen && *::CharPrev( szTempPath, &szTempPath[nTempPathLen] ) == _T('\\') ){
									q2 +=1; // \���΂�
								}
							}

							strcat( szTempPath, q2 );
							//if( folders[*q-'0'] != 0 ){	// 2009.10.10 aroka	�o�O�Ή��Ń`�F�b�N�s�v�ɂȂ���
							strcat( szTempPath, folders[*q-_T('0')] );
							//}
							q2 = q+1;
						}
					}
					++q;
				}

				// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
				if( !bOverflow ){
					if( _tcslen( szTempPath ) + _tcslen( q2 ) >= newPathCount ){
						bOverflow = true;
					}else{
						strcat( szTempPath, q2 );
					}
				}
			}
		}

		if( !bOverflow ){
			TCHAR temp[1024];
			TCHAR *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'�̂Ƃ��̃I�[�o���������C��
			TCHAR *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];

			while( strchr( szTempPath, _T('*') ) ){
				_tcscpy( temp, szTempPath );
				cp = strchr( temp, _T('*') );
				*cp = 0;

				// 2008.11.25 nasukoji	�o�b�t�@�I�[�o�[�t���[�`�F�b�N
				if( cp - temp + _tcslen( ep ) + _tcslen( cp + 1 ) >= newPathCount ){
					bOverflow = true;
					break;
				}

				wsprintf( szTempPath, _T("%s%s%s"), temp, ep, cp+1 );
			}

			if( !bOverflow ){
				//	??�̓o�b�N�A�b�v�A�Ԃɂ������Ƃ���ł͂��邪�C
				//	�A�ԏ����͖�����2���ɂ����Ή����Ă��Ȃ��̂�
				//	�g�p�ł��Ȃ�����?��_�ɕϊ����Ă��������
				while(( cp = strchr( szTempPath, _T('?') ) ) != NULL){
					*cp = _T('_');
				}
			}
		}
	}

	// �쐬�����p�X��szNewPath�Ɏ��܂�Ȃ���΃G���[
	if( bOverflow || _tcslen( szTempPath ) >= newPathCount ){
		return false;
	}

	_tcscpy( szNewPath, szTempPath );	// �쐬�����p�X���R�s�[

	return true;
}

/* �t�@�C���̔r�����b�N */
void CEditDoc::DoFileLock( void )
{
	BOOL	bCheckOnly;

	/* ���b�N���Ă��� */
	if( INVALID_HANDLE_VALUE != m_hLockedFile ){
		/* ���b�N���� */
		CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}

	/* �t�@�C�������݂��Ȃ� */
	if( !fexist( GetFilePath() ) ){
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
		return;
	}else{
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = m_pShareData->m_Common.m_sFile.m_nFileShareMode;
	}


	/* �t�@�C�����J���Ă��Ȃ� */
	if( !IsValidPath() ){
		return;
	}
	/* �ǂݎ���p���[�h */
	if( m_bReadOnly ){
		return;
	}


	if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ||
		m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
		bCheckOnly = FALSE;
	}else{
		/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���̂�return���Ȃ� */
//		return;
		bCheckOnly = TRUE;
	}
	/* �����݋֎~���ǂ������ׂ� */
	if( -1 == _taccess( GetFilePath(), 2 ) ){	/* �A�N�Z�X���F�������݋��� */
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_pcEditWnd->UpdateCaption();
		return;
	}

	HANDLE hLockedFile = CreateFile(
		GetFilePath(),					//�t�@�C����
		GENERIC_READ | GENERIC_WRITE,	//�ǂݏ����^�C�v
		FILE_SHARE_READ | FILE_SHARE_WRITE,	//���L���[�h
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);
	CloseHandle( hLockedFile );
	if( INVALID_HANDLE_VALUE == hLockedFile ){
		TopWarningMessage(
			m_pcEditWnd->m_hWnd,
			_T("%s\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B"),
			IsValidPath() ? GetFilePath() : _T("(����)")
		);
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_pcEditWnd->UpdateCaption();
		return;
	}

	DWORD dwShareMode=0;
	switch(m_pShareData->m_Common.m_sFile.m_nFileShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return;												break; //�r�����䖳��
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //�ǂݏ����֎~�����L����
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //�������݋֎~���ǂݍ��݂̂ݔF�߂�
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //�֎~�����Ȃ����ǂݏ������ɔF�߂�
	}

	m_hLockedFile = CreateFile(
		GetFilePath(),					//�t�@�C����
		GENERIC_READ ,					//�ǂݏ����^�C�v
		dwShareMode,					//���L���[�h
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);

	if( INVALID_HANDLE_VALUE == m_hLockedFile ){
		const TCHAR*	pszMode;
		switch( m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
		case SHAREMODE_DENY_READWRITE:	/* �ǂݏ��� */
			pszMode = _T("�ǂݏ����֎~���[�h");
			break;
		case SHAREMODE_DENY_WRITE:	/* ���� */
			pszMode = _T("�������݋֎~���[�h");
			break;
		default:
			pszMode = _T("����`�̃��[�h�i��肪����܂��j");
			break;
		}
		TopWarningMessage(
			m_pcEditWnd->m_hWnd,
			_T("%s\n��%s�Ń��b�N�ł��܂���ł����B\n���݂��̃t�@�C���ɑ΂���r������͖����ƂȂ�܂��B"),
			IsValidPath() ? GetFilePath() : _T("(����)"),
			pszMode
		);
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_pcEditWnd->UpdateCaption();
		return;
	}
	/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���ꍇ */
	if( bCheckOnly ){
		/* ���b�N���������� */
		DoFileUnlock();

	}
	return;
}


/* �t�@�C���̔r�����b�N���� */
void CEditDoc::DoFileUnlock( void )
{
	if( INVALID_HANDLE_VALUE != m_hLockedFile ){
		/* ���b�N���� */
		CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
	}
	return;
}

/*! ���[���t�@�C����1�s���Ǘ�����\����

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake ���O�ύX: oneRule��SOneRule
*/
struct SOneRule {
	char szMatch[256];
	int  nLength;
	char szGroupName[256];
};

/*! ���[���t�@�C����ǂݍ��݁A���[���\���̂̔z����쐬����

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ����nMaxCount��ǉ��B�o�b�t�@���`�F�b�N������悤�ɕύX
*/
int CEditDoc::ReadRuleFile( const char* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE*	pFile = _tfopen_absini( pszFilename, "r" );
	if( NULL == pFile ){
		return 0;
	}
	char	szLine[LINEREADBUFSIZE];
	const char*	pszDelimit = " /// ";
	const char*	pszKeySeps = ",\0";
	char*	pszWork;
	int nDelimitLen = strlen( pszDelimit );
	int nCount = 0;
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += nDelimitLen;

			/* �ŏ��̃g�[�N�����擾���܂��B */
			char* pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = _stricmp( pszKey, pszToken );
				int nLen = (int)lstrlen(pszWork);
				for( i = 0; i < nLen; ++i ){
					if( pszWork[i] == '\r' ||
						pszWork[i] == '\n' ){
						pszWork[i] = '\0';
						break;
					}
				}
				strncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				strncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = '\0';
				pcOneRule[nCount].szGroupName[255] = '\0';
				pcOneRule[nCount].nLength = strlen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ���[���t�@�C�������ɁA�g�s�b�N���X�g���쐬

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca �l�X�g�̐[�����ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
*/
void CEditDoc::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	char*					pszText;

	/* ���[���t�@�C���̓��e���o�b�t�@�ɓǂݍ��� */
	const int nRuleSize = 1024;
	SOneRule* test = new SOneRule[nRuleSize];	//	1024���B // 516*1024 = 528,384 byte
	int nCount = ReadRuleFile(GetDocumentAttribute().m_szOutlineRuleFilename, test, nRuleSize );
	if ( nCount < 1 ){
		return;
	}

	/*	�l�X�g�̐[���́A32���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�256�����܂ŋ��
		�i256�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char pszStack[nMaxStack][256];
	char szTitle[256];			//	�ꎞ�̈�
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		for( j = 0; j < nCount; j++ ){
			if ( 0 == strncmp( (const char*)&pLine[i], test[j].szMatch, test[j].nLength ) ){
				strcpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}
		/*	���[���Ƀ}�b�`�����s�́A�A�E�g���C�����ʂɕ\������B
		*/
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		int nTextLen = lstrlen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
				break;
			}
		}
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.LogicToLayout(
			0,
			nLineCount,
			&nPosX,
			&nPosY
		);
		/* nDepth���v�Z */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			strcpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[�������邩��K������
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	delete [] test;
	return;
}

/* �ҏW�t�@�C�������i�[ */
void CEditDoc::GetEditInfo(
	EditInfo* pfi	//!< [out]
)
{
	//�t�@�C���p�X
	_tcscpy( pfi->m_szPath, GetFilePath() );

	//�\����
	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().m_nViewTopLine;	/* �\����̈�ԏ�̍s(0�J�n) */
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().m_nViewLeftCol;	/* �\����̈�ԍ��̌�(0�J�n) */

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_cLayoutMgr.LayoutToLogic(
		m_pcEditWnd->GetActiveView().m_ptCaretPos.x,	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
		m_pcEditWnd->GetActiveView().m_ptCaretPos.y,	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */
		&pfi->m_ptCursor.x,
		&pfi->m_ptCursor.y
	);

	//�e����
	pfi->m_bIsModified = IsModified();			/* �ύX�t���O */
	pfi->m_nCharCode = m_nCharCode;				/* �����R�[�h��� */
	pfi->m_nTypeId = GetDocumentType();

	//GREP���[�h
	pfi->m_bIsGrep = m_bGrepMode;
	_tcscpy( pfi->m_szGrepKey, m_szGrepKey );

	//�f�o�b�O���j�^ (�A�E�g�v�b�g�E�C���h�E) ���[�h
	pfi->m_bIsDebug = m_bDebugMode;
}

/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp	/* �X�V�̊Ď� */
	 // Dec. 4, 2002 genta
	 && m_eWatchUpdate != WU_NONE
	 && m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE	/* �t�@�C���̔r�����䃂�[�h */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* �A�N�e�B�u? */
	 && hwndActive == m_pcEditWnd->m_hWnd
	 && IsValidPath()
	 && ( !m_FileTime.IsZero() ) 	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */

	){
		/* �t�@�C���X�^���v���`�F�b�N���� */

		// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
		CFileTime ftime;
		if( GetLastWriteTimestamp( GetFilePath(), &ftime )){
			if( 0 != ::CompareFileTime( &m_FileTime.GetFILETIME(), &ftime.GetFILETIME() ) )	//	Aug. 13, 2003 wmlhq �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
			{
				bUpdate = TRUE;
				m_FileTime = ftime.GetFILETIME();
			}
		}
	}

	//	From Here Dec. 4, 2002 genta
	if( bUpdate ){
		switch( m_eWatchUpdate ){
		case WU_NOTIFY:
			{
				TCHAR szText[40];
				const CFileTime& ctime = m_FileTime;
				wsprintf( szText, _T("���t�@�C���X�V %02d:%02d:%02d"), ctime->wHour, ctime->wMinute, ctime->wSecond );
				m_pcEditWnd->SendStatusMessage( szText );
			}	
			break;
		default:
			{
				m_eWatchUpdate = WU_NONE; // �X�V�Ď��̗}��

				CDlgFileUpdateQuery dlg( GetFilePath(), IsModified() );
				int result = dlg.DoModal(
					m_hInstance,
					m_pcEditWnd->m_hWnd,
					IDD_FILEUPDATEQUERY,
					0
				);

				switch( result ){
				case 1:	// �ēǍ�
					/* ����t�@�C���̍ăI�[�v�� */
					ReloadCurrentFile( m_nCharCode, m_bReadOnly );
					m_eWatchUpdate = WU_QUERY;
					break;
				case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
					m_eWatchUpdate = WU_NOTIFY;
					break;
				case 3:	// �Ȍ�X�V���Ď����Ȃ�
					m_eWatchUpdate = WU_NONE;
					break;
				case 0:	// CLOSE
				default:
					m_eWatchUpdate = WU_QUERY;
					break;
				}
			}
			break;
		}
	}
	//	To Here Dec. 4, 2002 genta
}





/*! ����t�@�C���̍ăI�[�v�� */
void CEditDoc::ReloadCurrentFile(
	ECodeType	nCharCode,		/*!< [in] �����R�[�h��� */
	bool	bReadOnly		/*!< [in] �ǂݎ���p���[�h */
)
{
	if( !fexist( GetFilePath() ) ){
		/* �t�@�C�������݂��Ȃ� */
		//	Jul. 26, 2003 ryoji BOM��W���ݒ��
		m_nCharCode = nCharCode;
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = true;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = false;
			break;
		}
		return;
	}


	BOOL	bOpened;
	char	szFilePath[MAX_PATH];
	int		nViewTopLine = m_pcEditWnd->GetActiveView().m_nViewTopLine;	/* �\����̈�ԏ�̍s(0�J�n) */
	int		nViewLeftCol = m_pcEditWnd->GetActiveView().m_nViewLeftCol;	/* �\����̈�ԍ��̌�(0�J�n) */
	CLayoutPoint	ptCaretPosXY = m_pcEditWnd->GetActiveView().m_ptCaretPos;

	strcpy( szFilePath, GetFilePath() );

	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	AddToMRU();

	/* �����f�[�^�̃N���A */
	InitDoc();

	/* �S�r���[�̏����� */
	InitAllView();

	// ����ԍ��擾
	CShareData::getInstance()->GetNoNameNumber( m_pcEditWnd->m_hWnd );

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption();

	/* �t�@�C���ǂݍ��� */
	FileRead(
		szFilePath,
		&bOpened,
		nCharCode,	/* �����R�[�h�������� */
		bReadOnly,	/* �ǂݎ���p�� */
		false		/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
	);

	// ���C�A�E�g�s�P�ʂ̃J�[�\���ʒu����
	// �������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������
	// 2007.08.23 ryoji �\���̈敜��
	if( ptCaretPosXY.y < m_cLayoutMgr.GetLineCount() ){
		m_pcEditWnd->GetActiveView().m_nViewTopLine = nViewTopLine;
		m_pcEditWnd->GetActiveView().m_nViewLeftCol = nViewLeftCol;
	}
	m_pcEditWnd->GetActiveView().MoveCursorProperly( ptCaretPosXY, true );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcEditWnd->GetActiveView().m_nCaretPosX_Prev = m_pcEditWnd->GetActiveView().m_ptCaretPos.x;

	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );
}

//	From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void CEditDoc::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( m_pcEditWnd->m_hWnd );

	//	�ŉ��ʃr�b�g��IME���g��On/Off����
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( m_pcEditWnd->m_hWnd, hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$x�̓W�J

	���ꕶ���͈ȉ��̒ʂ�
	@li $  $���g
	@li A  �A�v����
	@li F  �J���Ă���t�@�C���̃t���p�X�B���O���Ȃ����(����)�B
	@li f  �J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
	@li g  �J���Ă���t�@�C���̖��O�i�g���q�����j
	@li /  �J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
	@li N  �J���Ă���t�@�C���̖��O(�ȈՕ\��)
	@li n  ����̒ʂ��ԍ�
	@li E  �J���Ă���t�@�C���̂���t�H���_�̖��O(�ȈՕ\��)
	@li e  �J���Ă���t�@�C���̂���t�H���_�̖��O
	@li B  �^�C�v�ʐݒ�̖��O
	@li b  �J���Ă���t�@�C���̊g���q
	@li Q  ����y�[�W�ݒ�̖��O
	@li C  ���ݑI�𒆂̃e�L�X�g
	@li x  ���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
	@li y  ���݂̕����s�ʒu(1�J�n)
	@li d  ���݂̓��t(���ʐݒ�̓��t����)
	@li t  ���݂̎���(���ʐݒ�̎�������)
	@li p  ���݂̃y�[�W
	@li P  ���y�[�W
	@li D  �t�@�C���̃^�C���X�^���v(���ʐݒ�̓��t����)
	@li T  �t�@�C���̃^�C���X�^���v(���ʐݒ�̎�������)
	@li V  �G�f�B�^�̃o�[�W����������
	@li h  Grep�����L�[�̐擪32byte
	@li S  �T�N���G�f�B�^�̃t���p�X
	@li I  ini�t�@�C���̃t���p�X
	@li M  ���ݎ��s���Ă���}�N���t�@�C���p�X

	@date 2003.04.03 genta strncpy_ex�����ɂ��for���̍팸
	@date 2005.09.15 FILE ���ꕶ��S, M�ǉ�
	@date 2007.09.21 kobake ���ꕶ��A(�A�v����)��ǉ�
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
	@date 2012.10.11 Moca ���ꕶ��n�ǉ�
*/
void CEditDoc::ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen)
{
	// Apr. 03, 2003 genta �Œ蕶������܂Ƃ߂�
	static const char PRINT_PREVIEW_ONLY[] = "(����v���r���[�ł̂ݎg�p�ł��܂�)";
	const int PRINT_PREVIEW_ONLY_LEN = sizeof( PRINT_PREVIEW_ONLY ) - 1;
	static const char NO_TITLE[] = "(����)";
	const int NO_TITLE_LEN = sizeof( NO_TITLE ) - 1;
	static const char NOT_SAVED[] = "(�ۑ�����Ă��܂���)";
	const int NOT_SAVED_LEN = sizeof( NOT_SAVED ) - 1;

	const char *p, *r;	//	p�F�ړI�̃o�b�t�@�Br�F��Ɨp�̃|�C���^�B
	char *q, *q_max;

	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case '$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case 'A':	//�A�v����
			q = strncpy_ex( q, q_max - q, GSTR_APPNAME, _tcslen(GSTR_APPNAME) );
			++p;
			break;
		case 'F':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�j
			if ( !IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFilePath();
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'f':	//	�J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
			// Oct. 28, 2001 genta
			//	�t�@�C�����݂̂�n���o�[�W����
			//	�|�C���^�𖖔���
			if ( ! IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFileName(); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				//	����\\�������ɂ����Ă����̌��ɂ�\0������̂ŃA�N�Z�X�ᔽ�ɂ͂Ȃ�Ȃ��B
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'g':	//	�J���Ă���t�@�C���̖��O�i�g���q�������t�@�C�����̂݁j
			//	From Here Sep. 16, 2002 genta
			if ( ! IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�|�C���^�𖖔���
				const char *dot_position, *end_of_path;
				r = GetFileName(); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				end_of_path = dot_position =
					r + strlen( r );
				//	��납��.��T��
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	r�Ɠ����ꏊ�܂ōs���Ă��܂�����.����������
				if( dot_position == r )
					dot_position = end_of_path;

				q = strncpy_ex( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case '/':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
			// Oct. 28, 2001 genta
			if ( !IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�p�X�̋�؂�Ƃ���'/'���g���o�[�W����
				for( r = GetFilePath(); *r != '\0' && q < q_max; ++r, ++q ){
					if( *r == '\\' )
						*q = '/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case 'N':	//	�J���Ă���t�@�C���̖��O(�ȈՕ\��)
			if( !IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				TCHAR szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( GetFilePath(), szText, 1023 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		case 'n':
			if( !IsValidPath() ){
				if( m_bGrepMode ){
				}else if( m_bDebugMode ){
				}else{
					TCHAR szText[10];
					const EditNode* node = CShareData::getInstance()->GetEditNode( m_pcEditWnd->m_hWnd );
					if( 0 < node->m_nId ){
						_stprintf( szText, _T("%d"), node->m_nId );
						q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					}
				}
			}
			++p;
			break;
		case 'E':	// �J���Ă���t�@�C���̂���t�H���_�̖��O(�ȈՕ\��)	2012/12/2 Uchi
			if( !IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
			}
			else {
				TCHAR	buff[_MAX_PATH];		// \�̏����������TCHAR
				TCHAR*	pEnd;
				TCHAR*	p;

				_tcscpy( buff, GetFilePath() );
				pEnd = NULL;
				for ( p = buff; *p != '\0'; p++) {
					if (*p == '\\') {
						pEnd = p;
					}
				}
				if (pEnd != NULL) {
					// �Ō��\�̌�ŏI�[
					*(pEnd+1) = '\0';
				}

				// �ȈՕ\���ɕϊ�
				TCHAR szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( buff, szText, _countof(szText)-1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
			}
			++p;
			break;
		case 'e':	// �J���Ă���t�@�C���̂���t�H���_�̖��O		2012/12/2 Uchi
			if( !IsValidPath() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
			}
			else {
				const TCHAR*	pStr;
				const TCHAR*	pEnd;
				const TCHAR*	p;

				pStr = GetFilePath();
				pEnd = pStr - strlen(pStr) - 1;
				for ( p = pStr; *p != '\0'; p++) {
					if (*p == '\\') {
						pEnd = p;
					}
				}
				q = strncpy_ex( q, q_max - q, pStr, _tcslen(pStr) );
			}
			++p;
			break;
		//	From Here Jan. 15, 2002 hor
		case 'B':	// �^�C�v�ʐݒ�̖��O			2013/03/28 Uchi
			{
				const STypeConfig&	sTypeCongig = GetDocumentAttribute();
				if (sTypeCongig.m_nIdx > 0) {	// ��{�͕\�����Ȃ�
					q = strncpy_ex( q, q_max - q, sTypeCongig.m_szTypeName, strlen(sTypeCongig.m_szTypeName) );
				}
				++p;
			}
			break;
		case 'b':	// �J���Ă���t�@�C���̊g���q	2013/03/28 Uchi
			if ( IsValidPath() ){
				//	�|�C���^�𖖔���
				const TCHAR	*dot_position, *end_of_path;
				r = GetFileName();
				end_of_path = dot_position = r + strlen( r );
				//	��납��.��T��
				while (--dot_position >= r && *dot_position != '.')
					;
				//	.�𔭌�(�g���q�L��)
				if (*dot_position == '.') {
					q = strncpy_ex( q, q_max - q, dot_position +1, end_of_path - dot_position -1 );
				}
			}
			++p;
			break;
		case 'Q':	// ����y�[�W�ݒ�̖��O			2013/03/28 Uchi
			{
				PRINTSETTING*	ps = &m_pShareData->m_PrintSettingArr[
					 GetDocumentAttribute().m_nCurrentPrintSetting];
				q = strncpy_ex( q, q_max - q, ps->m_szPrintSettingName, strlen(ps->m_szPrintSettingName) );
				++p;
			}
			break;
		case 'C':	//	���ݑI�𒆂̃e�L�X�g
			{
				CMemory cmemCurText;
				m_pcEditWnd->GetActiveView().GetCurrentTextForSearch( cmemCurText );

				q = strncpy_ex( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case 'x':	//	���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
			{
				char szText[11];
				_itot( m_pcEditWnd->GetActiveView().m_ptCaretPos_PHY.x + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'y':	//	���݂̕����s�ʒu(1�J�n)
			{
				char szText[11];
				_itot( m_pcEditWnd->GetActiveView().m_ptCaretPos_PHY.y + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case 'd':	//	���ʐݒ�̓��t����
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, _countof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 't':	//	���ʐݒ�̎�������
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'p':	//	���݂̃y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case 'P':	//	���y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case 'D':	//	�^�C���X�^���v
			if (m_FileTime.GetFILETIME().dwLowDateTime){
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetDateFormat(
					m_FileTime.GetSYSTEMTIME(),
					szText,
					_countof( szText ) - 1
				);
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case 'T':	//	�^�C���X�^���v
			if (m_FileTime.GetFILETIME().dwLowDateTime){
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetTimeFormat(
					m_FileTime.GetSYSTEMTIME(),
					szText,
					_countof( szText ) - 1
				);
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case 'V':	// Apr. 4, 2003 genta
			// Version number
			{
				char buf[28]; // 6(�����܂�WORD�̍ő咷) * 4 + 4(�Œ蕔��)
				//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
				int len = sprintf( buf, "%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = strncpy_ex( q, q_max - q, buf, len );
				++p;
			}
			break;
		case 'h':	//	Apr. 4, 2003 genta
			//	Grep Key������ MAX 32����
			//	���g��SetParentCaption()���ڐA
			{
				CMemory		cmemDes;
				LimitStringLengthB( m_szGrepKey, _tcslen( m_szGrepKey ),
					(q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)_tcslen( m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString( "...", 3 );
				}
				q = strncpy_ex( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case 'S':	//	Sep. 15, 2005 FILE
			//	�T�N���G�f�B�^�̃t���p�X
			{
				char	szPath[_MAX_PATH + 1];

				::GetModuleFileName( NULL, szPath, sizeof(szPath) );
				q = strncpy_ex( q, q_max - q, szPath, strlen(szPath) );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	ini�t�@�C���̃t���p�X
			{
				TCHAR	szPath[_MAX_PATH + 1];
				CShareData::getInstance()->GetIniFileName( szPath );
				q = strncpy_ex( q, q_max - q, szPath, strlen(szPath) );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	���ݎ��s���Ă���}�N���t�@�C���p�X�̎擾
			{
				// ���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:���� / STAND_KEYMACRO:�W���}�N��)
				switch( m_pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case TEMP_KEYMACRO:
					{
						const TCHAR* pszMacroFilePath = m_pcSMacroMgr->GetFile(TEMP_KEYMACRO);
						q = strncpy_ex( q, q_max - q, pszMacroFilePath, strlen(pszMacroFilePath) );
					}
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = CShareData::getInstance()->GetShareData()->m_Common.m_sMacro.m_szKeyMacroFileName;
						q = strncpy_ex( q, q_max - q, pszMacroFilePath, strlen(pszMacroFilePath) );
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( m_pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, sizeof(szMacroFilePath) );
						if ( 0 < n ){
							q = strncpy_ex( q, q_max - q, szMacroFilePath, strlen(szMacroFilePath) );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	��������
		//	${cond:string1$:string2$:string3$}
		//	
		case '{':	// ��������
			{
				int cond;
				cond = ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = ExParam_SkipCond( p + 1, cond );
			}
			break;
		case ':':	// ��������̒���
			//	��������̖����܂�SKIP
			p = ExParam_SkipCond( p + 1, -1 );
			break;
		case '}':	// ��������̖���
			//	���ɂ��邱�Ƃ͂Ȃ�
			++p;
			break;
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}



/*! @brief �����̓ǂݔ�΂�

	��������̍\�� ${cond:A0$:A1$:A2$:..$} �ɂ����āC
	�w�肵���ԍ��ɑΉ�����ʒu�̐擪�ւ̃|�C���^��Ԃ��D
	�w��ԍ��ɑΉ����镨���������$}�̎��̃|�C���^��Ԃ��D

	${���o�ꂵ���ꍇ�ɂ̓l�X�g�ƍl����$}�܂œǂݔ�΂��D

	@param pszSource [in] �X�L�������J�n���镶����̐擪�Dcond:�̎��̃A�h���X��n���D
	@param part [in] �ړ�����ԍ����ǂݔ�΂�$:�̐��D-1��^����ƍŌ�܂œǂݔ�΂��D

	@return �ړ���̃|�C���^�D�Y���̈�̐擪�����邢��$}�̒���D

	@author genta
	@date 2003.03.31 genta �쐬
*/
const char* CEditDoc::ExParam_SkipCond(const char* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// ����q�̃��x��
	bool next = true;	// �p���t���O
	const char *p;
	for( p = pszSource; next && *p != '\0'; ++p ) {
		if( *p == '$' && p[1] != '\0' ){ // $�������Ȃ疳��
			switch( *(++p)){
			case '{':	// ����q�̊J�n
				++nest;
				break;
			case '}':
				if( nest == 0 ){
					//	�I���|�C���g�ɒB����
					next = false; 
				}
				else {
					//	�l�X�g���x����������
					--nest;
				}
				break;
			case ':':
				if( nest == 0 && --part == 0){ // ����q�łȂ��ꍇ�̂�
					//	�ړI�̃|�C���g
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief �����̕]��

	@param pCond [in] ������ʐ擪�D'?'�܂ł������ƌ��Ȃ��ĕ]������
	@return �]���̒l

	@note
	�|�C���^�̓ǂݔ�΂���Ƃ͍s��Ȃ��̂ŁC'?'�܂ł̓ǂݔ�΂���
	�Ăяo�����ŕʓr�s���K�v������D

	@author genta
	@date 2003.03.31 genta �쐬

*/
int CEditDoc::ExParam_Evaluate( const char* pCond )
{
	switch( *pCond ){
	case 'R': // $R �ǂݎ���p
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			return 0;
		}
		else if( SHAREMODE_NOT_EXCLUSIVE != m_nFileShareModeOld && /* �t�@�C���̔r�����䃂�[�h */
			INVALID_HANDLE_VALUE == m_hLockedFile		/* ���b�N���Ă��Ȃ� */
		){
			return 1;
		}
		else{
			return 2;
		}
	case 'w': // $w Grep���[�h/Output Mode
		if( m_bGrepMode ){
			return 0;
		}else if( m_bDebugMode ){
			return 1;
		}else {
			return 2;
		}
	case 'M': // $M �L�[�{�[�h�}�N���̋L�^��
		if( m_pShareData->m_sFlags.m_bRecordingKeyMacro && m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == m_pcEditWnd->m_hWnd ){ /* �E�B���h�E */
			return 0;
		}else {
			return 1;
		}
	case 'U': // $U �X�V
		if( IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case 'N': // $N �V�K/(����)		2012/12/2 Uchi
		if (!IsValidPath()) {
			return 0;
		}
		else {
			return 1;
		}
	case 'I': // $I �A�C�R��������Ă��邩
		if( ::IsIconic( m_pcEditWnd->m_hWnd )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		break;
	}
	return 0;
}


/*[EOF]*/
