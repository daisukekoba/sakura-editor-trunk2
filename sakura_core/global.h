/*!	@file
	@brief ���ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji, Moca
	Copyright (C) 2007, ryoji, kobake, Moca, genta
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <commctrl.h> // PROPSHEETHEADER
#include <tchar.h>

/*
	2007.10.18 kobake
	�e���v���[�g�� min �Ƃ� max �Ƃ��B

	�ǂ����̕W���w�b�_�ɁA�����悤�Ȃ��̂��������C�����邯�ǁA
	NOMINMAX ���`����ɂ��Ă��A�Ȃ񂾂� min �Ƃ� max �Ƃ��������O���ƁA
	�e���v���[�g���Ă�ł�񂾂��}�N�����Ă�ł�񂾂��󕪂���Ȃ��̂ŁA
	�����I�Ɂut_�`�v�Ƃ������O�����֐���p�ӁB
*/

template <class T>
inline T t_min(T t1,T t2)
{
	return t1<t2?t1:t2;
}

template <class T>
inline T t_max(T t1,T t2)
{
	return t1>t2?t1:t2;
}

#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif

#if defined(__BORLANDC__)
#define __forceinline inline
#define _itoa itoa
#define _strlwr strlwr
#endif

#if defined(__GNUC__)
#define __forceinline
#include <limits.h>
#include <ole2.h>
#include <objbase.h>
#include <imm.h>
extern "C" unsigned char *   _mbsstr   (const unsigned char *__s1, const unsigned char *__s2);
#define _tcsncicmp _strnicmp
#define _ttempnam _tempnam
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
typedef struct tagNMKEY
{
    NMHDR hdr;
    UINT  nVKey;
    UINT  uFlags;
} NMKEY, FAR *LPNMKEY;
typedef DWORD HIMC;
#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
#define MONITORINFOF_PRIMARY        0x00000001
#endif

//Oct. 31, 2000 JEPRO TeX Keyword �̂��߂�'\'��ǉ�	//Nov. 9, 2000 JEPRO HSP Keyword �̂��߂�'@'��ǉ�
//#define IS_KEYWORD_CHAR(c) ((c) == '#' || (c) == '$' || __iscsym( (c) ))
//#define IS_KEYWORD_CHAR(c) ((c) == '#'/*35*/ || (c) == '$'/*36*/ || (c) == '@'/*64*/ || (c) == '\\'/*92*/ || __iscsym( (c) ))
extern const unsigned char gm_keyword_char[256];	//@@@ 2002.04.27
#define IS_KEYWORD_CHAR(c)	((int)(gm_keyword_char[(unsigned char)((c) & 0xff)]))	//@@@ 2002.04.27 ���P�[���Ɉˑ����Ȃ�


extern const char* GSTR_APPNAME;

//���l�萔�̕����� 2009.02.11 ryoji
#define _NUM_TO_STR(n) #n
#define NUM_TO_STR(n) _NUM_TO_STR(n)

//! �f�o�b�O���ʁA�萔�T�t�B�b�N�X 2007.09.20 kobake
#ifdef _DEBUG
	#define _DEBUG_SUFFIX_ "_DEBUG"
#else
	#define _DEBUG_SUFFIX_ ""
#endif

//! �^�[�Q�b�g�}�V������ 2010.08.21 Moca �ǉ�
#ifdef _WIN64
	#define CON_SKR_MACHINE_SUFFIX_ "M64"
#else
	#define CON_SKR_MACHINE_SUFFIX_ ""
#endif

#define	GSTR_SHAREDATA (_T("CShareData") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �~���[�e�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �A�v���P�[�V�������s���o�p(�C���X�g�[���Ŏg�p)
#define	GSTR_MUTEX_SAKURA _T("MutexSakuraEditor")

//! �R���g���[���v���Z�X
#define	GSTR_MUTEX_SAKURA_CP (_T("MutexSakuraEditorCP") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//! �m�[�}���v���Z�X����������
#define	GSTR_MUTEX_SAKURA_INIT (_T("MutexSakuraEditorInit") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//! �m�[�h���쓯��
#define	GSTR_MUTEX_SAKURA_EDITARR (_T("MutexSakuraEditorEditArr") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �����������C�x���g
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED (_T("EventSakuraEditorCPInitialized") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �E�B���h�E�N���X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �R���g���[���g���C
#define	GSTR_CEDITAPP (_T("CEditApp") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//! ���C���E�B���h�E
#define	GSTR_EDITWINDOWNAME (_T("TextEditorWindow") _T(CON_SKR_MACHINE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//! �r���[
#define	GSTR_VIEWNAME (_T("EditorClient"))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���\�[�X                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	Dec. 2, 2002 genta
//	�Œ�t�@�C����
#define FN_APP_ICON _T("my_appicon.ico")
#define FN_GREP_ICON _T("my_grepicon.ico")
#define FN_TOOL_BMP _T("my_icons.bmp")

//	�W���A�v���P�[�V�����A�C�R�����\�[�X��
#define ICON_DEFAULT_APP IDI_ICON_STD
#define ICON_DEFAULT_GREP IDI_ICON_GREP


/* �E�B���h�E��ID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000


// �����R�[�h�Z�b�g���
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT �ǉ�
enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open))
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< �����R�[�h��������
	CODE_ERROR      = -1,			//!< �G���[
	CODE_NONE       = -1,			//!< �����o
	CODE_DEFAULT    = CODE_SJIS,	//!< �f�t�H���g�̕����R�[�h
	/*
		- MS-CP50220 
			Unicode ���� cp50220 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����� JIS X 0208 �̕Љ����ɒu�������
		- MS-CP50221
			Unicode ���� cp50221 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́AG0 �W���ւ̎w���̃G�X�P�[�v�V�[�P���X ESC ( I ��p���ăG���R�[�h�����
		- MS-CP50222
			Unicode ���� cp50222 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́ASO/SI ��p���ăG���R�[�h�����
		
		�Q�l
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};

//2007.08.14 kobake �ǉ�
//!�L���ȕ����R�[�h�Z�b�g�Ȃ�true
inline bool IsValidCodeType(int code)
{
	return code>=0 && code<CODE_CODEMAX;
}

//2007.08.14 kobake �ǉ�
//!�L���ȕ����R�[�h�Z�b�g�Ȃ�true�B�������ASJIS�͏���(�t�@�C���ꗗ�ɕ����R�[�h��[]�t���ŕ\���̂���)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code!=CODE_SJIS;
}

extern LPCTSTR gm_pszCodeNameArr_1[];
extern LPCTSTR gm_pszCodeNameArr_2[];
extern LPCTSTR gm_pszCodeNameArr_3[];

/* �R���{�{�b�N�X�p �������ʂ��܂ޔz�� */
extern const int gm_nCodeComboValueArr[];
extern LPCTSTR gm_pszCodeComboNameArr[];
extern const int gm_nCodeComboNameArrNum;

/* �_�C�A���O�\�����@ */ // �A�E�g���C���E�B���h�E�p�ɍ쐬 20060201 aroka
enum enumShowDlg {
	SHOW_NORMAL			= 0,
	SHOW_RELOAD			= 1,
	SHOW_TOGGLE			= 2,
};


/* �I��̈�`��p�p�����[�^ */
extern const COLORREF	SELECTEDAREA_RGB;
extern const int		SELECTEDAREA_ROP2;




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�萔                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �F�萔��F�ԍ��ɕϊ����邽�߂̎���bit
#define COLORIDX_BLOCK_BIT (2 << 9)		//!< �u���b�N�R�����g����bit
#define COLORIDX_REGEX_BIT (2 << 10)	//!< ���K�\���L�[���[�h����bit

/*! �F�萔
	@date 2000.01.12 Stonee ������ύX�����Ƃ��́ACColorStrategy.cpp ��g_ColorAttributeArr�̒�`���ύX���ĉ������B
	@date 2000.09.18 JEPRO ���Ԃ�啝�ɓ���ւ���
	@date 2007.09.09 Moca  ���Ԃ̒�`�͂��C���ɕύX
	@date 2013.04.26 novice �F�萔��F�ԍ���ϊ����邽�߂̎���bit����
*/
enum EColorIndexType {
	COLORIDX_TEXT = 0,      //!< �e�L�X�g
	COLORIDX_RULER,         //!< ���[���[
	COLORIDX_CARET,         //!< �L�����b�g    // 2006.12.07 ryoji
	COLORIDX_CARET_IME,     //!< IME�L�����b�g // 2006.12.07 ryoji
	COLORIDX_UNDERLINE,     //!< �J�[�\���s�A���_�[���C��
	COLORIDX_CURSORVLINE,   //!< �J�[�\���ʒu�c�� // 2006.05.13 Moca
	COLORIDX_GYOU,          //!< �s�ԍ�
	COLORIDX_GYOU_MOD,      //!< �s�ԍ�(�ύX�s)
	COLORIDX_TAB,           //!< TAB�L��
	COLORIDX_SPACE,         //!< ���p�� //2002.04.28 Add by KK �ȍ~�S��+1
	COLORIDX_ZENSPACE,      //!< ���{���
	COLORIDX_CTRLCODE,      //!< �R���g���[���R�[�h
	COLORIDX_EOL,           //!< ���s�L��
	COLORIDX_WRAP,          //!< �܂�Ԃ��L��
	COLORIDX_VERTLINE,      //!< �w�茅�c��    // 2005.11.08 Moca
	COLORIDX_EOF,           //!< EOF�L��
	COLORIDX_DIGIT,         //!< ���p���l  //@@@ 2001.02.17 by MIK //�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH,        //!< ����������
	COLORIDX_KEYWORD1,      //!< �����L�[���[�h1 // 2002/03/13 novice
	COLORIDX_KEYWORD2,      //!< �����L�[���[�h2 // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,      //!< �����L�[���[�h3 // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,      //!< �����L�[���[�h4
	COLORIDX_KEYWORD5,      //!< �����L�[���[�h5
	COLORIDX_KEYWORD6,      //!< �����L�[���[�h6
	COLORIDX_KEYWORD7,      //!< �����L�[���[�h7
	COLORIDX_KEYWORD8,      //!< �����L�[���[�h8
	COLORIDX_KEYWORD9,      //!< �����L�[���[�h9
	COLORIDX_KEYWORD10,     //!< �����L�[���[�h10
	COLORIDX_COMMENT,       //!< �s�R�����g                        //Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,       //!< �V���O���N�H�[�e�[�V����������    //Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,       //!< �_�u���N�H�[�e�[�V����������      //Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,           //!< URL                               //Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1,        //!< ���K�\���L�[���[�h1  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,        //!< ���K�\���L�[���[�h2  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,        //!< ���K�\���L�[���[�h3  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,        //!< ���K�\���L�[���[�h4  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,        //!< ���K�\���L�[���[�h5  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,        //!< ���K�\���L�[���[�h6  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,        //!< ���K�\���L�[���[�h7  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,        //!< ���K�\���L�[���[�h8  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,        //!< ���K�\���L�[���[�h9  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,       //!< ���K�\���L�[���[�h10  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,   //!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,   //!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,   //!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR,  //!< �Ί���    // 02/09/18 ai Add
	COLORIDX_MARK,          //!< �u�b�N�}�[�N  // 02/10/16 ai Add

	//�J���[�̍Ō�
	COLORIDX_LAST,			//!< �J���[�̍Ō�

	//�J���[�\������p(�u���b�N�R�����g)
	COLORIDX_BLOCK1			= COLORIDX_BLOCK_BIT,			//!< �u���b�N�R�����g1(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)
	COLORIDX_BLOCK2,        								//!< �u���b�N�R�����g2(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)

	//�J���[�\������p(���K�\���L�[���[�h)
	COLORIDX_REGEX_FIRST	= COLORIDX_REGEX_BIT,						//!< ���K�\���L�[���[�h(�ŏ�)
	COLORIDX_REGEX_LAST  	= COLORIDX_REGEX_FIRST + COLORIDX_LAST - 1,	//!< ���K�\���L�[���[�h(�Ō�)

	// -- -- �ʖ� -- -- //
	COLORIDX_DEFAULT		= COLORIDX_TEXT,							//!< �f�t�H���g
};



//@@@ From Here 2003.05.31 MIK
/*! �^�u�E�C���h�E�p���b�Z�[�W�T�u�R�}���h */
enum ETabWndNotifyType {
	TWNT_REFRESH	= 0,		//�ĕ\��
	TWNT_ADD		= 1,		//�E�C���h�E�o�^
	TWNT_DEL		= 2,		//�E�C���h�E�폜
	TWNT_ORDER		= 3,		//�E�C���h�E�����ύX
	TWNT_FILE		= 4,		//�t�@�C�����ύX
	TWNT_MODE_ENABLE= 5,		//�^�u���[�h�L����	//2004.07.14 Kazika �ǉ�
	TWNT_MODE_DISABLE= 6,		//�^�u���[�h������	//2004.08.27 Kazika �ǉ�
	TWNT_WNDPL_ADJUST= 7,		//�E�B���h�E�ʒu���킹	// 2007.04.03 ryoji �ǉ�
};

/*! �o�[�̕\���E��\�� */
enum EBarChangeNotifyType {
	MYBCN_TOOLBAR	= 0,		//�c�[���o�[
	MYBCN_FUNCKEY	= 1,		//�t�@���N�V�����L�[
	MYBCN_TAB		= 2,		//�^�u
	MYBCN_STATUSBAR	= 3,		//�X�e�[�^�X�o�[
};
//@@@ To Here 2003.05.31 MIK

//�^�u�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
//�E�N���b�N���j���[�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


/*!< �F�^�C�v */
//@@@ From Here 2006.12.18 ryoji
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
//#define COLOR_ATTRIB_NO_TEXT		0x00000010	�\��l
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	�\��l
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct SColorAttributeData{
	const TCHAR*	szName;
	unsigned int	fAttribute;
};
extern const SColorAttributeData g_ColorAttributeArr[];

//@@@ To Here 2006.12.18 ryoji

/*!< �ݒ�l�̏���E���� */
//	���[���̍���
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

// Feb. 18, 2003 genta �ő�l�̒萔���ƒl�ύX
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;


// novice 2002/09/14
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL	int
#endif

//	Aug. 14, 2005 genta �萔��`�ǉ�
// 2007.09.07 kobake �萔���ύX: MAXLINESIZE��MAXLINEKETAS
// 2007.09.07 kobake �萔���ύX: MINLINESIZE��MINLINEKETAS
const int MAXLINEKETAS	= 10240;	//!<	1�s�̌����̍ő�l
const int MINLINEKETAS	= 10;		//!<	1�s�̌����̍ŏ��l

const int LINEREADBUFSIZE	= 10240;	//!<	�t�@�C������1�s���f�[�^��ǂݍ��ނƂ��̃o�b�t�@�T�C�Y

/**	�}�E�X�N���b�N�ƃL�[��`�̑Ή�

	@date 2007.11.04 genta �V�K�쐬�D���l����Ɣ͈̓T�C�Y��`�̂���
*/
enum MOUSEFUNCTION_ASSIGN {
	MOUSEFUNCTION_DOUBLECLICK	= 0,	//!< �_�u���N���b�N
	MOUSEFUNCTION_RIGHT			= 1,	//!< �E�N���b�N
	MOUSEFUNCTION_CENTER		= 2,	//!< ���N���b�N
	MOUSEFUNCTION_LEFTSIDE		= 3,	//!< ���T�C�h�N���b�N
	MOUSEFUNCTION_RIGHTSIDE		= 4,	//!< �E�T�C�h�N���b�N
	MOUSEFUNCTION_TRIPLECLICK	= 5,	//!< �g���v���N���b�N
	MOUSEFUNCTION_QUADCLICK		= 6,	//!< �N�A�h���v���N���b�N
	MOUSEFUNCTION_KEYBEGIN		= 7,	//!< �}�E�X�ւ̊��蓖�Č����{���̃L�[���蓖�Đ擪INDEX
};

// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
enum WRAP_TEXT_WRAP_METHOD {
	WRAP_NO_TEXT_WRAP		= 0,		// �܂�Ԃ��Ȃ��i�X�N���[���o�[���e�L�X�g���ɍ��킹��j
	WRAP_SETTING_WIDTH,					// �w�茅�Ő܂�Ԃ�
	WRAP_WINDOW_WIDTH,					// �E�[�Ő܂�Ԃ�
};

//2007.09.06 kobake �ǉ�
//!��������
enum ESearchDirection{
	SEARCH_BACKWARD = 0, //�O������ (�O������)
	SEARCH_FORWARD  = 1, //������� (��������) (����)
};

//2007.09.06 kobake �ǉ�
struct SSearchOption{
//	ESearchDirection	eDirection;
//	bool	bPrevOrNext;	//!< false==�O������ true==�������
	bool	bRegularExp;	//!< true==���K�\��
	bool	bLoHiCase;		//!< true==�p�啶���������̋��
	bool	bWordOnly;		//!< true==�P��̂݌���

	SSearchOption() : bRegularExp(false), bLoHiCase(false), bWordOnly(false) { }
	SSearchOption(
		bool _bRegularExp,
		bool _bLoHiCase,
		bool _bWordOnly
	)
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
	{
	}
	void Reset()
	{
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	//���Z�q
	bool operator == (const SSearchOption& rhs) const
	{
		//�Ƃ肠����memcmp�ł�����
		return memcmp(this,&rhs,sizeof(*this))==0;
	}
	bool operator != (const SSearchOption& rhs) const
	{
		return !operator==(rhs);
	}

};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �P�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// -- -- �ʏ��int�ŒP�ʌ^���`

//���W�b�N�P��
typedef int CLogicInt;

//���C�A�E�g�P��
typedef int CLayoutInt;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �Q�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//

struct CLogicPoint {
	CLogicInt x;
	CLogicInt y;
};

struct CLayoutPoint {
	CLayoutInt x;
	CLayoutInt y;
};

struct CLayoutRange {
	CLayoutPoint m_ptFrom;
	CLayoutPoint m_ptTo;
};

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �悭�g���p�@                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! SHIFT�������Ă��邩�ǂ���
	inline bool GetKeyState_Shift()
	{
		return (::GetKeyState(VK_SHIFT)&0x8000)!=0;
	}

	//! CTRL�������Ă��邩�ǂ���
	inline bool GetKeyState_Control()
	{
		return (::GetKeyState(VK_CONTROL)&0x8000)!=0;
	}

	//! ALT�������Ă��邩�ǂ���
	inline bool GetKeyState_Alt()
	{
		return (::GetKeyState(VK_MENU)&0x8000)!=0;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �萔                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const size_t sizeof_old_PROPSHEETHEADER = sizeof(PROPSHEETHEADER);
	#else
		static const size_t sizeof_old_PROPSHEETHEADER = 40;
	#endif

	//	Jan. 29, 2002 genta
	//	Win95/NT���[������sizeof( MENUITEMINFO )
	//	����ȊO�̒l��^����ƌÂ�OS�ł����Ɠ����Ă���Ȃ��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const int SIZEOF_MENUITEMINFO = sizeof(MENUITEMINFO);
	#else
		static const int SIZEOF_MENUITEMINFO = 44;
	#endif
}
using namespace ApiWrap;




//	Sep. 22, 2003 MIK
//	�Â�SDK�΍�D�V����SDK�ł͕s�v
#ifndef _WIN64
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif
#ifndef ULONG_PTR
#define ULONG_PTR ULONG
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef UINT_PTR
#define UINT_PTR UINT
#endif
#ifndef INT_PTR
#define INT_PTR INT
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE GWL_HINSTANCE
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT DWL_MSGRESULT
#endif
#endif  //_WIN64

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */


/*[EOF]*/
