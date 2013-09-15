/*!	@file
	@brief 共通設定ダイアログボックス、「全般」ページ

	@author Norio Nakatani
	@date 1998/12/24 新規作成
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, hor, Stonee, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, MIK, Moca, こおり
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji, Moca, nasukoji
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <windows.h>
#include <commctrl.h>
#include <assert.h>
#include "CPropCommon.h"
#include "global.h"
#include "etc_uty.h"
#include "Debug.h"
#include "CEditApp.h"
#include "sakura_rc.h"

int	CPropCommon::SearchIntArr( int nKey, int* pnArr, int nArrNum )
{
	int i;
	for( i = 0; i < nArrNum; ++i ){
		if( nKey == pnArr[i] ){
			return i;
		}
	}
	return -1;
}


/*!
	プロパティページごとのWindow Procedureを引数に取ることで
	処理の共通化を狙った．

	@param DispatchPage 真のWindow Procedureのメンバ関数ポインタ
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CPropCommon::DlgProc(
	INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE*	pPsp;
	CPropCommon*	pCPropCommon;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropCommon = ( CPropCommon* )(pPsp->lParam);
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropCommon = ( CPropCommon* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}
//	To Here Jun. 2, 2001 genta

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CPropCommon::CPropCommon()
{
//	int		i;
//	long	lPathLen;

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = ID_PAGENUM_GENERAL;

	return;
}





CPropCommon::~CPropCommon()
{
}





/* 初期化 */
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
void CPropCommon::Create( HINSTANCE hInstApp, HWND hwndParent, CImageListMgr* cIcons, CMenuDrawer* pMenuDrawer )
{
	m_hInstance = hInstApp;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_pcIcons = cIcons;

	// 2007.11.02 ryoji マクロ設定を変更したあと、画面を閉じないでカスタムメニュー、ツールバー、
	//                  キー割り当ての画面に切り替えた時に各画面でマクロ設定の変更が反映されるよう、
	//                  m_MacroTable（ローカルメンバ）でm_cLookupを初期化する
	m_cLookup.Init( m_hInstance, m_Common.m_sMacro.m_MacroTable, &m_Common );	//	機能名・番号resolveクラス．

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	m_pcMenuDrawer = pMenuDrawer;

	return;
}





/* 色選択ダイアログ */
BOOL CPropCommon::SelectColor( HWND hwndParent, COLORREF* pColor )
{
	int			i;
	CHOOSECOLOR	cc;
	DWORD	dwCustColors[16] ;
	for( i = 0; i < 16; i++ ){
		dwCustColors[i] = (DWORD)RGB( 255, 255, 255 );
	}
	cc.lStructSize = sizeof( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = (LPDWORD) dwCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( FALSE == ::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}


//	From Here Jun. 2, 2001 genta
/*!
	「共通設定」プロパティシートの作成時に必要な情報を
	保持する構造体
*/
struct ComPropSheetInfo {
	const TCHAR* szTabname;									//!< TABの表示名
	unsigned int resId;										//!< Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	キーワード：共通設定タブ順序(プロパティシート)
/*! プロパティシートの作成
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
INT_PTR CPropCommon::DoPropertySheet( int nPageNum )
{
	INT_PTR				nRet;
	int					nIdx;

	//	From Here Jun. 2, 2001 genta
	//	Feb. 11, 2007 genta URLをTABと入れ換え	// 2007.02.13 順序変更（TABをWINの次に）
	//!	「共通設定」プロパティシートの作成時に必要な情報の配列．
	//	順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に	2008/6/22 Uchi 
	//	DProcの変更	2010/5/9 Uchi
	static const ComPropSheetInfo ComPropSheetInfoList[] = {
		{ _T("全般"), 				IDD_PROP_GENERAL,	CPropGeneral::DlgProc_page },
		{ _T("ウィンドウ"),			IDD_PROP_WIN,		CPropWin::DlgProc_page },

		{ _T("ツールバー"),			IDD_PROP_TOOLBAR,	CPropToolbar::DlgProc_page },
		{ _T("タブバー"),			IDD_PROP_TAB,		CPropTab::DlgProc_page },

		{ _T("編集"),				IDD_PROP_EDIT,		CPropEdit::DlgProc_page },
		{ _T("ファイル"),			IDD_PROP_FILE,		CPropFile::DlgProc_page },
		{ _T("ファイル名表示"),		IDD_PROP_FNAME,		CPropFileName::DlgProc_page },
		{ _T("バックアップ"),		IDD_PROP_BACKUP,	CPropBackup::DlgProc_page },
		{ _T("書式"),				IDD_PROP_FORMAT,	CPropFormat::DlgProc_page },
		{ _T("検索"),				IDD_PROP_GREP,		CPropGrep::DlgProc_page },	// 2006.08.23 ryoji タイトル変更（Grep -> 検索）
		{ _T("キー割り当て"),		IDD_PROP_KEYBIND,	CPropKeybind::DlgProc_page },
		{ _T("カスタムメニュー"),	IDD_PROP_CUSTMENU,	CPropCustmenu::DlgProc_page },
		{ _T("強調キーワード"),		IDD_PROP_KEYWORD,	CPropKeyword::DlgProc_page },
		{ _T("支援"),				IDD_PROP_HELPER,	CPropHelper::DlgProc_page },
		{ _T("マクロ"),				IDD_PROP_MACRO,		CPropMacro::DlgProc_page },
	};

	PROPSHEETPAGE		psp[_countof(ComPropSheetInfoList)];
	for( nIdx = 0; nIdx < _countof(ComPropSheetInfoList); nIdx++ ){
		assert( ComPropSheetInfoList[nIdx].szTabname != NULL );

		PROPSHEETPAGE *p = &psp[nIdx];
		memset( p, 0, sizeof( *p ) );
		p->dwSize      = sizeof( *p );
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = m_hInstance;
		p->pszTemplate = MAKEINTRESOURCE( ComPropSheetInfoList[nIdx].resId );
		p->pszIcon     = NULL;
		p->pfnDlgProc  = ComPropSheetInfoList[nIdx].DProc;
		p->pszTitle    = ComPropSheetInfoList[nIdx].szTabname;
		p->lParam      = (LPARAM)this;
		p->pfnCallback = NULL;
	}
	//	To Here Jun. 2, 2001 genta

	PROPSHEETHEADER		psh;
	memset( &psh, 0, sizeof( psh ) );
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	JEPROtest Sept. 30, 2000 共通設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = m_hInstance;
	psh.pszIcon    = NULL;
	psh.pszCaption = _T("共通設定");
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage は unsigned なので負にならない
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}else
	if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}else{
		psh.nStartPage = nPageNum;
	}
	if( psh.nPages - 1 < psh.nStartPage ){
		psh.nStartPage = psh.nPages - 1;
	}

	psh.ppsp = psp;
	psh.pfnCallback = NULL;

	nRet = MyPropertySheet( &psh );	// 2007.05.24 ryoji 独自拡張プロパティシート
	if( -1 == nRet ){
		TCHAR*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			_T("CPropCommon::DoPropertySheet()内でエラーが出ました。\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()失敗\n")
			_T("\n")
			_T("%s\n"),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}

/*!	ShareDataから一時領域へ設定をコピーする
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::InitData( void )
{
	m_Common = m_pShareData->m_Common;

	//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			m_Types_nKeyWordSetIdx[i][j] = m_pShareData->m_Types[i].m_nKeyWordSetIdx[j];
		}
	}
}

/*!	ShareData に 設定を適用・コピーする
	@note ShareDataにコピーするだけなので，更新要求などは，利用する側で処理してもらう
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::ApplyData( void )
{
	m_pShareData->m_Common = m_Common;

	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
		/* 変更された設定値のコピー */
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			m_pShareData->m_Types[i].m_nKeyWordSetIdx[j] = m_Types_nKeyWordSetIdx[i][j];
		}
	}
}



/* ヘルプ */
//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
void CPropCommon::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP_GENERAL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//	変更は少し後の行(Sept. 9, 2000)で行っている
//	case IDD_PROP1P5:
//		nContextID = 84;
//		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	// From Here Sept. 9, 2000 JEPRO 共通設定のヘルプボタンが効かなくなっていた部分を以下の追加によって修正
	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_TAB:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TAB);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	// To Here Sept. 9, 2000
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;
	case IDD_PROP_FNAME:	// 2002.12.09 Moca FNAME追加
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FNAME);
		break;

	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
	return;
}


/*[EOF]*/
