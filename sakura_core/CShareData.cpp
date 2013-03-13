/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta, MIK
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, ai, Moca, MIK, YAZAKI, hor, KK, aroka
	Copyright (C) 2003, Moca, aroka, MIK, genta, wmlhq, sui
	Copyright (C) 2004, Moca, novice, genta, isearch, MIK
	Copyright (C) 2005, Moca, MIK, genta, ryoji, ���, aroka
	Copyright (C) 2006, aroka, ryoji, genta, fon, rastiv, Moca, maru
	Copyright (C) 2007, ryoji, genta, maru, Moca, nasukoji
	Copyright (C) 2008, ryoji, nasukoji, kobake, novice
	Copyright (C) 2009, nasukoji, ryoji, syat, salarm
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <io.h>
#include "CShareData.h"
#include "CEditApp.h"
#include "mymessage.h"
#include "Debug.h"
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include "my_tchar.h" // 2003/01/06 Moca
#include "charcode.h"  // 2006/06/28 rastiv
#include "CEol.h"  // 2006/06/28 rastiv

struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

//!	���L�������̃o�[�W����
/*!
	���L�������̃o�[�W�����ԍ��B���L�������̌`����ύX�����Ƃ��͂�����1���₷�B

	���̒l�͋��L�������̃o�[�W�����t�B�[���h�Ɋi�[����A�قȂ�\���̋��L��������
	�g���G�f�B�^�������ɋN�����Ȃ��悤�ɂ���B

	�ݒ�\�Ȓl�� 1�`unsinged int�̍ő�l

	@sa Init()

	Version 1�`24:
	���낢��
	
	Version 25:
	m_bStopsBothEndsWhenSearchWord�ǉ�
	
	Version 26:
	MacroRec�ɁAm_bReloadWhenExecute��ǉ� 2002/03/11 YAZAKI
	EditInfo�ɁAm_szDocType�ǉ� Mar. 7, 2002 genta
	
	Version 27:
	STypeConfig�ɁAm_szOutlineRuleFilename��ǉ� 2002.04.01 YAZAKI 
	
	Version 28:
	PRINTSETTING�ɁAm_bPrintKinsokuHead�Am_bPrintKinsokuTail��ǉ� 2002.04.09 MIK
	STypeConfig�ɁAm_bKinsokuHead�Am_bKinsokuTail�Am_szKinsokuHead�Am_szKinsokuTail��ǉ� 2002.04.09 MIK

	Version 29:
	PRINTSETTING�ɁAm_bPrintKinsokuRet��ǉ� 2002.04.13 MIK
	STypeConfig�ɁAm_bKinsokuRet��ǉ� 2002.04.13 MIK

	Version 30:
	PRINTSETTING�ɁAm_bPrintKinsokuKuto��ǉ� 2002.04.17 MIK
	STypeConfig�ɁAm_bKinsokuKuto��ǉ� 2002.04.17 MIK

	Version 31:
	Common�ɁAm_bStopsBothEndsWhenSearchParagraph��ǉ� 2002/04/26 YAZAKI

	Version 32:
	Common����STypeConfig�ցAm_bAutoIndent�Am_bAutoIndent_ZENSPACE���ړ� 2002/04/30 YAZAKI

	Version 33:
	Common�ɁAm_lf_kh��ǉ� 2002/05/21 ai
	m_nDiffFlgOpt��ǉ� 2002.05.27 MIK
	STypeConfig-Color��COLORIDX_DIFF_APPEND,COLORIDX_DIFF_CHANGE,COLORIDX_DIFF_DELETE��ǉ�

	Version 34:
	STypeConfig��m_bUseDocumentIcon �ǉ��D 2002.09.10 genta
	
	Version 35:
	Common��m_nLineNumRightSpace �ǉ��D2002.09.18 genta

	Version 36:
	Common��m_bGrepKanjiCode_AutoDetect���폜�Am_nGrepCharSet��ǉ� 2002/09/21 Moca

	Version 37:
	STypeConfig��LineComment�֘A��m_cLineComment�ɕύX�D  @@@ 2002.09.23 YAZAKI
	STypeConfig��BlockComment�֘A��m_cBlockComment�ɕύX�D@@@ 2002.09.23 YAZAKI

	Version 38:
	STypeConfig��m_bIndentLayout��ǉ�. @@@ 2002.09.29 YAZAKI
	2002.10.01 genta m_nIndentLayout�ɖ��O�ύX

	Version 39:
	Common��m_nFUNCKEYWND_GroupNum��ǉ��D 2002/11/04 Moca

	Version 40:
	�t�@�C�����ȈՕ\�L�֘A��ǉ��D 2002/12/08�`2003/01/15 Moca

	Version 41:
	STypeConfig��m_szTabViewString�T�C�Y�g��
	m_nWindowSizeX/Y m_nWindowOriginX/Y�ǉ� 2003.01.26 aroka

	Version 42:
	STypeConfig�ɓƎ�TAB�}�[�N�t���O�ǉ� 2003.03.28 MIK

	Version 43:
	�ŋߎg�����t�@�C���E�t�H���_�ɂ��C�ɓ����ǉ� 2003.04.08 MIK

	Version 44:
	Window Caption������̈��Common�ɒǉ� 2003.04.05 genta

	Version 45:
	�^�O�t�@�C���쐬�p�R�}���h�I�v�V�����ۑ��̈�(m_nTagsOpt,m_szTagsCmdLine)��ǉ� 2003.05.12 MIK

	Version 46:
	�ҏW�E�C���h�E���C���A�^�u�E�C���h�E�p���ǉ�

	Version 47:
	�t�@�C������̕⊮��STypeConfig�ɒǉ� 2003.06.28 Moca

	Version 48:
	Grep���A���^�C���\���ǉ� 2003.06.28 Moca

	Version 49:
	�t�@�C������IsDebug�ǉ� (�^�u�\���p) 2003.10.13 MIK
	
	Version 50:
	�E�B���h�E�ʒu�Œ�E�p����ǉ� 2004.05.13 Moca

	Version 51:
	�^�O�W�����v�@�\�ǉ� 2004/06/21 novice

	Version 52:
	�O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O�ǉ�	2004.10.03 genta

	Version 53:
	���݂��Ȃ��t�@�C�����J�����Ƃ����ꍇ�Ɍx������t���O�̒ǉ�	2004.10.09 genta

	Version 54:
	�}�E�X�T�C�h�{�^���Ή� 2004/10/10 novice

	Version 55:
	�}�E�X���{�^���Ή� 2004/10/11 novice

	Version 56:
	�C���N�������^���T�[�`(Migemo path�p) 2004/10/13 isearch
	
	Version 57:
	�����L�[���[�h�w��g�� 2005/01/13 MIK
	
	Version 58:
	�����L�[���[�h�Z�b�g�ϒ����蓖�� 2005/01/25 Moca

	Version 59:
	�}�N�����𑝂₵�� 2005/01/30 genta
	
	Version 60:
	�L�[���[�h�w��^�O�W�����v����ۑ� 2005/04/03 MIK

	Version 61:
	���s�ōs���̋󔒂��폜����I�v�V����(�^�C�v�ʐݒ�) 2005/10/11 ryoji

	Version 62:
	�o�b�N�A�b�v�t�H���_ 2005.11.07 aroka

	Version 63:
	�w�茅�c���\���ǉ� 2005.11.08 Moca

	Version 64:
	�^�u�̋@�\�g��(�����A�A�C�R���\��) 2006/01/28 ryoji
	�A�E�g�v�b�g�E�B���h�E�ʒu 2006.02.01 aroka

	Version 65:
	�^�u�ꗗ���\�[�g���� 2006/05/10 ryoji

	Version 66:
	�L�[���[�h�w���v�@�\�g�� 2006.04.10 fon
		�L�[���[�h�w���v�@�\�ݒ��ʃ^�u�Ɉړ�, �����̕������ɑΉ�, �L�����b�g�ʒu�L�[���[�h�����ǉ�

	Version 67:
	�L�����b�g�F�w���ǉ� 2006.12.07 ryoji

	Version 68:
	�t�@�C���_�C�A���O�̃t�B���^�ݒ� 2006.11.16 ryoji

	Version 69:
	�u���ׂĒu���v�͒u���̌J�Ԃ� 2006.11.16 ryoji

	Version 70:
	[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� 2006.12.25 ryoji

	Version 71:
	�^�u����铮��𐧌䂷��I�v�V����2��ǉ� 2007.02.11 genta
		m_bRetainEmptyTab, m_bCloseOneWinInTabMode

	Version 72:
	�^�u�ꗗ���t���p�X�\������ 2007.02.28 ryoji

	Version 73:
	�ҏW�E�B���h�E�ؑ֒� 2007.04.03 ryoji

	Version 74:
	�J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji

	Version 75:
	�}�E�X�z�C�[���ŃE�B���h�E�؂�ւ� 2006.03.26 ryoji

	Version 76:
	�^�u�̃O���[�v�� 2007.06.20 ryoji

	Version 77:
	ini�t�H���_�ݒ� 2007.05.31 ryoji

	Version 78:
	�G�f�B�^�|�g���C�Ԃł�UI���������m�F�̂��߂̃o�[�W�������킹 2007.06.07 ryoji

	Version 79:
	�O���R�}���h���s�̃I�v�V�����g�� 2007.04.29 maru

	Version 80:
	���K�\�����C�u�����̐؂�ւ� 2007.07.22 genta

	Version 81:
	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji

	Version 82:
	���C�����[�h�\��t�����\�ɂ��� 2007.10.08 ryoji

	Version 83:
	�I���Ȃ��ŃR�s�[���\�ɂ��� 2007.11.18 ryoji

	Version 84:
	��ʃL���b�V�����g�� 2007.09.09 Moca

	Version 85:
	�L�[���[�h�Z�b�g�ő�l���� 2007.12.01 genta

	Version 86:
	�^�C�v�ʐݒ�ő�l���� 2007.12.13 ryoji

	Version 87:
	�e�L�X�g�̐܂�Ԃ����@�ǉ� 2008.05.30 nasukoji

	Version 88:
	�z�C�[���X�N���[���𗘗p�����y�[�W�X�N���[���E���X�N���[���Ή� 2009.01.12 nasukoji

	Version 89:
	STypeConfig��m_szTabViewString�T�C�Y�g���iVersion 41�j�̖߂� 2009.02.11 ryoji

	Version 90:
	�������s�}�N�� 2006/08/31 ryoji

	Version 91:
	��Ǔ_�Ԃ牺������ 2009.08.07 ryoji

	Version 92:
	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p) 2009.08.15 nasukoji

	Version 93:
	�u�J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx���v�@�\�ǉ�  2010.03.03 Moca Unicode�ł���backport

	Version 94:
	ICON�����ׂẴR�}���h��

	Version 95:
	CBlockComment�ύX
	�����^�u�����SSearchOption�ɂ܂Ƃ߂�
	�O������N�����ɐV�����E�C���h�E���J��

	Version 96:
	KEYDATA�̃T�C�Y�ύX 2012.11.04 aroka
	�f�t�H���g�̕����R�[�h/�f�t�H���g���s�R�[�h/�f�t�H���gBOM

	Version 97:
	���s�R�[�h��ϊ����ē\��t����

	Version 98-105:
	CommonSetting�\���̐����J�n

	Version 106:
	MRU�Ƀ^�C�v�ʐݒ�ǉ�

	Version 107:
	�E�B���h�E�Ƀ��j���[�������΍��ݒ�ǉ�

	Version 108:
	��DPI�p�Ƀt�H���g�T�C�Y�i1/10�|�C���g�P�ʁj��ǉ�

	Version 109:
	�^�u�o�[�t�H���g�w��

	Version 110:
	�t�@�C���_�C�A���O�̏����ʒu
*/

extern const unsigned int uShareDataVersion;
const unsigned int uShareDataVersion = 110;

/*
||	Singleton��
*/
CShareData* CShareData::_instance = NULL;

// GetOpenedWindowArr�p�ÓI�ϐ��^�\����
static BOOL s_bSort;	// �\�[�g�w��
static BOOL s_bGSort;	// �O���[�v�w��

/*! @brief CShareData::m_pEditArr�ی�pMutex

	�����̃G�f�B�^���񓯊��Ɉ�ē��삵�Ă���Ƃ��ł��ACShareData::m_pEditArr��
	���S�ɑ���ł���悤���쒆��Mutex��Lock()����B

	@par�i�񓯊���ē���̗�j
		�����̃E�B���h�E��\�����Ă��ăO���[�v����L���ɂ����^�X�N�o�[�Łu�O���[�v�����v����������Ƃ�

	@par�i�ی삷��ӏ��̗�j
		CShareData::AddEditWndList(): �G���g���̒ǉ��^���ёւ�
		CShareData::DeleteEditWndList(): �G���g���̍폜
		CShareData::GetOpenedWindowArr(): �z��̃R�s�[�쐬

	����ɂǂ��ɂł������ƃf�b�h���b�N����댯������̂œ����Ƃ��͐T�d�ɁB
	�iLock()���Ԓ���SendMessage()�Ȃǂő��E�B���h�E�̑��������Ɗ댯����j
	CShareData::m_pEditArr�𒼐ڎQ�Ƃ�����ύX����悤�ȉӏ��ɂ͐��ݓI�Ȋ댯�����邪�A
	�Θb�^�ŏ������삵�Ă���͈͂ł���΂܂����͋N���Ȃ��B

	@date 2007.07.05 ryoji �V�K����
	@date 2007.07.07 genta CShareData�̃����o�ֈړ�
*/
static CMutex g_cEditArrMutex( FALSE, GSTR_MUTEX_SAKURA_EDITARR );

// GetOpenedWindowArr�p�\�[�g�֐�
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// �قȂ�O���[�v�̂Ƃ��̓O���[�v��r����
	int nGroup1;
	int nGroup2;

	if( s_bGSort )
	{
		// �I���W�i���̃O���[�v�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}
	else
	{
		// �O���[�v��MRU�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if( nGroup1 != nGroup2 )
	{
		return nGroup1 - nGroup2;	// �O���[�v��r
	}

	// �O���[�v��r���s���Ȃ������Ƃ��̓E�B���h�E��r����
	if( s_bSort )
		return ( ((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex );	// �E�B���h�E�ԍ���r
	return ( ((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p );	// �E�B���h�EMRU��r�i�\�[�g���Ȃ��j
}



//	CShareData_new2.cpp�Ɠ���
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ�
CShareData::CShareData()
{
	m_hFileMap   = NULL;
	m_pShareData = NULL;
	m_nTransformFileNameCount = -1;
}

/*!
	���L�������̈悪����ꍇ�̓v���Z�X�̃A�h���X��Ԃ���
	���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v����B
*/
CShareData::~CShareData()
{
	if( m_pShareData ){
		/* �v���Z�X�̃A�h���X��Ԃ��� ���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v���܂� */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
}


//! CShareData�N���X�̏���������
/*!
	CShareData�N���X�𗘗p����O�ɕK���Ăяo�����ƁB

	@retval true ����������
	@retval false ���������s

	@note ���ɑ��݂��鋤�L�������̃o�[�W���������̃G�f�B�^���g�����̂�
	�قȂ�ꍇ�͒v���I�G���[��h�����߂�false��Ԃ��܂��BCProcess::Initialize()
	��Init()�Ɏ��s����ƃ��b�Z�[�W���o���ăG�f�B�^�̋N���𒆎~���܂��B
*/
bool CShareData::InitShareData()
{
	MY_RUNNINGTIMER(cRunningTimer,"CShareData::InitShareData" );

	if (CShareData::_instance == NULL)	//	Singleton��
		CShareData::_instance = this;

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	int		i;
	int		j;

	/* �t�@�C���}�b�s���O�I�u�W�F�N�g */
	m_hFileMap = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,	//	Sep. 6, 2003 wmlhq
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		GSTR_SHAREDATA
	);
	if( NULL == m_hFileMap ){
		::MessageBox(
			NULL,
			_T("CreateFileMapping()�Ɏ��s���܂���"),
			_T("�\�����ʃG���["),
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}
	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* �I�u�W�F�N�g�����݂��Ă��Ȃ������ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);

		// 2007.05.19 ryoji ���s�t�@�C���t�H���_->�ݒ�t�@�C���t�H���_�ɕύX
		TCHAR	szIniFolder[_MAX_PATH];
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = false;
		GetInidir( szIniFolder );
		AddLastChar( szIniFolder, _MAX_PATH, _T('\\') );

		m_pShareData->m_vStructureVersion = uShareDataVersion;
		_tcscpy(m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName, _T(""));	/* �L�[���[�h�}�N���̃t�@�C���� */ //@@@ 2002.1.24 YAZAKI
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;		/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

		// 2004.05.13 Moca ���\�[�X���琻�i�o�[�W�����̎擾
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&m_pShareData->m_sVersion.m_dwProductVersionMS, &m_pShareData->m_sVersion.m_dwProductVersionLS );
		m_pShareData->m_sHandles.m_hwndTray = NULL;
		m_pShareData->m_sHandles.m_hAccel = NULL;
		m_pShareData->m_sHandles.m_hwndDebug = NULL;
		m_pShareData->m_sNodes.m_nSequences = 0;					/* �E�B���h�E�A�� */
		m_pShareData->m_sNodes.m_nGroupSequences = 0;			/* �^�u�O���[�v�A�� */	// 2007.06.20 ryoji
		m_pShareData->m_sNodes.m_nEditArrNum = 0;

		m_pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji

		m_pShareData->m_Common.m_sGeneral.m_nMRUArrNum_MAX = 15;	/* �t�@�C���̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
		CMRUFile cMRU;
		cMRU.ClearAll();
		m_pShareData->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 15;	/* �t�H���_�̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
		CMRUFolder cMRUFolder;
		cMRUFolder.ClearAll();

		m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum = 0;
		for( i = 0; i < MAX_SEARCHKEY; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum = 0;
		for( i = 0; i < MAX_REPLACEKEY; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum = 0;
		for( i = 0; i < MAX_GREPFILE; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum = 1;
		_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[0], _T("*.*") );

		m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum = 0;
		for( i = 0; i < MAX_GREPFOLDER; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[i], _T("") );
		}
		_tcscpy( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, szIniFolder );	/* �}�N���p�t�H���_ */
		_tcscpy( m_pShareData->m_sHistory.m_szIMPORTFOLDER, szIniFolder );	/* �ݒ�C���|�[�g�p�t�H���_ */

		for( i = 0; i < MAX_TRANSFORM_FILENAME; ++i ){
			_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i], _T("") );
			_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i], _T("") );
		}
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[0], _T("%DeskTop%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[0],   _T("�f�X�N�g�b�v\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[1], _T("%Personal%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[1],   _T("�}�C�h�L�������g\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[2],   _T("IE�L���b�V��\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[3], _T("%TEMP%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[3],   _T("TEMP\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[4],   _T("���L�f�X�N�g�b�v\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[5], _T("%Common Documents%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[5],   _T("���L�h�L�������g\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[6], _T("%AppData%\\") );	// 2007.05.19 ryoji �ǉ�
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[6],   _T("�A�v���f�[�^\\") );	// 2007.05.19 ryoji �ǉ�
		m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum = 7;
		
		/* m_PrintSettingArr[0]��ݒ肵�āA�c���1�`7�ɃR�s�[����B
			�K�v�ɂȂ�܂Œx�点�邽�߂ɁACPrint�ɁACShareData�𑀍삷�錠����^����B
			YAZAKI.
		*/
		{
			/*
				2006.08.16 Moca �������P�ʂ� PRINTSETTING�ɕύX�BCShareData�ɂ͈ˑ����Ȃ��B
			*/
			TCHAR szSettingName[64];
			i = 0;
			wsprintf( szSettingName, _T("����ݒ� %d"), i + 1 );
			CPrint::SettingInitialize( m_pShareData->m_PrintSettingArr[0], szSettingName );	//	���������߁B
		}
		for( i = 1; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
			wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, _T("����ݒ� %d"), i + 1 );	/* ����ݒ�̖��O */
		}

		//	Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
		//	2007.11.04 genta �߂�l�`�F�b�N�Dfalse�Ȃ�N�����f�D
		if( ! InitKeyAssign( m_pShareData )){
			return false;
		}

//	From Here Sept. 19, 2000 JEPRO �R�����g�A�E�g�ɂȂ��Ă������߂̃u���b�N�𕜊������̉����R�����g�A�E�g
//	MS �S�V�b�N�W���X�^�C��10pt�ɐݒ�
//		/* LOGFONT�̏����� */
		memset( &m_pShareData->m_Common.m_sView.m_lf, 0, sizeof( m_pShareData->m_Common.m_sView.m_lf ) );
		m_pShareData->m_Common.m_sView.m_lf.lfHeight			= DpiPointsToPixels(-10);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j;
		m_pShareData->m_Common.m_sView.m_lf.lfWidth				= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfEscapement		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfOrientation		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfWeight			= 400;
		m_pShareData->m_Common.m_sView.m_lf.lfItalic			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfUnderline			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfStrikeOut			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfCharSet			= 0x80;
		m_pShareData->m_Common.m_sView.m_lf.lfOutPrecision		= 0x3;
		m_pShareData->m_Common.m_sView.m_lf.lfClipPrecision		= 0x2;
		m_pShareData->m_Common.m_sView.m_lf.lfQuality			= 0x1;
		m_pShareData->m_Common.m_sView.m_lf.lfPitchAndFamily	= 0x31;
		_tcscpy( m_pShareData->m_Common.m_sView.m_lf.lfFaceName, _T("�l�r �S�V�b�N") );
		m_pShareData->m_Common.m_sView.m_nPointSize = 0;	// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj ���Â��o�[�W��������̈ڍs���l�����Ė����l�ŏ�����	// 2009.10.01 ryoji

		// �L�[���[�h�w���v�̃t�H���g ai 02/05/21 Add S
		LOGFONT lfIconTitle;	// �G�N�X�v���[���̃t�@�C�����\���Ɏg�p�����t�H���g
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&lfIconTitle,					// depends on action to be taken
			0										// user profile update flag
		);
		// ai 02/05/21 Add E
		m_pShareData->m_Common.m_sHelper.m_lf_kh = lfIconTitle;
		m_pShareData->m_Common.m_sHelper.m_ps_kh = 0;	// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj ���Â��o�[�W��������̈ڍs���l�����Ė����l�ŏ�����	// 2009.10.01 ryoji

//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;				/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */

//		m_pShareData->m_Common.m_bUseCaretKeyWord = FALSE;		/* �L�����b�g�ʒu�̒P�����������-�@�\OFF */	// 2006.03.24 fon sakura�N������FALSE�Ƃ��A���������Ȃ�


		/* �o�b�N�A�b�v */
		CommonSetting_Backup& sBackup = m_pShareData->m_Common.m_sBackup;
		sBackup.m_bBackUp = false;										/* �o�b�N�A�b�v�̍쐬 */
		sBackup.m_bBackUpDialog = true;									/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		sBackup.m_bBackUpFolder = false;								/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		sBackup.m_szBackUpFolder[0] = _T('\0');							/* �o�b�N�A�b�v���쐬����t�H���_ */
		sBackup.m_nBackUpType = 2;										/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		sBackup.m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;	/* �o�b�N�A�b�v�t�@�C�����F���t */
		sBackup.m_nBackUpType_Opt2 = ('b' << 16 ) + 10;					/* �o�b�N�A�b�v�t�@�C�����F�A�Ԃ̐��Ɛ擪���� */
		sBackup.m_nBackUpType_Opt3 = 5;									/* �o�b�N�A�b�v�t�@�C�����FOption3 */
		sBackup.m_nBackUpType_Opt4 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption4 */
		sBackup.m_nBackUpType_Opt5 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption5 */
		sBackup.m_nBackUpType_Opt6 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption6 */
		sBackup.m_bBackUpDustBox = false;								/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
		sBackup.m_bBackUpPathAdvanced = false;							/* 20051107 aroka �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� */
		sBackup.m_szBackUpPathAdvanced[0] = _T('\0');					/* 20051107 aroka �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� */

		m_pShareData->m_Common.m_sGeneral.m_nCaretType = 0;					/* �J�[�\���̃^�C�v 0=win 1=dos */
		m_pShareData->m_Common.m_sGeneral.m_bIsINSMode = true;				/* �}���^�㏑�����[�h */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;		/* �t���[�J�[�\�����[�h�� */	//Oct. 29, 2000 JEPRO �u�Ȃ��v�ɕύX

		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord = FALSE;	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */
		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph = FALSE;	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */

		m_pShareData->m_Common.m_sSearch.m_sSearchOption.Reset();			// �����I�v�V����
		m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = 0;				/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji
		m_pShareData->m_Common.m_sSearch.m_bSelectedArea = FALSE;			/* �I��͈͓��u�� */
		m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] = _T('\0');			/* �O���w���v�P */
		m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] = _T('\0');		/* �O��HTML�w���v */
		
		m_pShareData->m_Common.m_sHelper.m_szMigemoDll[0] = _T('\0');		/* migemo dll */
		m_pShareData->m_Common.m_sHelper.m_szMigemoDict[0] = _T('\0');		/* migemo dict */

		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;			/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */

		m_pShareData->m_Common.m_sGeneral.m_bCloseAllConfirm = FALSE;		/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
		m_pShareData->m_Common.m_sGeneral.m_bExitConfirm = FALSE;			/* �I�����̊m�F������ */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum = 3;		/* �L�[���s�[�g���̃X�N���[���s�� */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = FALSE;	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
		m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel = 0;			/* �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���Ńy�[�W�X�N���[������ */	// 2009.01.12 nasukoji
		m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel = 0;	/* �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������ */		// 2009.01.12 nasukoji

		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy = false;			/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
		m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = TRUE;			/* Grep: �T�u�t�H���_������ */
		m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = TRUE;			/* Grep: �s���o�͂��邩�Y�����������o�͂��邩 */
		m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = 1;			/* Grep: �o�͌`�� */
		m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder=FALSE;		/* Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ��� */
		m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = CODE_AUTODETECT;	/* Grep: �����R�[�h�Z�b�g */
		m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView = FALSE;		/* 2003.06.28 Moca Grep���ʂ̃��A���^�C���\�� */
		m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch = TRUE;		/* 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� */
		m_pShareData->m_Common.m_sSearch.m_szRegexpLib[0] = _T('\0');		/* 2007.08.12 genta ���K�\��DLL */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_RETURN = TRUE;				/* �G���^�[�L�[�Ń^�O�W�����v */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK = TRUE;			/* �_�u���N���b�N�Ń^�O�W�����v */

//�L�[���[�h�F�c�[���o�[����
		//	Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
		InitToolButtons( m_pShareData );

		m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR = TRUE;				/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR = TRUE;			/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND = FALSE;			/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place = 1;			/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 4;		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

		m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd = FALSE;				//�^�u�E�C���h�E�\��	//@@@ 2003.05.31 MIK
		m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin = FALSE;		//�^�u�E�C���h�E�\��	//@@@ 2003.05.31 MIK
		_tcscpy(	//@@@ 2003.06.13 MIK
			m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption,
			_T("${w?�yGrep�z$h$:�y�A�E�g�v�b�g�z$:$f$}${U?(�X�V)$}${R?(�ǂݎ���p)$:(�㏑���֎~)$}${M?�y�L�[�}�N���̋L�^���z$}")
		);
		m_pShareData->m_Common.m_sTabBar.m_bSameTabWidth = FALSE;			//�^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon = FALSE;			//�^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bSortTabList = TRUE;				//�^�u�ꗗ���\�[�g����		//@@@ 2006.05.10 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin = TRUE;		// �Ō�̃t�@�C��������ꂽ�Ƃ�(����)���c��	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin = FALSE;		// �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull = FALSE;			//�^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel = FALSE;			//�}�E�X�z�C�[���ŃE�B���h�E�ؑ�	//@@@ 2006.03.26 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bNewWindow = FALSE;				// �O������N������Ƃ��͐V�����E�C���h�E�ŊJ��

		m_pShareData->m_Common.m_sTabBar.m_tabFont = lfIconTitle;
		m_pShareData->m_Common.m_sTabBar.m_tabFontPs = 0;

		m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll = TRUE;		// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�
		m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll = TRUE;		// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�

		/* �J�X�^�����j���[��� */
		wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], _T("�E�N���b�N���j���["), i );
		for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i], _T("���j���[%d"), i );
			m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] = 0;
			for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[i][j] = 0;
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}
		wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND], _T("�^�u���j���[") );	//@@@ 2003.06.13 MIK


		/* ���o���L�� */
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szMidashiKigou, _T("�P�Q�R�S�T�U�V�W�X�O�i(�m[�u�w�y�������������������������E��������@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]���O�l�ܘZ������\���Q��") );
		/* ���p�� */
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, _T("> ") );	/* ���p�� */
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;					/* ���͕⊮�@�\���g�p���� */

		// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜
		//	2004.05.13 Moca �E�B���h�E�T�C�Y�Œ�w��ǉ��ɔ����w����@�ύX
		m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize = WINSIZEMODE_SAVE;	// �E�B���h�E�T�C�Y�p��
		m_pShareData->m_Common.m_sWindow.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCY = 0;
		
		//	2004.05.13 Moca �E�B���h�E�ʒu
		m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos = WINSIZEMODE_DEF;	// �E�B���h�E�ʒu�Œ�E�p��
		m_pShareData->m_Common.m_sWindow.m_nWinPosX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinPosY = 0;

		m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����g�� */
#ifdef _DEBUG
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = FALSE;				/* �^�X�N�g���C�̃A�C�R�����풓 */
#else
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����풓 */
#endif
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = _T('Z');		/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop = TRUE;			/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource = TRUE;			/* OLE�ɂ��h���b�O���ɂ��邩 */
		m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog = FALSE;		/* �I���_�C�A���O��\������ */
		m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL = TRUE;			/* URL���N���b�N���ꂽ��I�����邩 */
		m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm = FALSE;		/* Grep���[�h�ŕۑ��m�F���邩 */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;							/* ���[���[�\�� */
		m_pShareData->m_Common.m_sWindow.m_nRulerHeight = 13;				/* ���[���[�̍��� */
		m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace = 0;			/* ���[���[�ƃe�L�X�g�̌��� */
		m_pShareData->m_Common.m_sWindow.m_nRulerType = 0;					/* ���[���[�̃^�C�v */
		//	Sep. 18, 2002 genta
		m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace = 0;			/* �s�ԍ��̉E�̌��� */
		m_pShareData->m_Common.m_sWindow.m_nVertLineOffset = -1;			// 2005.11.10 Moca �w�茅�c��
		m_pShareData->m_Common.m_sWindow.m_bUseCompatibleBMP = FALSE;		// 2007.09.09 Moca ��ʃL���b�V�����g��
		m_pShareData->m_Common.m_sEdit.m_bCopyAndDisablSelection = FALSE;	/* �R�s�[������I������ */
		m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy = TRUE;		/* �I���Ȃ��ŃR�s�[���\�ɂ��� */	// 2007.11.18 ryoji
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste = true;		/* ���C�����[�h�\��t�����\�ɂ��� */	// 2007.10.08 ryoji
		m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle = TRUE;		/* HtmlHelp�r���[�A�͂ЂƂ� */
		m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = TRUE;		/* ������r��A���E�ɕ��ׂĕ\�� */
		m_pShareData->m_Common.m_sEdit.m_bConvertEOLPaste = false;			/* ���s�R�[�h��ϊ����ē\��t���� */	// 2009.02.28 salarm

		//[�t�@�C��]�^�u
		m_pShareData->m_Common.m_sFile.m_nFileShareMode = OF_SHARE_DENY_WRITE;// �t�@�C���̔r�����䃂�[�h
		m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp = true;		// �X�V�̊Ď�

		//�t�@�C���̕ۑ�
		m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite = FALSE;// ���ύX�ł��㏑�����邩

		//�t�@�C���I�[�v��
		m_pShareData->m_Common.m_sFile.m_bDropFileAndClose = false;			// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
		m_pShareData->m_Common.m_sFile.m_nDropFileNumMax = 8;				// ��x�Ƀh���b�v�\�ȃt�@�C����
		m_pShareData->m_Common.m_sFile.m_bRestoreCurPosition = true;		//	�J�[�\���ʒu����	//	Oct. 27, 2000 genta
		m_pShareData->m_Common.m_sFile.m_bRestoreBookmarks = true;			// �u�b�N�}�[�N���� //2002.01.16 hor 
		//	Nov. 12, 2000 genta
		m_pShareData->m_Common.m_sFile.m_bAutoMIMEdecode = false;			// �t�@�C���ǂݍ��ݎ���MIME�̃f�R�[�h���s����	//Jul. 13, 2001 JEPRO


		m_pShareData->m_Common.m_sEdit.m_bNotOverWriteCRLF = TRUE;			/* ���s�͏㏑�����Ȃ� */
		::SetRect( &m_pShareData->m_Common.m_sOthers.m_rcOpenDialog, 0, 0, 0, 0 );	/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		m_pShareData->m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_CUR;
		_tcscpy(m_pShareData->m_Common.m_sEdit.m_OpenDialogSelDir, _T("%Personal%\\"));
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = TRUE;			/* �����_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_sSearch.m_bSearchAll		 = FALSE;			/* �����^�u���^�u�b�N�}�[�N  �擪�i�����j����Č��� 2002.01.26 hor */
		m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz = TRUE;				/* �����X�N���[���o�[���g�� */
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = FALSE;		/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */	//Nov. 18, 2000 JEPRO TRUE��FALSE �ɕύX
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = TRUE;		/* �u�� �_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste = TRUE;			/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
		m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation = FALSE;	/* �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji (add by genta) */

		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB	= FALSE;		/* VK_TAB   �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE �⊮����L�[���L��/���� */

		m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable	=	FALSE;	//�A�E�g���C���_�C�A���O�Ńu�b�N�}�[�N�̋�s�𖳎�			2002.02.08 aroka,hor
		m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump	=	FALSE;	//�A�E�g���C���_�C�A���O�ŃW�����v������t�H�[�J�X���ڂ�	2002.02.08 hor

		/*
			�����w��q�̈Ӗ���Windows SDK��GetDateFormat(), GetTimeFormat()���Q�Ƃ̂���
		*/

		m_pShareData->m_Common.m_sFormat.m_nDateFormatType = 0;	//���t�����̃^�C�v
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szDateFormat, _T("yyyy\'�N\'M\'��\'d\'��(\'dddd\')\'") );	//���t����
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType = 0;	//���������̃^�C�v
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szTimeFormat, _T("tthh\'��\'mm\'��\'ss\'�b\'")  );			//��������

		m_pShareData->m_Common.m_sWindow.m_bMenuIcon = TRUE;		/* ���j���[�ɃA�C�R����\������ */
		m_pShareData->m_Common.m_sWindow.m_bMenuWChar = FALSE;		/* ���j���[�̎������΍���s��(Win2K�ȍ~�̂�) */

		//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̎��ɖ₢���킹���s����
		m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange = true;
		//	Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
		m_pShareData->m_Common.m_sFile.m_bAlertIfFileNotExist = false;
		m_pShareData->m_Common.m_sFile.m_bAlertIfLargeFile = false;  // �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
		m_pShareData->m_Common.m_sFile.m_nAlertFileSize = 10;        // �x�����n�߂�t�@�C���T�C�Y�iMB�P�ʁj

		// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
		m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew = true;	// �V�K����ۑ����͑S�t�@�C���\��
		m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile = true;	// �V�K�ȊO����ۑ����͑S�t�@�C���\��

		m_pShareData->m_Common.m_sKeyBind.m_bCreateAccelTblEachWin = FALSE;	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)	// 2009.08.15 nasukoji

		for( i = 0; i < MAX_CMDARR; i++ ){
			/* ������ */
			m_pShareData->m_sHistory.m_szCmdArr[i][0] = '\0';
		}
		m_pShareData->m_sHistory.m_nCmdArrNum = 0;

		InitKeyword( m_pShareData );
		InitTypeConfigs( m_pShareData );
		InitPopupMenu( m_pShareData );

		//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̏����l
		//	Aug. 16, 2003 genta $N(�t�@�C�����ȗ��\��)���f�t�H���g�ɕύX
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive, 
			_T("${w?$h$:�A�E�g�v�b�g$:${I?$f$:$N$}$}${U?(�X�V)$} -")
			_T(" $A $V ${R?(�ǂݎ���p)$:�i�㏑���֎~�j$}${M?  �y�L�[�}�N���̋L�^���z$}") );
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive, 
			_T("${w?$h$:�A�E�g�v�b�g$:$f$}${U?(�X�V)$} -")
			_T(" $A $V ${R?(�ǂݎ���p)$:�i�㏑���֎~�j$}${M?  �y�L�[�}�N���̋L�^���z$}") );

		//	From Here Sep. 14, 2001 genta
		//	Macro�o�^�̏�����
		MacroRec *mptr = m_pShareData->m_Common.m_sMacro.m_MacroTable;
		for( i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
			mptr->m_szName[0] = '\0';
			mptr->m_szFile[0] = '\0';
			mptr->m_bReloadWhenExecute = FALSE;
		}
		//	To Here Sep. 14, 2001 genta
		m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened = -1;	/* �I�[�v���㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
		m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged = -1;	/* �^�C�v�ύX�㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
		m_pShareData->m_Common.m_sMacro.m_nMacroOnSave = -1;	/* �ۑ��O�������s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji

		// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
		m_pShareData->m_sTagJump.m_TagJumpNum = 0;
		// 2004.06.22 Moca �^�O�W�����v�̐擪
		m_pShareData->m_sTagJump.m_TagJumpTop = 0;
		m_pShareData->m_nExecFlgOpt = 1;	/* �O���R�}���h���s�́u�W���o�͂𓾂�v */	// 2006.12.03 maru �I�v�V�����̊g���̂���
		m_pShareData->m_bLineNumIsCRLF = TRUE;	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */

		m_pShareData->m_nDiffFlgOpt = 0;	/* DIFF�����\�� */	//@@@ 2002.05.27 MIK

		m_pShareData->m_nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK
		_tcscpy( m_pShareData->m_szTagsCmdLine, _T("") );	/* CTAGS */	//@@@ 2003.05.12 MIK
		//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��History�ۊ�
		m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum = 0;
		for( i = 0; i < MAX_TAGJUMP_KEYWORD; ++i ){
			_tcscpy( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], _T("") );
		}
		m_pShareData->m_sTagJump.m_bTagJumpICase = FALSE;
		m_pShareData->m_sTagJump.m_bTagJumpAnyWhere = FALSE;
		//To Here 2005.04.03 MIK 

	}else{
		/* �I�u�W�F�N�g�����łɑ��݂���ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		//	From Here Oct. 27, 2000 genta
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ){
			//	���̋��L�f�[�^�̈�͎g���Ȃ��D
			//	�n���h�����������
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}




/* KEYDATA�z��Ƀf�[�^���Z�b�g */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	char*			pszKeyName,
	short			nFuncCode_0,
	short			nFuncCode_1,
	short			nFuncCode_2,
	short			nFuncCode_3,
	short			nFuncCode_4,
	short			nFuncCode_5,
	short			nFuncCode_6,
	short			nFuncCode_7
)
{
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
}

/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeOfExt �ɓn�������D
*/
int CShareData::GetDocumentTypeOfPath( const char* pszFilePath )
{
	char	szExt[_MAX_EXT];

	if( NULL != pszFilePath && 0 < (int)strlen( pszFilePath ) ){
		_splitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == '.' )
			return GetDocumentTypeOfExt( szExt + 1 );
		else
			return GetDocumentTypeOfExt( szExt );
	}
	return 0;
}

/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��,�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
*/
int CShareData::GetDocumentTypeOfExt( const char* pszExt )
{
	const char	pszSeps[] = " ;,";	// separator

	int		i;
	char*	pszToken;
	char	szText[256];

	for( i = 0; i < MAX_TYPES; ++i ){
		strcpy( szText, m_pShareData->m_Types[i].m_szTypeExts );
		pszToken = strtok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == my_stricmp( pszExt, pszToken ) ){
				return i;	//	�ԍ�
			}
			pszToken = strtok( NULL, pszSeps );
		}
	}
	return 0;	//	�n�Y��
}


/** �ҏW�E�B���h�E���X�g�ւ̓o�^

	@param hWnd [in] �o�^����ҏW�E�B���h�E�̃n���h��
	@param nGroup [in] �V�K�o�^�̏ꍇ�̃O���[�vID

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji �V�K�E�B���h�E�ɂ̓O���[�vID��t�^����
*/
BOOL CShareData::AddEditWndList( HWND hWnd, int nGroup )
{
	int		nSubCommand = TWNT_ADD;
	int		nIndex;
	CRecent	cRecentEditNode;
	EditNode	sMyEditNode;
	EditNode	*p;

	memset( &sMyEditNode, 0, sizeof( sMyEditNode ) );
	sMyEditNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock�̈�
		LockGuard<CMutex> guard( g_cEditArrMutex );

		cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );

		//�o�^�ς݂��H
		nIndex = cRecentEditNode.FindItem( (const char*)&hWnd );
		if( -1 != nIndex )
		{
			//��������ȏ�o�^�ł��Ȃ����H
			if( cRecentEditNode.GetItemCount() >= cRecentEditNode.GetArrayCount() )
			{
				cRecentEditNode.Terminate();
				return FALSE;
			}
			nSubCommand = TWNT_ORDER;

			//�ȑO�̏����R�s�[����B
			p = (EditNode*)cRecentEditNode.GetItem( nIndex );
			if( p )
			{
				memcpy( &sMyEditNode, p, sizeof( sMyEditNode ) );
			}
		}

		/* �E�B���h�E�A�� */

		if( 0 == ::GetWindowLongPtr( hWnd, sizeof(LONG_PTR) ) )
		{
			m_pShareData->m_sNodes.m_nSequences++;
			::SetWindowLongPtr( hWnd, sizeof(LONG_PTR) , (LONG_PTR)m_pShareData->m_sNodes.m_nSequences );

			//�A�Ԃ��X�V����B
			sMyEditNode.m_nIndex = m_pShareData->m_sNodes.m_nSequences;

			/* �^�u�O���[�v�A�� */
			if( nGroup > 0 )
			{
				sMyEditNode.m_nGroup = nGroup;	// �w��̃O���[�v
			}
			else
			{
				p = (EditNode*)cRecentEditNode.GetItem( 0 );
				if( NULL == p )
					sMyEditNode.m_nGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;	// �V�K�O���[�v
				else
					sMyEditNode.m_nGroup = p->m_nGroup;	// �ŋ߃A�N�e�B�u�̃O���[�v
			}

			sMyEditNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			sMyEditNode.m_bClosing = FALSE;
		}

		//�ǉ��܂��͐擪�Ɉړ�����B
		cRecentEditNode.AppendItem( (const char*)&sMyEditNode );
		cRecentEditNode.Terminate();
	}	// 2007.07.07 genta Lock�̈�I���

	//�E�C���h�E�o�^���b�Z�[�W���u���[�h�L���X�g����B
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)nSubCommand, (LPARAM)hWnd, hWnd, GetGroupId( hWnd ) );

	return TRUE;
}


/** �ҏW�E�B���h�E���X�g����̍폜

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.07.05 ryoji mutex�ŕی�
*/
void CShareData::DeleteEditWndList( HWND hWnd )
{
	int nGroup = GetGroupId( hWnd );

	//�E�C���h�E�����X�g����폜����B
	{	// 2007.07.07 genta Lock�̈�
		LockGuard<CMutex> guard( g_cEditArrMutex );

		CRecent	cRecentEditNode;
		cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
		cRecentEditNode.DeleteItem( (const char*)&hWnd );
		cRecentEditNode.Terminate();
	}

	//�E�C���h�E�폜���b�Z�[�W���u���[�h�L���X�g����B
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_DEL, (LPARAM)hWnd, hWnd, nGroup );
}

/** �O���[�v��ID���Z�b�g����

	@date 2007.06.20 ryoji
*/
void CShareData::ResetGroupId( void )
{
	int nGroup;
	int	i;

	nGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
		{
			m_pShareData->m_sNodes.m_pEditArr[i].m_nGroup = nGroup;
		}
	}
}

/** �ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji

	@warning ���̊֐���m_pEditArr���̗v�f�ւ̃|�C���^��Ԃ��D
	m_pEditArr���ύX���ꂽ��ł̓A�N�Z�X���Ȃ��悤���ӂ��K�v�D
*/
EditNode* CShareData::GetEditNode( HWND hWnd )
{
	int	i;

	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( hWnd == m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd )
		{
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
				return &m_pShareData->m_sNodes.m_pEditArr[i];
		}
	}

	return NULL;
}

/** �O���[�vID���擾����

	@date 2007.06.20 ryoji
*/
int CShareData::GetGroupId( HWND hWnd )
{
	EditNode* pEditNode;
	pEditNode = GetEditNode( hWnd );
	return (pEditNode != NULL)? pEditNode->m_nGroup: -1;
}

/** ����O���[�v���ǂ����𒲂ׂ�

	@param[in] hWnd1 ��r����E�B���h�E1
	@param[in] hWnd2 ��r����E�B���h�E2
	
	@return 2�̃E�B���h�E������O���[�v�ɑ����Ă����true

	@date 2007.06.20 ryoji
*/
bool CShareData::IsSameGroup( HWND hWnd1, HWND hWnd2 )
{
	int nGroup1;
	int nGroup2;

	if( hWnd1 == hWnd2 )
		return true;

	nGroup1 = GetGroupId( hWnd1 );
	if( nGroup1 < 0 )
		return false;

	nGroup2 = GetGroupId( hWnd2 );
	if( nGroup2 < 0 )
		return false;

	return ( nGroup1 == nGroup2 );
}

/* �󂢂Ă���O���[�v�ԍ����擾���� */
int CShareData::GetFreeGroupId( void )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	return ++pShareData->m_sNodes.m_nGroupSequences;	// �V�K�O���[�v
}

/** �w��ʒu�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetEditNodeAt( int nGroup, int nIndex )
{
	int	i;
	int iIndex;

	iIndex = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( nGroup == 0 || nGroup == m_pShareData->m_sNodes.m_pEditArr[i].m_nGroup )
		{
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
			{
				if( iIndex == nIndex )
					return &m_pShareData->m_sNodes.m_pEditArr[i];
				iIndex++;
			}
		}
	}

	return NULL;
}

/** �擪�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetTopEditNode( HWND hWnd )
{
	int nGroup;

	nGroup = GetGroupId( hWnd );
	return GetEditNodeAt( nGroup, 0 );
}

/** �擪�̕ҏW�E�B���h�E���擾����

	@return �^����ꂽ�G�f�B�^�E�B���h�E�Ɠ���O���[�v�ɑ���
	�擪�E�B���h�E�̃n���h��

	@date 2007.06.20 ryoji
*/
HWND CShareData::GetTopEditWnd( HWND hWnd )
{
	EditNode* p = GetTopEditNode( hWnd );

	return ( p != NULL )? p->m_hWnd: NULL;
}

/* ���L�f�[�^�̃��[�h */
BOOL CShareData::LoadShareData( void )
{
	return ShareData_IO_2( true );
}




/* ���L�f�[�^�̕ۑ� */
void CShareData::SaveShareData( void )
{
	ShareData_IO_2( false );
	return;
}


/** �������̃E�B���h�E�֏I���v�����o��

	@param pWndArr [in] EditNode�̔z��Bm_hWnd��NULL�̗v�f�͏������Ȃ�
	@param nArrCnt [in] pWndArr�̒���
	@param bExit [in] TRUE: �ҏW�̑S�I�� / FALSE: ���ׂĕ���
	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j
	@param bCheckConfirm [in] FALSE:�����E�B���h�E�����Ƃ��̌x�����o���Ȃ� / TRUE:�x�����o���i�ݒ�ɂ��j
	@param hWndFrom [in] �I���v�����̃E�B���h�E�i�x�����b�Z�[�W�̐e�ƂȂ�j

	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2009.07.20 syat �S�ā��������ɕύX�B�����E�B���h�E����鎞�̌x�����b�Z�[�W��ǉ�
*/
BOOL CShareData::RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, int nGroup, BOOL bCheckConfirm, HWND hWndFrom )
{
	int nCloseCount = 0;
	int i;

	/* �N���[�Y�ΏۃE�B���h�E�̐��𒲂ׂ� */
	for( i = 0; i < nArrCnt; i++ ){
		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( pWndArr[i].m_hWnd ){
				nCloseCount++;
			}
		}
	}

	if( bCheckConfirm && GetShareData()->m_Common.m_sGeneral.m_bCloseAllConfirm ){	//[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
		if( 1 < nCloseCount ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				_T("�����ɕ����̕ҏW�p�E�B���h�E����悤�Ƃ��Ă��܂��B��������܂���?")
			) ){
				return FALSE;
			}
		}
	}

	for( i = 0; i < nArrCnt; ++i ){
		/* m_hWnd��NULL��ݒ肵��EditNode�͂Ƃ΂� */
		if( pWndArr[i].m_hWnd == NULL )continue;

		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( pWndArr[i].m_hWnd );
				/* �g���C����G�f�B�^�ւ̏I���v�� */
				if( !::SendMessage( pWndArr[i].m_hWnd, MYWM_CLOSE, bExit, 0 ) ){	// 2007.02.13 ryoji bExit�������p��
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}


/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂ�
	
	�w��̃t�@�C�����J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h����Ԃ�

	@retval	TRUE ���łɊJ���Ă���
	@retval	FALSE �J���Ă��Ȃ�����
*/
BOOL CShareData::IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner )
{
	int			i;
	EditInfo*	pfi;
	*phwndOwner = NULL;

	// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�
	if( 0 ==  GetEditorWindowsNum( 0 ) ){
		return FALSE;
	}
	
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			// ����p�X�̃t�@�C�������ɊJ����Ă��邩
			if( 0 == my_stricmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F

	�������łɊJ���Ă���΃A�N�e�B�u�ɂ��āA�E�B���h�E�̃n���h����Ԃ��B
	����ɁA�����R�[�h���قȂ�Ƃ��̃��[�j���O����������B
	���������ɎU��΂������d�I�[�v���������W��������̂��ړI�B

	@retval	�J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h��

	@note	CEditDoc::FileRead�ɐ旧���Ď��s����邱�Ƃ����邪�A
			CEditDoc::FileRead��������s�����K�v�����邱�Ƃɒ��ӁB
			(�t�H���_�w��̏ꍇ��CEditDoc::FileRead�����ڎ��s�����ꍇ�����邽��)

	@retval	TRUE ���łɊJ���Ă���
	@retval	FALSE �J���Ă��Ȃ�����

	@date 2007.03.12 maru �V�K�쐬
*/
BOOL CShareData::ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, int nCharCode )
{
	if( IsPathOpened( pszPath, phwndOwner ) ){
		EditInfo*		pfi;
		CMRUFile		cMRU;
		::SendMessage( *phwndOwner, MYWM_GETFILEINFO, 0, 0 );
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			char*	pszCodeNameCur = NULL;
			char*	pszCodeNameNew = NULL;
			if( IsValidCodeType(nCharCode) ){
				pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
			}
			if( IsValidCodeType(pfi->m_nCharCode) ){
				pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
			}

			if(NULL != pszCodeNameCur && pszCodeNameNew){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						_T("%s\n\n\n���ɊJ���Ă���t�@�C�����Ⴄ�����R�[�h�ŊJ���ꍇ�́A\n")
						_T("�t�@�C�����j���[����u�J�������v���g�p���Ă��������B\n")
						_T("\n")
						_T("���݂̕����R�[�h�Z�b�g=[%s]\n")
						_T("�V���������R�[�h�Z�b�g=[%s]"),
						pszPath,
						pszCodeNameCur,
						pszCodeNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					_T("%s\n\n���d�I�[�v���̊m�F�ŕs���ȕ����R�[�h���w�肳��܂����B\n")
					_T("\n")
					_T("���݂̕����R�[�h�Z�b�g=%d [%s]\n�V���������R�[�h�Z�b�g=%d [%s]"),
					pszPath,
					pfi->m_nCharCode,
					NULL==pszCodeNameCur?_T("�s��"):pszCodeNameCur,
					nCharCode,
					NULL==pszCodeNameNew?_T("�s��"):pszCodeNameNew
				);
			}
		}

		// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
		ActivateFrameWindow( *phwndOwner );

		// MRU���X�g�ւ̓o�^
		cMRU.Add( pfi );
		return TRUE;
	}
	else {
		return FALSE;
	}

}




/** ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�

	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j
	@param bExcludeClosing [in] �I�����̕ҏW�E�B���h�E�̓J�E���g���Ȃ�

	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.04.19 ryoji bExcludeClosing������ǉ�
*/
int CShareData::GetEditorWindowsNum( int nGroup, bool bExcludeClosing/* = true */ )
{
	int		i;
	int		j;

	j = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			if( nGroup != 0 && nGroup != GetGroupId( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
				continue;
			if( bExcludeClosing && m_pShareData->m_sNodes.m_pEditArr[i].m_bClosing )
				continue;
			j++;
		}
	}
	return j;
}


/** �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����

	@date 2005.01.24 genta hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/*!< �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/*!< ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/*!< ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast,	/*!< �Ō�ɑ��肽���E�B���h�E */
	int			nGroup/* = 0*/	/*!< ���肽���O���[�v(0:�S�O���[�v) */
)
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}

/** �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�

	@date 2005.01.24 genta m_hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/* ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/* ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast,	/* �Ō�ɑ��肽���E�B���h�E */
	int			nGroup		/*!< [in] �O���[�v�w��i0:�S�O���[�v�j */
)
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}

/** ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

	@param[out] ppEditNode �z����󂯎��|�C���^
		�߂�l��0�̏ꍇ��NULL���Ԃ���邪�C��������҂��Ȃ����ƁD
		�܂��C�s�v�ɂȂ�����delete []���Ȃ��Ă͂Ȃ�Ȃ��D
	@param[in] bSort TRUE: �\�[�g���� / FALSE: �\�[�g����
	@param[in]bGSort TRUE: �O���[�v�\�[�g���� / FALSE: �O���[�v�\�[�g����

	���Ƃ̕ҏW�E�B���h�E���X�g�̓\�[�g���Ȃ���΃E�B���h�E��MRU���ɕ���ł���
	-------------------------------------------------
	bSort	bGSort	��������
	-------------------------------------------------
	FALSE	FALSE	�O���[�vMRU���|�E�B���h�EMRU��
	TRUE	FALSE	�O���[�vMRU���|�E�B���h�E�ԍ���
	FALSE	TRUE	�O���[�v�ԍ����|�E�B���h�EMRU��
	TRUE	TRUE	�O���[�v�ԍ����|�E�B���h�E�ԍ���
	-------------------------------------------------

	@return �z��̗v�f����Ԃ�
	@note �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete []���Ă�������

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji bGroup�����ǉ��A�\�[�g���������O�̂��̂���qsort�ɕύX
*/
int CShareData::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	int nRet;

	LockGuard<CMutex> guard( g_cEditArrMutex );
	nRet = _GetOpenedWindowArrCore( ppEditNode, bSort, bGSort );

	return nRet;
}

// GetOpenedWindowArr�֐��R�A������
int CShareData::_GetOpenedWindowArrCore( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	//�ҏW�E�C���h�E�����擾����B
	EditNodeEx* pNode;	// �\�[�g�����p�̊g�����X�g
	int		nRowNum;	//�ҏW�E�C���h�E��
	int		i;

	//�ҏW�E�C���h�E�����擾����B
	*ppEditNode = NULL;
	if( m_pShareData->m_sNodes.m_nEditArrNum <= 0 )
		return 0;

	//�ҏW�E�C���h�E���X�g�i�[�̈���쐬����B
	*ppEditNode = new EditNode[ m_pShareData->m_sNodes.m_nEditArrNum ];
	if( NULL == *ppEditNode )
		return 0;

	// �g�����X�g���쐬����
	pNode = new EditNodeEx[ m_pShareData->m_sNodes.m_nEditArrNum ];
	if( NULL == pNode )
	{
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g�̊e�v�f�ɕҏW�E�B���h�E���X�g�̊e�v�f�ւ̃|�C���^���i�[����
	nRowNum = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[ i ].m_hWnd ) )
		{
			pNode[ nRowNum ].p = &m_pShareData->m_sNodes.m_pEditArr[ i ];	// �|�C���^�i�[
			pNode[ nRowNum ].nGroupMru = -1;	// �O���[�v�P�ʂ�MRU�ԍ�������
			nRowNum++;
		}
	}
	if( nRowNum <= 0 )
	{
		delete []pNode;
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g��ŃO���[�v�P�ʂ�MRU�ԍ�������
	if( !bGSort )
	{
		int iGroupMru = 0;	// �O���[�v�P�ʂ�MRU�ԍ�
		int nGroup = -1;
		for( i = 0; i < nRowNum; i++ )
		{
			if( pNode[ i ].nGroupMru == -1 && nGroup != pNode[ i ].p->m_nGroup )
			{
				nGroup = pNode[ i ].p->m_nGroup;
				iGroupMru++;
				pNode[ i ].nGroupMru = iGroupMru;	// MRU�ԍ��t�^

				// ����O���[�v�̃E�B���h�E�ɓ���MRU�ԍ�������
				int j;
				for( j = i + 1; j < nRowNum; j++ )
				{
					if( pNode[ j ].p->m_nGroup == nGroup )
						pNode[ j ].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// �g�����X�g���\�[�g����
	// Note. �O���[�v���P�����̏ꍇ�͏]���ibGSort ���������j�Ɠ������ʂ�������
	//       �i�O���[�v������ݒ�łȂ���΃O���[�v�͂P�j
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort( pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr );

	// �g�����X�g�̃\�[�g���ʂ����ƂɕҏW�E�C���h�E���X�g�i�[�̈�Ɍ��ʂ��i�[����
	for( i = 0; i < nRowNum; i++ )
	{
		(*ppEditNode)[i] = *pNode[i].p;

		//�C���f�b�N�X��t����B
		//���̃C���f�b�N�X�� m_pEditArr �̔z��ԍ��ł��B
		(*ppEditNode)[i].m_nIndex = pNode[i].p - m_pShareData->m_sNodes.m_pEditArr;	// �|�C���^���Z���z��ԍ�
	}

	delete []pNode;

	return nRowNum;
}

/** �E�B���h�E�̕��ёւ�

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E

	@author ryoji
	@date 2007.07.07 genta �E�B���h�E�z�񑀍암��CTabWnd���ړ�
*/
bool CShareData::ReorderTab( HWND hwndSrc, HWND hwndDst )
{
	EditNode	*p = NULL;
	int			nCount;
	int			i;

	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<CMutex> guard( g_cEditArrMutex );
	nCount = _GetOpenedWindowArrCore( &p, TRUE );	// ���b�N�͎����ł���Ă���̂Œ��ڃR�A���Ăяo��
	for( i = 0; i < nCount; i++ )
	{
		if( hwndSrc == p[i].m_hWnd )
			nSrcTab = i;
		if( hwndDst == p[i].m_hWnd )
			nDstTab = i;
	}

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
	{
		if( p ) delete []p;
		return false;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[ nDstTab ].m_nIndex;
	nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// �^�u���������[�e�[�g
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// �^�u�E�������[�e�[�g
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = nIndex;

	if( p ) delete []p;
	return true;
}

/** �^�u�ړ��ɔ����E�B���h�E����

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E�D�V�K�Ɨ�����NULL�D
	@param[in] bSrcIsTop �ړ�����E�B���h�E�����E�B���h�E�Ȃ�true
	@param[in] notifygroups �^�u�̍X�V���K�v�ȃO���[�v�̃O���[�vID�Dint[2]���Ăяo�����ŗp�ӂ���D

	@return �X�V���ꂽhwndDst (�ړ��悪���ɕ���ꂽ�ꍇ�Ȃǂ�NULL�ɕύX����邱�Ƃ�����)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()���Ɨ�
*/
HWND CShareData::SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] )
{
	LockGuard<CMutex> guard( g_cEditArrMutex );

	EditNode* pSrcEditNode = GetEditNode( hwndSrc );
	EditNode* pDstEditNode = GetEditNode( hwndDst );
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if( pDstEditNode == NULL )
	{
		hwndDst = NULL;
		nDstGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;	// �V�K�O���[�v
	}
	else
	{
		nDstGroup = pDstEditNode->m_nGroup;	// �����O���[�v
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++m_pShareData->m_sNodes.m_nSequences;	// �^�u���т̍Ō�i�N�����̍Ō�j�ɂ����Ă���

	// ��\���̃^�u�������O���[�v�Ɉړ�����Ƃ��͔�\���̂܂܂ɂ���̂�
	// ���������擪�ɂ͂Ȃ�Ȃ��悤�A�K�v�Ȃ�擪�E�B���h�E�ƈʒu����������B
	if( !bSrcIsTop && pDstEditNode != NULL )
	{
		if( pSrcEditNode < pDstEditNode )
		{
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}

/*!
	�A�E�g�v�b�g�E�C���h�E�ɏo��

	�A�E�g�v�b�g�E�C���h�E��������΃I�[�v������
	@param lpFmt [in] �����w�蕶����
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{

	if( NULL == m_pShareData->m_sHandles.m_hwndDebug
	|| !IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// �A�E�g�v�b�g�E�B���h�E���쐬���Ɠ����O���[�v�ɍ쐬���邽�߂� m_hwndTraceOutSource ���g���Ă��܂�
		// �im_hwndTraceOutSource �� CEditWnd::Create() �ŗ\�ߐݒ�j
		// ������ƕs���D�����ǁATraceOut() �̈����ɂ��������N�������w�肷��̂��D�D�D
		CEditApp::OpenNewEditor( NULL, m_hwndTraceOutSource, "-DEBUGMODE", CODE_SJIS, false, true );
		//	2001/06/23 N.Nakatani �����o��܂ŃE�G�C�g��������悤�ɏC��
		//�A�E�g�v�b�g�E�C���h�E���o����܂�5�b���炢�҂B
		//	Jun. 25, 2001 genta OpenNewEditor�̓����@�\�𗘗p����悤�ɕύX

		/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	wvsprintf( m_pShareData->m_sWorkBuffer.m_szWork, lpFmt, argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
	return;
}

/*
	CShareData::CheckMRUandOPENFOLDERList
	MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����

	@note ���݂͎g���Ă��Ȃ��悤���B
	@par History
	2001.12.26 �폜�����B�iYAZAKI�j
	
*/
/*!	idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����D

	@param pszPath [in]	�p�X���̏o�͐�D�����݂̂�m�肽���Ƃ���NULL������D
	@param idx [in]		�}�N���ԍ�
	@param nBufLen [in]	pszPath�Ŏw�肳�ꂽ�o�b�t�@�̃o�b�t�@�T�C�Y

	@retval >0 : �p�X���̒����D
	@retval  0 : �G���[�C���̃}�N���͎g���Ȃ��C�t�@�C�������w�肳��Ă��Ȃ��D
	@retval <0 : �o�b�t�@�s���D�K�v�ȃo�b�t�@�T�C�Y�� -(�߂�l)+1

	@author YAZAKI
	@date 2003.06.08 Moca ���[�J���ϐ��ւ̃|�C���^��Ԃ��Ȃ��悤�Ɏd�l�ύX
	@date 2003.06.14 genta �����񒷁C�|�C���^�̃`�F�b�N��ǉ�
	@date 2003.06.24 Moca idx��-1�̂Ƃ��A�L�[�}�N���̃t���p�X��Ԃ�.
	
	@note idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B(�����Ő������`�F�b�N���s���Ă��Ȃ�)
*/
int CShareData::GetMacroFilename( int idx, TCHAR *pszPath, int nBufLen )
{
	if( -1 != idx && !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return 0;
	TCHAR *ptr;
	TCHAR *pszFile;

	if( -1 == idx ){
		pszFile = _T("RecKey.mac");
	}else{
		pszFile = m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szFile;
	}
	if( pszFile[0] == _T('\0') ){	//	�t�@�C����������
		if( pszPath != NULL ){
			pszPath[0] = _T('\0');
		}
		return 0;
	}
	ptr = pszFile;
	int nLen = _tcslen( ptr ); // Jul. 21, 2003 genta strlen�Ώۂ�����Ă������߃}�N�����s���ł��Ȃ�

	if( !_IS_REL_PATH( pszFile )	// ��΃p�X
		|| m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER[0] == _T('\0') ){	//	�t�H���_�w��Ȃ�
		if( pszPath == NULL || nBufLen <= nLen ){
			return -nLen;
		}
		_tcscpy( pszPath, pszFile );
		return nLen;
	}
	else {	//	�t�H���_�w�肠��
		//	���΃p�X����΃p�X
		int nFolderSep = AddLastChar( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, sizeof(m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER), _T('\\') );
		int nAllLen;
		TCHAR *pszDir;

		 // 2003.06.24 Moca �t�H���_�����΃p�X�Ȃ���s�t�@�C������̃p�X
		// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		if( _IS_REL_PATH( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ) ){
			TCHAR szDir[_MAX_PATH + sizeof( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER )];
			GetInidirOrExedir( szDir, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER );
			pszDir = szDir;
		}else{
			pszDir = m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER;
		}

		int nDirLen = _tcslen( pszDir );
		nAllLen = nDirLen + nLen + ( -1 == nFolderSep ? 1 : 0 );
		if( pszPath == NULL || nBufLen <= nAllLen ){
			return -nAllLen;
		}

		_tcscpy( pszPath, pszDir );
		ptr = pszPath + nDirLen;
		if( -1 == nFolderSep ){
			*ptr++ = _T('\\');
		}
		_tcscpy( ptr, pszFile );
		return nAllLen;
	}

}

/*!	idx�Ŏw�肵���}�N����m_bReloadWhenExecute���擾����B
	idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B
	YAZAKI
*/
bool CShareData::BeReloadWhenExecuteMacro( int idx )
{
	if( !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return false;

	return ( m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_bReloadWhenExecute == TRUE );
}

/*!	m_szSEARCHKEYArr��pszSearchKey��ǉ�����B
	YAZAKI
*/
void CShareData::AddToSearchKeyArr( const char* pszSearchKey )
{
	CRecent	cRecentSearchKey;
	cRecentSearchKey.EasyCreate( RECENT_FOR_SEARCH );
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_szREPLACEKEYArr��pszReplaceKey��ǉ�����
	YAZAKI
*/
void CShareData::AddToReplaceKeyArr( const char* pszReplaceKey )
{
	CRecent	cRecentReplaceKey;
	cRecentReplaceKey.EasyCreate( RECENT_FOR_REPLACE );
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_szGREPFILEArr��pszGrepFile��ǉ�����
	YAZAKI
*/
void CShareData::AddToGrepFileArr( const char* pszGrepFile )
{
	CRecent	cRecentGrepFile;
	cRecentGrepFile.EasyCreate( RECENT_FOR_GREP_FILE );
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_nGREPFOLDERArrNum��pszGrepFolder��ǉ�����
	YAZAKI
*/
void CShareData::AddToGrepFolderArr( const char* pszGrepFolder )
{
	CRecent	cRecentGrepFolder;
	cRecentGrepFolder.EasyCreate( RECENT_FOR_GREP_FOLDER );
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}

/*!	�O��Win�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CShareData::ExtWinHelpIsSet( int nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != '\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (nTypeNo < 0 || MAX_TYPES <= nTypeNo ){
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (m_pShareData->m_Types[nTypeNo].m_szExtHelp[0] != '\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
char* CShareData::GetExtWinHelp( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHelp[0] != '\0'){
		return m_pShareData->m_Types[nTypeNo].m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}
/*!	�O��HTML�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CShareData::ExtHTMLHelpIsSet( int nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != '\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (nTypeNo < 0 || MAX_TYPES <= nTypeNo ){
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
char* CShareData::GetExtHTMLHelp( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}
/*!	�r���[�A�𕡐��N�����Ȃ���ON����Ԃ��B
*/
bool CShareData::HTMLHelpIsSingle( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return (m_pShareData->m_Types[nTypeNo].m_bHtmlHelpIsSingle != FALSE);
	}
	
	return (m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle != FALSE);
}

/*! ���t���t�H�[�}�b�g
	systime�F�����f�[�^
	
	pszDest�F�t�H�[�}�b�g�ς݃e�L�X�g�i�[�p�o�b�t�@
	nDestLen�FpszDest�̒���
	
	pszDateFormat�F
		�J�X�^���̂Ƃ��̃t�H�[�}�b�g
*/
const char* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen )
{
	return MyGetDateFormat( systime, pszDest, nDestLen, m_pShareData->m_Common.m_sFormat.m_nDateFormatType, m_pShareData->m_Common.m_sFormat.m_szDateFormat );
}

const char* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, char* szDateFormat )
{
	const char* pszForm;
	DWORD dwFlags;
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/* �������t�H�[�}�b�g */
const char* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen )
{
	return MyGetTimeFormat( systime, pszDest, nDestLen, m_pShareData->m_Common.m_sFormat.m_nTimeFormatType, m_pShareData->m_Common.m_sFormat.m_szTimeFormat );
}

/* �������t�H�[�}�b�g */
const char* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen, int nTimeFormatType, char* szTimeFormat )
{
	const char* pszForm;
	DWORD dwFlags;
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}

/*!	���L�f�[�^�̐ݒ�ɏ]���ăp�X���k���\�L�ɕϊ�����
	@param pszSrc   [in]  �t�@�C����
	@param pszDest  [out] �ϊ���̃t�@�C�����̊i�[��
	@param nDestLen [in]  �I�[��NULL���܂�pszDest��TCHAR�P�ʂ̒��� _MAX_PATH �܂�
	@date 2003.01.27 Moca �V�K�쐬
	@note �A�����ČĂяo���ꍇ�̂��߁A�W�J�ς݃��^��������L���b�V�����č��������Ă���B
*/
LPTSTR CShareData::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen )
{
	int i;
	TCHAR szBuf[_MAX_PATH + 1];

	if( -1 == m_nTransformFileNameCount ){
		TransformFileName_MakeCache();
	}

	if( 0 < m_nTransformFileNameCount ){
		GetFilePathFormat( pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]] );
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			_tcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
		}
	}else{
		// �ϊ�����K�v���Ȃ� �R�s�[��������
		_tcsncpy( pszDest, pszSrc, nDestLen - 1 );
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}


/*!	�W�J�ς݃��^������̃L���b�V�����쐬�E�X�V����
	@retval �L���ȓW�J�ςݒu���O������̐�
	@date 2003.01.27 Moca �V�K�쐬
	@date 2003.06.23 Moca �֐����ύX
*/
int CShareData::TransformFileName_MakeCache( void ){
	int i;
	int nCount = 0;
	for( i = 0; i < m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		if( '\0' != m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i],
			 m_szTransformFileNameFromExp[nCount], _MAX_PATH ) ){
				// m_szTransformFileNameTo��m_szTransformFileNameFromExp�̔ԍ�������邱�Ƃ�����̂ŋL�^���Ă���
				m_nTransformFileNameOrgId[nCount] = i;
				nCount++;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}


/*!	�t�@�C���E�t�H���_����u�����āA�ȈՕ\�������擾����
	@param pszSrc   [in]  �t�@�C����
	@param pszDest  [out] �ϊ���̃t�@�C�����̊i�[��
	@param nDestLen [in]  �I�[��NULL���܂�pszDest��TCHAR�P�ʂ̒���
	@param pszFrom  [in]  �u���O������
	@param pszTo    [in]  �u���㕶����
	@date 2002.11.27 Moca �V�K�쐬
	@note �召��������ʂ��Ȃ��BnDestLen�ɒB�����Ƃ��͌���؂�̂Ă���
*/
LPCTSTR CShareData::GetFilePathFormat( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo )
{
	int i, j;
	int nSrcLen;
	int nFromLen, nToLen;
	int nCopy;

	nSrcLen  = _tcslen( pszSrc );
	nFromLen = _tcslen( pszFrom );
	nToLen   = _tcslen( pszTo );

	nDestLen--;

	for( i = 0, j = 0; i < nSrcLen && j < nDestLen; i++ ){
#if defined(_MBCS)
		if( 0 == my_strnicmp( &pszSrc[i], pszFrom, nFromLen ) )
#else
		if( 0 == _tcsncicmp( &pszSrc[i], pszFrom, nFromLen ) )
#endif
		{
			nCopy = __min( nToLen, nDestLen - j );
			memcpy( &pszDest[j], pszTo, nCopy * sizeof( TCHAR ) );
			j += nCopy;
			i += nFromLen - 1;
		}else{
#if defined(_MBCS)
// SJIS ��p����
			if( _IS_SJIS_1( (unsigned char)pszSrc[i] ) && i + 1 < nSrcLen && _IS_SJIS_2( (unsigned char)pszSrc[i + 1] ) ){
				if( j + 1 < nDestLen ){
					pszDest[j] = pszSrc[i];
					j++;
					i++;
				}else{
					// SJIS�̐�s�o�C�g�����R�s�[�����̂�h��
					break;// goto end_of_func;
				}
			}
#endif
			pszDest[j] = pszSrc[i];
			j++;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%�Ȃǂ̃p�����[�^�w������ۂ̃p�X���ɕϊ�����

	@param pszSrc  [in]  �ϊ��O������
	@param pszDes  [out] �ϊ��㕶����
	@param nDesLen [in]  pszDes��NULL���܂�TCHAR�P�ʂ̒���
	@retval true  ����ɕϊ��ł���
	@retval false �o�b�t�@������Ȃ������C�܂��̓G���[�BpszDes�͕s��
	@date 2002.11.27 Moca �쐬�J�n
*/
bool CShareData::ExpandMetaToFolder( LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen )
{
	LPCTSTR ps;
	LPTSTR  pd, pd_end;
	LPTSTR  pStr;

#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias{
		LPTSTR szAlias;
		int nLenth;
		LPTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	pd_end = pszDes + ( nDesLen - 1 );
	for( ps = pszSrc, pd = pszDes; '\0' != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = '\0';
			}
			return false;
		}

		if( '%' != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% �� %
		if( '%' == ps[1] ){
			*pd = '%';
			pd++;
			ps++;
			continue;
		}

		if( '\0' != ps[1] ){
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			ps++;
			// %SAKURA%
			if( 0 == my_tcsnicmp( _T("SAKURA%"), ps, 7 ) ){
				// exe�̂���t�H���_
				GetExedir( szPath );
				nMetaLen = 6;
			// %SAKURADATA%	// 2007.06.06 ryoji
			}else if( 0 == my_tcsnicmp( _T("SAKURADATA%"), ps, 11 ) ){
				// ini�̂���t�H���_
				GetInidir( szPath );
				nMetaLen = 10;
			// ���^��������ۂ�
			}else if( NULL != (pStr = _tcschr( (LPTSTR)ps, '%' ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					_tmemcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = '\0';
				}else{
					*pd = '\0';
					return false;
				}
#ifdef _USE_META_ALIAS
				// ���^�����񂪃G�C���A�X���Ȃ珑��������
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // �ǂݔ�΂�
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == my_tcsicmp( pAlias->szAlias, szMeta ) ){
						_tcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// ���ڃ��W�X�g���Œ��ׂ�
				szPath[0] = '\0';
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, sizeof( szPath ) );
				if( false == bFolderPath || '\0' == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, sizeof( szPath ) );
				}
				if( false == bFolderPath || '\0' == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// ���ϐ�
					if( NULL != pStr ){
						nPathLen = _tcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							_tcscpy( szPath, pStr );
						}else{
							*pd = '\0';
							return false;
						}
					}
					// ����`�̃��^������� ���͂��ꂽ%...%���C���̂܂ܕ����Ƃ��ď�������
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = '%';
						_tmemcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = '%';
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = '\0';
						return false;
					}
				}
			}else{
				// %...%�̏I����%���Ȃ� �Ƃ肠�����C%���R�s�[
				*pd = '%';
				pd++;
				ps--; // ���ps++���Ă��܂����̂Ŗ߂�
				continue;
			}

			// �����O�t�@�C�����ɂ���
			nPathLen = _tcslen( szPath );
			pStr = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr = szMeta;
				}
			}

			// �Ō�̃t�H���_��؂�L�����폜����
			// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
			for(nPathLen = 0; pStr[nPathLen] != '\0'; nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr[nPathLen + 1] ) ){
					// SJIS�ǂݔ�΂�
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( '\\' == pStr[nPathLen] && '\0' == pStr[nPathLen + 1] ){
					pStr[nPathLen] = '\0';
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				_tmemcpy( pd, pStr, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = '\0';
				return false;
			}
		}else{
			// �Ō�̕�����%������
			*pd = *ps;
			pd++;
		}
	}
	*pd = '\0';
	return true;
}

static const char* ppszKeyWordsCPP[] = {
	"#define",
	"#elif",
	"#else",
	"#endif",
	"#error",
	"#if",
	"#ifdef",
	"#ifndef",
	"#include",
	"#line",
	"#pragma",
	"#undef",
	"__FILE__",
	"__declspec",
	"asm",
	"auto",
	"bool",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"const_cast",
	"continue",
	"default",
	"define",
	"defined",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"elif",
	"else",
	"endif",
	"enum",
	"error",
	"explicit",
	"export",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"ifdef",
	"ifndef",
	"include",
	"inline",
	"int",
	"line",
	"long",
	"mutable",
	"namespace",
	"new",
	"operator",
	"pragma",
	"private",
	"protected",
	"public",
	"register",
	"reinterpret_cast",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"try",
	"typedef",
	"typeid",
	"typename",
	"undef",
	"union",
	"unsigned",
	"using",
	"virtual",
	"void",
	"volatile",
	"wchar_t",
	"while"
};

static const char* ppszKeyWordsHTML[] = {
	"_blank",
	"_parent",
	"_self",
	"_top",
	"A",
	"ABBR",
	"ABOVE",
	"absbottom",
	"absmiddle",
	"ACCESSKEY",
	"ACRONYM",
	"ACTION",
	"ADDRESS",
	"ALIGN",
	"all",
	"APPLET",
	"AREA",
	"AUTOPLAY",
	"AUTOSTART",
	"B",
	"BACKGROUND",
	"BASE",
	"BASEFONT",
	"baseline",
	"BEHAVIOR",
	"BELOW",
	"BGCOLOR",
	"BGSOUND",
	"BIG",
	"BLINK",
	"BLOCKQUOTE",
	"BODY",
	"BORDER",
	"BORDERCOLOR",
	"BORDERCOLORDARK",
	"BORDERCOLORLIGHT",
	"BOTTOM",
	"box",
	"BR",
	"BUTTON",
	"CAPTION",
	"CELLPADDING",
	"CELLSPACING",
	"CENTER",
	"CHALLENGE",
	"char",
	"checkbox",
	"CHECKED",
	"CITE",
	"CLEAR",
	"CLIP",
	"CODE",
	"CODEBASE",
	"CODETYPE",
	"COL",
	"COLGROUP",
	"COLOR",
	"COLS",
	"COLSPAN",
	"COMMENT",
	"CONTROLS",
	"DATA",
	"DD",
	"DECLARE",
	"DEFER",
	"DEL",
	"DELAY",
	"DFN",
	"DIR",
	"DIRECTION",
	"DISABLED",
	"DIV",
	"DL",
	"DOCTYPE",
	"DT",
	"EM",
	"EMBED",
	"ENCTYPE",
	"FACE",
	"FIELDSET",
	"file",
	"FONT",
	"FOR",
	"FORM",
	"FRAME",
	"FRAMEBORDER",
	"FRAMESET",
	"GET",
	"groups",
	"GROUPS",
	"GUTTER",
	"H1",
	"H2",
	"H3",
	"H4",
	"H5",
	"H6",
	"H7",
	"HEAD",
	"HEIGHT",
	"HIDDEN",
	"Hn",
	"HR",
	"HREF",
	"hsides",
	"HSPACE",
	"HTML",
	"I",
	"ID",
	"IFRAME",
	"ILAYER",
	"image",
	"IMG",
	"INDEX",
	"inherit",
	"INPUT",
	"INS",
	"ISINDEX",
	"JavaScript",
	"justify",
	"KBD",
	"KEYGEN",
	"LABEL",
	"LANGUAGE",
	"LAYER",
	"LEFT",
	"LEGEND",
	"lhs",
	"LI",
	"LINK",
	"LISTING",
	"LOOP",
	"MAP",
	"MARQUEE",
	"MAXLENGTH",
	"MENU",
	"META",
	"METHOD",
	"METHODS",
	"MIDDLE",
	"MULTICOL",
	"MULTIPLE",
	"NAME",
	"NEXT",
	"NEXTID",
	"NOBR",
	"NOEMBED",
	"NOFRAMES",
	"NOLAYER",
	"none",
	"NOSAVE",
	"NOSCRIPT",
	"NOTAB",
	"NOWRAP",
	"OBJECT",
	"OL",
	"onBlur",
	"onChange",
	"onClick",
	"onFocus",
	"onLoad",
	"onMouseOut",
	"onMouseOver",
	"onReset",
	"onSelect",
	"onSubmit",
	"OPTION",
	"P",
	"PAGEX",
	"PAGEY",
	"PALETTE",
	"PANEL",
	"PARAM",
	"PARENT",
	"password",
	"PLAINTEXT",
	"PLUGINSPAGE",
	"POST",
	"PRE",
	"PREVIOUS",
	"Q",
	"radio",
	"REL",
	"REPEAT",
	"reset",
	"REV",
	"rhs",
	"RIGHT",
	"rows",
	"ROWSPAN",
	"RULES",
	"S",
	"SAMP",
	"SAVE",
	"SCRIPT",
	"SCROLLAMOUNT",
	"SCROLLDELAY",
	"SELECT",
	"SELECTED",
	"SERVER",
	"SHAPES",
	"show",
	"SIZE",
	"SMALL",
	"SONG",
	"SPACER",
	"SPAN",
	"SRC",
	"STANDBY",
	"STRIKE",
	"STRONG",
	"STYLE",
	"SUB",
	"submit",
	"SUMMARY",
	"SUP",
	"TABINDEX",
	"TABLE",
	"TARGET",
	"TBODY",
	"TD",
	"TEXT",
	"TEXTAREA",
	"textbottom",
	"TEXTFOCUS",
	"textmiddle",
	"texttop",
	"TFOOT",
	"TH",
	"THEAD",
	"TITLE",
	"TOP",
	"TR",
	"TT",
	"TXTCOLOR",
	"TYPE",
	"U",
	"UL",
	"URN",
	"USEMAP",
	"VALIGN",
	"VALUE",
	"VALUETYPE",
	"VAR",
	"VISIBILITY",
	"void",
	"vsides",
	"VSPACE",
	"WBR",
	"WIDTH",
	"WRAP",
	"XMP"
};

static const char* ppszKeyWordsPLSQL[] = {
	"AND",
	"AS",
	"BEGIN",
	"BINARY_INTEGER",
	"BODY",
	"BOOLEAN",
	"BY",
	"CHAR",
	"CHR",
	"COMMIT",
	"COUNT",
	"CREATE",
	"CURSOR",
	"DATE",
	"DECLARE",
	"DEFAULT",
	"DELETE",
	"ELSE",
	"ELSIF",
	"END",
	"ERRORS",
	"EXCEPTION",
	"FALSE",
	"FOR",
	"FROM",
	"FUNCTION",
	"GOTO",
	"HTP",
	"IDENT_ARR",
	"IF",
	"IN",
	"INDEX",
	"INTEGER",
	"IS",
	"LOOP",
	"NOT",
	"NO_DATA_FOUND",
	"NULL",
	"NUMBER",
	"OF",
	"OR",
	"ORDER",
	"OUT",
	"OWA_UTIL",
	"PACKAGE",
	"PRAGMA",
	"PRN",
	"PROCEDURE",
	"REPLACE",
	"RESTRICT_REFERENCES",
	"RETURN",
	"ROWTYPE",
	"SELECT",
	"SHOW",
	"SUBSTR",
	"TABLE",
	"THEN",
	"TRUE",
	"TYPE",
	"UPDATE",
	"VARCHAR",
	"VARCHAR2",
	"WHEN",
	"WHERE",
	"WHILE",
	"WNDS",
	"WNPS",
	"RAISE",
	"INSERT",
	"INTO",
	"VALUES",
	"SET",
	"SYSDATE",
	"RTRIM",
	"LTRIM",
	"TO_CHAR",
	"DUP_VAL_ON_INDEX",
	"ROLLBACK",
	"OTHERS",
	"SQLCODE"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const char* ppszKeyWordsCOBOL[] = {
	"ACCEPT",
	"ADD",
	"ADVANCING",
	"AFTER",
	"ALL",
	"AND",
	"ARGUMENT",
	"ASSIGN",
	"AUTHOR",
	"BEFORE",
	"BLOCK",
	"BY",
	"CALL",
	"CHARACTERS",
	"CLOSE",
	"COMP",
	"COMPILED",
	"COMPUTE",
	"COMPUTER",
	"CONFIGURATION",
	"CONSOLE",
	"CONTAINS",
	"CONTINUE",
	"CONTROL",
	"COPY",
	"DATA",
	"DELETE",
	"DISPLAY",
	"DIVIDE",
	"DIVISION",
	"ELSE",
	"END",
	"ENVIRONMENT",
	"EVALUATE",
	"EXAMINE",
	"EXIT",
	"EXTERNAL",
	"FD",
	"FILE",
	"FILLER",
	"FROM",
	"GIVING",
	"GO",
	"GOBACK",
	"HIGH-VALUE",
	"IDENTIFICATION"
	"IF",
	"INITIALIZE",
	"INPUT",
	"INTO",
	"IS",
	"LABEL",
	"LINKAGE",
	"LOW-VALUE",
	"MODE",
	"MOVE",
	"NOT",
	"OBJECT",
	"OCCURS",
	"OF",
	"ON",
	"OPEN",
	"OR",
	"OTHER",
	"OUTPUT",
	"PERFORM",
	"PIC",
	"PROCEDURE",
	"PROGRAM",
	"READ",
	"RECORD",
	"RECORDING",
	"REDEFINES",
	"REMAINDER",
	"REMARKS",
	"REPLACING",
	"REWRITE",
	"ROLLBACK",
	"SECTION",
	"SELECT",
	"SOURCE",
	"SPACE",
	"STANDARD",
	"STOP",
	"STORAGE",
	"SYSOUT",
	"TEST",
	"THEN",
	"TO",
	"TODAY",
	"TRANSFORM",
	"UNTIL",
	"UPON",
	"USING",
	"VALUE",
	"VARYING",
	"WHEN",
	"WITH",
	"WORKING",
	"WRITE",
	"WRITTEN",
	"ZERO"
};

static const char* ppszKeyWordsJAVA[] = {
	"abstract",
	"assert",	// Mar. 8, 2003 genta
	"boolean",
	"break",
	"byte",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"extends",
	"final",
	"finally",
	"float",
	"for",
	"goto",
	"if",
	"implements",
	"import",
	"instanceof",
	"int",
	"interface",
	"long",
	"native",
	"new",
	"package",
	"private",
	"protected",
	"public",
	"return",
	"short",
	"static",
	"strictfp",	// Mar. 8, 2003 genta
	"super",
	"switch",
	"synchronized",
	"this",
	"throw",
	"throws",
	"transient",
	"try",
	"void",
	"volatile",
	"while"
};

static const char* ppszKeyWordsCORBA_IDL[] = {
	"any",
	"attribute",
	"boolean",
	"case",
	"char",
	"const",
	"context",
	"default",
	"double",
	"enum",
	"exception",
	"FALSE",
	"fixed",
	"float",
	"in",
	"inout",
	"interface",
	"long",
	"module",
	"Object",
	"octet",
	"oneway",
	"out",
	"raises",
	"readonly",
	"sequence",
	"short",
	"string",
	"struct",
	"switch",
	"TRUE",
	"typedef",
	"unsigned",
	"union",
	"void",
	"wchar",
	"wstring"
};

static const char* ppszKeyWordsAWK[] = {
	"BEGIN",
	"END",
	"next",
	"exit",
	"func",
	"function",
	"return",
	"if",
	"else",
	"for",
	"in",
	"do",
	"while",
	"break",
	"continue",
	"$0",
	"$1",
	"$2",
	"$3",
	"$4",
	"$5",
	"$6",
	"$7",
	"$8",
	"$9",
	"$10",
	"$11",
	"$12",
	"$13",
	"$14",
	"$15",
	"$16",
	"$17",
	"$18",
	"$19",
	"$20",
	"FS",
	"OFS",
	"NF",
	"RS",
	"ORS",
	"NR",
	"FNR",
	"ARGV",
	"ARGC",
	"ARGIND",
	"FILENAME",
	"ENVIRON",
	"ERRNO",
	"OFMT",
	"CONVFMT",
	"FIELDWIDTHS",
	"IGNORECASE",
	"RLENGTH",
	"RSTART",
	"SUBSEP",
	"delete",
	"index",
	"jindex",
	"length",
	"jlength",
	"substr",
	"jsubstr",
	"match",
	"split",
	"sub",
	"gsub",
	"sprintf",
	"tolower",
	"toupper",
	"print",
	"printf",
	"getline",
	"system",
	"close",
	"sin",
	"cos",
	"atan2",
	"exp",
	"log",
	"int",
	"sqrt",
	"srand",
	"rand",
	"strftime",
	"systime"
};

static const char* ppszKeyWordsBAT[] = {
	"PATH",
	"PROMPT",
	"TEMP",
	"TMP",
	"TZ",
	"CONFIG",
	"COMSPEC",
	"DIRCMD",
	"COPYCMD",
	"winbootdir",
	"windir",
	"DIR",
	"CALL",
	"CHCP",
	"RENAME",
	"REN",
	"ERASE",
	"DEL",
	"TYPE",
	"REM",
	"COPY",
	"PAUSE",
	"DATE",
	"TIME",
	"VER",
	"VOL",
	"CD",
	"CHDIR",
	"MD",
	"MKDIR",
	"RD",
	"RMDIR",
	"BREAK",
	"VERIFY",
	"SET",
	"EXIT",
	"CTTY",
	"ECHO",
	"@ECHO",	//Oct. 31, 2000 JEPRO '@' �������\�ɂ����̂Œǉ�
	"LOCK",
	"UNLOCK",
	"GOTO",
	"SHIFT",
	"IF",
	"FOR",
	"DO",	//Nov. 2, 2000 JEPRO �ǉ�
	"IN",	//Nov. 2, 2000 JEPRO �ǉ�
	"ELSE",	//Nov. 2, 2000 JEPRO �ǉ� Win2000�Ŏg����
	"CLS",
	"TRUENAME",
	"LOADHIGH",
	"LH",
	"LFNFOR",
	"ON",
	"OFF",
	"NOT",
	"ERRORLEVEL",
	"EXIST",
	"NUL",
	"CON",
	"AUX",
	"COM1",
	"COM2",
	"COM3",
	"COM4",
	"PRN",
	"LPT1",
	"LPT2",
	"LPT3",
	"CLOCK",
	"CLOCK$",
	"CONFIG$"
};

static const char* ppszKeyWordsPASCAL[] = {
	"and",
	"exports",
	"mod",
	"shr",
	"array",
	"file",
	"nil",
	"string",
	"as",
	"finalization",
	"not",
	"stringresource",
	"asm",
	"finally",
	"object",
	"then",
	"begin",
	"for",
	"of",
	"case",
	"function",
	"or",
	"to",
	"class",
	"goto",
	"out",
	"try",
	"const",
	"if",
	"packed",
	"type",
	"constructor",
	"implementation",
	"procedure",
	"unit",
	"destructor",
	"in",
	"program",
	"until",
	"dispinterface",
	"inherited",
	"property",
	"uses",
	"div",
	"initialization",
	"raise",
	"var",
	"do",
	"inline",
	"record",
	"while",
	"downto",
	"interface",
	"repeat",
	"with",
	"else",
	"is",
	"resourcestring",
	"xor",
	"end",
	"label",
	"set",
	"except",
	"library",
	"shl",
	"private",
	"public",
	"published",
	"protected",
	"override"
};

static const char* ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	�啝�ǉ� & �኱�C���E�폜 --�قƂ�ǃR�}���h�̂�
	"error",
	"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
	"\\aa",
	"\\AA",
	"\\acute",
	"\\addcontentsline",
	"\\addtocounter",
	"\\addtolength",
	"\\ae",
	"\\AE",
	"\\aleph",
	"\\alpha",
	"\\alph",
	"\\Alph",
	"\\and",
	"\\angle",
	"\\appendix",
	"\\approx",
	"\\arabic",
	"\\arccos",
	"\\arctan",
	"\\arg",
	"\\arrayrulewidth",
	"\\arraystretch",
	"\\ast",
	"\\atop",
	"\\author",
	"\\b",
	"\\backslash",
	"\\bar",
	"\\baselineskip",
	"\\baselinestretch",
	"\\begin",
	"\\beta",
	"\\bf",
	"\\bibitem",
	"\\bibliography",
	"\\bibliographystyle",
	"\\big",
	"\\Big",
	"\\bigcap",
	"\\bigcirc",
	"\\bigcup",
	"\\bigg",
	"\\Bigg",
	"\\Biggl",
	"\\Biggm",
	"\\biggl",
	"\\biggm",
	"\\biggr",
	"\\Biggr",
	"\\bigl",
	"\\bigm",
	"\\Bigm",
	"\\Bigl",
	"\\bigodot",
	"\\bigoplus",
	"\\bigotimes",
	"\\bigr",
	"\\Bigr",
	"\\bigskip",
	"\\bigtriangledown",
	"\\bigtriangleup",
	"\\boldmath",
	"\\bot",
	"\\Box",
	"\\brace",
	"\\breve",
	"\\bullet",
	"\\bye",
	"\\c",
	"\\cal",
	"\\cap",
	"\\caption",
	"\\cc",
	"\\cdot",
	"\\cdots",
	"\\centering",
	"\\chapter",
	"\\check",
	"\\chi",
	"\\choose",
	"\\circ",
	"\\circle",
	"\\cite",
	"\\clearpage",
	"\\cline",
	"\\closing",
	"\\clubsuit",
	"\\colon",
	"\\columnsep",
	"\\columnseprule",
	"\\cong",
	"\\cot",
	"\\coth",
	"\\cr",
	"\\cup",
	"\\d",
	"\\dag",
	"\\dagger",
	"\\date",
	"\\dashbox",
	"\\ddag",
	"\\ddot",
	"\\ddots",
	"\\def",
	"\\deg",
	"\\delta",
	"\\Delta",
	"\\det",
	"\\diamond",
	"\\diamondsuit",
	"\\dim",
	"\\displaystyle",
	"\\documentclass",
	"\\documentstyle",
	"\\dot",
	"\\doteq",
	"\\dotfill",
	"\\Downarrow",
	"\\downarrow",
	"\\ell",
	"\\em",
	"\\emptyset",
	"\\encl",
	"\\end",
	"\\enspace",
	"\\enskip",
	"\\epsilon",
	"\\eqno",
	"\\equiv",
	"\\evensidemargin",
	"\\eta",
	"\\exists",
	"\\exp",
	"\\fbox",
	"\\fboxrule",
	"\\flat",
	"\\footnote",
	"\\footnotesize",
	"\\forall",
	"\\frac",
	"\\frame",
	"\\framebox",
	"\\gamma",
	"\\Gamma",
	"\\gcd",
	"\\ge",
	"\\geq",
	"\\gets",
	"\\gg",
	"\\grave",
	"\\gt",
	"\\H",
	"\\hat",
	"\\hbar",
	"\\hbox",
	"\\headsep",
	"\\heartsuit",
	"\\hfil",
	"\\hfill",
	"\\hline",
	"\\hom",
	"\\hrulefill",
	"\\hskip",
	"\\hspace",
	"\\hspace*",
	"\\huge",
	"\\Huge",
	"\\i",
	"\\Im",
	"\\imath",
	"\\in",
	"\\include",
	"\\includegraphics",
	"\\includeonly",
	"\\indent",
	"\\index",
	"\\inf",
	"\\infty",
	"\\input",
	"\\int",
	"\\iota",
	"\\it",
	"\\item",
	"\\itemsep",
	"\\j",
	"\\jmath",
	"\\kappa",
	"\\ker",
	"\\kern",
	"\\kill",
	"\\l",
	"\\L",
	"\\label",
	"\\lambda",
	"\\Lambda",
	"\\land",
	"\\langle",
	"\\large",
	"\\Large",
	"\\LARGE",
	"\\LaTeX",
	"\\LaTeXe",
	"\\lceil",
	"\\ldots",
	"\\le",
	"\\leftarrow",
	"\\Leftarrow",
	"\\lefteqn",
	"\\leftharpoondown",
	"\\leftharpoonup",
	"\\leftmargin",
	"\\leftrightarrow",
	"\\Leftrightarrow",
	"\\leq",
	"\\leqno",
	"\\lfloor",
	"\\lg",
	"\\lim",
	"\\liminf",
	"\\limsup",
	"\\line",
	"\\linebreak",
	"\\linewidth",
	"\\listoffigures",
	"\\listoftables",
	"\\ll",
	"\\llap",
	"\\ln",
	"\\lnot",
	"\\log",
	"\\longleftarrow",
	"\\Longleftarrow",
	"\\longleftrightarrow",
	"\\Longleftrightarrow",
	"\\longrightarrow",
	"\\Longrightarrow",
	"\\lor",
	"\\lower",
	"\\magstep",
	"\\makeatletter",
	"\\makeatother",
	"\\makebox",
	"\\makeindex",
	"\\maketitle",
	"\\makelabels",
	"\\mathop",
	"\\mapsto",
	"\\markboth",
	"\\markright",
	"\\mathstrut",
	"\\max",
	"\\mbox",
	"\\mc",
	"\\medskip",
	"\\mid",
	"\\min",
	"\\mit",
	"\\mp",
	"\\mu",
	"\\multicolumn",
	"\\multispan",
	"\\multiput",
	"\\nabla",
	"\\natural",
	"\\ne",
	"\\neg",
	"\\nearrow",
	"\\nwarrow",
	"\\neq",
	"\\newblock",
	"\\newcommand",
	"\\newenvironment",
	"\\newfont",
	"\\newlength",
	"\\newline",
	"\\newpage",
	"\\newtheorem",
	"\\ni",
	"\\noalign",
	"\\noindent",
	"\\nolimits",
	"\\nolinebreak",
	"\\nonumber",
	"\\nopagebreak",
	"\\normalsize",
	"\\not",
	"\\notice",
	"\\notin",
	"\\nu",
	"\\o",
	"\\O",
	"\\oddsidemargin",
	"\\odot",
	"\\oe",
	"\\OE",
	"\\oint",
	"\\Omega",
	"\\omega",
	"\\ominus",
	"\\oplus",
	"\\opening",
	"\\otimes",
	"\\owns",
	"\\overleftarrow",
	"\\overline",
	"\\overrightarrow",
	"\\overvrace",
	"\\oval",
	"\\P",
	"\\pagebreak",
	"\\pagenumbering",
	"\\pageref",
	"\\pagestyle",
	"\\par",
	"\\parallel",
	"\\paragraph",
	"\\parbox",
	"\\parindent",
	"\\parskip",
	"\\partial",
	"\\perp",
	"\\phi",
	"\\Phi",
	"\\pi",
	"\\Pi",
	"\\pm",
	"\\Pr",
	"\\prime",
	"\\printindex",
	"\\prod",
	"\\propto",
	"\\ps",
	"\\psi",
	"\\Psi",
	"\\put",
	"\\qquad",
	"\\quad",
	"\\raisebox",
	"\\rangle",
	"\\rceil",
	"\\Re",
	"\\ref",
	"\\renewcommand",
	"\\renewenvironment",
	"\\rfloor",
	"\\rho",
	"\\right",
	"\\rightarrow",
	"\\Rightarrow",
	"\\rightharpoondown",
	"\\rightharpoonup",
	"\\rightleftharpoonup",
	"\\rightmargin",
	"\\rm",
	"\\rule",
	"\\roman",
	"\\Roman",
	"\\S",
	"\\samepage",
	"\\sb",
	"\\sc",
	"\\scriptsize",
	"\\scriptscriptstyle",
	"\\scriptstyle",
	"\\searrow",
	"\\sec",
	"\\section",
	"\\setcounter",
	"\\setlength",
	"\\settowidth",
	"\\setminus",
	"\\sf",
	"\\sharp",
	"\\sigma",
	"\\Sigma",
	"\\signature",
	"\\sim",
	"\\simeq",
	"\\sin",
	"\\sinh",
	"\\sl",
	"\\sloppy",
	"\\small",
	"\\smash",
	"\\smallskip",
	"\\sp",
	"\\spadesuit",
	"\\special",
	"\\sqrt",
	"\\ss",
	"\\star",
	"\\stackrel",
	"\\strut",
	"\\subparagraph",
	"\\subsection",
	"\\subset",
	"\\subseteq",
	"\\subsubsection",
	"\\sum",
	"\\sup",
	"\\supset",
	"\\supseteq",
	"\\swarrow",
	"\\t",
	"\\tableofcontents",
	"\\tan",
	"\\tanh",
	"\\tau",
	"\\TeX",
	"\\textbf",
	"\\textgreater",
	"\\textgt",
	"\\textheight",
	"\\textit",
	"\\textless",
	"\\textmc",
	"\\textrm",
	"\\textsc",
	"\\textsf",
	"\\textsl",
	"\\textstyle",
	"\\texttt",
	"\\textwidth",
	"\\thanks",
	"\\thebibliography",
	"\\theequation",
	"\\thepage",
	"\\thesection",
	"\\theta",
	"\\Theta",
	"\\thicklines",
	"\\thinlines",
	"\\thinspace",
	"\\thisepage",
	"\\thisepagestyle",
	"\\tie",
	"\\tilde",
	"\\times",
	"\\tiny",
	"\\title",
	"\\titlepage",
	"\\to",
	"\\toaddress",
	"\\topmargin",
	"\\triangle",
	"\\tt",
	"\\twocolumn",
	"\\u",
	"\\underline",
	"\\undervrace",
	"\\unitlength",
	"\\Uparrow",
	"\\uparrow",
	"\\updownarrow",
	"\\Updownarrow",
	"\\uplus",
	"\\upsilon",
	"\\Upsilon",
	"\\usepackage",
	"\\v",
	"\\varepsilon",
	"\\varphi",
	"\\varpi",
	"\\varrho",
	"\\varsigma",
	"\\vartheta",
	"\\vbox",
	"\\vcenter",
	"\\vec",
	"\\vector",
	"\\vee",
	"\\verb",
	"\\verb*",
	"\\verbatim",
	"\\vert",
	"\\Vert",
	"\\vfil",
	"\\vfill",
	"\\vrule",
	"\\vskip",
	"\\vspace",
	"\\vspace*",
	"\\wedge",
	"\\widehat",
	"\\widetilde",
	"\\wp",
	"\\wr",
	"\\wrapfigure",
	"\\xi",
	"\\Xi",
	"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
};

//Jan. 19, 2001 JEPRO	TeX �̃L�[���[�h2�Ƃ��ĐV�K�ǉ� & �ꕔ���� --���R�}���h�ƃI�v�V�����������S
static const char* ppszKeyWordsTEX2[] = {
	//	���R�}���h
	//Jan. 19, 2001 JEPRO �{����{}�t���ŃL�[���[�h�ɂ������������P��Ƃ��ĔF�����Ă���Ȃ��̂Ŏ~�߂�
	"abstract",
	"array"
	"center",
	"description",
	"document",
	"displaymath",
	"em",
	"enumerate",
	"eqnarray",
	"eqnarray*",
	"equation",
	"figure",
	"figure*",
	"floatingfigure",
	"flushleft",
	"flushright",
	"itemize",
	"letter",
	"list",
	"math",
	"minipage",
	"multicols",
	"namelist",
	"picture",
	"quotation",
	"quote",
	"sloppypar",
	"subeqnarray",
	"subeqnarray*",
	"subequations",
	"subfigure",
	"tabbing",
	"table",
	"table*",
	"tabular",
	"tabular*",
	"tatepage",
	"thebibliography",
	"theindex",
	"titlepage",
	"trivlist",
	"verbatim",
	"verbatim*",
	"verse",
	"wrapfigure",
	//	�X�^�C���I�v�V����
	"a4",
	"a4j",
	"a5",
	"a5j",
	"Alph",
	"alph",
	"annote",
	"arabic",
	"b4",
	"b4j",
	"b5",
	"b5j",
	"bezier",
	"booktitle",
	"boxedminipage",
	"boxit",
//		"bp",
//		"cm",
	"dbltopnumber",
//		"dd",
	"eclepsf",
	"eepic",
	"enumi",
	"enumii",
	"enumiii",
	"enumiv",
	"epic",
	"epsbox",
	"epsf",
	"fancybox",
	"fancyheadings",
	"fleqn",
	"footnote",
	"howpublished",
	"jabbrv",
	"jalpha",
//		"article",
	"jarticle",
	"jsarticle",
//		"book",
	"jbook",
	"jsbook",
//		"letter",
	"jletter",
//		"plain",
	"jplain",
//		"report",
	"jreport",
	"jtwocolumn",
	"junsrt",
	"leqno",
	"makeidx",
	"markboth",
	"markright",
//		"mm",
	"multicol",
	"myheadings",
	"openbib",
//		"pc",
//		"pt",
	"secnumdepth",
//		"sp",
	"titlepage",
	"tjarticle",
	"topnumber",
	"totalnumber",
	"twocolumn",
	"twoside",
	"yomi"//,
//		"zh",
//		"zw"
};

static const char* ppszKeyWordsPERL[] = {
	//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
	"break",
	"continue",
	"do",
	"elsif",
	"else",
	"for",
	"foreach",
	"goto",
	"if",
	"last",
	"next",
	"return",
	"sub",
	"undef",
	"unless",
	"until",
	"while",
	"abs",
	"accept",
	"alarm",
	"atan2",
	"bind",
	"binmode",
	"bless",
	"caller",
	"chdir",
	"chmod",
	"chomp",
	"chop",
	"chown",
	"chr",
	"chroot",
	"close",
	"closedir",
	"connect",
	"continue",
	"cos",
	"crypt",
	"dbmclose",
	"dbmopen",
	"defined",
	"delete",
	"die",
	"do",
	"dump",
	"each",
	"eof",
	"eval",
	"exec",
	"exists",
	"exit",
	"exp",
	"fcntl",
	"fileno",
	"flock",
	"fork",
	"format",
	"formline",
	"getc",
	"getlogin",
	"getpeername",
	"getpgrp",
	"getppid",
	"getpriority",
	"getpwnam",
	"getgrnam",
	"gethostbyname",
	"getnetbyname",
	"getprotobyname",
	"getpwuid",
	"getgrgid",
	"getservbyname",
	"gethostbyaddr",
	"getnetbyaddr",
	"getprotobynumber",
	"getservbyport",
	"getpwent",
	"getgrent",
	"gethostent",
	"getnetent",
	"getprotoent",
	"getservent",
	"setpwent",
	"setgrent",
	"sethostent",
	"setnetent",
	"setprotoent",
	"setservent",
	"endpwent",
	"endgrent",
	"endhostent",
	"endnetent",
	"endprotoent",
	"endservent",
	"getsockname",
	"getsockopt",
	"glob",
	"gmtime",
	"goto",
	"grep",
	"hex",
	"import",
	"index",
	"int",
	"ioctl",
	"join",
	"keys",
	"kill",
	"last",
	"lc",
	"lcfirst",
	"length",
	"link",
	"listen",
	"local",
	"localtime",
	"log",
	"lstat",
//			"//m",
	"map",
	"mkdir",
	"msgctl",
	"msgget",
	"msgsnd",
	"msgrcv",
	"my",
	"next",
	"no",
	"oct",
	"open",
	"opendir",
	"ord",
	"our",	// 2006.04.20 genta
	"pack",
	"package",
	"pipe",
	"pop",
	"pos",
	"print",
	"printf",
	"prototype",
	"push",
//			"//q",
	"qq",
	"qr",
	"qx",
	"qw",
	"quotemeta",
	"rand",
	"read",
	"readdir",
	"readline",
	"readlink",
	"readpipe",
	"recv",
	"redo",
	"ref",
	"rename",
	"require",
	"reset",
	"return",
	"reverse",
	"rewinddir",
	"rindex",
	"rmdir",
//			"//s",
	"scalar",
	"seek",
	"seekdir",
	"select",
	"semctl",
	"semget",
	"semop",
	"send",
	"setpgrp",
	"setpriority",
	"setsockopt",
	"shift",
	"shmctl",
	"shmget",
	"shmread",
	"shmwrite",
	"shutdown",
	"sin",
	"sleep",
	"socket",
	"socketpair",
	"sort",
	"splice",
	"split",
	"sprintf",
	"sqrt",
	"srand",
	"stat",
	"study",
	"sub",
	"substr",
	"symlink",
	"syscall",
	"sysopen",
	"sysread",
	"sysseek",
	"system",
	"syswrite",
	"tell",
	"telldir",
	"tie",
	"tied",
	"time",
	"times",
	"tr",
	"truncate",
	"uc",
	"ucfirst",
	"umask",
	"undef",
	"unlink",
	"unpack",
	"untie",
	"unshift",
	"use",
	"utime",
	"values",
	"vec",
	"wait",
	"waitpid",
	"wantarray",
	"warn",
	"write"
};

//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
// 2008/05/05 novice �d��������폜
static const char* ppszKeyWordsPERL2[] = {
	"$ARGV",
	"$_",
	"$1",
	"$2",
	"$3",
	"$4",
	"$5",
	"$6",
	"$7",
	"$8",
	"$9",
	"$0",
	"$MATCH",
	"$&",
	"$PREMATCH",
	"$`",
	"$POSTMATCH",
	"$'",
	"$LAST_PAREN_MATCH",
	"$+",
	"$MULTILINE_MATCHING",
	"$*",
	"$INPUT_LINE_NUMBER",
	"$NR",
	"$.",
	"$INPUT_RECORD_SEPARATOR",
	"$RS",
	"$/",
	"$OUTPUT_AUTOFLUSH",
	"$|",
	"$OUTPUT_FIELD_SEPARATOR",
	"$OFS",
	"$,",
	"$OUTPUT_RECORD_SEPARATOR",
	"$ORS",
	"$\\",
	"$LIST_SEPARATOR",
	"$\"",
	"$SUBSCRIPT_SEPARATOR",
	"$SUBSEP",
	"$;",
	"$OFMT",
	"$#",
	"$FORMAT_PAGE_NUMBER",
	"$%",
	"$FORMAT_LINES_PER_PAGE",
	"$=",
	"$FORMAT_LINES_LEFT",
	"$-",
	"$FORMAT_NAME",
	"$~",
	"$FORMAT_TOP_NAME",
	"$^",
	"$FORMAT_LINE_BREAK_CHARACTERS",
	"$:",
	"$FORMAT_FORMFEED",
	"$^L",
	"$ACCUMULATOR",
	"$^A",
	"$CHILD_ERROR",
	"$?",
	"$OS_ERROR",
	"$ERRNO",
	"$!",
	"$EVAL_ERROR",
	"$@",
	"$PROCESS_ID",
	"$PID",
	"$$",
	"$REAL_USER_ID",
	"$UID",
	"$<",
	"$EFFECTIVE_USER_ID",
	"$EUID",
	"$>",
	"$REAL_GROUP_ID",
	"$GID",
	"$(",
	"$EFFECTIVE_GROUP_ID",
	"$EGID",
	"$)",
	"$PROGRAM_NAME",
	"$[",
	"$PERL_VERSION",
	"$]",
	"$DEBUGGING",
	"$^D",
	"$SYSTEM_FD_MAX",
	"$^F",
	"$INPLACE_EDIT",
	"$^I",
	"$PERLDB",
	"$^P",
	"$BASETIME",
	"$^T",
	"$WARNING",
	"$^W",
	"$EXECUTABLE_NAME",
	"$^X",
	"$ENV",
	"$SIG"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const char* ppszKeyWordsVB[] = {
	"And",
	"As",
	"Attribute",
	"Begin",
	"BeginProperty",
	"Boolean",
	"ByVal",
	"Byte",
	"Call",
	"Case",
	"Const",
	"Currency",
	"Date",
	"Declare",
	"Dim",
	"Do",
	"Double",
	"Each",
	"Else",
	"ElseIf",
	"Empty",
	"End",
	"EndProperty",
	"Error",
	"Eqv",
	"Exit",
	"False",
	"For",
	"Friend",
	"Function",
	"Get",
	"GoTo",
	"If",
	"Imp",
	"Integer",
	"Is",
	"Let",
	"Like",
	"Long",
	"Loop",
	"Me",
	"Mod",
	"New",
	"Next",
	"Not",
	"Null",
	"Object",
	"On",
	"Option",
	"Or",
	"Private",
	"Property",
	"Public",
	"RSet",
	"ReDim",
	"Rem",
	"Resume",
	"Select",
	"Set",
	"Single",
	"Static",
	"Step",
	"Stop",
	"String",
	"Sub",
	"Then",
	"To",
	"True",
	"Type",
	"Wend",
	"While",
	"With",
	"Xor",
	"#If",
	"#Else",
	"#End",
	"#Const",
	"AddressOf",
	"Alias",
	"Append",
	"Array",
	"ByRef",
	"Explicit",
	"Global",
	"In",
	"Lib",
	"Nothing",
	"Optional",
	"Output",
	"Terminate",
	"Until",
	//=========================================================
	// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
	//=========================================================
	"DefBool",
	"DefByte",
	"DefCur",
	"DefDate",
	"DefDbl",
	"DefInt",
	"DefLng",
	"DefObj",
	"DefSng",
	"DefStr",
	"DefVar",
	"LSet",
	"GoSub",
	"Return",
	"Variant",
	//			"Option Base
	//			"As Any
	//=========================================================
	// �ȉ���VB.NET�p�L�[���[�h�ł�
	//=========================================================
	//BitAnd
	//BitOr
	//BitNot
	//BitXor
	//Delegate
	//Short
	//Structure
};

//Jul. 10, 2001 JEPRO �ǉ�
static const char* ppszKeyWordsVB2[] = {
	"AppActivate",
	"Beep",
	"BeginTrans",
	"ChDir",
	"ChDrive",
	"Close",
	"CommitTrans",
	"CompactDatabase",
	"Date",
	"DeleteSetting",
	"Erase",
	"FileCopy",
	"FreeLocks",
	"Input",
	"Kill",
	"Load",
	"Lock",
	"Mid",
	"MidB",
	"MkDir",
	"Name",
	"Open",
	"Print",
	"Put",
	"Randomize",
	"RegisterDatabase",
	"RepairDatabase",
	"Reset",
	"RmDir",
	"Rollback",
	"SavePicture",
	"SaveSetting",
	"Seek",
	"SendKeys",
	"SetAttr",
	"SetDataAccessOption",
	"SetDefaultWorkspace",
	"Time",
	"Unload",
	"Unlock",
	"Width",
	"Write",
	"Array",
	"Asc",
	"AscB",
	"Atn",
	"CBool",
	"CByte",
	"CCur",
	"CDate",
	"CDbl",
	"CInt",
	"CLng",
	"CSng",
	"CStr",
	"CVErr",
	"CVar",
	"Choose",
	"Chr",
	"ChrB",
	"Command",
	"Cos",
	"CreateDatabase",
	"CreateObject",
	"CurDir",
	"DDB",
	"Date",
	"DateAdd",
	"DateDiff",
	"DatePart",
	"DateSerial",
	"DateValue",
	"Day",
	"Dir",
	"DoEvents",
	"EOF",
	"Environ",
	"Error",
	"Exp",
	"FV",
	"FileAttr",
	"FileDateTime",
	"FileLen",
	"Fix",
	"Format",
	"FreeFile",
	"GetAllSettings",
	"GetAttr",
	"GetObject",
	"GetSetting",
	"Hex",
	"Hour",
	"IIf",
	"IMEStatus",
	"IPmt",
	"IRR",
	"InStr",
	"Input",
	"Int",
	"IsArray",
	"IsDate",
	"IsEmpty",
	"IsError",
	"IsMissing",
	"IsNull",
	"IsNumeric",
	"IsObject",
	"LBound",
	"LCase",
	"LOF",
	"LTrim",
	"Left",
	"LeftB",
	"Len",
	"LoadPicture",
	"Loc",
	"Log",
	"MIRR",
	"Mid",
	"MidB",
	"Minute",
	"Month",
	"MsgBox",
	"NPV",
	"NPer",
	"Now",
	"Oct",
	"OpenDatabase",
	"PPmt",
	"PV",
	"Partition",
	"Pmt",
	"QBColor",
	"RGB",
	"RTrim",
	"Rate",
	"ReadProperty",
	"Right",
	"RightB",
	"Rnd",
	"SLN",
	"SYD",
	"Second",
	"Seek",
	"Sgn",
	"Shell",
	"Sin",
	"Space",
	"Spc",
	"Sqr",
	"Str",
	"StrComp",
	"StrConv",
	"Switch",
	"Tab",
	"Tan",
	"Time",
	"TimeSerial",
	"TimeValue",
	"Timer",
	"Trim",
	"TypeName",
	"UBound",
	"UCase",
	"Val",
	"VarType",
	"Weekday",
	"Year",
	"Hide",
	"Line",
	"Refresh",
	"Show",
	//=========================================================
	// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
	//=========================================================
	//$�t���֐��e��
	"Dir$",
	"LCase$",
	"Left$",
	"LeftB$",
	"Mid$",
	"MidB$",
	"RightB$",
	"Right$",
	"Space$",
	"Str$",
	"String$",
	"Trim$",
	"UCase$",
	//VB5,6�̉B���֐�
	"VarPtr",
	"StrPtr",
	"ObjPtr",
	"VarPrtArray",
	"VarPtrStringArray"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const char* ppszKeyWordsRTF[] = {
	"\\ansi",
	"\\b",
	"\\bin",
	"\\box",
	"\\brdrb",
	"\\brdrbar",
	"\\brdrdb",
	"\\brdrdot",
	"\\brdrl",
	"\\brdrr",
	"\\brdrs",
	"\\brdrsh",
	"\\brdrt",
	"\\brdrth",
	"\\cell",
	"\\cellx",
	"\\cf",
	"\\chftn",
	"\\clmgf",
	"\\clmrg",
	"\\colortbl",
	"\\deff",
	"\\f",
	"\\fi",
	"\\field",
	"\\fldrslt",
	"\\fonttbl",
	"\\footnote",
	"\\fs",
	"\\i"
	"\\intbl",
	"\\keep",
	"\\keepn",
	"\\li",
	"\\line",
	"\\mac",
	"\\page",
	"\\par",
	"\\pard",
	"\\pc",
	"\\pich",
	"\\pichgoal",
	"\\picscalex",
	"\\picscaley",
	"\\pict",
	"\\picw",
	"\\picwgoal",
	"\\plain",
	"\\qc",
	"\\ql",
	"\\qr",
	"\\ri",
	"\\row",
	"\\rtf",
	"\\sa",
	"\\sb",
	"\\scaps",
	"\\sect",
	"\\sl",
	"\\strike",
	"\\tab",
	"\\tqc",
	"\\tqr",
	"\\trgaph",
	"\\trleft",
	"\\trowd",
	"\\trqc",
	"\\trql",
	"\\tx",
	"\\ul",
	"\\uldb",
	"\\v",
	"\\wbitmap",
	"\\wbmbitspixel",
	"\\wbmplanes",
	"\\wbmwidthbytes",
	"\\wmetafile",
	"bmc",
	"bml",
	"bmr",
	"emc",
	"eml",
	"emr"
};

/*!	@brief ���L������������/�����L�[���[�h

	�����L�[���[�h�֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
		�L�[���[�h��`���֐��̊O�ɏo���C�o�^���}�N�������ĊȌ��ɁD
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* �����L�[���[�h�̃e�X�g�f�[�^ */
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,ary) \
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, _countof(ary), (ary) );
	
	PopulateKeyword( "C/C++", true, ppszKeyWordsCPP );			/* �Z�b�g 0�̒ǉ� */
	PopulateKeyword( "HTML", false, ppszKeyWordsHTML );			/* �Z�b�g 1�̒ǉ� */
	PopulateKeyword( "PL/SQL", false, ppszKeyWordsPLSQL );		/* �Z�b�g 2�̒ǉ� */
	PopulateKeyword( "COBOL", true ,ppszKeyWordsCOBOL );		/* �Z�b�g 3�̒ǉ� */
	PopulateKeyword( "Java", true, ppszKeyWordsJAVA );			/* �Z�b�g 4�̒ǉ� */
	PopulateKeyword( "CORBA IDL", true, ppszKeyWordsCORBA_IDL );/* �Z�b�g 5�̒ǉ� */
	PopulateKeyword( "AWK", true, ppszKeyWordsAWK );			/* �Z�b�g 6�̒ǉ� */
	PopulateKeyword( "MS-DOS batch", false, ppszKeyWordsBAT );	/* �Z�b�g 7�̒ǉ� */	//Oct. 31, 2000 JEPRO '�o�b�`�t�@�C��'��'batch' �ɒZ�k
	PopulateKeyword( "Pascal", false, ppszKeyWordsPASCAL );		/* �Z�b�g 8�̒ǉ� */	//Nov. 5, 2000 JEPRO ��E�������̋�ʂ�'���Ȃ�'�ɕύX
	PopulateKeyword( "TeX", true, ppszKeyWordsTEX );			/* �Z�b�g 9�̒ǉ� */	//Sept. 2, 2000 jepro Tex ��TeX �ɏC�� Bool�l�͑�E�������̋��
	PopulateKeyword( "TeX2", true, ppszKeyWordsTEX2 );			/* �Z�b�g10�̒ǉ� */	//Jan. 19, 2001 JEPRO �ǉ�
	PopulateKeyword( "Perl", true, ppszKeyWordsPERL );			/* �Z�b�g11�̒ǉ� */
	PopulateKeyword( "Perl2", true, ppszKeyWordsPERL2 );		/* �Z�b�g12�̒ǉ� */	//Jul. 10, 2001 JEPRO Perl����ϐ��𕪗��E�Ɨ�
	PopulateKeyword( "Visual Basic", false, ppszKeyWordsVB );	/* �Z�b�g13�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( "Visual Basic2", false, ppszKeyWordsVB2 );	/* �Z�b�g14�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( "���b�`�e�L�X�g", true, ppszKeyWordsRTF );	/* �Z�b�g15�̒ǉ� */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

//	Sep. 14, 2000 JEPRO
//	Shift+F1 �Ɂu�R�}���h�ꗗ�v, Alt+F1 �Ɂu�w���v�ڎ��v, Shift+Alt+F1 �Ɂu�L�[���[�h�����v��ǉ�	//Nov. 25, 2000 JEPRO �E���Ă����̂��C���E����
//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 �Ɂu�o�[�W�������v��ǉ�
// 2001.12.03 hor F2�Ƀu�b�N�}�[�N�֘A������
//Sept. 21, 2000 JEPRO	Ctrl+F3 �Ɂu�����}�[�N�̃N���A�v��ǉ�
//Aug. 12, 2002 ai	Ctrl+Shift+F3 �Ɂu�����J�n�ʒu�֖߂�v��ǉ�
//Oct. 7, 2000 JEPRO	Alt+F4 �Ɂu�E�B���h�E�����v, Shift+Alt+F4 �Ɂu���ׂẴE�B���h�E�����v��ǉ�
//	Ctrl+F4�Ɋ��蓖�Ă��Ă����u�c���ɕ����v���u����(����)�v�ɕύX�� Shift+Ctrl+F4 �Ɂu���ĊJ���v��ǉ�
//Jan. 14, 2001 Ctrl+Alt+F4 �Ɂu�e�L�X�g�G�f�B�^�̑S�I���v��ǉ�
//Jun. 2001�u�T�N���G�f�B�^�̑S�I���v�ɉ���
//2006.10.21 ryoji Alt+F4 �ɂ͉������蓖�ĂȂ��i�f�t�H���g�̃V�X�e���R�}���h�u����v�����s�����悤�Ɂj
//2007.02.13 ryoji Shift+Ctrl+F4��F_WIN_CLOSEALL����F_EXITALLEDITORS�ɕύX
//2007.02.22 ryoji Ctrl+F4 �ւ̊��蓖�Ă��폜�i�f�t�H���g�̃R�}���h�����s�j
//	Sep. 20, 2000 JEPRO Ctrl+F5 �Ɂu�O���R�}���h���s�v��ǉ�  �Ȃ��}�N������CMMAND ����COMMAND �ɕύX�ς�
//Oct. 28, 2000 F5 �́u�ĕ`��v�ɕύX	//Jan. 14, 2001 Alt+F5 �Ɂuuudecode���ĕۑ��v, Ctrl+ Alt+F5 �ɁuTAB���󔒁v��ǉ�
//	May 28, 2001 genta	S-C-A-F5��SPACE-to-TAB��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F6 �Ɂu�������v, Alt+F6 �ɁuBase64�f�R�[�h���ĕۑ��v��ǉ�
// 2007.10.05 nasukoji	�g���v���N���b�N�E�N�A�h���v���N���b�N�Ή�
//Jan. 14, 2001 JEPRO	Ctrl+F7 �Ɂu�啶���v, Alt+F7 �ɁuUTF-7��SJIS�R�[�h�ϊ��v, Shift+Alt+F7 �ɁuSJIS��UTF-7�R�[�h�ϊ��v, Ctrl+Alt+F7 �ɁuUTF-7�ŊJ�������v��ǉ�
//Nov. 9, 2000 JEPRO	Shift+F8 �ɁuCRLF���s�ŃR�s�[�v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F8 �Ɂu�S�p�����p�v, Alt+F8 �ɁuUTF-8��SJIS�R�[�h�ϊ��v, Shift+Alt+F8 �ɁuSJIS��UTF-8�R�[�h�ϊ��v, Ctrl+Alt+F8 �ɁuUTF-8�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F9 �Ɂu���p�{�S�Ђ灨�S�p�E�J�^�J�i�v, Alt+F9 �ɁuUnicode��SJIS�R�[�h�ϊ��v, Ctrl+Alt+F9 �ɁuUnicode�ŊJ�������v��ǉ�
//Oct. 28, 2000 JEPRO F10 �ɁuSQL*Plus�Ŏ��s�v��ǉ�(F5����̈ړ�)
//Jan. 14, 2001 JEPRO	Ctrl+F10 �Ɂu���p�{�S�J�^���S�p�E�Ђ炪�ȁv, Alt+F10 �ɁuEUC��SJIS�R�[�h�ϊ��v, Shift+Alt+F10 �ɁuSJIS��EUC�R�[�h�ϊ��v, Ctrl+Alt+F10 �ɁuEUC�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Shift+F11 �ɁuSQL*Plus���A�N�e�B�u�\���v, Ctrl+F11 �Ɂu���p�J�^�J�i���S�p�J�^�J�i�v, Alt+F11 �ɁuE-Mail(JIS��SJIS)�R�[�h�ϊ��v, Shift+Alt+F11 �ɁuSJIS��JIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuJIS�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F12 �Ɂu���p�J�^�J�i���S�p�Ђ炪�ȁv, Alt+F12 �Ɂu�������ʁ�SJIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuSJIS�ŊJ�������v��ǉ�
//Sept. 1, 2000 JEPRO	Alt+Enter �Ɂu�t�@�C���̃v���p�e�B�v��ǉ�	//Oct. 15, 2000 JEPRO Ctrl+Enter �Ɂu�t�@�C�����e��r�v��ǉ�
//Oct. 7, 2000 JEPRO �����̂Ŗ��̂��ȗ��`�ɕύX(BackSpace��BkSp)
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Insert��Ins)
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Delete��Del)
//Jun. 26, 2001 JEPRO	Shift+Del �Ɂu�؂���v��ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+���Ɂu�c�����ɍő剻�v��ǉ�
//Jun. 27, 2001 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\����ړ�(�Q�s����)�v���u�e�L�X�g���P�s���փX�N���[���v�ɕύX
//2001.02.10 by MIK Shift+Ctrl+Alt+���Ɂu�������ɍő剻�v��ǉ�
//Sept. 14, 2000 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�E�N���b�N���j���[�v���u�J�[�\�����ړ�(�Q�s����)�v�ɕύX
//	����ɕt�����Ă���Ɂu�E�N���b�N���j���[�v��Ctrl�{Alt�{���ɕύX
//Jun. 27, 2001 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\�����ړ�(�Q�s����)�v���u�e�L�X�g���P�s��փX�N���[���v�ɕύX
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�_�E���v, �u(�I��)�P�y�[�W�_�E���v��ǉ�
//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollUp��PgDn) //Oct. 10, 2000 JEPRO ���̕ύX
//2001.12.03 hor 1Page/HalfPage ���ւ�
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�A�b�v�v, �u(�I��)�P�y�[�W�A�b�v�v��ǉ�
//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollDown��PgUp) //Oct. 10, 2000 JEPRO ���̕ύX
//2001.12.03 hor 1Page/HalfPage ���ւ�
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(SpaceBar��Space)
//Oct. 7, 2000 JEPRO	Ctrl+0 ���u�^�C�v�ʐݒ�ꗗ�v���u����`�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+0 �Ɂu�J�X�^�����j���[10�v, Shift+Alt+0 �Ɂu�J�X�^�����j���[20�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+1 ���u�^�C�v�ʐݒ�v���u�c�[���o�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+1 �Ɂu�J�X�^�����j���[1�v, Shift+Alt+1 �Ɂu�J�X�^�����j���[11�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 �Ɂu�J�X�^�����j���[21�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+2 ���u���ʐݒ�v���u�t�@���N�V�����L�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+2 ���u�A�E�g�v�b�g�v���u�J�X�^�����j���[2�v�ɕύX���u�A�E�g�v�b�g�v�� Alt+O �Ɉړ�, Shift+Alt+2 �Ɂu�J�X�^�����j���[12�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 �Ɂu�J�X�^�����j���[22�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+3 ���u�t�H���g�ݒ�v���u�X�e�[�^�X�o�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+3 �Ɂu�J�X�^�����j���[3�v, Shift+Alt+3 �Ɂu�J�X�^�����j���[13�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 �Ɂu�J�X�^�����j���[23�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+4 ���u�c�[���o�[�̕\���v���u�^�C�v�ʐݒ�ꗗ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+4 �Ɂu�J�X�^�����j���[4�v, Shift+Alt+4 �Ɂu�J�X�^�����j���[14�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 �Ɂu�J�X�^�����j���[24�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+5 ���u�t�@���N�V�����L�[�̕\���v���u�^�C�v�ʐݒ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+5 �Ɂu�J�X�^�����j���[5�v, Shift+Alt+5 �Ɂu�J�X�^�����j���[15�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+6 ���u�X�e�[�^�X�o�[�̕\���v���u���ʐݒ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+6 �Ɂu�J�X�^�����j���[6�v, Shift+Alt+6 �Ɂu�J�X�^�����j���[16�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+7 �Ɂu�t�H���g�ݒ�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+7 �Ɂu�J�X�^�����j���[7�v, Shift+Alt+7 �Ɂu�J�X�^�����j���[17�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+8 �Ɂu�J�X�^�����j���[8�v, Shift+Alt+8 �Ɂu�J�X�^�����j���[18�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+9 �Ɂu�J�X�^�����j���[9�v, Shift+Alt+9 �Ɂu�J�X�^�����j���[19�v��ǉ�
//2001.12.06 hor Alt+A ���uSORT_ASC�v�Ɋ���
//Jan. 13, 2001 JEPRO	Ctrl+B �Ɂu�u���E�Y�v��ǉ�
//Jan. 16, 2001 JEPRO	SHift+Ctrl+C �Ɂu.h�Ɠ�����.c(�Ȃ����.cpp)���J���v��ǉ�
//Feb. 07, 2001 JEPRO	SHift+Ctrl+C ���u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
//Jan. 16, 2001 JEPRO	Ctrl+D �Ɂu�P��؂���v, Shift+Ctrl+D �Ɂu�P��폜�v��ǉ�
//2001.12.06 hor Alt+D ���uSORT_DESC�v�Ɋ���
//Oct. 7, 2000 JEPRO	Ctrl+Alt+E �Ɂu�d�˂ĕ\���v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+E �Ɂu�s�؂���(�܂�Ԃ��P��)�v, Shift+Ctrl+E �Ɂu�s�폜(�܂�Ԃ��P��)�v��ǉ�
//Oct. 07, 2000 JEPRO	Ctrl+Alt+H �Ɂu�㉺�ɕ��ׂĕ\���v��ǉ�
//Jan. 16, 2001 JEPRO	Ctrl+H ���u�J�[�\���O���폜�v���u�J�[�\���s���E�B���h�E�����ցv�ɕύX��	Shift+Ctrl+H �Ɂu.c�܂���.cpp�Ɠ�����.h���J���v��ǉ�
//Feb. 07, 2001 JEPRO	SHift+Ctrl+H ���u.c�܂���.cpp�Ɠ�����.h���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
//Jan. 21, 2001	JEPRO	Ctrl+I �Ɂu�s�̓�d���v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+K �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+E �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+Alt+L �Ɂu�������v, Shift+Ctrl+Alt+L �Ɂu�啶���v��ǉ�
//Jan. 16, 2001 Ctrl+L ���u�J�[�\���s���E�B���h�E�����ցv���u�L�[�}�N���̓ǂݍ��݁v�ɕύX���u�J�[�\���s���E�B���h�E�����ցv�� Ctrl+H �Ɉړ�
//2001.12.03 hor Alt+L ���uLTRIM�v�Ɋ���
//Jan. 16, 2001 JEPRO	Ctrl+M �Ɂu�L�[�}�N���̕ۑ��v��ǉ�
//2001.12.06 hor Alt+M ���uMERGE�v�Ɋ���
//Oct. 20, 2000 JEPRO	Alt+N �Ɂu�ړ�����: ���ցv��ǉ�
//Jan. 13, 2001 JEPRO	Alt+O �Ɂu�A�E�g�v�b�g�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+P �Ɂu����v, Shift+Ctrl+P �Ɂu����v���r���[�v, Ctrl+Alt+P �Ɂu�y�[�W�ݒ�v��ǉ�
//Oct. 20, 2000 JEPRO	Alt+P �Ɂu�ړ�����: �O�ցv��ǉ�
//Jan. 24, 2001	JEPRO	Ctrl+Q �Ɂu�L�[���蓖�Ĉꗗ���R�s�[�v��ǉ�
//2001.12.03 hor Alt+R ���uRTRIM�v�Ɋ���
//Oct. 7, 2000 JEPRO	Shift+Ctrl+S �Ɂu���O��t���ĕۑ��v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+Alt+T �Ɂu���E�ɕ��ׂĕ\���v��ǉ�
//Jan. 21, 2001	JEPRO	Ctrl+T �Ɂu�^�O�W�����v�v, Shift+Ctrl+T �Ɂu�^�O�W�����v�o�b�N�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+Alt+U �Ɂu���݂̃E�B���h�E���Ő܂�Ԃ��v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+U �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+U �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+X ���u�J�X�^�����j���[1�v���u����`�v�ɕύX���u�J�X�^�����j���[1�v�� Alt+1 �Ɉړ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+- �Ɂu�㉺�ɕ����v��ǉ�
// 2002.02.08 hor Ctrl+-�Ƀt�@�C�������R�s�[��ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ �Ɂu���E�ɕ����v��ǉ�
//Sept. 20, 2000 JEPRO	Ctrl+@ �Ɂu�t�@�C�����e��r�v��ǉ�  //Oct. 15, 2000 JEPRO�u�I��͈͓��S�s�R�s�[�v�ɕύX
//	Aug. 16, 2000 genta
//	���Ό����̊��ʂɂ����ʌ�����ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+; �Ɂu�c���ɕ����v��ǉ�	//Jan. 16, 2001	Alt+; �Ɂu���t�}���v��ǉ�
//Sept. 14, 2000 JEPRO	Ctrl+: �Ɂu�I��͈͓��S�s�s�ԍ��t���R�s�[�v��ǉ�	//Jan. 16, 2001	Alt+: �Ɂu�����}���v��ǉ�
//Sept. 14, 2000 JEPRO	Ctrl+. �Ɂu�I��͈͓��S�s���p���t���R�s�[�v��ǉ�
//	Nov. 15, 2000 genta PC/AT�L�[�{�[�h�ɍ��킹�ăL�[�R�[�h��ύX
//	PC98�~�ς̂��߁C�]���̃L�[�R�[�h�ɑΉ����鍀�ڂ�ǉ��D
//Oct. 7, 2000 JEPRO	�����ĕ\��������Ȃ������łĂ��Ă��܂��̂ŃA�v���P�[�V�����L�[���A�v���L�[�ɒZ�k
//2008.05.03 kobake �ǐ����������ቺ���Ă����̂ŁA�����𐮗��B
// 2008.05.30 nasukoji	Ctrl+Alt+S �Ɂu�w�茅�Ő܂�Ԃ��v��ǉ�
// 2008.05.30 nasukoji	Ctrl+Alt+W �Ɂu�E�[�Ő܂�Ԃ��v��ǉ�
// 2008.05.30 nasukoji	Ctrl+Alt+X �Ɂu�܂�Ԃ��Ȃ��v��ǉ�

/*!	@brief ���L������������/�L�[���蓖��

	�f�t�H���g�L�[���蓖�Ċ֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番��
	@date 2007.11.04 genta �L�[�ݒ萔��DLLSHARE�̗̈�𒴂�����N���ł��Ȃ��悤��
*/
bool CShareData::InitKeyAssign(DLLSHAREDATA* pShareData)
{
	/********************/
	/* ���ʐݒ�̋K��l */
	/********************/
	struct KEYDATAINIT {
		short			nKeyCode;		//!< Key Code (0 for non-keybord button)
		TCHAR*			pszKeyName;		//!< Key Name (for display)
		short			nFuncCode_0;	//!<                      Key
		short			nFuncCode_1;	//!< Shift +              Key
		short			nFuncCode_2;	//!<         Ctrl +       Key
		short			nFuncCode_3;	//!< Shift + Ctrl +       Key
		short			nFuncCode_4;	//!<                Alt + Key
		short			nFuncCode_5;	//!< Shift +        Alt + Key
		short			nFuncCode_6;	//!<         Ctrl + Alt + Key
		short			nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: ���Ԃ�2�i�ŉ���3�r�b�g[Alt][Ctrl][Shift]�̑g�����̏�(�����2���������l)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		/* �}�E�X�{�^�� */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 0, _T("�_�u���N���b�N"), F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD }, //Feb. 19, 2001 JEPRO Alt�ƉE�N���b�N�̑g�����͌����Ȃ��̂ŉE�N���b�N���j���[�̃L�[���蓖�Ă��͂�����
		{ 0, _T("�E�N���b�N"), F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_0, F_0, F_0, F_0 },
		{ 0, _T("���N���b�N"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 }, // novice 2004/10/11 �}�E�X���{�^���Ή�
		{ 0, _T("���T�C�h�N���b�N"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 }, // novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
		{ 0, _T("�E�T�C�h�N���b�N"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ 0, _T("�g���v���N���b�N"), F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE },
		{ 0, _T("�N�A�h���v���N���b�N"), F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL },

		/* �t�@���N�V�����L�[ */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, F_0, F_0 },
		{ VK_F2,_T("F2"), F_BOOKMARK_NEXT, F_BOOKMARK_PREV, F_BOOKMARK_SET, F_BOOKMARK_RESET, F_BOOKMARK_VIEW, F_0, F_0, F_0 },
		{ VK_F3,_T("F3"), F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, F_JUMP_SRCHSTARTPOS, F_0, F_0, F_0, F_0 },
		{ VK_F4,_T("F4"), F_SPLIT_V, F_SPLIT_H, F_0, F_FILECLOSE_OPEN, F_0, F_EXITALLEDITORS, F_EXITALL, F_0 },
		{ VK_F5,_T("F5"), F_REDRAW, F_0, F_EXECMD_DIALOG, F_0, F_UUDECODE, F_0, F_TABTOSPACE, F_SPACETOTAB },
		{ VK_F6,_T("F6"), F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, F_0, F_BASE64DECODE, F_0, F_0, F_0 },
		{ VK_F7,_T("F7"), F_CUT, F_0, F_TOUPPER, F_0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, F_0 },
		{ VK_F8,_T("F8"), F_COPY, F_COPY_CRLF, F_TOHANKAKU, F_0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, F_0 },
		{ VK_F9,_T("F9"), F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, F_0, F_CODECNV_UNICODE2SJIS, F_0, F_FILE_REOPEN_UNICODE, F_0 },
		{ VK_F10,_T("F10"), _SQL_RUN, F_DUPLICATELINE, F_TOZENKAKUHIRA, F_0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, F_0 },
		{ VK_F11,_T("F11"), F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKATA, F_0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, F_0 },
		{ VK_F12,_T("F12"), F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENHIRA, F_0, F_CODECNV_AUTO2SJIS, F_0, F_FILE_REOPEN_SJIS, F_0 },
		{ VK_F13,_T("F13"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F14,_T("F14"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F15,_T("F15"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F16,_T("F16"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F17,_T("F17"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F18,_T("F18"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F19,_T("F19"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F20,_T("F20"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F21,_T("F21"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F22,_T("F22"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F23,_T("F23"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F24,_T("F24"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },

		/* ����L�[ */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ VK_TAB,_T("Tab"), F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, F_0, F_0, F_0, F_0 },
			{ VK_RETURN,_T("Enter"), F_0, F_0, F_COMPARE, F_0, F_PROPERTY_FILE, F_0, F_0, F_0 },
		{ VK_ESCAPE,_T("Esc"), F_CANCEL_MODE, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_BACK,_T("BkSp"), F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
		{ VK_INSERT,_T("Ins"), F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
		{ VK_DELETE,_T("Del"), F_DELETE, F_CUT, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		{ VK_HOME,_T("Home"), F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, F_0, F_0, F_0, F_0 },
		{ VK_END,_T("End(Help)"), F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, F_0, F_0, F_0, F_0 },
		{ VK_LEFT,_T("��"), F_LEFT, F_LEFT_SEL, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, F_0, F_0, F_0 },
		{ VK_UP,_T("��"), F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		{ VK_RIGHT,_T("��"), F_RIGHT, F_RIGHT_SEL, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_H },
		{ VK_DOWN,_T("��"), F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
		{ VK_NEXT,_T("PgDn(RollUp)"), F_1PageDown, F_1PageDown_Sel, F_HalfPageDown, F_HalfPageDown_Sel, F_0, F_0, F_0, F_0 },
		{ VK_PRIOR,_T("PgUp(RollDn)"), F_1PageUp, F_1PageUp_Sel, F_HalfPageUp, F_HalfPageUp_Sel, F_0, F_0, F_0, F_0 },
		{ VK_SPACE,_T("Space"), F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },

		/* ���� */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ '0', _T("0"), F_0, F_0, F_0, F_0, F_CUSTMENU_10, F_CUSTMENU_20, F_0, F_0 },
		{ '1', _T("1"), F_0, F_0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, F_0, F_0 },
		{ '2', _T("2"), F_0, F_0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2, F_CUSTMENU_12, F_0, F_0 },
		{ '3', _T("3"), F_0, F_0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, F_0, F_0 },
		{ '4', _T("4"), F_0, F_0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, F_0, F_0 },
		{ '5', _T("5"), F_0, F_0, F_OPTION_TYPE, F_0, F_CUSTMENU_5, F_CUSTMENU_15, F_0, F_0 },
		{ '6', _T("6"), F_0, F_0, F_OPTION, F_0, F_CUSTMENU_6, F_CUSTMENU_16, F_0, F_0 },
		{ '7', _T("7"), F_0, F_0, F_FONT, F_0, F_CUSTMENU_7, F_CUSTMENU_17, F_0, F_0 },
		{ '8', _T("8"), F_0, F_0, F_0, F_0, F_CUSTMENU_8, F_CUSTMENU_18, F_0, F_0 },
		{ '9', _T("9"), F_0, F_0, F_0, F_0, F_CUSTMENU_9, F_CUSTMENU_19, F_0, F_0 },

		/* �A���t�@�x�b�g */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 'A', _T("A"), F_0, F_0, F_SELECTALL, F_0, F_SORT_ASC, F_0, F_0, F_0 },
		{ 'B', _T("B"), F_0, F_0, F_BROWSE, F_0, F_0, F_0, F_0, F_0 },
		{ 'C', _T("C"), F_0, F_0, F_COPY, F_OPEN_HfromtoC, F_0, F_0, F_0, F_0 },
		{ 'D', _T("D"), F_0, F_0, F_WordCut, F_WordDelete, F_SORT_DESC, F_0, F_0, F_0 },
		{ 'E', _T("E"), F_0, F_0, F_CUT_LINE, F_DELETE_LINE, F_0, F_0, F_CASCADE, F_0 },
		{ 'F', _T("F"), F_0, F_0, F_SEARCH_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'G', _T("G"), F_0, F_0, F_GREP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'H', _T("H"), F_0, F_0, F_CURLINECENTER, F_OPEN_HfromtoC, F_0, F_0, F_TILE_V, F_0 },
		{ 'I', _T("I"), F_0, F_0, F_DUPLICATELINE, F_0, F_0, F_0, F_0, F_0 },
		{ 'J', _T("J"), F_0, F_0, F_JUMP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'K', _T("K"), F_0, F_0, F_LineCutToEnd, F_LineDeleteToEnd, F_0, F_0, F_0, F_0 },
		{ 'L', _T("L"), F_0, F_0, F_LOADKEYMACRO, F_EXECKEYMACRO, F_LTRIM, F_0, F_TOLOWER, F_TOUPPER },
		{ 'M', _T("M"), F_0, F_0, F_SAVEKEYMACRO, F_RECKEYMACRO, F_MERGE, F_0, F_0, F_0 },
		{ 'N', _T("N"), F_0, F_0, F_FILENEW, F_0, F_JUMPHIST_NEXT, F_0, F_0, F_0 },
		{ 'O', _T("O"), F_0, F_0, F_FILEOPEN, F_0, F_0, F_0, F_0, F_0 },
		{ 'P', _T("P"), F_0, F_0, F_PRINT, F_PRINT_PREVIEW, F_JUMPHIST_PREV, F_0, F_PRINT_PAGESETUP, F_0 },
		{ 'Q', _T("Q"), F_0, F_0, F_CREATEKEYBINDLIST, F_0, F_0, F_0, F_0, F_0 },
		{ 'R', _T("R"), F_0, F_0, F_REPLACE_DIALOG, F_0, F_RTRIM, F_0, F_0, F_0 },
		{ 'S', _T("S"), F_0, F_0, F_FILESAVE, F_FILESAVEAS_DIALOG, F_0, F_0, F_TMPWRAPSETTING, F_0 },
		{ 'T', _T("T"), F_0, F_0, F_TAGJUMP, F_TAGJUMPBACK, F_0, F_0, F_TILE_H, F_0 },
		{ 'U', _T("U"), F_0, F_0, F_LineCutToStart, F_LineDeleteToStart, F_0, F_0, F_WRAPWINDOWWIDTH, F_0 },
		{ 'V', _T("V"), F_0, F_0, F_PASTE, F_0, F_0, F_0, F_0, F_0 },
		{ 'W', _T("W"), F_0, F_0, F_SELECTWORD, F_0, F_0, F_0, F_TMPWRAPWINDOW, F_0 },
		{ 'X', _T("X"), F_0, F_0, F_CUT, F_0, F_0, F_0, F_TMPWRAPNOWRAP, F_0 },
		{ 'Y', _T("Y"), F_0, F_0, F_REDO, F_0, F_0, F_0, F_0, F_0 },
		{ 'Z', _T("Z"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },

		/* �L�� */
		//keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 0x00bd, _T("-"), F_0, F_0, F_COPYFNAME, F_SPLIT_V, F_0, F_0, F_0, F_0 },
		{ 0x00de, _T("^(�p��')"), F_0, F_0, F_COPYTAG, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00dc, _T("\\"), F_0, F_0, F_COPYPATH, F_SPLIT_H, F_0, F_0, F_0, F_0 },
		{ 0x00c0, _T("@(�p��`)"), F_0, F_0, F_COPYLINES, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00db, _T("["), F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bb, _T(";"), F_0, F_0, F_0, F_SPLIT_VH, F_INS_DATE, F_0, F_0, F_0 },
		{ 0x00ba, _T(":"), F_0, F_0, _COPYWITHLINENUM, F_0, F_INS_TIME, F_0, F_0, F_0 },
		{ 0x00dd, _T("]"), F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bc, _T(","), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00be, _T("."), F_0, F_0, F_COPYLINESASPASSAGE, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bf, _T("/"), F_0, F_0, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00e2, _T("_"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00df, _T("_(PC-98)"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ VK_APPS, _T("�A�v���L�["), F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = _countof( KeyDataInit );
	//	From Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~
	if( nKeyDataInitNum > _countof( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr ) ) {
		PleaseReportToAuthor( NULL, _T("�L�[�ݒ萔�ɑ΂���DLLSHARE::m_nKeyNameArr[]�̃T�C�Y���s�����Ă��܂�") );
		return false;
	}
	//	To Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~
	for( int i = 0; i < nKeyDataInitNum; ++i ){
		SetKeyNameArrVal(
			pShareData,
			i,
			KeyDataInit[i].nKeyCode,
			KeyDataInit[i].pszKeyName,
			KeyDataInit[i].nFuncCode_0,
			KeyDataInit[i].nFuncCode_1,
			KeyDataInit[i].nFuncCode_2,
			KeyDataInit[i].nFuncCode_3,
			KeyDataInit[i].nFuncCode_4,
			KeyDataInit[i].nFuncCode_5,
			KeyDataInit[i].nFuncCode_6,
			KeyDataInit[i].nFuncCode_7
		 );
	}
	pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyDataInitNum;
	return true;
}

/*!	@brief ���L������������/�c�[���o�[

	�c�[���o�[�֘A�̏���������

	@author genta
	@date 2005.01.30 genta CShareData::Init()���番���D
		����ݒ肵�Ȃ��ň�C�Ƀf�[�^�]������悤�ɁD
*/
void CShareData::InitToolButtons(DLLSHAREDATA* pShareData)
{
		/* �c�[���o�[�{�^���\���� */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cpp�łł��邾���n���ƂɏW�܂�悤�ɃA�C�R���̏��Ԃ�啝�ɓ���ւ����̂ɔ����ȉ��̏����ݒ�l��ύX
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	//�V�K�쐬
		25,		//�t�@�C�����J��(DropDown)
		3,		//�㏑���ۑ�		//Sept. 16, 2000 JEPRO 3��11�ɕύX	//Oct. 25, 2000 11��3
		4,		//���O��t���ĕۑ�	//Sept. 19, 2000 JEPRO �ǉ�
		0,

		33,	//���ɖ߂�(Undo)	//Sept. 16, 2000 JEPRO 7��19�ɕύX	//Oct. 25, 2000 19��33
		34,	//��蒼��(Redo)	//Sept. 16, 2000 JEPRO 8��20�ɕύX	//Oct. 25, 2000 20��34
		0,

		87,	//�ړ�����: �O��	//Dec. 24, 2000 JEPRO �ǉ�
		88,	//�ړ�����: ����	//Dec. 24, 2000 JEPRO �ǉ�
		0,

		225,	//����		//Sept. 16, 2000 JEPRO 9��22�ɕύX	//Oct. 25, 2000 22��225
		226,	//��������	//Sept. 16, 2000 JEPRO 16��23�ɕύX	//Oct. 25, 2000 23��226
		227,	//�O������	//Sept. 16, 2000 JEPRO 17��24�ɕύX	//Oct. 25, 2000 24��227
		228,	//�u��		// Oct. 7, 2000 JEPRO �ǉ�
		229,	//�����}�[�N�̃N���A	//Sept. 16, 2000 JEPRO 41��25�ɕύX(Oct. 7, 2000 25��26)	//Oct. 25, 2000 25��229
		230,	//Grep		//Sept. 16, 2000 JEPRO 14��31�ɕύX	//Oct. 25, 2000 31��230
		232,	//�A�E�g���C�����	//Dec. 24, 2000 JEPRO �ǉ�
		0,

		264,	//�^�C�v�ʐݒ�ꗗ	//Sept. 16, 2000 JEPRO �ǉ�
		265,	//�^�C�v�ʐݒ�		//Sept. 16, 2000 JEPRO 18��36�ɕύX	//Oct. 25, 2000 36��265
		266,	//���ʐݒ�			//Sept. 16, 2000 JEPRO 10��37�ɕύX �������u�ݒ�v���p�e�B�V�[�g�v����ύX	//Oct. 25, 2000 37��266
		0,		//Oct. 8, 2000 jepro ���s�̂��߂ɒǉ�
		346,	//�R�}���h�ꗗ	//Oct. 8, 2000 JEPRO �ǉ�
	};

	//	�c�[���o�[�A�C�R�����̍ő�l�𒴂��Ȃ����߂̂��܂��Ȃ�
	//	�ő�l�𒴂��Ē�`���悤�Ƃ���Ƃ����ŃR���p�C���G���[�ɂȂ�܂��D
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1:0 ];
	dummy[0]=0;

	memcpy(
		(void*)pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	/* �c�[���o�[�{�^���̐� */
	pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat = !IsVisualStyle();			/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */	// 2006.06.23 ryoji �r�W���A���X�^�C���ł͏����l���m�[�}���ɂ���
	
}

/*!	@brief ���L������������/�^�C�v�ʐݒ�

	�^�C�v�ʐݒ�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
*/
void CShareData::InitTypeConfigs(DLLSHAREDATA* pShareData)
{
//�L�[���[�h�F�f�t�H���g�J���[�ݒ�
/************************/
/* �^�C�v�ʐݒ�̋K��l */
/************************/
	int nIdx = 0;
	int i;
	pShareData->m_Types[nIdx].m_nTextWrapMethod = WRAP_SETTING_WIDTH;	// �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	pShareData->m_Types[nIdx].m_nMaxLineKetas = MAXLINEKETAS;			/* �܂�Ԃ������� */
	pShareData->m_Types[nIdx].m_nColmSpace = 0;					/* �����ƕ����̌��� */
	pShareData->m_Types[nIdx].m_nLineSpace = 1;					/* �s�Ԃ̂����� */
	pShareData->m_Types[nIdx].m_nTabSpace = 4;					/* TAB�̕����� */
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		pShareData->m_Types[nIdx].m_nKeyWordSetIdx[i] = -1;
	}
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	strcpy( pShareData->m_Types[nIdx].m_szTabViewString, "^       " );	/* TAB�\�������� */
//#endif
	pShareData->m_Types[nIdx].m_bTabArrow = FALSE;				/* �^�u���\�� */	// 2001.12.03 hor
	pShareData->m_Types[nIdx].m_bInsSpace = FALSE;				/* �X�y�[�X�̑}�� */	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI �ȉ��Am_cLineComment��m_cBlockComment���g���悤�ɏC��
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(0, "", -1);	/* �s�R�����g�f���~�^ */
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(1, "", -1);	/* �s�R�����g�f���~�^2 */
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(2, "", -1);	/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
	pShareData->m_Types[nIdx].m_cBlockComments[0].SetBlockCommentRule("", "");	/* �u���b�N�R�����g�f���~�^ */
	pShareData->m_Types[nIdx].m_cBlockComments[1].SetBlockCommentRule("", "");	/* �u���b�N�R�����g�f���~�^2 */

	pShareData->m_Types[nIdx].m_nStringType = 0;					/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[nIdx].m_szIndentChars, _T("") );		/* ���̑��̃C���f���g�Ώە��� */

	pShareData->m_Types[nIdx].m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pShareData->m_Types[nIdx].m_szHokanFile, _T("") );		/* ���͕⊮ �P��t�@�C�� */
	// 2001/06/14 End

	// 2001/06/19 asa-o
	pShareData->m_Types[nIdx].m_bHokanLoHiCase = false;			/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */

	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	pShareData->m_Types[nIdx].m_bUseHokanByFile = false;			/*! ���͕⊮ �J���Ă���t�@�C�����������T�� */

	// �����R�[�h�ݒ�
	pShareData->m_Types[nIdx].m_eDefaultCodetype = CODE_SJIS;
	pShareData->m_Types[nIdx].m_eDefaultEoltype = EOL_CRLF;
	pShareData->m_Types[nIdx].m_bDefaultBom = false;

	//@@@2002.2.4 YAZAKI
	pShareData->m_Types[nIdx].m_szExtHelp[0] = _T('\0');
	pShareData->m_Types[nIdx].m_szExtHtmlHelp[0] = _T('\0');
	pShareData->m_Types[nIdx].m_bHtmlHelpIsSingle = TRUE;

	pShareData->m_Types[nIdx].m_bAutoIndent = TRUE;			/* �I�[�g�C���f���g */
	pShareData->m_Types[nIdx].m_bAutoIndent_ZENSPACE = TRUE;	/* ���{��󔒂��C���f���g */
	pShareData->m_Types[nIdx].m_bRTrimPrevLine = FALSE;			/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */

	pShareData->m_Types[nIdx].m_nIndentLayout = 0;	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */

	static ColorInfoIni ColorInfo_DEFAULT[] = {
	//	Nov. 9, 2000 Jepro note: color setting (�ڍׂ� CshareData.h ���Q�Ƃ̂���)
	//	0,							1(Disp),	 2(FatFont),3(UnderLIne) , 4(colTEXT),	5(colBACK),
	//	szName(���ږ�),				�F�����^�\��, ����,		����,		�����F,		�w�i�F,
	//
	//Oct. 8, 2000 JEPRO �w�i�F��^����RGB(255,255,255)��(255,251,240)�ɕύX(ῂ�������������)
		"�e�L�X�g",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),
		"���[���[",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 ),
		"�J�[�\��",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
		"�J�[�\��(IME ON)",					TRUE , FALSE, FALSE, RGB( 255, 0, 0 )		, RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
		"�J�[�\���s�A���_�[���C��",			TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
 		"�J�[�\���ʒu�c��",					FALSE, FALSE, FALSE, RGB( 128, 128, 255 )	, RGB( 255, 251, 240 ),// 2007.09.09 Moca
		"�s�ԍ�",							TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
		"�s�ԍ�(�ύX�s)",					TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
		"TAB�L��",							TRUE , FALSE, FALSE, RGB( 128, 128, 128 )	, RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)���Z���O���[�ɕύX
		"���p��"		,					FALSE , FALSE, FALSE , RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ), //2002.04.28 Add by KK
		"���{���",						TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ),
		"�R���g���[���R�[�h",				TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 251, 240 ),
		"���s�L��",							TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 251, 240 ),
		"�܂�Ԃ��L��",						TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 251, 240 ),
		"�w�茅�c��",						FALSE, FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ), //2005.11.08 Moca
		"EOF�L��",							TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ),
//#ifdef COMPILE_COLOR_DIGIT
		"���p���l",							FALSE, FALSE, FALSE, RGB( 235, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)��ύX  Mar.10, 2001 �W���͐F�Ȃ���
//#endif
		"����������",						TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
		"�����L�[���[�h1",					TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h2",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
		"�����L�[���[�h3",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
		"�����L�[���[�h4",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h5",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h6",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h7",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h8",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h9",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�����L�[���[�h10",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"�R�����g",							TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 251, 240 ),
	//Sept. 4, 2000 JEPRO �V���O���N�H�[�e�[�V����������ɐF�����蓖�Ă邪�F�����\���͂��Ȃ�
	//Oct. 17, 2000 JEPRO �F�����\������悤�ɕύX(�ŏ���FALSE��TRUE)
	//"�V���O���N�H�[�e�[�V����������", FALSE, FALSE, FALSE, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
		"�V���O���N�H�[�e�[�V����������",	TRUE , FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 251, 240 ),
		"�_�u���N�H�[�e�[�V����������",		TRUE , FALSE, FALSE, RGB( 128, 0, 64 )		, RGB( 255, 251, 240 ),
		"URL",								TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
		"���K�\���L�[���[�h1",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h2",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h3",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h4",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h5",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h6",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h7",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h8",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h9",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"���K�\���L�[���[�h10",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"DIFF�����\��(�ǉ�)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"DIFF�����\��(�ύX)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"DIFF�����\��(�폜)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"�Ί��ʂ̋����\��",			FALSE , TRUE,  FALSE, RGB( 128, 0, 0 )		, RGB( 255, 251, 240 ),	// 02/09/18 ai
		"�u�b�N�}�[�N",				TRUE  , FALSE, FALSE, RGB( 255, 251, 240 )	, RGB( 0, 128, 192 ),	// 02/10/16 ai
	};
//	To Here Sept. 18, 2000


	for( i = 0; i < COLORIDX_LAST; ++i ){
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_nColorIdx		= i;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bDisp			= ColorInfo_DEFAULT[i].m_bDisp;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bFatFont		= ColorInfo_DEFAULT[i].m_bFatFont;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bUnderLine		= ColorInfo_DEFAULT[i].m_bUnderLine;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colTEXT			= ColorInfo_DEFAULT[i].m_colTEXT;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colBACK			= ColorInfo_DEFAULT[i].m_colBACK;
		strcpy( pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ColorInfo_DEFAULT[i].m_pszName );
	}
	pShareData->m_Types[nIdx].m_bLineNumIsCRLF = TRUE;				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	pShareData->m_Types[nIdx].m_nLineTermType = 1;					/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
	pShareData->m_Types[nIdx].m_cLineTermChar = ':';					/* �s�ԍ���؂蕶�� */
	pShareData->m_Types[nIdx].m_bWordWrap = FALSE;					/* �p�����[�h���b�v������ */
	pShareData->m_Types[nIdx].m_nCurrentPrintSetting = 0;				/* ���ݑI�����Ă������ݒ� */
	pShareData->m_Types[nIdx].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
	pShareData->m_Types[nIdx].m_nSmartIndent = SMARTINDENT_NONE;		/* �X�}�[�g�C���f���g��� */
	pShareData->m_Types[nIdx].m_nImeState = IME_CMODE_NOCONVERSION;	/* IME���� */

	pShareData->m_Types[nIdx].m_szOutlineRuleFilename[0] = '\0';	//Dec. 4, 2000 MIK
	pShareData->m_Types[nIdx].m_bKinsokuHead = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[nIdx].m_bKinsokuTail = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[nIdx].m_bKinsokuRet  = FALSE;				/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	pShareData->m_Types[nIdx].m_bKinsokuKuto = FALSE;				/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuHead, "" );		/* �s���֑� */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuTail, "" );		/* �s���֑� */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuKuto, "�A�B�C�D��,." );	/* ��Ǔ_�Ԃ牺������ */	// 2009.08.07 ryoji

	pShareData->m_Types[nIdx].m_bUseDocumentIcon = FALSE;			/* �����Ɋ֘A�Â���ꂽ�A�C�R�����g�� */

//@@@ 2001.11.17 add start MIK
	for(i = 0; i < 100; i++)
	{
		pShareData->m_Types[nIdx].m_RegexKeywordArr[i].m_szKeyword[0] = _T('\0');
		pShareData->m_Types[nIdx].m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pShareData->m_Types[nIdx].m_bUseRegexKeyword = FALSE;
//		pShareData->m_Types[nIdx].m_nRegexKeyMagicNumber = 1;
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	for(i = 0; i < MAX_KEYHELP_FILE; i++){
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_nUse = 0;
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_szPath[0] = _T('\0');
	}
	pShareData->m_Types[nIdx].m_bUseKeyWordHelp = FALSE;	/* �����I���@�\�̎g�p�� */
	pShareData->m_Types[nIdx].m_nKeyHelpNum = 0;			/* �o�^������ */
	pShareData->m_Types[nIdx].m_bUseKeyHelpAllSearch = FALSE;	/* �q�b�g�������̎���������(&A) */
	pShareData->m_Types[nIdx].m_bUseKeyHelpKeyDisp = FALSE;		/* 1�s�ڂɃL�[���[�h���\������(&W) */
	pShareData->m_Types[nIdx].m_bUseKeyHelpPrefix = FALSE;		/* �I��͈͂őO����v����(&P) */
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca �w��ʒu�c���̐ݒ�
	for( i = 0; i < MAX_VERTLINES; i++ ){
		pShareData->m_Types[nIdx].m_nVertLineIdx[i] = 0;
	}

	static char* pszTypeNameArr[] = {
		"��{",
		"�e�L�X�g",
		"C/C++",
		"HTML",
		"PL/SQL",
		"COBOL",
		"Java",
		"�A�Z���u��",
		"AWK",
		"MS-DOS�o�b�`�t�@�C��",
		"Pascal",
		"TeX",				//Oct. 31, 2000 JEPRO TeX  ���[�U�ɑ���
		"Perl",				//Jul. 08, 2001 JEPRO Perl ���[�U�ɑ���
		"Visual Basic",		//JUl. 10, 2001 JEPRO VB   ���[�U�ɑ���
		"���b�`�e�L�X�g",	//JUl. 10, 2001 JEPRO WinHelp���̂ɂ���P����
		"�ݒ�t�@�C��",		//Nov. 9, 2000 JEPRO Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
	};
	static char* pszTypeExts[] = {
		"",
		//Nov. 15, 2000 JEPRO PostScript�t�@�C�����ǂ߂�悤�ɂ���
		//Jan. 12, 2001 JEPRO readme.1st ���ǂ߂�悤�ɂ���
		//Feb. 12, 2001 JEPRO .err �G���[���b�Z�[�W
		//Nov.  6, 2002 genta doc��MS Word�ɏ����Ă�������͊O���i�֘A�Â��h�~�̂��߁j
		//Nov.  6, 2002 genta log ��ǉ�
		"txt,log,1st,err,ps",
		//	Jan. 24, 2004 genta �֘A�Â���D�܂����Ȃ��̂�dsw,dsp,dep,mak�͂͂���
		"c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm",	//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂ߂�悤�ɂ���
			//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++��ǉ�	//Mar. 15, 2001 JEPRO .hm��ǉ�
		"html,htm,shtml,plg",	//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂݍ��߂�悤�ɂ���
		"sql,plsql",
		"cbl,cpy,pco,cob",	//Jun. 04, 2001 JEPRO KENCH���̏����ɏ]���ǉ�
		"java,jav",
		"asm",
		"awk",
		"bat",
		"dpr,pas",
		"tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo",		//Oct. 31, 2000 JEPRO TeX ���[�U�ɑ���	//Mar. 10, 2001 JEPRO �ǉ�
		"cgi,pl,pm",			//Jul. 08, 2001 JEPRO �ǉ�
		"bas,frm,cls,ctl,pag,dob,dsr,vb",	//Jul. 09, 2001 JEPRO �ǉ� //Dec. 16, 2002 MIK�ǉ� // Feb. 19, 2006 genta .vb�ǉ�
		"rtf",					//Jul. 10, 2001 JEPRO �ǉ�
		"ini,inf,cnf,kwd,col",	//Nov. 9, 2000 JEPRO Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
	};

	pShareData->m_Types[0].m_nIdx = 0;
	strcpy( pShareData->m_Types[0].m_szTypeName, pszTypeNameArr[0] );				/* �^�C�v�����F���� */
	strcpy( pShareData->m_Types[0].m_szTypeExts, pszTypeExts[0] );				/* �^�C�v�����F�g���q���X�g */
	for( nIdx = 1; nIdx < MAX_TYPES; ++nIdx ){
		pShareData->m_Types[nIdx] = pShareData->m_Types[0];
		pShareData->m_Types[nIdx].m_nIdx = nIdx;

		//	From Here 2005.02.20 ��� �z�񐔂��ݒ萔��菬�����P�[�X�̍l��
		const char* pszTypeName;
		const char* pszTypeExt;
		char szTypeName[20];
		if(nIdx < (sizeof(pszTypeNameArr)/sizeof(char*)))
			pszTypeName = pszTypeNameArr[nIdx];
		else{
			sprintf( szTypeName, "�ݒ�%d", nIdx + 1 );
			pszTypeName = szTypeName;
		}
		if(nIdx < (sizeof(pszTypeExts)/sizeof(char*)))
			pszTypeExt = pszTypeExts[nIdx];
		else
			pszTypeExt = "";
		strcpy( m_pShareData->m_Types[nIdx].m_szTypeName, pszTypeName );
		strcpy( m_pShareData->m_Types[nIdx].m_szTypeExts, pszTypeExt );
		//	To Here 2005.02.20 ���
	}


	/* ��{ */
	pShareData->m_Types[0].m_nMaxLineKetas = MAXLINEKETAS;			/* �܂�Ԃ������� */
//		pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_UNKNOWN;	/* �A�E�g���C����͕��@ */	//Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
	pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
	//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;

//		nIdx = 0;
	/* �e�L�X�g */
	//From Here Sept. 20, 2000 JEPRO �e�L�X�g�̋K��l��80��120�ɕύX(�s��ꗗ.txt��������x�ǂ݂₷������)
	pShareData->m_Types[1].m_nMaxLineKetas = 120;					/* �܂�Ԃ������� */
	//To Here Sept. 20, 2000
	pShareData->m_Types[1].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
	//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	pShareData->m_Types[1].m_bKinsokuHead = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[1].m_bKinsokuTail = FALSE;				/* �s���֑� */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[1].m_bKinsokuRet  = FALSE;				/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	pShareData->m_Types[1].m_bKinsokuKuto = FALSE;				/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
//		strcpy( pShareData->m_Types[1].m_szKinsokuHead, "!%),.:;?]}�����f�h�񁌁����A�B�X�r�t�v�x�z�l�����������������J�K�T�U�@�B�D�F�H�b�������������E�[�R�S�I���j�C�D�F�G�H�n�p���������������߁�" );		/* �s���֑� */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[1].m_szKinsokuHead, "!%),.:;?]}�����f�h�񁌁����A�B�X�r�t�v�x�z�l�J�K�T�U�E�R�S�I���j�C�D�F�G�H�n�p�����߁�" );		/* �s���֑� */	//@@@ 2002.04.13 MIK
	strcpy( pShareData->m_Types[1].m_szKinsokuTail, "$([{��\\�e�g�q�s�u�w�y�k���i�m�o�����" );		/* �s���֑� */	//@@@ 2002.04.08 MIK
	// pShareData->m_Types[1].m_szKinsokuKuto�i��Ǔ_�Ԃ牺�������j�͂����ł͂Ȃ��S�^�C�v�Ƀf�t�H���g�ݒ�	// 2009.08.07 ryoji
	//Jun. 2, 2009 syat     �N���b�J�u���p�X�p���K�\���L�[���[�h //Jan.17, 2012 syat ��{����e�L�X�g�Ɉړ�
	pShareData->m_Types[1].m_bUseRegexKeyword = TRUE;							// ���K�\���L�[���[�h���g����
	pShareData->m_Types[1].m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	strcpy(pShareData->m_Types[1].m_RegexKeywordArr[0].m_szKeyword,				// ���K�\���L�[���[�h
		   "/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k");					// ""�ŋ��܂ꂽ C:\�`, \\�` �Ƀ}�b�`����p�^�[��
	pShareData->m_Types[1].m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	strcpy(pShareData->m_Types[1].m_RegexKeywordArr[1].m_szKeyword,				// ���K�\���L�[���[�h
		   "/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k");			// C:\�`, \\�` �Ƀ}�b�`����p�^�[��


	// nIdx = 1;
	/* C/C++ */
	pShareData->m_Types[2].m_cLineComment.CopyTo( 0, "//", -1 );			/* �s�R�����g�f���~�^ */
	pShareData->m_Types[2].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );		/* �u���b�N�R�����g�f���~�^ */
	pShareData->m_Types[2].m_cBlockComments[1].SetBlockCommentRule("#if 0", "#endif" );	/* �u���b�N�R�����g�f���~�^2 */	//Jul. 11, 2001 JEPRO
	pShareData->m_Types[2].m_nKeyWordSetIdx[0] = 0;						/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[2].m_nDefaultOutline = OUTLINE_CPP;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[2].m_nSmartIndent = SMARTINDENT_CPP;			/* �X�}�[�g�C���f���g��� */
	//Mar. 10, 2001 JEPRO	���p���l��F�����\��
	pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;
	//	Sep. 21, 2002 genta �Ί��ʂ̋������f�t�H���gON��
	pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	pShareData->m_Types[2].m_bUseHokanByFile = true;			/*! ���͕⊮ �J���Ă���t�@�C�����������T�� */

	/* HTML */
	pShareData->m_Types[3].m_cBlockComments[0].SetBlockCommentRule("<!--", "-->" );	/* �u���b�N�R�����g�f���~�^ */
	pShareData->m_Types[3].m_nStringType = 0;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	pShareData->m_Types[3].m_nKeyWordSetIdx[0] = 1;						/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[3].m_nDefaultOutline = OUTLINE_HTML;			/* �A�E�g���C����͕��@ */
	// Feb. 2, 2005 genta �������̂ŃV���O���N�H�[�g�̐F������HTML�ł͍s��Ȃ�
	pShareData->m_Types[3].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

	// nIdx = 3;
	/* PL/SQL */
	pShareData->m_Types[4].m_cLineComment.CopyTo( 0, "--", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[4].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );	/* �u���b�N�R�����g�f���~�^ */
	pShareData->m_Types[4].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[4].m_szIndentChars, "|��" );			/* ���̑��̃C���f���g�Ώە��� */
	pShareData->m_Types[4].m_nKeyWordSetIdx[0] = 2;						/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[4].m_nDefaultOutline = OUTLINE_PLSQL;			/* �A�E�g���C����͕��@ */

	/* COBOL */
	pShareData->m_Types[5].m_cLineComment.CopyTo( 0, "*", 6 );	//Jun. 02, 2001 JEPRO �C��
	pShareData->m_Types[5].m_cLineComment.CopyTo( 1, "D", 6 );	//Jun. 04, 2001 JEPRO �ǉ�
	pShareData->m_Types[5].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[5].m_szIndentChars, "*" );			/* ���̑��̃C���f���g�Ώە��� */
	pShareData->m_Types[5].m_nKeyWordSetIdx[0] = 3;						/* �L�[���[�h�Z�b�g */		//Jul. 10, 2001 JEPRO
	pShareData->m_Types[5].m_nDefaultOutline = OUTLINE_COBOL;			/* �A�E�g���C����͕��@ */

	// 2005.11.08 Moca �w�茅�c��
	pShareData->m_Types[5].m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = TRUE;
	pShareData->m_Types[5].m_nVertLineIdx[0] = 7;
	pShareData->m_Types[5].m_nVertLineIdx[1] = 8;
	pShareData->m_Types[5].m_nVertLineIdx[2] = 12;
	pShareData->m_Types[5].m_nVertLineIdx[3] = 73;


	/* Java */
	pShareData->m_Types[6].m_cLineComment.CopyTo( 0, "//", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[6].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );	/* �u���b�N�R�����g�f���~�^ */
	pShareData->m_Types[6].m_nKeyWordSetIdx[0] = 4;						/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[6].m_nDefaultOutline = OUTLINE_JAVA;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[6].m_nSmartIndent = SMARTINDENT_CPP;			/* �X�}�[�g�C���f���g��� */
	//Mar. 10, 2001 JEPRO	���p���l��F�����\��
	pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;
	//	Sep. 21, 2002 genta �Ί��ʂ̋������f�t�H���gON��
	pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;

	/* �A�Z���u�� */
	//	2004.05.01 MIK/genta
	pShareData->m_Types[7].m_cLineComment.CopyTo( 0, ";", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[7].m_nDefaultOutline = OUTLINE_ASM;			/* �A�E�g���C����͕��@ */
	//Mar. 10, 2001 JEPRO	���p���l��F�����\��
	pShareData->m_Types[7].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

	/* awk */
	pShareData->m_Types[8].m_cLineComment.CopyTo( 0, "#", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[8].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[8].m_nKeyWordSetIdx[0] = 6;						/* �L�[���[�h�Z�b�g */

	/* MS-DOS�o�b�`�t�@�C�� */
	pShareData->m_Types[9].m_cLineComment.CopyTo( 0, "REM ", -1 );	/* �s�R�����g�f���~�^ */
	pShareData->m_Types[9].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[9].m_nKeyWordSetIdx[0] = 7;						/* �L�[���[�h�Z�b�g */

	/* Pascal */
	pShareData->m_Types[10].m_cLineComment.CopyTo( 0, "//", -1 );		/* �s�R�����g�f���~�^ */		//Nov. 5, 2000 JEPRO �ǉ�
	pShareData->m_Types[10].m_cBlockComments[0].SetBlockCommentRule("{", "}" );	/* �u���b�N�R�����g�f���~�^ */	//Nov. 5, 2000 JEPRO �ǉ�
	pShareData->m_Types[10].m_cBlockComments[1].SetBlockCommentRule("(*", "*)" );	/* �u���b�N�R�����g�f���~�^2 */	//@@@ 2001.03.10 by MIK
	pShareData->m_Types[10].m_nStringType = 1;						/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO �ǉ�
	pShareData->m_Types[10].m_nKeyWordSetIdx[0] = 8;						/* �L�[���[�h�Z�b�g */
	//Mar. 10, 2001 JEPRO	���p���l��F�����\��
	pShareData->m_Types[10].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	//@@@ 2001.11.11 upd MIK

	//From Here Oct. 31, 2000 JEPRO
	/* TeX */
	pShareData->m_Types[11].m_cLineComment.CopyTo( 0, "%", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[11].m_nDefaultOutline = OUTLINE_TEX;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[11].m_nKeyWordSetIdx[0]  = 9;					/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[11].m_nKeyWordSetIdx[1] = 10;					/* �L�[���[�h�Z�b�g2 */	//Jan. 19, 2001 JEPRO
	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//URL�ɃA���_�[���C���������Ȃ�(����ς��߂�)
//		pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
	//To Here Oct. 31, 2000

	//From Here Jul. 08, 2001 JEPRO
	/* Perl */
	pShareData->m_Types[12].m_cLineComment.CopyTo( 0, "#", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[12].m_nDefaultOutline = OUTLINE_PERL;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[12].m_nKeyWordSetIdx[0]  = 11;					/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[12].m_nKeyWordSetIdx[1] = 12;					/* �L�[���[�h�Z�b�g2 */
	pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
	//To Here Jul. 08, 2001
	//	Sep. 21, 2002 genta �Ί��ʂ̋������f�t�H���gON��
	pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;

	//From Here Jul. 10, 2001 JEPRO
	/* Visual Basic */
	pShareData->m_Types[13].m_cLineComment.CopyTo( 0, "'", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[13].m_nDefaultOutline = OUTLINE_VB;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[13].m_nKeyWordSetIdx[0]  = 13;					/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[13].m_nKeyWordSetIdx[1] = 14;					/* �L�[���[�h�Z�b�g2 */
	pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
	pShareData->m_Types[13].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

	/* ���b�`�e�L�X�g */
	pShareData->m_Types[14].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
	pShareData->m_Types[14].m_nKeyWordSetIdx[0]  = 15;					/* �L�[���[�h�Z�b�g */
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
	pShareData->m_Types[14].m_nStringType = 0;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//URL�ɃA���_�[���C���������Ȃ�
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
	//To Here Jul. 10, 2001

	//From Here Nov. 9, 2000 JEPRO
	/* �ݒ�t�@�C�� */
	pShareData->m_Types[15].m_cLineComment.CopyTo( 0, "//", -1 );		/* �s�R�����g�f���~�^ */
	pShareData->m_Types[15].m_cLineComment.CopyTo( 1, ";", -1 );		/* �s�R�����g�f���~�^2 */
	pShareData->m_Types[15].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//To Here Nov. 9, 2000
}

/*!	@brief ���L������������/�|�b�v�A�b�v���j���[

	�|�b�v�A�b�v���j���[�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
*/
void CShareData::InitPopupMenu(DLLSHAREDATA* pShareData)
{
	/* �J�X�^�����j���[ �K��l */
	
	CommonSetting_CustomMenu& rMenu = m_pShareData->m_Common.m_sCustomMenu;

	/* �E�N���b�N���j���[ */
	int n = 0;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_UNDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'U';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_REDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'R';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_CUT;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'T';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'P';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_DELETE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'D';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO �uCRLF���s�ŃR�s�[�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'H';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO �u��`�\��t���v�𕜊�
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'A';
	n++;

	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 3, 2000 JEPRO �ȉ��Ɂu�^�O�W�����v�v�Ɓu�^�O�W�����v�o�b�N�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO �u�R�s�[�v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(T��G)
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'B';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 15, 2000 JEPRO �ȉ��Ɂu�I��͈͓��S�s�R�s�[�v�Ɓu���p���t���R�s�[�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '@';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '.';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\\';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO �u��蒼���v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(R��F)
	n++;
	rMenu.m_nCustMenuItemNumArr[0] = n;

	/* �J�X�^�����j���[�P */
	rMenu.m_nCustMenuItemNumArr[1] = 7;
	rMenu.m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	rMenu.m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (F��O)
	rMenu.m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [1][1] = 'S';
	rMenu.m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	rMenu.m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (O��N)
	rMenu.m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	rMenu.m_nCustMenuItemKeyArr [1][3] = 'L';
	rMenu.m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	rMenu.m_nCustMenuItemKeyArr [1][4] = 'U';
	rMenu.m_nCustMenuItemFuncArr[1][5] = F_0;
	rMenu.m_nCustMenuItemKeyArr [1][5] = '\0';
	rMenu.m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [1][6] = 'C';

	/* �^�u���j���[ */	//@@@ 2003.06.14 MIK
	n = 0;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';	// 2007.06.26 ryoji B -> R
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_GROUPCLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'G';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSEOTHER;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'O';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSELEFT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'H';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSERIGHT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'M';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	n++;
	rMenu.m_nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}

/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
BOOL IsSakuraMainWindow( HWND hWnd )
{
	TCHAR	szClassName[64];
	if( hWnd == NULL ){	// 2007.06.20 ryoji �����ǉ�
		return FALSE;
	}
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, _countof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

/*[EOF]*/
