/*
	Copyright (C) 2008, kobake

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

#include "StdAfx.h"
#include "CEditDoc.h"
#include "charcode.h"
#include "CFuncInfoArr.h"// 2002/2/10 aroka

/* Java��̓��[�h */
enum EFuncListJavaMode {
	FL_JAVA_MODE_NORMAL = 0,
	FL_JAVA_MODE_WORD = 1,
	FL_JAVA_MODE_SYMBOL = 2,
	FL_JAVA_MODE_COMMENT = 8,
	FL_JAVA_MODE_SINGLE_QUOTE = 20,
	FL_JAVA_MODE_DOUBLE_QUOTE = 21,
	FL_JAVA_MODE_TOO_LONG_WORD = 999
};

/* Java�֐����X�g�쐬 */
void CEditDoc::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[100];
	int			nFuncLine = 0;
	int			nFuncId;
	int			nFuncNum;
	char		szClass[1024];

	int			nClassNestArrNum;
	std::vector<int>	nClassNestArr(0);
	std::vector<int>	nNestLevel2Arr(0);

	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = FL_JAVA_MODE_NORMAL;
	//nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = '\0';
	nClassNestArrNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			//nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( FL_JAVA_MODE_SINGLE_QUOTE == nMode ){
				if( '\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( FL_JAVA_MODE_DOUBLE_QUOTE == nMode ){
				if( '"' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( FL_JAVA_MODE_COMMENT == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}
			/* �P��ǂݍ��ݒ� */
			else if( FL_JAVA_MODE_WORD == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = FL_JAVA_MODE_TOO_LONG_WORD;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					/* �N���X�錾������������ */
					//	Oct. 10, 2002 genta interface���Ώۂ�
					if( 0 == strcmp( "class", szWordPrev ) ||
						0 == strcmp( "interface", szWordPrev )
					 ){
						nClassNestArr.push_back( nNestLevel );
						nNestLevel2Arr.push_back( 0 );
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							strcat( szClass, "\\" );
						}
						strcat( szClass, szWord );

						nFuncId = FL_OBJ_DEFINITION;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.LogicToLayout(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
						char szWork[256];
						wsprintf( szWork, "%s::%s", szClass, "��`�ʒu" );
						pcFuncInfoArr->AppendData( nPosY + 1/*nFuncLine*/, nPosY + 1, szWork, nFuncId );

					}

					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( FL_JAVA_MODE_SYMBOL == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i] ||
					'{' == pLine[i] ||
					'}' == pLine[i] ||
					'(' == pLine[i] ||
					')' == pLine[i] ||
					';' == pLine[i]	||
					'\'' == pLine[i] ||
					'"' == pLine[i] ||
					'/' == pLine[i] ||
					'.' == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}else{
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( FL_JAVA_MODE_TOO_LONG_WORD == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( FL_JAVA_MODE_NORMAL == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_COMMENT;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_SINGLE_QUOTE;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = FL_JAVA_MODE_DOUBLE_QUOTE;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	���\�b�h���ł���Ƀ��\�b�h���`���邱�Ƃ͂Ȃ��̂�
						//	�l�X�g���x������ǉ� class/interface�̒����̏ꍇ�̂ݔ��肷��
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = FL_OBJ_FUNCTION;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.LogicToLayout(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( '}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArr.pop_back();
						nNestLevel2Arr.pop_back();
						nClassNestArrNum--;
						int k;
						for( k = lstrlen( szClass ) - 1; k >= 0; k-- ){
							if( '\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = '\0';
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( '(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != strcmp( "new", szWordPrev )
					){
						strcpy( szFuncName, szWord );
						nFuncLine = nLineCount + 1;
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( ')' == pLine[i] ){
					int			k;
					const char*	pLine2;
					int			nLineLen2;
					int			nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != ' ' && pLine2[k] != TAB && pLine2[k] != CR && pLine2[k] != LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == '*' && pLine2[k + 1] == '/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_cDocLineMgr.GetLineStr( nLineCount2, &nLineLen2 );
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abstract �ɂ��Ή�
						if( pLine2[k] == '{' || pLine2[k] == ';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// �֐��̒��ŕʂ̊֐��̐錾�����g�����Ƃ��āCJava�ł���́H
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = FL_OBJ_DECLARE;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.LogicToLayout(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
					if( '_' == pLine[i] ||
						':' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )||
						'.' == pLine[i]
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = FL_JAVA_MODE_WORD;
					}else{
						nMode = FL_JAVA_MODE_NORMAL;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}

/*[EOF]*/
