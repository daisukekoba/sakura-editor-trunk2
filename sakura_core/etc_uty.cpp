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

// 2006.04.21 ryoji �}���`���j�^�̃G�~�����[�V�����֐��̎��̐����̂���
#define COMPILE_MULTIMON_STUBS

#include "StdAfx.h"
#include <HtmlHelp.h>
#include <io.h>
#include <memory.h>		// Apr. 03, 2003 genta
#include <CdErr.h> // Nov. 3, 2005 genta	//CDERR_FINDRESFAILURE��
#include "etc_uty.h"
#include "shell.h"
#include "file.h"
#include "os.h"
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
#include "CMRUFile.h"
#include "CMRUFolder.h"
#include "CUxTheme.h"	// 2007.04.01 ryoji

int CDPI::nDpiX = 96;
int CDPI::nDpiY = 96;
bool CDPI::bInitialized = false;

/*! �w���v�t�@�C���̃t���p�X��Ԃ�
 
    @return �p�X���i�[�����o�b�t�@�̃|�C���^
 
    @note ���s�t�@�C���Ɠ����ʒu�� sakura.chm �t�@�C����Ԃ��B
        �p�X�� UNC �̂Ƃ��� _MAX_PATH �Ɏ��܂�Ȃ��\��������B
 
    @date 2002/01/19 aroka �GnMaxLen �����ǉ�
	@date 2007/10/23 kobake ���������̌����C��(in��out)
	@date 2007/10/23 kobake CEditApp�̃����o�֐��ɕύX
	@date 2007/10/23 kobake �V�O�j�`���ύX�Bconst�|�C���^��Ԃ������̃C���^�[�t�F�[�X�ɂ��܂����B
*/
static LPCTSTR GetHelpFilePath()
{
	static TCHAR szHelpFile[_MAX_PATH] = _T("");
	if(szHelpFile[0]==_T('\0')){
		GetExedir( szHelpFile, _T("sakura.chm") );
	}
	return szHelpFile;
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
				if( pShareData->m_sFlags.m_bEditWndChanging )
					return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
				pShareData->m_sFlags.m_bEditWndChanging = TRUE;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

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
		pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji

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
		{ "file://",	7,	false }, /* 1 */
		{ "ftp://",		6,	false }, /* 2 */
		{ "gopher://",	9,	false }, /* 3 */
		{ "http://",	7,	false }, /* 4 */
		{ "https://",	8,	false }, /* 5 */
		{ "mailto:",	7,	true  }, /* 6 */
		{ "news:",		5,	false }, /* 7 */
		{ "nntp://",	7,	false }, /* 8 */
		{ "prospero://",11,	false }, /* 9 */
		{ "telnet://",	9,	false }, /* 10 */
		{ "tp://",		5,	false }, /* 11 */	//2004.02.02
		{ "ttp://",		6,	false }, /* 12 */	//2004.02.02
		{ "wais://",	7,	false }, /* 13 */
		{ "{",			0,	false }  /* 14 */  /* '{' is 'z'+1 : terminate */
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

void GetLineColumn( const char* pLine, int* pnJumpToLine, int* pnJumpToColumn )
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
			*pnJumpToColumn = atoi( szNumber );
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
			pcEol->SetTypeByString( &pData[i], nDataLen - i );
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

/*
	scanf�I���S�X�L����

	�g�p��:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//����: a[0]=1, a[1]=23, a[2]=4 �ƂȂ�B
*/
int scan_ints(
	const TCHAR*	pszData,	//!< [in]
	const TCHAR*	pszFormat,	//!< [in]
	int*			anBuf		//!< [out]
)
{
	//�v�f��
	int num = 0;
	const TCHAR* p = pszFormat;
	while(*p){
		if(*p==_T('%'))num++;
		p++;
	}

	//�X�L����
	int dummy[32];
	memset(dummy,0,sizeof(dummy));
	int nRet = _stscanf(
		pszData,pszFormat,
		&dummy[ 0],&dummy[ 1],&dummy[ 2],&dummy[ 3],&dummy[ 4],&dummy[ 5],&dummy[ 6],&dummy[ 7],&dummy[ 8],&dummy[ 9],
		&dummy[10],&dummy[11],&dummy[12],&dummy[13],&dummy[14],&dummy[15],&dummy[16],&dummy[17],&dummy[18],&dummy[19],
		&dummy[20],&dummy[21],&dummy[22],&dummy[23],&dummy[24],&dummy[25],&dummy[26],&dummy[27],&dummy[28],&dummy[29]
	);

	//���ʃR�s�[
	int i;
	for(i=0;i<num;i++){
		anBuf[i]=dummy[i];
	}

	return nRet;
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

/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
int GetColorIndexByName( const char *name )
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
const char* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
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

/*[EOF]*/
