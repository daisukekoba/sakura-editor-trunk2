/*!	@file
	@brief Entry Point

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date	2001/06/26 genta ワード単位のGrepのためのコマンドライン処理追加
	@date	2002/01/08 aroka 処理の流れを整理、未使用コードを削除
	@date	2002/01/18 aroka 虫取り＆リリース
	@date	2009/01/07 ryoji WinMainにOleInitialize/OleUninitializeを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
/* LMP (Lucien Murray-Pitts) : 2011-02-26 Added Basic English Translation Resources */

#include "stdafx.h"
#include <windows.h>
#include "CProcessFactory.h"
#include "CProcess.h"
#include "etc_uty.h"
#include "CRunningTimer.h"

/*!
	Windows Entry point

	1つ目のエディタプロセスの場合は、このプロセスはコントロールプロセスと
	なり、新しいエディタプロセスを起動する。そうでないときはエディタプロセス
	となる。

	コントロールプロセスはCControlProcessクラスのインスタンスを作り、
	エディタプロセスはCNormalProcessクラスのインスタンスを作る。
*/
extern char gm_pszCodeComboNameArr_Auto[];
int WINAPI WinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
{
	// LMP: Added
	// 1036 = french, 1031 = german 
	SetThreadUILanguage( MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT) );		// Vista / Win7
	SetThreadLocale(MAKELCID(LANG_JAPANESE, SORT_DEFAULT));					// Win2000/XP

	SetThreadUILanguage( MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT) );		// Vista / Win7
	SetThreadLocale(MAKELCID(LANG_ENGLISH, SORT_DEFAULT));					// Win2000/XP

	// Global String Table setup - FIXME : HACK
	::LoadString( GetModuleHandle(NULL), STR_ERR_GLOBAL01, gm_pszCodeComboNameArr_Auto, 32 );  // LMP: Added
	

	MY_RUNNINGTIMER(cRunningTimer, "WinMain" );
	{
		// 2010.08.28 Moca OleInitialize用に移動
		CCurrentDirectoryBackupPoint dirBack;
		ChangeCurrentDirectoryToExeDir();
 
		::OleInitialize( NULL );	// 2009.01.07 ryoji 追加
	}

	CProcessFactory aFactory;
	CProcess *process = 0;
	try{
		process = aFactory.Create( hInstance, lpCmdLine );
		MY_TRACETIME( cRunningTimer, "ProcessObject Created" );
	}
	catch(...){
	}
	if( 0 != process ){
		process->Run();
		delete process;
	}

	::OleUninitialize();	// 2009.01.07 ryoji 追加
	return 0;
}

/*[EOF]*/
