#include "stdafx.h"
#include "CColor_KeywordSet.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードセット                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
EColorIndexType CColor_KeywordSet::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	
	if(pInfo->nCOMMENTMODE!=COLORIDX_TEXT)return _COLORIDX_NOCHANGE;

	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
		pInfo->IsPosKeywordHead() && IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] )
	){
		// キーワードの開始 -> iKeyBegin
		int iKeyBegin = pInfo->nPos;

		// キーワードの終端 -> iKeyEnd
		int iKeyEnd;
		for( iKeyEnd = iKeyBegin + 1; iKeyEnd <= pInfo->nLineLen - 1; ++iKeyEnd ){
			if( !IS_KEYWORD_CHAR( pInfo->pLine[iKeyEnd] ) ){
				break;
			}
		}

		// キーワードの長さ -> nKeyLen
		int nKeyLen = iKeyEnd - iKeyBegin;

		// キーワードが色変え対象であるか調査
		for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ )
		{
			if( TypeDataPtr->m_nKeyWordSetIdx[i] != -1 && // キーワードセット
				TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + i].m_bDisp)								//MIK
			{																							//MIK
				/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
				int nIdx = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2(							//MIK 2000.12.01 binary search
					TypeDataPtr->m_nKeyWordSetIdx[i] ,													//MIK
					&pInfo->pLine[iKeyBegin],															//MIK
					nKeyLen																				//MIK
				);																						//MIK
				if( nIdx != -1 ){																		//MIK
					pInfo->nCOMMENTEND = iKeyEnd;														//MIK
					return (EColorIndexType)(COLORIDX_KEYWORD1 + i);
				}																						//MIK
			}																							//MIK
		}
	}
	return _COLORIDX_NOCHANGE;
}

bool CColor_KeywordSet::EndColor(SColorStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

