/*!	@file
	@brief ���ʊ֐��Q

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, shoji masami, Stonee, MIK
	Copyright (C) 2002, aroka, hor, MIK, YAZAKI
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, novice
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, ryoji, rastiv
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, kobake

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _ETC_UTY_H_
#define _ETC_UTY_H_

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "global.h"
#include <shlobj.h>

// 2007.12.21 ryoji
// Windows SDK for Vista �ȍ~�ɂ� NewApis.h �͊܂܂�Ȃ��̂Ŏg�p��������߂�
// multimon.h ��������͍폜����Ă��܂���������Ȃ��̂Ŏg�p���Ȃ�
// Win95/NT4.0�ɂ͑��݂��Ȃ� API ���g���悤�ɂȂ�̂ł����Â� OS �ւ̑Ή��͕s�ł�
//
// Note.
// �V���� Windows SDK ���g�p����ꍇ�ANewApis.h, multimon.h �����O����̂�
// WINVER �� 0x0400 �̂悤�ȌÂ� OS �Ή��̒l���w�肷��ƃR���p�C���G���[�ɂȂ�܂�
// �����w�肪�Ȃ��ꍇ�̃f�t�H���g�� 0x0600 �ȏ�Ȃ̂œ��Ɏw�肷��K�v�͂���܂���
// ���Ȃ݂ɒʏ�� Windows.h ���p�ł͐̂��玟�̂悤�ɂ�����ƕςȂƂ��낪����܂�
// �EGetLongPathName() API �� Win95(0x0400) ��Ή��Ȃ̂� 0x0400 �ɂ��Ă��G���[�ɂȂ�܂���
// �E�}���`���j�^�֘A API �� Win98(0x0410) �Ή��Ȃ̂� 0x0500 �ȏ�ɂ��Ȃ��ƃG���[�ɂȂ�܂�

#ifndef _INC_SDKDDKVER	// �V���� Windows SDK �ł� windows.h �� sdkddkver.h �� include ����
#define WANT_GETLONGPATHNAME_WRAPPER
#include <NewAPIs.h>
#include <MultiMon.h>
#endif

#if (_MSC_VER >= 1500) && (WINVER <= 0x0400)
#include <multimon.h>
#endif

/*!
	@brief ��� DPI �X�P�[�����O
	@note 96 DPI �s�N�Z����z�肵�Ă���f�U�C�����ǂꂾ���X�P�[�����O���邩

	@date 2009.10.01 ryoji ��DPI�Ή��p�ɍ쐬
*/
class CDPI{
	static void Init()
	{
		if( !bInitialized )
		{
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	static int nDpiX;
	static int nDpiY;
	static bool bInitialized;
public:
	static int ScaleX(int x){Init(); return ::MulDiv(x, nDpiX, 96);}
	static int ScaleY(int y){Init(); return ::MulDiv(y, nDpiY, 96);}
	static int UnscaleX(int x){Init(); return ::MulDiv(x, 96, nDpiX);}
	static int UnscaleY(int y){Init(); return ::MulDiv(y, 96, nDpiY);}
	static void ScaleRect(LPRECT lprc)
	{
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	static void UnscaleRect(LPRECT lprc)
	{
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	static int PointsToPixels(int pt, int ptMag = 1){Init(); return ::MulDiv(pt, nDpiY, 72 * ptMag);}	// ptMag: �����̃|�C���g���ɂ������Ă���{��
	static int PixelsToPoints(int px, int ptMag = 1){Init(); return ::MulDiv(px * ptMag, 72, nDpiY);}	// ptMag: �߂�l�̃|�C���g���ɂ�����{��
};

inline int DpiScaleX(int x){return CDPI::ScaleX(x);}
inline int DpiScaleY(int y){return CDPI::ScaleY(y);}
inline int DpiUnscaleX(int x){return CDPI::UnscaleX(x);}
inline int DpiUnscaleY(int y){return CDPI::UnscaleY(y);}
inline void DpiScaleRect(LPRECT lprc){CDPI::ScaleRect(lprc);}
inline void DpiUnscaleRect(LPRECT lprc){CDPI::UnscaleRect(lprc);}
inline int DpiPointsToPixels(int pt, int ptMag = 1){return CDPI::PointsToPixels(pt, ptMag);}
inline int DpiPixelsToPoints(int px, int ptMag = 1){return CDPI::PixelsToPoints(px, ptMag);}

#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100

#include "CHtmlHelp.h"	//	Jul.  6, 2001 genta
class CMemory;// 2002/2/3 aroka �w�b�_�y�ʉ�
class CEol;// 2002/2/3 aroka �w�b�_�y�ʉ�
class CBregexp;// 2002/2/3 aroka �w�b�_�y�ʉ�

BOOL MyWinHelp(HWND hWndMain, UINT uCommand, DWORD_PTR dwData);	/* WinHelp �̂����� HtmlHelp ���Ăяo�� */	// 2006.07.22 ryoji

//!�t�H���g�I���_�C�A���O
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool );   // 2009.10.01 ryoji �|�C���g�T�C�Y�i1/10�|�C���g�P�ʁj�����ǉ�

int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca �Ō�̕������w�肳�ꂽ�����łȂ��Ƃ��͕t������ */
int LimitStringLengthB( const char*, int, int, CMemory& );/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪���� */
const char* GetNextLine( const char*, int, int*, int*, CEol* );/* CR0LF0,CRLF,LFCR,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
void GetLineColumn( const char*, int*, int* );
BOOL IsURL( const char*, int, int* );/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
BOOL IsMailAddress( const char*, int, int* );	/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
int IsNumber( const char*, int, int );/* ���l�Ȃ炻�̒�����Ԃ� */	//@@@ 2001.02.17 by MIK
void ActivateFrameWindow( HWND );	/* �A�N�e�B�u�ɂ��� */
void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
BOOL GetAbsolutePath( const char*, char*, BOOL );	/* ���΃p�X����΃p�X */
BOOL CheckEXT( const TCHAR*, const TCHAR* );	/* �g���q�𒲂ׂ� */
char* my_strtok( char*, int, int*, const char* );
/* Shell Interface�n(?) */
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR* );	/* �t�H���_�I���_�C�A���O */
//�s�v�������ԈႢ�̂���
//ITEMIDLIST* CreateItemIDList( const char* );	/* �p�X���ɑ΂���A�C�e���h�c���X�g���擾���� */
//BOOL DeleteItemIDList( ITEMIDLIST* );/* �A�C�e���h�c���X�g���폜���� */
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);/* �V���[�g�J�b�g(.lnk)�̉��� */
void ResolvePath(TCHAR* pszPath); //!< �V���[�g�J�b�g�̉����ƃ����O�t�@�C�����֕ϊ����s���B

/*
||	�������̃��[�U�[������\�ɂ���
||	�u���b�L���O�t�b�N(?)(���b�Z�[�W�z��)
*/
BOOL BlockingHook( HWND hwndDlgCancel );

HWND OpenHtmlHelp( HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag = true);
DWORD NetConnect ( const char strNetWorkPass[] );

int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);
int cescape_j(const char* org, char* out, char cesc, char cwith);

/* �w���v�̖ڎ���\�� */
void ShowWinHelpContents( HWND hwnd );

/*!	&�̓�d��
	���j���[�Ɋ܂܂��&��&&�ɒu��������
	@author genta
	@date 2002/01/30 cescape�Ɋg�����C
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{	cescape( org, out, _T('&'), _T('&') ); }


/*
	scanf�I���S�X�L����

	�g�p��:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//����: a[0]=1, a[1]=23, a[2]=4 �ƂȂ�B
*/
int scan_ints(
	const TCHAR*	pszData,	//!< [in]  �f�[�^������
	const TCHAR*	pszFormat,	//!< [in]  �f�[�^�t�H�[�}�b�g
	int*			anBuf		//!< [out] �擾�������l (�v�f���͍ő�32�܂�)
);

///////////////////////////////////////////////////////////////////////

/* �J���[�������C���f�b�N�X�ԍ��̕ϊ� */	//@@@ 2002.04.30
int GetColorIndexByName( const char *name );
const char* GetColorNameByIndex( int index );

//	Dec. 2, 2002 genta
HICON GetAppIcon( HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall = false);

//	Apr. 03, 2003 genta
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count);

//	Apr. 30, 2003 genta
//	�f�B���N�g���̐[���𒲂ׂ�
int CalcDirectoryDepth(const TCHAR* path);

HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// �w�肵���E�B���h�E�̑c��̃n���h�����擾����	// 2007.07.01 ryoji

// 20051121 aroka
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const TCHAR* ver );	//�o�[�W�����ԍ��̉��
int CompareVersion( const TCHAR* verA, const TCHAR* verB );	//�o�[�W�����ԍ��̔�r

int getCtrlKeyState();

// 2005.11.26 aroka
bool IsLocalDrive( const TCHAR* pszDrive );

DWORD GetDllVersion( LPCTSTR lpszDllName );	// �V�F����R�����R���g���[�� DLL �̃o�[�W�����ԍ����擾	// 2006.06.17 ryoji

void ChangeCurrentDirectoryToExeDir();
HMODULE LoadLibraryExedir(LPCTSTR pszDll);

BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath );	// ����t�H���_�̃p�X���擾����	// 2007.05.19 ryoji
INT_PTR MyPropertySheet( LPPROPSHEETHEADER lppsph );	// �Ǝ��g���v���p�e�B�V�[�g	// 2007.05.24 ryoji

#endif /* _ETC_UTY_H_ */


/*[EOF]*/
