//	$Id$
/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "CShareData.h" // 2002/03/13 novice

//レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//なし
#define	KINSOKU_TYPE_WORDWRAP		1	//ワードラップ中
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//行頭禁則中
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//行末禁則中
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//句読点ぶら下げ中

//	/*
//	|| 	新しい折り返し文字数に合わせて全データのレイアウト情報を再生成します
//	||
//	*/
//	void CLayoutMgr::DoLayout(
//			int		nMaxLineSize,
//			BOOL	bWordWrap,	/* 英文ワードラップをする */
//			HWND	hwndProgress,
//			BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
//			BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
//	)
//	{
//		m_nMaxLineSize = nMaxLineSize;
//		m_bWordWrap = bWordWrap;		/* 英文ワードラップをする */
//		DoLayout( hwndProgress, bDispSSTRING, bDispWSTRING );
//		return;
//	}


/*
|| 	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します
||
*/
void CLayoutMgr::DoLayout(
		HWND	hwndProgress,
		BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DoLayout" );
#endif
	int			nLineNum;
	int			nLineLen;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	int			nPos;
	int			nPosX;
	int			nCharChars;
	int			nCharChars2;
	int			nCharChars_2;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nWordBgn;
	int			nWordLen;
	int			nAllLineNum;
	bool		bKinsokuFlag;	//@@@ 2002.04.08 MIK
	int			nCharChars3;	//@@@ 2002.04.17 MIK
	int			nKinsokuType;	//@@@ 2002.04.20 MIK


// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}

	Empty();
	Init();
	nLineNum = 0;

//	pLine = m_pcDocLineMgr->GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更

// 2002/03/13 novice
	if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
		nCOMMENTMODE_Prev = COLORIDX_TEXT;
	}
	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();
//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nCharChars = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int nEol_1 = pCDocLine->m_cEol.GetLen() - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		while( nPos < nLineLen - nEol_1 ){
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}

			SEARCH_START:;
			
			//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//禁則処理の最後尾に達したら禁則処理中を解除する
				if( nPos >= nWordBgn + nWordLen )
				{
					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* 英単語の先頭か */
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						for( i = nPos + 1; i <= nLineLen - 1; ){
							nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 == 1 &&
//								( pLine[i] == '#' || __iscsym( pLine[i] ) )
								IS_KEYWORD_CHAR( pLine[i] )
							){
							}else{
								break;
							}
							i += nCharChars2;
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						if( nPosX + i - nPos >= m_nMaxLineSize
						 && nPos - nBgn > 0
						){
							AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
//?							continue;
						}
					}
//				}else{
//					if( nPos == nWordBgn + nWordLen ){
//						nWordLen = 0;
//					}
//				}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_bKinsokuKuto
				 && (m_nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( m_nMaxLineSize - nPosX )
					{
					case 1:	// 1文字前
						if( nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 0:	// 
						if( nCharChars2 == 1 || nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
						if( ! (m_bKinsokuRet && (nPos + 1 == nLineLen - nEol_1) && nEol_1 ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							nPosX = 0;
						}
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (m_nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					nCharChars3 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2];
					switch( m_nMaxLineSize - nPosX )
					{
					//    321012  ↓マジックナンバー
					// 3 "る）" : 22 "）"の2バイト目で折り返しのとき
					// 2  "Z）" : 12 "）"の2バイト目で折り返しのとき
					// 2  "る）": 22 "）"で折り返しのとき
					// 2  "る)" : 21 ")"で折り返しのとき
					// 1   "Z）": 12 "）"で折り返しのとき
					// 1   "Z)" : 11 ")"で折り返しのとき
					//↑何文字前か？
					// ※ただし、"るZ"部分が禁則なら処理しない。
					case 3:	// 3文字前
						if( nCharChars2 == 2 && nCharChars3 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 2:	// 2文字前
						if( 2 == nCharChars2 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( nCharChars3 == 2 )
							{
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						nPosX = 0;
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (m_nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( m_nMaxLineSize - nPosX )
					{
					case 3:	// 3文字前
						if( nCharChars2 == 2 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "（あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 2:	// 2文字前
						if( nCharChars2 == 2 )
						{
							// "（あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 )
						{
							// "(あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						nPosX = 0;
					}
				}
				//@@@ 2002.04.08 MIK end
			}	//if( KINSOKU_TYPE_NONE != nKinsokuTyoe ) 禁則処理中

			switch( nCOMMENTMODE ){
			case COLORIDX_TEXT: // 2002/03/13 novice
				if( ( NULL != m_pszLineComment &&	/* 行コメントデリミタ */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment ) &&	/* 行コメントデリミタ */
					  0 == memicmp( &pLine[nPos], m_pszLineComment, (int)strlen( m_pszLineComment ) )
					) ||
					( NULL != m_pszLineComment2 &&	/* 行コメントデリミタ2 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment2 ) &&	/* 行コメントデリミタ2 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment2, (int)strlen( m_pszLineComment2 ) )
					) ||	//Jun. 01, 2001 JEPRO 3つ目を追加
					( NULL != m_pszLineComment3 &&	/* 行コメントデリミタ3 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment3 ) &&	/* 行コメントデリミタ3 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment3, (int)strlen( m_pszLineComment3 ) )
					)
				){
					nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
				}else
				if( NULL != m_pszBlockCommentFrom &&	/* ブロックコメントデリミタ(From) */
					NULL != m_pszBlockCommentTo &&		/* ブロックコメントデリミタ(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom ) &&	/* ブロックコメントデリミタ(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom, (int)strlen( m_pszBlockCommentFrom ) )
				){
					nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				}else
				if( NULL != m_pszBlockCommentFrom2 &&	/* ブロックコメントデリミタ2(From) */
					NULL != m_pszBlockCommentTo2 &&		/* ブロックコメントデリミタ2(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom2 ) &&	/* ブロックコメントデリミタ2(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom2, (int)strlen( m_pszBlockCommentFrom2 ) )
				){
					nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom2 ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#endif
				}else
				if( pLine[nPos] == '\'' &&
					bDispSSTRING  /* シングルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( pLine[nPos] == '"' &&
					bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}
				break;
			case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
				break;
			case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom2 )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
//#endif
			case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
			case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
			}
			if( pLine[nPos] == TAB ){
				nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
				if( nPosX + nCharChars > m_nMaxLineSize ){
					AddLineBottom( pCDocLine, /*pLine,*/nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					nPosX = 0;
					continue;
				}
				nPosX += nCharChars;
				nCharChars = 1;
				nPos+= nCharChars;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > m_nMaxLineSize ){
					if( ! (m_bKinsokuRet && (nPos + 1 == nLineLen - nEol_1) && nEol_1 ) )	//改行文字をぶら下げる	//@@@ 2002.04.14 MIK
					{	//@@@ 2002.04.14 MIK
						AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						nPosX = 0;
						continue;
					}	//@@@ 2002.04.14 MIK
				}
				nPos+= nCharChars;
				nPosX += nCharChars;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( pCDocLine, /*pLine,*/ nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( NULL != hwndProgress && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nLineNum * 100 / nAllLineNum , 0 );
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( NULL ) ){
				return;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;
	}
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}
	return;
}


//	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
//	int CLayoutMgr::DoLayout3(
//				CLayout* pLayoutPrev,
//				int		nLineNum,
//				int		nDelLogicalLineFrom,
//				int		nDelLogicalColFrom
//	 )
//	{
//		int			nLineNumWork;
//		int			nLineLen;
//		int			nCurLine;
//		const char* pLine;
//		int			nBgn;
//		int			nPos;
//		int			nPosX;
//		int			nCharChars;
//		CLayout*	pLayout;
//		int			nModifyLayoutLinesNew;
//		int			nCOMMENTMODE;
//		int			nCOMMENTMODE_Prev;
//	//	int			nCOMMENTEND;
//		nLineNumWork = 0;
//
//		if( 0 == nLineNum ){
//			return 0;
//		}
//		pLayout = pLayoutPrev;
//		if( NULL == pLayout ){
//			nCurLine = 0;
//		}else{
//			nCurLine = pLayout->m_nLinePhysical + 1;
//		}
//		nCOMMENTMODE = 0;
//		nCOMMENTMODE_Prev = 0;
//
//		pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
//		nModifyLayoutLinesNew = 0;
//
//		while( NULL != pLine ){
//			nPosX = 0;
//			nCharChars = 0;
//			nBgn = 0;
//			nPos = 0;
//			while( nPos < nLineLen ){
//				if( pLine[nPos] == TAB ){
//					nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
//					if( nPosX + nCharChars > m_nMaxLineSize ){
//						pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//						nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//						nBgn = nPos;
//						nPosX = 0;
//						if( ( nDelLogicalLineFrom == nCurLine &&
//							  nDelLogicalColFrom < nPos ) ||
//							( nDelLogicalLineFrom < nCurLine )
//						){
//							(nModifyLayoutLinesNew)++;;
//						}
//						continue;
//					}
//					nPos++;
//				}else{
//					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
//					if( 0 == nCharChars ){
//						nCharChars = 1;
//						break;
//					}
//					if( nPosX + nCharChars > m_nMaxLineSize ){
//						pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//						nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//						nBgn = nPos;
//						nPosX = 0;
//						if( ( nDelLogicalLineFrom == nCurLine &&
//							  nDelLogicalColFrom < nPos ) ||
//							( nDelLogicalLineFrom < nCurLine )
//						){
//							(nModifyLayoutLinesNew)++;;
//						}
//						continue;
//					}
//					nPos+= nCharChars;
//				}
//
//				nPosX += nCharChars;
//			}
//			if( nPos - nBgn > 0 ){
//				if( nCOMMENTMODE == 1 ){	/* 行コメントである */
//					nCOMMENTMODE = 0;
//				}
//				pLayout = InsertLineNext( pLayout, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
//				nCOMMENTMODE_Prev = nCOMMENTMODE;
//
//				if( ( nDelLogicalLineFrom == nCurLine &&
//					  nDelLogicalColFrom < nPos ) ||
//					( nDelLogicalLineFrom < nCurLine )
//				){
//					(nModifyLayoutLinesNew)++;;
//				}
//			}
//
//			nLineNumWork++;
//			nCurLine++;
//			if( nLineNumWork >= nLineNum ){
//				break;
//			}
//			pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
//		}
//		m_nPrevReferLine = 0;
//		m_pLayoutPrevRefer = NULL;
//		m_pLayoutCurrent = NULL;
//		return nModifyLayoutLinesNew;
//	}



/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
int CLayoutMgr::DoLayout3_New(
			CLayout* pLayoutPrev,
//			CLayout* pLayoutNext,
			int		nLineNum,
			int		nDelLogicalLineFrom,
			int		nDelLogicalColFrom,
			int		nCurrentLineType,
			int*	pnExtInsLineNum,
			BOOL	bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
			BOOL	bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DoLayout3_New" );
//#endif
	int			nLineNumWork;
	int			nLineLen;
	int			nCurLine;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	int			nPos;
	int			nPosX;
	int			nCharChars;
	int			nCharChars2;
	int			nCharChars_2;
	CLayout*	pLayout;
	int			nModifyLayoutLinesNew;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	CLayout*	pLayoutNext;
	int			bAdd = FALSE;
	int			nWordBgn;
	int			nWordLen;
	bool		bKinsokuFlag;	//@@@ 2002.04.08 MIK
	int			nCharChars3;	//@@@ 2002.04.17 MIK
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	nLineNumWork = 0;
	*pnExtInsLineNum = 0;
	if( 0 == nLineNum ){
		return 0;
	}
	pLayout = pLayoutPrev;
	if( NULL == pLayout ){
		nCurLine = 0;
	}else{
		nCurLine = pLayout->m_nLinePhysical + 1;
	}
	nCOMMENTMODE = nCurrentLineType;
	nCOMMENTMODE_Prev = nCOMMENTMODE;

//	pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetLineInfo( nCurLine );




//	if( nCOMMENTMODE_Prev == 1 ){	/* 行コメントである */
//		nCOMMENTMODE_Prev = 0;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;

	nModifyLayoutLinesNew = 0;

//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nCharChars = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int nEol_1 = pCDocLine->m_cEol.GetLen() - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		while( nPos < nLineLen - nEol_1 ){
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			SEARCH_START:;
			
			//禁則処理中ならスキップする	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//禁則処理の最後尾に達したら禁則処理中を解除する
				if( nPos >= nWordBgn + nWordLen )
				{
					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ワードラップ処理 */
				if( m_bWordWrap	/* 英文ワードラップをする */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* 英単語の先頭か */
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* キーワード文字列の終端を探す */
						int	i;
						for( i = nPos + 1; i <= nLineLen - 1; ){
							nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 == 1 && ( pLine[i] == '#' || __iscsym( pLine[i] ) ) ){
							}else{
								break;
							}
							i += nCharChars2;
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

						if( nPosX + i - nPos >= m_nMaxLineSize
						 && nPos - nBgn > 0
						){
							pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							if( bAdd ){
								CLayout*	pLayoutWork;
								pLayoutWork = pLayoutNext;
								pLayoutNext = pLayoutNext->m_pNext;
								pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
								if( NULL != pLayoutNext ){
									pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
								}else{
									m_pLayoutBot = pLayoutWork->m_pPrev;
								}

#ifdef _DEBUG
								if( m_pLayoutPrevRefer == pLayoutWork ){
									MYTRACE( "バグバグ\n" );
								}
#endif
								delete pLayoutWork;
								m_nLines--;

								(*pnExtInsLineNum)++;
							}

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine &&
								  nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
//?							continue;
						}
					}
//				}else{
//					if( nPos == nWordBgn + nWordLen ){
//						nWordLen = 0;
//					}
//				}
				}

				//@@@ 2002.04.07 MIK start
				/* 句読点のぶらさげ */
				if( m_bKinsokuKuto
				 && (m_nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( m_nMaxLineSize - nPosX )
					{
					case 1:	// 1文字前
						if( nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 0:	// 
						if( nCharChars2 == 1 || nCharChars2 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
						if( ! (m_bKinsokuRet && (nPos + 1 == nLineLen - nEol_1) && nEol_1 ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
						{
							pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							if( bAdd ){
								CLayout*	pLayoutWork;
								pLayoutWork = pLayoutNext;
								pLayoutNext = pLayoutNext->m_pNext;
								pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
								if( NULL != pLayoutNext ){
									pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
								}else{
									m_pLayoutBot = pLayoutWork->m_pPrev;
								}

#ifdef _DEBUG
								if( m_pLayoutPrevRefer == pLayoutWork ){
									MYTRACE( "バグバグ\n" );
								}
#endif
								delete pLayoutWork;
								m_nLines--;

								(*pnExtInsLineNum)++;
							}

							nBgn = nPos;
							nPosX = 0;
							if( ( nDelLogicalLineFrom == nCurLine &&
								  nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
						}
					}
				}

				/* 行頭禁則 */
				if( m_bKinsokuHead
				 && (m_nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					nCharChars3 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2];
					switch( m_nMaxLineSize - nPosX )
					{
					//    321012  ↓マジックナンバー
					// 3 "る）" : 22 "）"の2バイト目で折り返しのとき
					// 2  "Z）" : 12 "）"の2バイト目で折り返しのとき
					// 2  "る）": 22 "）"で折り返しのとき
					// 2  "る)" : 21 ")"で折り返しのとき
					// 1   "Z）": 12 "）"で折り返しのとき
					// 1   "Z)" : 11 ")"で折り返しのとき
					//↑何文字前か？
					// ※ただし、"るZ"部分が禁則なら処理しない。
					case 3:	// 3文字前
						if( nCharChars2 == 2 && nCharChars3 == 2 )
						{
							bKinsokuFlag = true;
						}
						break;
					case 2:	// 2文字前
						if( 2 == nCharChars2 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						else if( nCharChars2 == 1 )
						{
							if( nCharChars3 == 2 )
							{
								bKinsokuFlag = true;
							}
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 /*&& nCharChars3 > 0*/ )
						{
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1文字前が行頭禁則でない
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//句読点でない
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						if( bAdd ){
							CLayout*	pLayoutWork;
							pLayoutWork = pLayoutNext;
							pLayoutNext = pLayoutNext->m_pNext;
							pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
							if( NULL != pLayoutNext ){
								pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
							}else{
								m_pLayoutBot = pLayoutWork->m_pPrev;
							}

#ifdef _DEBUG
							if( m_pLayoutPrevRefer == pLayoutWork ){
								MYTRACE( "バグバグ\n" );
							}
#endif
							delete pLayoutWork;
							m_nLines--;

							(*pnExtInsLineNum)++;
						}

						nBgn = nPos;
						nPosX = 0;
						if( ( nDelLogicalLineFrom == nCurLine &&
							  nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}

				/* 行末禁則 */
				if( m_bKinsokuTail
				 && (m_nMaxLineSize - nPosX < 4)
				 && nPosX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう) */
					bKinsokuFlag = false;
					nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					switch( m_nMaxLineSize - nPosX )
					{
					case 3:	// 3文字前
						if( nCharChars2 == 2 )
						{
							if( CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos+nCharChars2] ) - &pLine[nPos+nCharChars2] == 2 )
							{
								// "（あ": "あ"の2バイト目で折り返しのとき
								bKinsokuFlag = true;
							}
						}
						break;
					case 2:	// 2文字前
						if( nCharChars2 == 2 )
						{
							// "（あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					case 1:	// 1文字前
						if( nCharChars2 == 1 )
						{
							// "(あ": "あ"で折り返しのとき
							bKinsokuFlag = true;
						}
						break;
					}

					if( bKinsokuFlag && IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						if( bAdd ){
							CLayout*	pLayoutWork;
							pLayoutWork = pLayoutNext;
							pLayoutNext = pLayoutNext->m_pNext;
							pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
							if( NULL != pLayoutNext ){
								pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
							}else{
								m_pLayoutBot = pLayoutWork->m_pPrev;
							}

#ifdef _DEBUG
							if( m_pLayoutPrevRefer == pLayoutWork ){
								MYTRACE( "バグバグ\n" );
							}
#endif
							delete pLayoutWork;
							m_nLines--;

							(*pnExtInsLineNum)++;
						}

						nBgn = nPos;
						nPosX = 0;
						if( ( nDelLogicalLineFrom == nCurLine &&
							  nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			switch( nCOMMENTMODE ){
			case COLORIDX_TEXT: // 2002/03/13 novice
				if( ( NULL != m_pszLineComment &&	/* 行コメントデリミタ */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment ) &&	/* 行コメントデリミタ */
					  0 == memicmp( &pLine[nPos], m_pszLineComment, (int)strlen( m_pszLineComment ) )
					) ||
					( NULL != m_pszLineComment2 &&	/* 行コメントデリミタ2 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment2 ) &&	/* 行コメントデリミタ2 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment2, (int)strlen( m_pszLineComment2 ) )
					) ||	//Jun. 01, 2001 JEPRO 3つ目を追加
					( NULL != m_pszLineComment3 &&	/* 行コメントデリミタ3 */
					  nPos <= nLineLen - (int)strlen( m_pszLineComment3 ) &&	/* 行コメントデリミタ3 */
					  0 == memicmp( &pLine[nPos], m_pszLineComment3, (int)strlen( m_pszLineComment3 ) )
					)
				){
					nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice
				}else
				if( NULL != m_pszBlockCommentFrom &&	/* ブロックコメントデリミタ(From) */
					NULL != m_pszBlockCommentTo &&		/* ブロックコメントデリミタ(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom ) &&	/* ブロックコメントデリミタ(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom, (int)strlen( m_pszBlockCommentFrom ) )
				){
					nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				}else
				if( NULL != m_pszBlockCommentFrom2 &&	/* ブロックコメントデリミタ2(From) */
					NULL != m_pszBlockCommentTo2 &&		/* ブロックコメントデリミタ2(To) */
					nPos <= nLineLen - (int)strlen( m_pszBlockCommentFrom2 ) &&	/* ブロックコメントデリミタ2(From) */
					0 == memicmp( &pLine[nPos], m_pszBlockCommentFrom2, (int)strlen( m_pszBlockCommentFrom2 ) )
				){
					nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + (int)strlen( m_pszBlockCommentFrom2 ); i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
//#endif
				}else
				if( pLine[nPos] == '\'' &&
					bDispSSTRING  /* シングルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( pLine[nPos] == '"' &&
					bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
				){
					nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos + 1; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}
				break;
			case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
				break;
			case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo, (int)strlen( m_pszBlockCommentTo )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* この物理行にブロックコメントの終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + (int)strlen( m_pszBlockCommentFrom2 )*/; i <= nLineLen - (int)strlen( m_pszBlockCommentTo2 ); ++i ){
						nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars_2 ){
							nCharChars_2 = 1;
						}
						if( 0 == memicmp( &pLine[i], m_pszBlockCommentTo2, (int)strlen( m_pszBlockCommentTo2 )	) ){
							nCOMMENTEND = i + (int)strlen( m_pszBlockCommentTo2 );
							break;
						}
						if( 2 == nCharChars_2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
//#endif
			case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* シングルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\'' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
			case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
				if( 0 == nCOMMENTEND ){
					/* ダブルクォーテーション文字列の終端があるか */
					int i;
					nCOMMENTEND = nLineLen;
					for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
						nCharChars2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
						if( 0 == nCharChars2 ){
							nCharChars2 = 1;
						}
						if(	m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '\\' ){
								++i;
							}else
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								nCOMMENTEND = i + 1;
								break;
							}
						}else
						if(	m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
							if( 1 == nCharChars2 && pLine[i] == '"' ){
								if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
									++i;
								}else{
									nCOMMENTEND = i + 1;
									break;
								}
							}
						}
						if( 2 == nCharChars2 ){
							++i;
						}
					}
				}else
				if( nPos == nCOMMENTEND ){
					nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
					goto SEARCH_START;
				}
				break;
			}


			if( pLine[nPos] == TAB ){
				nCharChars = m_nTabSpace - ( nPosX % m_nTabSpace );
				if( nPosX + nCharChars > m_nMaxLineSize ){
					pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					if( bAdd ){
						CLayout*	pLayoutWork;
						pLayoutWork = pLayoutNext;
						pLayoutNext = pLayoutNext->m_pNext;
						pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
						if( NULL != pLayoutNext ){
							pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
						}else{
							m_pLayoutBot = pLayoutWork->m_pPrev;
						}
#ifdef _DEBUG
						if( m_pLayoutPrevRefer == pLayoutWork ){
							MYTRACE( "バグバグ\n" );
						}
#endif
						delete pLayoutWork;
						m_nLines--;

						(*pnExtInsLineNum)++;
					}
					nBgn = nPos;
					nPosX = 0;
					if( ( nDelLogicalLineFrom == nCurLine &&
						  nDelLogicalColFrom < nPos ) ||
						( nDelLogicalLineFrom < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos++;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > m_nMaxLineSize ){
					if( ! (m_bKinsokuRet && (nPos + 1 == nLineLen - nEol_1) && nEol_1 ) )	//改行文字をぶら下げる		//@@@ 2002.04.14 MIK
					{	//@@@ 2002.04.14 MIK
						pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						if( bAdd ){
							CLayout*	pLayoutWork;
							pLayoutWork = pLayoutNext;
							pLayoutNext = pLayoutNext->m_pNext;
							pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
							if( NULL != pLayoutNext ){
								pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
							}else{
								m_pLayoutBot = pLayoutWork->m_pPrev;
							}
#ifdef _DEBUG
							if( m_pLayoutPrevRefer == pLayoutWork ){
								MYTRACE( "バグバグ\n" );
							}
#endif
							delete pLayoutWork;
							m_nLines--;

							(*pnExtInsLineNum)++;
						}

						nBgn = nPos;
						nPosX = 0;
						if( ( nDelLogicalLineFrom == nCurLine &&
							  nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
						continue;
					}
				}
				nPos+= nCharChars;
			}

			nPosX += nCharChars;
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* 行コメントである */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			pLayout = InsertLineNext( pLayout, pCDocLine, /*pLine,*/ nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nCOMMENTMODE );
			nCOMMENTMODE_Prev = nCOMMENTMODE;
			if( bAdd ){
				CLayout*	pLayoutWork;
				pLayoutWork = pLayoutNext;
				pLayoutNext = pLayoutNext->m_pNext;
				pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
				if( NULL != pLayoutNext ){
					pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
				}else{
					m_pLayoutBot = pLayoutWork->m_pPrev;
				}
#ifdef _DEBUG
				if( m_pLayoutPrevRefer == pLayoutWork ){
					MYTRACE( "バグバグ\n" );
				}
#endif
				delete pLayoutWork;
				m_nLines--;

				(*pnExtInsLineNum)++;
			}

			if( ( nDelLogicalLineFrom == nCurLine &&
				  nDelLogicalColFrom < nPos ) ||
				( nDelLogicalLineFrom < nCurLine )
			){
				(nModifyLayoutLinesNew)++;;
			}
		}

		nLineNumWork++;
		nCurLine++;
		if( nLineNumWork >= nLineNum ){
//			pLayoutNext = pLayout->m_pNext;
			if( NULL != pLayout
			 && NULL != ( pLayoutNext = pLayout->m_pNext )
			){
				if( nCOMMENTMODE_Prev == pLayoutNext->m_nTypePrev ){
					break;
				}else{
//					CLayout*	pLayoutWork;
//					pLayoutWork = pLayoutNext;
//					pLayoutNext = pLayoutNext->m_pNext;
//					pLayoutWork->m_pPrev->m_pNext = pLayoutNext;
//					if( NULL != pLayoutNext ){
//						pLayoutNext->m_pPrev = pLayoutWork->m_pPrev;
//					}else{
//						m_pLayoutBot = pLayoutWork->m_pPrev;
//					}
//					delete pLayoutWork;
//					m_nLines--;

					bAdd = TRUE;



//					int i;
//					i = 1;
//					break;
				}
			}else{
				break;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* 行コメントである */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;

	}

// 1999.12.22 レイアウト情報がなくなる訳ではないので
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return nModifyLayoutLinesNew;
}

bool CLayoutMgr::IsKinsokuHead( const char *pLine, int length )
{
	const unsigned char	*p;

	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuHead_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuHead_2;
	else return false;

	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

bool CLayoutMgr::IsKinsokuTail( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuTail_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuTail_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

bool CLayoutMgr::IsKutoTen( unsigned char c1, unsigned char c2 )
{
	static const char	*KUTOTEN_1 = "｡､,.";
	static const char	*KUTOTEN_2 = "。、，．";
	unsigned const char	*p;

	if( c2 )	//全角
	{
		for( p = (const unsigned char *)KUTOTEN_2; *p; p += 2 )
		{
			if( *p == c1 && *(p + 1) == c2 ) return true;
		}
	}
	else		//半角
	{
		for( p = (const unsigned char *)KUTOTEN_1; *p; p++ )
		{
			if( *p == c1 ) return true;
		}
	}

	return false;
}

bool CLayoutMgr::IsKinsokuKuto( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuKuto_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuKuto_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*[EOF]*/
