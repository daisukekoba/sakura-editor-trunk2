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
#include "CFuncInfoArr.h" /// 2002/2/3 aroka

/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()���g�p����悤�ɉ����B
	@date 2002.11.03 Moca �K�w���ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
*/
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						nCharChars;
	int						nCharChars2;
	const char*				pszStarts;
	int						nStartsLen;


	pszStarts = m_pShareData->m_Common.m_sFormat.m_szMidashiKigou; 	/* ���o���L�� */
	nStartsLen = lstrlen( pszStarts );

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char pszStack[nMaxStack][32];
	char szTitle[32];			//	�ꎞ�̈�
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		for( j = 0; j < nStartsLen; j+=nCharChars2 ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars2 = CMemory::GetSizeOfChar( pszStarts, nStartsLen, j );
			if( nCharChars == nCharChars2 ){
				if( 0 == memcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					strncpy( szTitle, &pszStarts[j], nCharChars);	//	szTitle�ɕێ��B
					szTitle[nCharChars] = '\0';
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}
		/* ���o��������(���܂܂�Ă��邱�Ƃ��O��ɂȂ��Ă���! */
		if( nCharChars == 1 && pLine[i] == '(' ){
			if( pLine[i + 1] >= '0' && pLine[i + 1] <= '9' )  {
				strcpy( szTitle, "(0)" );
			}
			else if ( pLine[i + 1] >= 'A' && pLine[i + 1] <= 'Z' ) {
				strcpy( szTitle, "(A)" );
			}
			else if ( pLine[i + 1] >= 'a' && pLine[i + 1] <= 'z' ) {
				strcpy( szTitle, "(a)" );
			}
			else {
				continue;
			}
		}else
		if( 2 == nCharChars ){
			// 2003.06.28 Moca 1���ڂ���n�܂��Ă��Ȃ��Ɠ��ꃌ�x���ƔF�����ꂸ��
			//	�ǂ�ǂ�q�m�[�h�ɂȂ��Ă��܂��̂��C�������ɂ�炸���ꃌ�x���ƔF�������悤��
			/* �S�p���� */
			if( pLine[i] == 0x82 && ( pLine[i + 1] >= 0x4f && pLine[i + 1] <= 0x58 ) ) {
				strcpy( szTitle, "�O" );
			}
			/* �@�`�S */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x40 && pLine[i + 1] <= 0x53 ) ){
				strcpy( szTitle, "�@" );
			}
			/* �T�`�] */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x54 && pLine[i + 1] <= 0x5d ) ){
				strcpy( szTitle, "�T" );
			}
			// 2003.06.28 Moca ������������K�w��
			//	���������قȂ遁�ԍ����قȂ�ƈقȂ錩�o���L���ƔF������Ă����̂�
			//	�F�����K�w�Ǝ��ʂ����悤��
			else{
				char szCheck[3];
				szCheck[0] = pLine[i];
				szCheck[1] = pLine[i + 1];
				szCheck[2] = '\0';
				/* ��`�\ */
				if( NULL != strstr( "�Z���O�l�ܘZ������\�S����Q��", szCheck ) ){
					strcpy( szTitle, "��" );
				}
			}
		}

		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		const char* pszText = (const char *)&pLine[i];
		nLineLen -= i;
		for( i = 0; i < nLineLen; ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				break;
			}
		}
		std::string strText( pszText, i );
		pszText = strText.c_str();

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_cLayoutMgr.LogicToLayout(
			0, nLineCount,
			&ptPos.x, &ptPos.y
		);

		/* nDepth���v�Z */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			strcpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = false;
		}

		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, ptPos.y + 1 , pszText, 0, nDepth );
			nDepth++;
		}
	}
	return;
}





/*! �K�w�t���e�L�X�g �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2003.05.25 genta �������@�ꕔ�C��
	@date 2003.06.21 Moca �K�w��2�i�ȏ�[���Ȃ�ꍇ���l��
*/
void CEditDoc::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(int nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const char*	pLine;
		int			nLineLen;

		pLine = m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta ���菇���ύX
		if( *pLine == '.' )
		{
			const char* pPos;	//	May 25, 2003 genta
			int			nLength;
			char		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for( pPos = pLine + 1 ; *pPos == '.' ; ++pPos )
				;

			int	nPosX;
			int	nPosY;
			m_cLayoutMgr.LogicToLayout(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1,
						"(����)", FUNCINFO_NOCLIPTEXT, dummyLevel - 1 );
				}
			}
			levelPrev = level;

			nLength = wsprintf(szTitle,"%d - ", level );
			
			char *pDest = szTitle + nLength; // �������ݐ�
			char *pDestEnd = szTitle + sizeof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos =='\r' || *pPos =='\n' || *pPos == '\0')
				{
					break;
				}
				//	May 25, 2003 genta 2�o�C�g�����̐ؒf��h��
				else if( _IS_SJIS_1( *pPos )){
					*pDest++ = *pPos++;
					*pDest++ = *pPos++;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = '\0';
			pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle, 0, level - 1);
		}
	}
}

/*[EOF]*/
