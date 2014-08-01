/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon, maru
	Copyright (C) 2007, ryoji, maru, genta, Moca, nasukoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji, ryoji, genta, salarm
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CProfile.h"

//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "CMRUFile.h"
#include "CMRUFolder.h"

//20020129 aroka
#include "Funccode.h"
#include "CMemory.h"

#include "CMutex.h"	// 2007.07.07 genta

#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

#include "maxdata.h"	//@@@ 2002.09.22 YAZAKI

#include "CEol.h"

#include "EditInfo.h"

//! �ҏW�E�B���h�E�m�[�h
struct EditNode {
	int				m_nIndex;
	int				m_nGroup;					//!< �O���[�vID								//@@@ 2007.06.20 ryoji
	HWND			m_hWnd;
	int				m_nId;						//!< ����Id
	char			m_szTabCaption[_MAX_PATH];	//!< �^�u�E�C���h�E�p�F�L���v�V������		//@@@ 2003.05.31 MIK
	char			m_szFilePath[_MAX_PATH];	//!< �^�u�E�C���h�E�p�F�t�@�C����			//@@@ 2006.01.28 ryoji
	bool			m_bIsGrep;					//!< Grep�̃E�B���h�E��						//@@@ 2006.01.28 ryoji
	UINT			m_showCmdRestore;			//!< ���̃T�C�Y�ɖ߂��Ƃ��̃T�C�Y���		//@@@ 2007.06.20 ryoji
	BOOL			m_bClosing;					//!< �I�������i�u�Ō�̃t�@�C������Ă�(����)���c���v�p�j	//@@@ 2007.06.20 ryoji
};

//! �g���\����
struct EditNodeEx{
	EditNode*	p;			//!< �ҏW�E�B���h�E�z��v�f�ւ̃|�C���^
	int			nGroupMru;	//!< �O���[�v�P�ʂ�MRU�ԍ�
};

//! �t�H���g����
struct SFontAttr{
	bool		m_bBoldFont;		//!< ����
	bool		m_bUnderLine;		//!< ����
};

//! �F����
struct SColorAttr{
	COLORREF	m_cTEXT;			//!< �����F
	COLORREF	m_cBACK;			//!< �w�i�F
};

//! �F�ݒ�
struct ColorInfo {
	int			m_nColorIdx;
	bool		m_bDisp;			//!< �F����/�\�� ������
	SFontAttr	m_sFontAttr;		//!< �t�H���g����
	SColorAttr	m_sColorAttr;		//!< �F����
	char		m_szName[32];		//!< ���O
};

//! �F�ݒ�(�ۑ��p)
struct ColorInfoIni {
	const TCHAR*	m_pszName;			//!< �F��
	bool			m_bDisp;			//!< �F����/�\�� ������
	SFontAttr		m_sFontAttr;		//!< �t�H���g����
	SColorAttr		m_sColorAttr;		//!< �F����
};

//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	char	m_szKeyword[100];	//���K�\���L�[���[�h
	int	m_nColorIndex;		//�F�w��ԍ�
};
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< �����̐����̍ő咷 -1 */
struct KeyHelpInfo {
	int		m_nUse;						/*!< ������ �g�p����/���Ȃ� */
	char	m_szAbout[DICT_ABOUT_LEN];	/*!< �����̐���(�����t�@�C����1�s�ڂ��琶��) */
	char	m_szPath[_MAX_PATH];		/*!< �t�@�C���p�X */
};
//@@@ 2006.04.10 fon ADD-end

//! �A�E�g���C����͂̎��
enum EOutlineType {
	OUTLINE_C,
	OUTLINE_CPP,
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,		//	Sep. 8, 2000 genta
	OUTLINE_VB,			//	June 23, 2001 N.Nakatani
	OUTLINE_WZTXT,		// 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
	OUTLINE_HTML,		// 2003.05.20 zenryaku HTML�A�E�g���C�����
	OUTLINE_TEX,		// 2003.07.20 naoh TeX�A�E�g���C�����
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ���[���t�@�C���p
	OUTLINE_PYTHON,		//	2007.02.08 genta Python�A�E�g���C�����
	OUTLINE_ERLANG,		//	2009.08.11 genta Erlang�A�E�g���C�����
	//	�V�����A�E�g���C����͕͂K�����̒��O�֑}��
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99,
	OUTLINE_TREE = 100,		// �ėp�c���[ 2010.03.28 syat
	OUTLINE_CLSTREE = 200,	// �ėp�c���[(�N���X) 2010.03.28 syat
	OUTLINE_LIST = 300,		// �ėp���X�g 2010.03.28 syat
};

//! �X�}�[�g�C���f���g���
enum ESmartIndentType {
	SMARTINDENT_NONE,		//!< �Ȃ�
	SMARTINDENT_CPP			//!< C/C++
};

//! �G���R�[�h�I�v�V����
struct SEncodingConfig{
	ECodeType			m_eDefaultCodetype;				//!< �f�t�H���g�����R�[�h
	EEolType			m_eDefaultEoltype;				//!< �f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	bool				m_bDefaultBom;					//!< �f�t�H���gBOM			// 2011.01.24 ryoji
};

//! �^�C�v�ʐݒ�
struct STypeConfig {
	//2007.09.07 �ϐ����ύX: m_nMaxLineSize��m_nMaxLineKetas
	int					m_nIdx;
	char				m_szTypeName[64];				//!< �^�C�v�����F����
	char				m_szTypeExts[MAX_TYPES_EXTS];	//!< �^�C�v�����F�g���q���X�g
	int					m_nTextWrapMethod;				//!< �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	int					m_nMaxLineKetas;				//!< �܂�Ԃ�������
	int					m_nColumnSpace;					//!< �����ƕ����̌���
	int					m_nLineSpace;					//!< �s�Ԃ̂�����
	int					m_nTabSpace;					//!< TAB�̕�����
	BOOL				m_bTabArrow;					//!< �^�u���\��		//@@@ 2003.03.26 MIK
	char				m_szTabViewString[8+1];			//!< TAB�\��������	// 2003.1.26 aroka �T�C�Y�g��	// 2009.02.11 ryoji �T�C�Y�߂�(17->8+1)
	int					m_bInsSpace;					//!< �X�y�[�X�̑}��	// 2001.12.03 hor
	// 2005.01.13 MIK �z��
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	//!< �L�[���[�h�Z�b�g

	CLineComment		m_cLineComment;					//!< �s�R�����g�f���~�^			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComments[2];			//!< �u���b�N�R�����g�f���~�^	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					//!< �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	char				m_szIndentChars[64];			//!< ���̑��̃C���f���g�Ώە���

	int					m_nColorInfoArrNum;				//!< �F�ݒ�z��̗L����
	ColorInfo			m_ColorInfoArr[64];				//!< �F�ݒ�z��

	bool				m_bLineNumIsCRLF;				//!< �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	int					m_nLineTermType;				//!< �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C��
	char				m_cLineTermChar;				//!< �s�ԍ���؂蕶��
	int					m_nVertLineIdx[MAX_VERTLINES];	//!< �w�茅�c��

	BOOL				m_bWordWrap;					//!< �p�����[�h���b�v������
	bool				m_bKinsokuHead;					//!< �s���֑�������		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuTail;					//!< �s���֑�������		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuRet;					//!< ���s�����̂Ԃ牺��	//@@@ 2002.04.13 MIK
	bool				m_bKinsokuKuto;					//!< ��Ǔ_�̂Ԃ炳��	//@@@ 2002.04.17 MIK
	char				m_szKinsokuHead[200];			//!< �s���֑�����	//@@@ 2002.04.08 MIK
	char				m_szKinsokuTail[200];			//!< �s���֑�����	//@@@ 2002.04.08 MIK
	char				m_szKinsokuKuto[200];			//!< ��Ǔ_�Ԃ炳������	// 2009.08.07 ryoji

	int					m_nCurrentPrintSetting;			//!< ���ݑI�����Ă������ݒ�

	int					m_nDefaultOutline;				//!< �A�E�g���C����͕��@
	char				m_szOutlineRuleFilename[_MAX_PATH];	//!< �A�E�g���C����̓��[���t�@�C��

	int					m_nSmartIndent;					//!< �X�}�[�g�C���f���g���
	int					m_nImeState;					//!< ����IME���	Nov. 20, 2000 genta

	//	2001/06/14 asa-o �⊮�̃^�C�v�ʐݒ�
	char				m_szHokanFile[_MAX_PATH];		//!< ���͕⊮ �P��t�@�C��
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	bool				m_bUseHokanByFile;				//!< ���͕⊮ �J���Ă���t�@�C�����������T��
	//	2001/06/19 asa-o
	bool				m_bHokanLoHiCase;				//!< ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����

	char				m_szExtHelp[_MAX_PATH];			//!< �O���w���v�P
	char				m_szExtHtmlHelp[_MAX_PATH];		//!< �O��HTML�w���v
	bool				m_bHtmlHelpIsSingle;			//!< HtmlHelp�r���[�A�͂ЂƂ�

	SEncodingConfig		m_encoding;						//!< �G���R�[�h�I�v�V����


//@@@ 2001.11.17 add start MIK
	bool				m_bUseRegexKeyword;								//!< ���K�\���L�[���[�h���g����
	int					m_nRegexKeyMagicNumber;							//!< ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[
	RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];			//!< ���K�\���L�[���[�h
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				m_bUseKeyWordHelp;				//!< �L�[���[�h�����Z���N�g�@�\���g����
	int					m_nKeyHelpNum;					//!< �L�[���[�h�����̍���
	KeyHelpInfo			m_KeyHelpArr[MAX_KEYHELP_FILE];	//!< �L�[���[�h�����t�@�C��
	bool				m_bUseKeyHelpAllSearch;			//!< �q�b�g�������̎���������(&A)
	bool				m_bUseKeyHelpKeyDisp;			//!< 1�s�ڂɃL�[���[�h���\������(&W)
	bool				m_bUseKeyHelpPrefix;			//!< �I��͈͂őO����v����(&P)
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Common����ړ��B
	bool				m_bAutoIndent;					//!< �I�[�g�C���f���g
	bool				m_bAutoIndent_ZENSPACE;			//!< ���{��󔒂��C���f���g
	bool				m_bRTrimPrevLine;				//!< 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜
	int					m_nIndentLayout;				//!< �܂�Ԃ���2�s�ڈȍ~���������\��

	//	Sep. 10, 2002 genta
	bool				m_bUseDocumentIcon;				//!< �t�@�C���Ɋ֘A�Â���ꂽ�A�C�R�����g��

	STypeConfig()
	: m_nMaxLineKetas(10) //	��ʐ܂�Ԃ�����TAB���ȉ��ɂȂ�Ȃ����Ƃ������l�ł��ۏ؂���	//	2004.04.03 Moca
	{
	}

}; /* STypeConfig */

// Apr. 05, 2003 genta WindowCaption�p�̈�i�ϊ��O�j�̒���
static const int MAX_CAPTION_CONF_LEN = 256;

static const int MAX_DATETIMEFOREMAT_LEN	= 100;
static const int MAX_CUSTOM_MENU			=  25;
static const int MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const int MAX_CUSTOM_MENU_ITEMS		=  48;
static const int MAX_TOOLBAR_BUTTON_ITEMS	= 384;	//�c�[���o�[�ɓo�^�\�ȃ{�^���ő吔
static const int MAX_TOOLBAR_ICON_X			=  32;	//�A�C�R��BMP�̌���
static const int MAX_TOOLBAR_ICON_Y			=  13;	//�A�C�R��BMP�̒i��
static const int MAX_TOOLBAR_ICON_COUNT		= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =416
//Oct. 22, 2000 JEPRO �A�C�R���̍ő�o�^����128���₵��(256��384)	
//2010/6/9 Uchi �A�C�R���̍ő�o�^����32���₵��(384��416)

//! �}�N�����
struct MacroRec {
	char	m_szName[MACRONAME_MAX];	//!< �\����
	char	m_szFile[_MAX_PATH+1];		//!< �t�@�C����(�f�B���N�g�����܂܂Ȃ�)
	BOOL	m_bReloadWhenExecute;		//!< ���s���ɓǂݍ��݂Ȃ������i�f�t�H���gon�j
	
	bool IsEnabled() const { return m_szFile[0] != '\0'; }
	const char* GetTitle() const { return m_szName[0] == '\0' ? m_szFile: m_szName; }	// 2007.11.02 ryoji �ǉ�
};
//	To Here Sep. 14, 2001 genta

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
//! �^�O�W�����v���
struct TagJump {
	HWND	hwndReferer;				//!< �Q�ƌ��E�B���h�E
	POINT	point;						//!< ���C��, �J����
};

//	Aug. 15, 2000 genta
//	Backup Flags
const int BKUP_YEAR		= 32;
const int BKUP_MONTH	= 16;
const int BKUP_DAY		= 8;
const int BKUP_HOUR		= 4;
const int BKUP_MIN		= 2;
const int BKUP_SEC		= 1;

//	Aug. 21, 2000 genta
const int BKUP_AUTO		= 64;

//	2004.05.13 Moca
//! �E�B���h�E�T�C�Y�E�ʒu�̐�����@
enum EWinSizeMode{
	WINSIZEMODE_DEF = 0,	//!< �w��Ȃ�
	WINSIZEMODE_SAVE = 1,	//!< �p��(�ۑ�)
	WINSIZEMODE_SET = 2		//!< ���ڎw��(�Œ�)
};

// ���łƈႢ�Abool�^�g����悤�ɂ��Ă���܂� by kobake

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �S��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_General
{
	//	Jul. 3, 2000 genta
	//	�A�N�Z�X�֐�(�Ȉ�)
	//	int���r�b�g�P�ʂɕ������Ďg��
	//	��4bit��CaretType�ɓ��ĂĂ���(�����̗\��ő��߂Ɏ���Ă���)
	int		GetCaretType(void) const { return m_nCaretType & 0xf; }
	void	SetCaretType(const int f){ m_nCaretType &= ~0xf; m_nCaretType |= f & 0xf; }

	//�J�[�\��
	int					m_nCaretType;							//!< �J�[�\���̃^�C�v 0=win 1=dos
	bool				m_bIsINSMode;							//!< �}���^�㏑�����[�h
	bool				m_bIsFreeCursorMode;					//!< �t���[�J�[�\�����[�h��
	BOOL				m_bStopsBothEndsWhenSearchWord;			//!< �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
	BOOL				m_bStopsBothEndsWhenSearchParagraph;	//!< �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩
	BOOL				m_bNoCaretMoveByActivation;				//!< �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji (add by genta)

	//�X�N���[��
	int					m_nRepeatedScrollLineNum;		//!< �L�[���s�[�g���̃X�N���[���s��
	BOOL				m_nRepeatedScroll_Smooth;		//!< �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	int					m_nPageScrollByWheel;			//!< �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���Ńy�[�WUP/DOWN����	// 2009.01.12 nasukoji
	int					m_nHorizontalScrollByWheel;		//!< �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������		// 2009.01.12 nasukoji

	//�^�X�N�g���C
	BOOL				m_bUseTaskTray;					//!< �^�X�N�g���C�̃A�C�R�����g��
	BOOL				m_bStayTaskTray;				//!< �^�X�N�g���C�̃A�C�R�����풓
	WORD				m_wTrayMenuHotKeyCode;			//!< �^�X�N�g���C���N���b�N���j���[ �L�[
	WORD				m_wTrayMenuHotKeyMods;			//!< �^�X�N�g���C���N���b�N���j���[ �L�[

	//����
	int					m_nMRUArrNum_MAX;				//!< �t�@�C���̗���MAX
	int					m_nOPENFOLDERArrNum_MAX;		//!< �t�H���_�̗���MAX

	//�m�[�J�e�S��
	BOOL				m_bCloseAllConfirm;				//!< [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����	// 2006.12.25 ryoji
	BOOL				m_bExitConfirm;					//!< �I�����̊m�F������

	//INI���ݒ�̂�
	BOOL				m_bDispExitingDialog;			//!< �I���_�C�A���O��\������
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �E�B���h�E                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Window
{
	//��{�ݒ�
	BOOL				m_bDispTOOLBAR;					//!< ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
	BOOL				m_bDispSTATUSBAR;				//!< ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������
	BOOL				m_bDispFUNCKEYWND;				//!< ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
	BOOL				m_bMenuIcon;					//!< ���j���[�ɃA�C�R����\������
	BOOL				m_bMenuWChar;					//!< ���j���[�̎������΍���s��(Win2K�ȍ~�̂�)
	BOOL				m_bScrollBarHorz;				//!< �����X�N���[���o�[���g��
	BOOL				m_bUseCompatibleBMP;			//!< �č��p�݊��r�b�g�}�b�v���g�� 2007.09.09 Moca

	//�ʒu�Ƒ傫���̐ݒ�
	EWinSizeMode		m_eSaveWindowSize;				//!< �E�B���h�E�T�C�Y�p���E�Œ� EWinSizeMode�ɏ����� 2004.05.13 Moca
	int					m_nWinSizeType;					//!< �傫���̎w��
	int					m_nWinSizeCX;					//!< ���ڎw�� ��
	int					m_nWinSizeCY;					//!< ���ڎw�� ����
	EWinSizeMode		m_eSaveWindowPos;				//!< �E�B���h�E�ʒu�p���E�Œ� EWinSizeMode�ɏ����� 2004.05.13 Moca
	int					m_nWinPosX;						//!< ���ڎw�� X���W
	int					m_nWinPosY;						//!< ���ڎw�� Y���W

	//�t�@���N�V�����L�[
	int					m_nFUNCKEYWND_Place;			//!< �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	int					m_nFUNCKEYWND_GroupNum;			//!< 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

	//���[���[�E�s�ԍ�
	int					m_nRulerHeight;					//!< ���[���[����
	int					m_nRulerBottomSpace;			//!< ���[���[�ƃe�L�X�g�̌���
	int					m_nRulerType;					//!< ���[���[�̃^�C�v
	int					m_nLineNumRightSpace;			//!< �s�ԍ��̉E�̃X�y�[�X Sep. 18, 2002 genta

	//�����E�B���h�E
	BOOL				m_bSplitterWndHScroll;			//!< �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o
	BOOL				m_bSplitterWndVScroll;			//!< �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o

	//�^�C�g���o�[
	char				m_szWindowCaptionActive[MAX_CAPTION_CONF_LEN];		//!< �^�C�g���o�[(�A�N�e�B�u��)
	char				m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];	//!< �^�C�g���o�[(��A�N�e�B�u��)

	//INI���ݒ�̂�
	int					m_nVertLineOffset;				//!< �c���̕`����W�I�t�Z�b�g 2005.11.10 Moca
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �^�u�o�[                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_TabBar
{
	BOOL				m_bDispTabWnd;					//!< �^�u�E�C���h�E�\������	//@@@ 2003.05.31 MIK
	BOOL				m_bDispTabWndMultiWin;			//!< �^�u���܂Ƃ߂Ȃ�	//@@@ 2003.05.31 MIK
	BOOL				m_bTab_RetainEmptyWin;			//!< �Ō�̕���������ꂽ�Ƃ�(����)���c��
	BOOL				m_bTab_CloseOneWin;				//!< �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���
	BOOL				m_bNewWindow;					//!< �O������N������Ƃ��͐V�����E�C���h�E�ŊJ��

	char				m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//!< �^�u�E�C���h�E�L���v�V����	//@@@ 2003.06.13 MIK
	BOOL				m_bSameTabWidth;				//!< �^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
	BOOL				m_bDispTabIcon;					//!< �^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
	BOOL				m_bSortTabList;					//!< �^�u�ꗗ���\�[�g����	//@@@ 2006.03.23 fon
	BOOL				m_bTab_ListFull;				//!< �^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji

	BOOL				m_bChgWndByWheel;				//!< �}�E�X�z�C�[���ŃE�B���h�E�؂�ւ�	//@@@ 2006.03.26 ryoji

	LOGFONT				m_lf;							//!< �^�u�t�H���g // 2011.12.01 Moca
	INT					m_nPointSize;					//!< �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ҏW                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! �t�@�C���_�C�A���O�̏����ʒu
enum EOpenDialogDir{
	OPENDIALOGDIR_CUR, //!< �J�����g�t�H���_
	OPENDIALOGDIR_MRU, //!< �ŋߎg�����t�H���_
	OPENDIALOGDIR_SEL, //!< �w��t�H���_
};

struct CommonSetting_Edit
{
	//�R�s�[
	bool				m_bAddCRLFWhenCopy;			//!< �܂�Ԃ��s�ɉ��s��t���ăR�s�[
	BOOL				m_bEnableNoSelectCopy;		//!< �I���Ȃ��ŃR�s�[���\�ɂ���	2007.11.18 ryoji
	BOOL				m_bCopyAndDisablSelection;	//!< �R�s�[������I������
	bool				m_bEnableLineModePaste;		//!< ���C�����[�h�\��t�����\�ɂ��� 2007.10.08 ryoji
	bool				m_bConvertEOLPaste;			//!< ���s�R�[�h��ϊ����ē\��t����  2009.2.28 salarm

	//�h���b�O���h���b�v
	BOOL				m_bUseOLE_DragDrop;			//!< OLE�ɂ��h���b�O & �h���b�v���g��
	BOOL				m_bUseOLE_DropSource;		//!< OLE�ɂ��h���b�O���ɂ��邩

	//�㏑�����[�h
	BOOL				m_bNotOverWriteCRLF;		//!< ���s�͏㏑�����Ȃ�

	//�N���b�J�u��URL
	BOOL				m_bJumpSingleClickURL;		//!< URL�̃V���O���N���b�N��Jump // ���g�p
	BOOL				m_bSelectClickedURL;		//!< URL���N���b�N���ꂽ��I�����邩

	EOpenDialogDir		m_eOpenDialogDir;			//!< �t�@�C���_�C�A���O�̏����ʒu
	TCHAR				m_OpenDialogSelDir[_MAX_PATH];	//!< �w��t�H���_

	// (�_�C�A���O���ږ���)
	BOOL				m_bAutoColumnPaste;			//!< ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
};

//! �t�@�C���̔r�����䃂�[�h  2007.10.11 kobake �쐬
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< �r�����䂵�Ȃ�
	SHAREMODE_DENY_WRITE,		//!< ���v���Z�X����̏㏑�����֎~
	SHAREMODE_DENY_READWRITE,	//!< ���v���Z�X����̓ǂݏ������֎~
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t�@�C��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File
{
	// �J�[�\���ʒu�𕜌����邩�ǂ���  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return m_bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ m_bRestoreCurPosition = i; } // ���g�p

	// �u�b�N�}�[�N�𕜌����邩�ǂ���  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return m_bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ m_bRestoreBookmarks = i; } // ���g�p

	// �t�@�C���ǂݍ��ݎ���MIME��decode���s����  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return m_bAutoMIMEdecode; }
	void	SetAutoMIMEdecode(bool i)			{ m_bAutoMIMEdecode = i; } // ���g�p

	// �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return m_bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ m_bQueryIfCodeChange = i; } // ���g�p
	
	// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return m_bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ m_bAlertIfFileNotExist = i; } // ���g�p

	//�t�@�C���̔r������
	EShareMode			m_nFileShareMode;		//!< �t�@�C���̔r�����䃂�[�h
	bool				m_bCheckFileTimeStamp;	//!< �X�V�̊Ď�

	//�t�@�C���̕ۑ�
	bool				m_bEnableUnmodifiedOverwrite;	//!< ���ύX�ł��㏑�����邩

	//�u���O��t���ĕۑ��v�Ńt�@�C���̎�ނ�[���[�U�[�w��]�̂Ƃ��̃t�@�C���ꗗ�\��
	//�t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	bool				m_bNoFilterSaveNew;		//!< �V�K����ۑ����͑S�t�@�C���\��
	bool				m_bNoFilterSaveFile;	//!< �V�K�ȊO����ۑ����͑S�t�@�C���\��

	//�t�@�C���I�[�v��
	bool				m_bDropFileAndClose;	//!< �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	int					m_nDropFileNumMax;		//!< ��x�Ƀh���b�v�\�ȃt�@�C����
	bool				m_bRestoreCurPosition;	//!< �t�@�C�����J�����Ƃ��J�[�\���ʒu�𕜌����邩
	bool				m_bRestoreBookmarks;	//!< �u�b�N�}�[�N�𕜌����邩�ǂ��� 2002.01.16 hor
	bool				m_bAutoMIMEdecode;		//!< �t�@�C���ǂݍ��ݎ���MIME��decode���s����
	bool				m_bQueryIfCodeChange;	//!< �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s�� Oct. 03, 2004 genta 
	bool				m_bAlertIfFileNotExist;	//!< �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������ Oct. 09, 2004 genta
	bool				m_bAlertIfLargeFile;    //!< �J�����Ƃ����t�@�C���T�C�Y���傫���ꍇ�Ɍx������
	int 				m_nAlertFileSize;       //!< �x�����n�߂�t�@�C���T�C�Y(MB)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �o�b�N�A�b�v                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Backup
{
	//	Aug. 15, 2000 genta
	//	Backup�ݒ�̃A�N�Z�X�֐�
	int		GetBackupType(void) const { return m_nBackUpType; }
	void	SetBackupType(int n){ m_nBackUpType = n; }

	bool	GetBackupOpt(int flag) const { return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(int flag, bool value){ m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 ); }

	//	�o�b�N�A�b�v��
	int		GetBackupCount(void) const { return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value){ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff ); }

	//	�o�b�N�A�b�v�̊g���q�擪����(1����)
	int		GetBackupExtChar(void) const { return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value){ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 ); } // ���g�p

	//	Aug. 21, 2000 genta
	//	����Backup
	bool	IsAutoBackupEnabled(void) const { return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag){ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const { return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i){ m_nBackUpType_Opt3 = i; }

	//	Backup�ڍאݒ�̃A�N�Z�X�֐�
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

	bool				m_bBackUp;							//!< �ۑ����Ƀo�b�N�A�b�v���쐬����
	bool				m_bBackUpDialog;					//!< �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	bool				m_bBackUpFolder;					//!< �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
	bool				m_bBackUpFolderRM;					//!< �w��t�H���_�Ƀo�b�N�A�b�v���쐬����(�����[�o�u�����f�B�A�̂�)
	char				m_szBackUpFolder[_MAX_PATH];		//!< �o�b�N�A�b�v���쐬����t�H���_
	int 				m_nBackUpType;						//!< �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	int 				m_nBackUpType_Opt1;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����1
	int 				m_nBackUpType_Opt2;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����2
	int 				m_nBackUpType_Opt3;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����3
	int 				m_nBackUpType_Opt4;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����4
	int 				m_nBackUpType_Opt5;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����5
	int 				m_nBackUpType_Opt6;					//!< �o�b�N�A�b�v�t�@�C�����F�I�v�V����6
	bool				m_bBackUpDustBox;					//!< �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	bool				m_bBackUpPathAdvanced;				//!< �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� 20051107 aroka 
	char				m_szBackUpPathAdvanced[_MAX_PATH];	//!< �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� 20051107 aroka 
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format
{
	//���t����
	int					m_nDateFormatType;							//!< ���t�����̃^�C�v
	TCHAR				m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];	//!< ���t����

	//��������
	int					m_nTimeFormatType;							//!< ���������̃^�C�v
	TCHAR				m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];	//!< ��������

	//���o���L��
	char				m_szMidashiKigou[256];						//!< ���o���L��

	//���p��
	char				m_szInyouKigou[32];							//!< ���p��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search
{
	SSearchOption		m_sSearchOption;				//!< �����^�u��  ����
	int					m_bConsecutiveAll;				//!< �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	int					m_bNOTIFYNOTFOUND;				//!< �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	int					m_bSelectedArea;				//!< �u��  �I��͈͓��u��

	int					m_bGrepSubFolder;				//!< Grep: �T�u�t�H���_������
	BOOL				m_bGrepOutputLine;				//!< Grep: �s���o�͂��邩�Y�����������o�͂��邩
	int					m_nGrepOutputStyle;				//!< Grep: �o�͌`��
	int					m_bGrepDefaultFolder;			//!< Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	ECodeType			m_nGrepCharSet;					//!< Grep: �����R�[�h�Z�b�g // 2002/09/20 Moca Add
	BOOL				m_bCaretTextForSearch;			//!< �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji
	TCHAR				m_szRegexpLib[_MAX_PATH];		//!< �g�p���鐳�K�\��DLL 2007.08.22 genta

	//Grep
	BOOL				m_bGrepExitConfirm;				//!< Grep���[�h�ŕۑ��m�F���邩
	BOOL				m_bGrepRealTimeView;			//!< Grep���ʂ̃��A���^�C���\�� 2003.06.16 Moca

	BOOL				m_bGTJW_RETURN;					//!< �G���^�[�L�[�Ń^�O�W�����v
	BOOL				m_bGTJW_LDBLCLK;				//!< �_�u���N���b�N�Ń^�O�W�����v

	//�����E�u���_�C�A���O
	BOOL				m_bAutoCloseDlgFind;			//!< �����_�C�A���O�������I�ɕ���
	BOOL				m_bAutoCloseDlgReplace;			//!< �u�� �_�C�A���O�������I�ɕ���
	BOOL				m_bSearchAll;					//!< �擪�i�����j����Č��� 2002.01.26 hor

	//INI���ݒ�̂�
	BOOL				m_bUseCaretKeyWord;				//!< �L�����b�g�ʒu�̒P�����������		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �L�[���蓖��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_KeyBind
{
	// �L�[���蓖��
	int					m_nKeyNameArrNum;			//!< �L�[���蓖�ĕ\�̗L���f�[�^��
	KEYDATA				m_pKeyNameArr[100];			//!< �L�[���蓖�ĕ\
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu
{
	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �c�[���o�[                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar
{
	int					m_nToolBarButtonNum;								//!< �c�[���o�[�{�^���̐�
	int					m_nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	//!< �c�[���o�[�{�^���\����
	int					m_bToolBarIsFlat;									//!< �t���b�g�c�[���o�[�ɂ���^���Ȃ�
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����L�[���[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword
{
	// �����L�[���[�h�ݒ�
	CKeyWordSetMgr		m_CKeyWordSetMgr;					//!< �����L�[���[�h
	char				m_szKeyWordSetDir[MAX_PATH];		//!< �����L�[���[�h�t�@�C���̃f�B���N�g�� //���g�p
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �x��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper
{
	//���͕⊮�@�\
	BOOL				m_bHokanKey_RETURN;				//!< VK_RETURN	�⊮����L�[���L��/����
	BOOL				m_bHokanKey_TAB;				//!< VK_TAB		�⊮����L�[���L��/����
	BOOL				m_bHokanKey_RIGHT;				//!< VK_RIGHT	�⊮����L�[���L��/����
	BOOL				m_bHokanKey_SPACE;				//!< VK_SPACE	�⊮����L�[���L��/����

	//�O���w���v�̐ݒ�
	char				m_szExtHelp[_MAX_PATH];			//!< �O���w���v�P

	//�O��HTML�w���v�̐ݒ�
	char				m_szExtHtmlHelp[_MAX_PATH];		//!< �O��HTML�w���v
	bool				m_bHtmlHelpIsSingle;			//!< HtmlHelp�r���[�A�͂ЂƂ�

	//migemo�ݒ�
	TCHAR				m_szMigemoDll[_MAX_PATH];		//!< migemo dll
	TCHAR				m_szMigemoDict[_MAX_PATH];		//!< migemo dict

	//�L�[���[�h�w���v
	LOGFONT				m_lf;							//!< �L�[���[�h�w���v�̃t�H���g��� 		// ai 02/05/21 Add
	INT					m_nPointSize;					//!< �L�[���[�h�w���v�̃t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji

	//INI���ݒ�̂�
	int					m_bUseHokan;					//!< ���͕⊮�@�\���g�p����
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �}�N��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro
{
	char				m_szKeyMacroFileName[MAX_PATH];	//!< �L�[�{�[�h�}�N���̃t�@�C����
	MacroRec			m_MacroTable[MAX_CUSTMACRO];	//!< �L�[���蓖�ėp�}�N���e�[�u��	Sep. 14, 2001 genta
	char				m_szMACROFOLDER[_MAX_PATH];		//!< �}�N���p�t�H���_
	int					m_nMacroOnOpened;				//!< �I�[�v���㎩�����s�}�N���ԍ�	@@@ 2006.09.01 ryoji
	int					m_nMacroOnTypeChanged;			//!< �^�C�v�ύX�㎩�����s�}�N���ԍ�	@@@ 2006.09.01 ryoji
	int					m_nMacroOnSave;					//!< �ۑ��O�������s�}�N���ԍ�	@@@ 2006.09.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �t�@�C�����\��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName
{
	int					m_nTransformFileNameArrNum;										//!< �t�@�C�����̊ȈՕ\���o�^��
	TCHAR				m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//!< �t�@�C�����̊ȈՕ\���ϊ��O������
	TCHAR				m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];		//!< �t�@�C�����̊ȈՕ\���ϊ��㕶����	//@@@ 2003.04.08 MIK
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�E�g���C��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_OutLine
{
	// 20060201 aroka �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��
	int					m_bRememberOutlineWindowPos;//!< �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L������
	int					m_widthOutlineWindow;		//!< �A�E�g���C��/�g�s�b�N���X�g �̃T�C�Y(��)
	int					m_heightOutlineWindow;		//!< �A�E�g���C��/�g�s�b�N���X�g �̃T�C�Y(����)
	int					m_xOutlineWindowPos;		//!< �A�E�g���C��/�g�s�b�N���X�g �̈ʒu(X���W)
	int					m_yOutlineWindowPos;		//!< �A�E�g���C��/�g�s�b�N���X�g �̈ʒu(Y���W)

	//IDD_FUNCLIST (�c�[�� - �A�E�g���C�����)
	BOOL				m_bAutoCloseDlgFuncList;	//!< �A�E�g���C���_�C�A���O�������I�ɕ���
	BOOL				m_bFunclistSetFocusOnJump;	//!< �t�H�[�J�X���ڂ� 2002.02.08 hor
	BOOL				m_bMarkUpBlankLineEnable;	//!< ��s�𖳎����� 2002.02.08 aroka,hor
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �t�@�C�����e��r                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare
{
	//�t�@�C�����e��r�_�C�A���O
	BOOL				m_bCompareAndTileHorz;		//!< ������r��A���E�ɕ��ׂĕ\��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �r���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View
{
	//INI���ݒ�̂�
	LOGFONT				m_lf;						//!< ���݂̃t�H���g���
	BOOL				m_bFontIs_FIXED_PITCH;		//!< ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	INT					m_nPointSize;				//!< �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others
{
	//INI���ݒ�̂�
	RECT				m_rcOpenDialog;				//!< �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �X�e�[�^�X�o�[                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �v���O�C��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���C�����j���[                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ���ʐݒ�
struct CommonSetting
{
	CommonSetting_General			m_sGeneral;			//!< �S��
	CommonSetting_Window			m_sWindow;			//!< �E�B���h�E
	CommonSetting_TabBar			m_sTabBar;			//!< �^�u�o�[
	CommonSetting_Edit				m_sEdit;			//!< �ҏW
	CommonSetting_File				m_sFile;			//!< �t�@�C��
	CommonSetting_Backup			m_sBackup;			//!< �o�b�N�A�b�v
	CommonSetting_Format			m_sFormat;			//!< ����
	CommonSetting_Search			m_sSearch;			//!< ����
	CommonSetting_KeyBind			m_sKeyBind;			//!< �L�[���蓖��
	//
	CommonSetting_CustomMenu		m_sCustomMenu;		//!< �J�X�^�����j���[
	CommonSetting_ToolBar			m_sToolBar;			//!< �c�[���o�[
	CommonSetting_SpecialKeyword	m_sSpecialKeyword;	//!< �����L�[���[�h
	CommonSetting_Helper			m_sHelper;			//!< �x��
	CommonSetting_Macro				m_sMacro;			//!< �}�N��
	CommonSetting_FileName			m_sFileName;		//!< �t�@�C�����\��
	//
	CommonSetting_OutLine			m_sOutline;			//!< �A�E�g���C��
	CommonSetting_Compare			m_sCompare;			//!< �t�@�C�����e��r
	CommonSetting_View				m_sView;			//!< �r���[
	CommonSetting_Others			m_sOthers;			//!< ���̑�
};


//! ini�t�H���_�ݒ�	// 2007.05.31 ryoji
struct IniFolder {
	bool m_bInit;							// �������σt���O
	bool m_bReadPrivate;					// �}���`���[�U�pini����̓ǂݏo���t���O
	bool m_bWritePrivate;					// �}���`���[�U�pini�ւ̏������݃t���O
	TCHAR m_szIniFile[_MAX_PATH];			// EXE���ini�t�@�C���p�X
	TCHAR m_szPrivateIniFile[_MAX_PATH];	// �}���`���[�U�p��ini�t�@�C���p�X
};	/* ini�t�H���_�ݒ� */

//! ���L�t���O
struct SShare_Flags{
	BOOL				m_bEditWndChanging;				// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		�L�[�{�[�h�}�N���́A�L�^�I���������_�Ńt�@�C���um_szKeyMacroFileName�v�ɏ����o�����Ƃɂ���B
		m_bRecordingKeyMacro��TRUE�̂Ƃ��́A�L�[�{�[�h�}�N���̋L�^���Ȃ̂ŁAm_szKeyMacroFileName�ɃA�N�Z�X���Ă͂Ȃ�Ȃ��B
	*/
	BOOL				m_bRecordingKeyMacro;		/* �L�[�{�[�h�}�N���̋L�^�� */
	HWND				m_hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
};

//! ���L���[�N�o�b�t�@
struct SShare_WorkBuffer{
	char				m_szWork[32000];
	EditInfo			m_EditInfo_MYWM_GETFILEINFO;	//MYWM_GETFILEINFO�f�[�^�󂯓n���p	####�������Ȃ�
};

//! ���L�n���h��
struct SShare_Handles{
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
};

//! ���L���������\����
struct SShare_Nodes{
	int					m_nEditArrNum;	//short->int�ɏC��	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//�ő�l�C��	@@@ 2003.05.31 MIK
	LONG				m_nSequences;	/* �E�B���h�E�A�� */
	LONG				m_nNonameSequences;	/* ����A�� */
	LONG				m_nGroupSequences;	// �^�u�O���[�v�A��	// 2007.06.20 ryoji
};

//! EXE���
struct SShare_Version{
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
};

//! ���L���������\����
struct SShare_SearchKeywords{
	// -- -- �����L�[ -- -- //
	int					m_nSEARCHKEYArrNum;
	char				m_szSEARCHKEYArr[MAX_SEARCHKEY][_MAX_PATH];
	int					m_nREPLACEKEYArrNum;
	char				m_szREPLACEKEYArr[MAX_REPLACEKEY][_MAX_PATH];
	int					m_nGREPFILEArrNum;
	char				m_szGREPFILEArr[MAX_GREPFILE][_MAX_PATH];
	int					m_nGREPFOLDERArrNum;
	char				m_szGREPFOLDERArr[MAX_GREPFOLDER][_MAX_PATH];
};

//! ���L���������\����
struct SShare_TagJump{
	//�f�[�^
	int					m_TagJumpNum;					//!< �^�O�W�����v���̗L���f�[�^��
	int					m_TagJumpTop;					//!< �X�^�b�N�̈�ԏ�̈ʒu
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< �^�O�W�����v���
	int					m_nTagJumpKeywordArrNum;
	char				m_szTagJumpKeywordArr[MAX_TAGJUMP_KEYWORD][_MAX_PATH];
	BOOL				m_bTagJumpICase;	//!< �啶���������𓯈ꎋ
	BOOL				m_bTagJumpAnyWhere;	//!< ������̓r���Ƀ}�b�`
};

//! ���L���������\����
struct SShare_FileNameManagement{
	IniFolder			m_IniFolder;	/**** ini�t�H���_�ݒ� ****/
};

//! ���L���������\����
struct SShare_History{
	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRU���o�R���Ă��������B
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRUFolder���o�R���Ă��������B
	int					m_nOPENFOLDERArrNum;
	char				m_szOPENFOLDERArr[MAX_OPENFOLDER][_MAX_PATH];
	bool				m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	//MRU�ȊO�̏��
	char				m_szIMPORTFOLDER[_MAX_PATH];	/* �ݒ�C���|�[�g�p�t�H���_ */
	int					m_nCmdArrNum;
	char				m_szCmdArr[MAX_CMDARR][MAX_CMDLEN];
};

//! ���L�f�[�^�̈�
struct DLLSHAREDATA {
	// -- -- �o�[�W���� -- -- //
	/*!
		�f�[�^�\�� Version	//	Oct. 27, 2000 genta
		�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
		�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
	*/
	unsigned int				m_vStructureVersion;
	unsigned int				m_nSize;

	// -- -- ��ۑ��Ώ� -- -- //
	SShare_Version				m_sVersion;	//���Ǎ��͍s��Ȃ����A�����͍s��
	SShare_WorkBuffer			m_sWorkBuffer;
	SShare_Flags				m_sFlags;
	SShare_Nodes				m_sNodes;
	SShare_Handles				m_sHandles;

	DWORD						m_dwCustColors[16];						//!< �t�H���gDialog�J�X�^���p���b�g

	// -- -- �ۑ��Ώ� -- -- //
	//�ݒ�
	CommonSetting				m_Common;								// ���ʐݒ�
	STypeConfig					m_Types[MAX_TYPES];						// �^�C�v�ʐݒ�
	PRINTSETTING				m_PrintSettingArr[MAX_PRINTSETTINGARR];	// ����y�[�W�ݒ�

	//���̑�
	SShare_SearchKeywords		m_sSearchKeywords;
	SShare_TagJump				m_sTagJump;
	SShare_FileNameManagement	m_sFileNameManagement;
	SShare_History				m_sHistory;

	//�O���R�}���h���s�_�C�A���O�̃I�v�V����
	int							m_nExecFlgOpt;		/* �O���R�}���h���s�I�v�V���� */	//	2006.12.03 maru �I�v�V�����̊g���̂���
	//DIFF�����\���_�C�A���O�̃I�v�V����
	int							m_nDiffFlgOpt;		/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	//�^�O�t�@�C���̍쐬�_�C�A���O�̃I�v�V����
	TCHAR						m_szTagsCmdLine[_MAX_PATH];	/* TAGS�R�}���h���C���I�v�V���� */	//@@@ 2003.05.12 MIK
	int							m_nTagsOpt;			/* TAGS�I�v�V����(�`�F�b�N) */	//@@@ 2003.05.12 MIK


	// -- -- �e���|���� -- -- //
	//�w��s�փW�����v�_�C�A���O�̃I�v�V����
	bool						m_bLineNumIsCRLF_ForJump;	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */
};


/*!	@brief ���L�f�[�^�̊Ǘ�

	CShareData��CProcess�̃����o�ł��邽�߁C���҂̎����͓���ł��D
	�{����CProcess�I�u�W�F�N�g��ʂ��ăA�N�Z�X����ׂ��ł����C
	CProcess���̃f�[�^�̈�ւ̃|�C���^��static�ϐ��ɕۑ����邱�Ƃ�
	Singleton�̂悤�ɂǂ�����ł��A�N�Z�X�ł���\���ɂȂ��Ă��܂��D

	���L�������ւ̃|�C���^��m_pShareData�ɕێ����܂��D���̃����o��
	���J����Ă��܂����CCShareData�ɂ����Map/Unmap����邽�߂�
	ChareData�̏��łɂ���ă|�C���^m_pShareData�������ɂȂ邱�Ƃ�
	���ӂ��Ă��������D

	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
class CShareData
{
public:
	static CShareData* getInstance(){
		static CShareData instance;

		return &instance;
	}

	CShareData();
	CShareData(CShareData const&);
	void operator=(CShareData const&);

public:
	~CShareData();

	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	/* CShareData�N���X�̏��������� */
	DLLSHAREDATA* GetShareData(){ return m_pShareData; }		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	int GetDocumentTypeOfPath( const char* pszFilePath );		/* �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	int GetDocumentTypeOfExt( const char* pszExt );				/* �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	BOOL AddEditWndList( HWND, int nGroup = 0 );				/* �ҏW�E�B���h�E�̓o�^ */	// 2007.06.26 ryoji nGroup�����ǉ�
	void DeleteEditWndList( HWND );								/* �ҏW�E�B���h�E���X�g����̍폜 */
	void ResetGroupId( void );									/* �O���[�v��ID���Z�b�g���� */
	EditNode* GetEditNode( HWND hWnd );							/* �ҏW�E�B���h�E�����擾���� */
	int GetNoNameNumber( HWND hWnd );
	int GetGroupId( HWND hWnd );								/* �O���[�vID���擾���� */
	bool IsSameGroup( HWND hWnd1, HWND hWnd2 );					/* ����O���[�v���ǂ����𒲂ׂ� */
	int GetFreeGroupId( void );									/* �󂢂Ă���O���[�v�ԍ����擾���� */
	bool ReorderTab( HWND hSrcTab, HWND hDstTab );				/* �^�u�ړ��ɔ����E�B���h�E�̕��ёւ� 2007.07.07 genta */
	HWND SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] );/* �^�u�����ɔ����E�B���h�E���� 2007.07.07 genta */
	EditNode* GetEditNodeAt( int nGroup, int nIndex );			/* �w��ʒu�̕ҏW�E�B���h�E�����擾���� */
	EditNode* GetTopEditNode( HWND hWnd );						/* �擪�̕ҏW�E�B���h�E�����擾���� */
	HWND GetTopEditWnd( HWND hWnd );							/* �擪�̕ҏW�E�B���h�E���擾���� */
	bool IsTopEditWnd( HWND hWnd ){ return (GetTopEditWnd( hWnd ) == hWnd); }	/* �擪�̕ҏW�E�B���h�E���ǂ����𒲂ׂ� */

	BOOL RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, int nGroup, BOOL bCheckConfirm, HWND hWndFrom );
																/* �������̃E�B���h�E�֏I���v�����o�� */	// 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�	// 2007.06.20 ryoji nGroup�����ǉ�
	BOOL IsPathOpened( const TCHAR*, HWND* );					/* �w��t�@�C�����J����Ă��邩���ׂ� */
	BOOL ActiveAlreadyOpenedWindow( const TCHAR*, HWND*, int );	/* �w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F */	// 2007.03.16
	int GetEditorWindowsNum( int nGroup, bool bExcludeClosing = true );				/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */	// 2007.06.20 ryoji nGroup�����ǉ�	// 2008.04.19 ryoji bExcludeClosing�����ǉ�
	BOOL PostMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */	// 2007.06.20 ryoji nGroup�����ǉ�
	BOOL SendMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂邷�� */	// 2007.06.20 ryoji nGroup�����ǉ�
	int GetOpenedWindowArr( EditNode** , BOOL, BOOL bGSort = FALSE );				/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, const char*,
		short, short, short, short,
		short, short, short, short );									/* KEYDATA�z��Ƀf�[�^���Z�b�g */
	static LONG MY_RegSetVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		CONST BYTE *lpData,		// address of value data
		DWORD cbData 			// size of value data
	);
	static LONG MY_RegQuerVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		BYTE *lpData,			// address of value data
		DWORD cbData 			// size of value data
	);
	void TraceOut( LPCTSTR lpFmt, ...);	/* �f�o�b�O���j�^�ɏo�� */
	void SetTraceOutSource( HWND hwnd ){ m_hwndTraceOutSource = hwnd; }	/* TraceOut�N�����E�B���h�E�̐ݒ� */
	bool LoadShareData( void );	/* ���L�f�[�^�̃��[�h */
	void SaveShareData( void );	/* ���L�f�[�^�̕ۑ� */

	//���j���[�ނ̃t�@�C�����쐬
	bool GetMenuFullLabel_WinList(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_MRU(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, bool bFavorite, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true);
	}
	bool GetMenuFullLabel_WinListNoEscape(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_File(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false);
	}
	bool GetMenuFullLabel_FileNoEscape(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false);
	}
	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const EditInfo* editInfo, int id, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const TCHAR* pszFile, int id, bool bModified, ECodeType nCharCode, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	static TCHAR GetAccessKeyByIndex(int index, bool bZeroOrigin);
	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* �\���ݒ�t�@�C������ini�t�@�C�������擾���� */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* ini�t�@�C�����̎擾 */	// 2007.05.19 ryoji
	BOOL IsPrivateSettings( void ){ return m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate; }			/* ini�t�@�C���̕ۑ��悪���[�U�ʐݒ�t�H���_���ǂ��� */	// 2007.05.25 ryoji
	bool ShareData_IO_2( bool );	/* ���L�f�[�^�̕ۑ� */
	static void IO_ColorSet( CProfile* , const char* , ColorInfo* );	/* �F�ݒ� I/O */ // Feb. 12, 2006 D.S.Koba

	//@@@ 2002.2.2 YAZAKI
	//	Jun. 14, 2003 genta �����ǉ��D�����ύX
	int			GetMacroFilename( int idx, TCHAR* pszPath, int nBufLen ); // idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����
	bool		BeReloadWhenExecuteMacro( int idx );	//	idx�Ŏw�肵���}�N���́A���s���邽�тɃt�@�C����ǂݍ��ސݒ肩�H
	void		AddToSearchKeyArr( const char* pszSearchKey );	//	m_szSEARCHKEYArr��pszSearchKey��ǉ�����
	void		AddToReplaceKeyArr( const char* pszReplaceKey );	//	m_szREPLACEKEYArr��pszReplaceKey��ǉ�����
	void		AddToGrepFileArr( const char* pszGrepFile );		//	m_szGREPFILEArr��pszGrepFile��ǉ�����
	void		AddToGrepFolderArr( const char* pszGrepFolder );	//	m_nGREPFOLDERArrNum��pszGrepFolder��ǉ�����

	//@@@ 2002.2.3 YAZAKI
	bool		ExtWinHelpIsSet( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��ɁA�O���w���v���ݒ肳��Ă��邩�B
	char*		GetExtWinHelp( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O���w���v�t�@�C�������擾�B
	bool		ExtHTMLHelpIsSet( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��ɁA�O��HTML�w���v���ݒ肳��Ă��邩�B
	char*		GetExtHTMLHelp( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�t�@�C�������擾�B
	bool		HTMLHelpIsSingle( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�u�r���[�A�𕡐��N�����Ȃ��v��ON�����擾�B

	//@@@ 2002.2.9 YAZAKI
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, TCHAR* szDateFormat );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, TCHAR* szTimeFormat );

	LPTSTR GetTransformFileNameFast( LPCTSTR, LPTSTR, int );
	int TransformFileName_MakeCache( void );
	static LPCTSTR GetFilePathFormat( LPCTSTR, LPTSTR, int, LPCTSTR, LPCTSTR );
	static bool ExpandMetaToFolder( LPCTSTR, LPTSTR, int );

	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	void PushTagJump(const TagJump *);		//!< �^�O�W�����v���̕ۑ�
	bool PopTagJump(TagJump *);				//!< �^�O�W�����v���̎Q��

protected:
	/*
	||  �����w���p�֐�
	*/
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;
	HWND			m_hwndTraceOutSource;	// TraceOut()�N�����E�B���h�E�i���������N�������w�肵�Ȃ��Ă��ނ悤�Ɂj

	// �t�@�C�����ȈՕ\���p�L���b�V��
	int m_nTransformFileNameCount; // �L����
	TCHAR m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];

	//	Jan. 30, 2005 genta �������֐��̕���
	void InitKeyword(DLLSHAREDATA*);
	bool InitKeyAssign(DLLSHAREDATA*); // 2007.11.04 genta �N�����~�̂��ߒl��Ԃ�
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfigs(DLLSHAREDATA*);
	void InitPopupMenu(DLLSHAREDATA*);

	// Feb. 12, 2006 D.S.Koba
	void ShareData_IO_Mru( CProfile& );
	void ShareData_IO_Keys( CProfile& );
	void ShareData_IO_Grep( CProfile& );
	void ShareData_IO_Folders( CProfile& );
	void ShareData_IO_Cmd( CProfile& );
	void ShareData_IO_Nickname( CProfile& );
	void ShareData_IO_Common( CProfile& );
	void ShareData_IO_Toolbar( CProfile& );
	void ShareData_IO_CustMenu( CProfile& );
	void ShareData_IO_Font( CProfile& );
	void ShareData_IO_KeyBind( CProfile& );
	void ShareData_IO_Print( CProfile& );
	void ShareData_IO_Types( CProfile& );
	void ShareData_IO_Type_One( CProfile& cProfile, int nType, const char* pszSecName);
	void ShareData_IO_KeyWords( CProfile& );
	void ShareData_IO_Macro( CProfile& );
	void ShareData_IO_Other( CProfile& );

	int _GetOpenedWindowArrCore( EditNode** , BOOL, BOOL bGSort = FALSE );			/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ��i�R�A�������j */
};

/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
BOOL IsSakuraMainWindow( HWND hWnd );


///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */


/*[EOF]*/
