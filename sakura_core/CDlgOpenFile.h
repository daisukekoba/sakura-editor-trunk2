/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, ryoji
	Copyright (C) 2004, genta, MIK
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, Moca, ryoji
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include <vector>
#include "design_template.h"
#include "CShareData.h"
#include "CEol.h"
#include "COsVersionInfo.h"	// 2005.11.02 ryoji

struct OPENFILENAMEZ;
class CDlgOpenFileMem;

/*! フィルタ設定 */
enum EFilter {
	EFITER_NONE,		//!< なし
	EFITER_TEXT,		//!< テキスト
	EFITER_MACRO,		//!< マクロ
	EFITER_MAX,
};

/*!	ファイルオープンダイアログボックス

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CDlgOpenFile
{
public:
	//コンストラクタ・デストラクタ
	CDlgOpenFile();
	~CDlgOpenFile();
	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU			= std::vector<LPCTSTR>(),
		const std::vector<LPCTSTR>& vOPENFOLDER		= std::vector<LPCTSTR>()
	);

	//操作
	bool DoModal_GetOpenFileName( TCHAR*, EFilter eAddFileter = EFITER_TEXT );	/* 開くダイアログ モーダルダイアログの表示 */	//2002/08/21 moca	引数追加
	bool DoModal_GetSaveFileName( TCHAR* );	/* 保存ダイアログ モーダルダイアログの表示 */	//2002/08/21 30,2002 moca	引数追加
	bool DoModalOpenDlg( TCHAR* , ECodeType*, bool*, std::vector<std::tstring>* );	/* 開くダイアグ モーダルダイアログの表示 */
	bool DoModalSaveDlg( char* , ECodeType*, CEol*, bool* );	/* 保存ダイアログ モーダルダイアログの表示 */

protected:
	CDlgOpenFileMem*	m_mem;

	/*
	||  実装ヘルパ関数
	*/

	//	May 29, 2004 genta エラー処理をまとめる (advised by MIK)
	void	DlgOpenFail(void);

	// 2005.11.02 ryoji OS バージョン対応の OPENFILENAME 初期化用関数
	void InitOfn( OPENFILENAMEZ* );

	// 2005.11.02 ryoji 初期レイアウト設定処理
	static void InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl );

	// 2005.10.29 ryoji コンボボックスのドロップダウン時処理
	static void OnCbnDropDown( HWND hwnd );

	// 2006.09.03 Moca ファイルダイアログのエラー回避
	//! リトライ機能付き GetOpenFileName
	bool _GetOpenFileNameRecover( OPENFILENAMEZ* ofn );
	//! リトライ機能付き GetOpenFileName
	bool GetSaveFileNameRecover( OPENFILENAMEZ* ofn );

	BOOL CheckPathLengthOverflow( const char *pszPath, int nLength, BOOL bErrDisp = TRUE );	// 2008.11.23 nasukoji	指定のファイルパスのバッファオーバーフローをチェックする

	friend UINT_PTR CALLBACK OFNHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );

public:
	// 設定フォルダ相対ファイル選択(共有データ,ini位置依存)
	static BOOL SelectFile(HWND parent, HWND hwndCtl, const TCHAR* filter, bool resolvePath,  EFilter eAddFilter = EFITER_TEXT);

private:
	DISALLOW_COPY_AND_ASSIGN(CDlgOpenFile);
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */


/*[EOF]*/
