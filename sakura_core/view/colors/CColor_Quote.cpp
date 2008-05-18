#include "stdafx.h"
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 �V���O���N�H�[�e�[�V����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_SingleQuote::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'\'' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
	){
		/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
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
		/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
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
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  �_�u���N�H�[�e�[�V����                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EColorIndexType CColor_DoubleQuote::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'"' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
	){
		/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
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
		/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'\\' ){
					++i;
				}
				else if( pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
				if( pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
		}
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		return true;
	}
	return false;
}
