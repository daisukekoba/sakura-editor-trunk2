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
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/*! �A�Z���u�� �A�E�g���C�����

	@author MIK
	@date 2004.04.12 ��蒼��
*/
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	int nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( int nLineCount = 0; nLineCount < nTotalLine; nLineCount++ ){
		const TCHAR* pLine;
		int nLineLen;
		TCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		TCHAR* token[MAX_ASM_TOKEN];
		int j;
		TCHAR* p;

		//1�s�擾����B
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//��Ɨp�ɃR�s�[���쐬����B�o�C�i�����������炻�̌��͒m��Ȃ��B
		pTmpLine = _tcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( _tcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//�o�C�i�����܂�ł�����Z���Ȃ�̂�...
			pTmpLine[ nLineLen ] = _T('\0');	//�w�蒷�Ő؂�l��
		}

		//�s�R�����g�폜
		p = _tcsstr( pTmpLine, _T(";") );
		if( p ) *p = _T('\0');

		length = _tcslen( pTmpLine );
		offset = 0;

		//�g�[�N���ɕ���
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok( pTmpLine, length, &offset, _T(" \t\r\n") );
			if( token[ j ] == NULL ) break;
			//�g�[�N���Ɋ܂܂��ׂ������łȂ����H
			if( _tcsstr( token[ j ], _T("\"")) != NULL
			 || _tcsstr( token[ j ], _T("\\")) != NULL
			 || _tcsstr( token[ j ], _T("'" )) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//�g�[�N����1�ȏ゠��
			int nFuncId = -1;
			TCHAR* entry_token = NULL;

			length = _tcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == _T(':') ){	//���x��
				token[ 0 ][ length - 1 ] = _T('\0');
				nFuncId = 51;
				entry_token = token[ 0 ];
			}else
			if( token[ 1 ] != NULL ){	//�g�[�N����2�ȏ゠��
				if( my_stricmp( token[ 1 ], _T("proc") ) == 0 ){	//�֐�
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( my_stricmp( token[ 1 ], _T("endp") ) == 0 ){	//�֐��I��
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//�}�N��
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//�\����
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
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
				pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}

/*[EOF]*/
