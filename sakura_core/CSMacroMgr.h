//	$Id$
/*!	@file
	キーボードマクロ(直接実行用)

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CSMacroMgr;

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>
#include <vector>
#include "CMemory.h"
#include "CShareData.h"

using namespace std;

class CEditView;

//#define MAX_STRLEN			70
//#define MAX_SMacroNUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	データの型宣言
	typedef vector<KeyMacroData> KeyMacroList;

	/*! マクロ１つの情報を格納する */
	struct Macro1 {
		KeyMacroList m_mac; //!< マクロ本体
		vector<string> m_strlist; //!< 補助文字列
		bool m_flag; //!< 読み込み済みかどうかを表すフラグ
	
		Macro1() : m_flag(false){}
		
		void Reset(void){
			m_flag = false;
			m_strlist.clear();
			m_mac.clear();
		}
		
		bool IsReady(void) const { return m_flag; }
	};
	
public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll( void );	/* キーマクロのバッファをクリアする */

	/*! キーボードマクロの実行 */
	BOOL Exec( HINSTANCE hInstance, CEditView* pViewClass, int idx );
	
	//	登録インターフェース
	//! 有効・無効の切り替え
	void Enable(int idx, bool state){
		if( idx < 0 || MAX_CUSTMACRO <= idx )
			m_pShareData->m_MacroTable[idx].Enable( state );
	}
	bool IsEnabled(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	表示する名前の設定
	const char* GetTitle(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
			( m_pShareData->m_MacroTable[idx].m_szName[0] == '\0' ?
				m_pShareData->m_MacroTable[idx].m_szFile : 
				m_pShareData->m_MacroTable[idx].m_szName)
			: NULL;
	}
	
	//!	表示名の設定
	BOOL SetName(int idx, const char *);
	
	//!	表示名の取得
	const char* GetName(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	//!	ファイル名の設定
	BOOL SetFile(int idx, const char *);
	
	//!	ファイル名の取得
	const char* GetFile(int idx, const char *) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

protected:
	/*! キーボードマクロの読み込み */
	BOOL Load( CSMacroMgr::Macro1& mbuf, HINSTANCE hInstance, const char* pszPath );
	
	/*! キーマクロのバッファにデータ追加 */
	int Append( CSMacroMgr::Macro1& mbuf, int nFuncID, LPARAM lParam1 );

private:
	vector<Macro1> m_vMacro;

	DLLSHAREDATA*	m_pShareData;

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
