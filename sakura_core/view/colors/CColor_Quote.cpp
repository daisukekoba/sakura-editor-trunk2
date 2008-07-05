#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 シングルクォーテーション                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_SingleQuote::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] == L'\'' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
	){
		/* シングルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLenOfLayoutWithNexts;
		for( i = pInfo->nPosInLogic + 1; i <= pInfo->nLineLenOfLayoutWithNexts - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLineOfLayout[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLenOfLayoutWithNexts && pInfo->pLineOfLayout[i + 1] == L'\'' ){
						++i;
					}
					else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
		}
		return COLORIDX_SSTRING;
	}
	return _COLORIDX_NOCHANGE;
}


bool CColor_SingleQuote::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* シングルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLenOfLayoutWithNexts;
		for( i = pInfo->nPosInLogic/* + 1*/; i <= pInfo->nLineLenOfLayoutWithNexts - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLineOfLayout[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLenOfLayoutWithNexts && pInfo->pLineOfLayout[i + 1] == L'\'' ){
						++i;
					}
					else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
		}
	}
	else if( pInfo->nPosInLogic == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  ダブルクォーテーション                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_DoubleQuote::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLineOfLayout)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLineOfLayout[pInfo->GetPosInLayout()] == L'"' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	){
		/* ダブルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLenOfLayoutWithNexts;
		for( i = pInfo->nPosInLogic + 1; i <= pInfo->nLineLenOfLayoutWithNexts - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\\' ){
					++i;
				}else
				if( pInfo->pLineOfLayout[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'"' ){
					if( i + 1 < pInfo->nLineLenOfLayoutWithNexts && pInfo->pLineOfLayout[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
		}
		return COLORIDX_WSTRING;
	}
	return _COLORIDX_NOCHANGE;
}



bool CColor_DoubleQuote::EndColor(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ダブルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLenOfLayoutWithNexts;
		for( i = pInfo->nPosInLogic/* + 1*/; i <= pInfo->nLineLenOfLayoutWithNexts - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLineOfLayout[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLineOfLayout[i] == L'"' ){
					if( i + 1 < pInfo->nLineLenOfLayoutWithNexts && pInfo->pLineOfLayout[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
		}
	}
	else if( pInfo->nPosInLogic == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}
