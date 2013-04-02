/*! @file
	@brief End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"

// 2002/09/22 Moca EOL_CRLF_UNICODEを廃止
/* 行終端子の種類 */
enum EEolType {
	EOL_NONE,			//!<
	EOL_CRLF,			//!< 0d0a
	EOL_LFCR,			//!< 0a0d
	EOL_LF,				//!< 0a
	EOL_CR,				//!< 0d
	EOL_CODEMAX,		//
	EOL_UNKNOWN = -1	//
};

/* 行終端子のデータ長 */
enum enumEOLLen {
	LEN_EOL_NONE			= 0,
	LEN_EOL_CRLF			= 2,
	LEN_EOL_LFCR			= 2,
	LEN_EOL_LF				= 1,
	LEN_EOL_CR				= 1
//	LEN_EOL_UNKNOWN		0
};

#define EOL_TYPE_NUM	5

/* 行終端子の配列 */
extern const EEolType gm_pnEolTypeArr[EOL_TYPE_NUM];


/*!
	@brief 行末の改行コードを管理

	管理とは言ってもオブジェクト化することで安全に設定を行えたり関連情報の取得を
	オブジェクトに対するメソッドで行えるだけだが、グローバル変数への参照を
	クラス内部に閉じこめることができるのでそれなりに意味はあると思う。
*/
class CEol{
public:
	//コンストラクタ・デストラクタ
	CEol(){ m_eEolType = EOL_NONE; }
	CEol( EEolType t ){ SetType(t); }

	//比較
	bool operator==( EEolType t ) const { return GetType() == t; }
	bool operator!=( EEolType t ) const { return GetType() != t; }

	//代入
	const CEol& operator=( const CEol& t ){ m_eEolType = t.m_eEolType; return *this; }

	//型変換
	operator EEolType() const { return GetType(); }

	//設定
	bool SetType( EEolType t);	//	Typeの設定
	void SetTypeByString( const char* pszData, int nDataLen );

	//設定（ファイル読み込み時に使用）

	void SetTypeByStringForFile_uni( const wchar_t* pszData, int nDataLen );
	void SetTypeByStringForFile_unibe( const wchar_t* pszData, int nDataLen );

	//取得
	EEolType GetType() const { return m_eEolType; }	//!< 現在のTypeを取得
	int GetLen() const;								//!< 現在のEOL長を取得
	const TCHAR* GetName() const;					//!< 現在のEOLの名称取得
	const char* GetValue() const;					//!< 現在のEOL文字列先頭へのポインタを取得
	const char* GetUnicodeValue() const;			//!< Unicode版GetValue	2002/5/9 Frozen
	const char* GetUnicodeBEValue() const;			//!< UnicodeBE版のGetValue 2002.05.30 Moca

private:
	EEolType		m_eEolType;	//!< 改行コードの種類
};


#endif


/*[EOF]*/
