#include "stdafx.h"
#include "CColor_KeywordSet.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�Z�b�g                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
EColorIndexType CColor_KeywordSet::BeginColor(SColorStrategyInfo* pInfo)
{
	if(!pInfo->pLine)return _COLORIDX_NOCHANGE;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	
	if(pInfo->nCOMMENTMODE!=COLORIDX_TEXT)return _COLORIDX_NOCHANGE;

	/*
		Summary:
			���݈ʒu����L�[���[�h�𔲂��o���A���̃L�[���[�h���o�^�P��Ȃ�΁A�F��ς���
	*/
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
		pInfo->IsPosKeywordHead() && IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] )
	){
		// �L�[���[�h�̊J�n -> iKeyBegin
		int iKeyBegin = pInfo->nPos;

		// �L�[���[�h�̏I�[ -> iKeyEnd
		int iKeyEnd;
		for( iKeyEnd = iKeyBegin + 1; iKeyEnd <= pInfo->nLineLen - 1; ++iKeyEnd ){
			if( !IS_KEYWORD_CHAR( pInfo->pLine[iKeyEnd] ) ){
				break;
			}
		}

		// �L�[���[�h�̒��� -> nKeyLen
		int nKeyLen = iKeyEnd - iKeyBegin;

		// �L�[���[�h���F�ς��Ώۂł��邩����
		for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ )
		{
			if( TypeDataPtr->m_nKeyWordSetIdx[i] != -1 && // �L�[���[�h�Z�b�g
				TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + i].m_bDisp)								//MIK
			{																							//MIK
				/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
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

