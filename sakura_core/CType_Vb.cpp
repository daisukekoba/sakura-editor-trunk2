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
#include "CDocLine.h"// 2002/2/10 aroka
#include "CEditWnd.h"
#include "Debug.h"
#include "etc_uty.h"
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "mymessage.h"	//	Oct. 9, 2004 genta
#include "CControlTray.h"

//	From Here June 23, 2001 N.Nakatani
//!	Visual Basic�֐����X�g�쐬�i�ȈՔŁj
/*!
	Visual Basic�̃R�[�h����P���Ƀ��[�U�[��`�̊֐���X�e�[�g�����g�����o��������s���B

    Jul 10, 2003 little YOSHI  �ׂ�����͂���悤�ɕύX
                               ���ׂẴL�[���[�h�͎����I�ɐ��`�����̂ŁA�啶���������͊��S�Ɉ�v����B
                               �t�H�[���⃂�W���[�������ł͂Ȃ��A�N���X�ɂ��Ή��B
							   �������AConst�́u,�v�ŘA���錾�ɂ͖��Ή�
	Jul. 21, 2003 genta �L�[���[�h�̑啶���E�������𓯈ꎋ����悤�ɂ���
	Aug  7, 2003 little YOSHI  �_�u���N�H�[�e�[�V�����ň͂܂ꂽ�e�L�X�g�𖳎�����悤�ɂ���
	                           �֐����Ȃǂ�VB�̖��O�t���K�����255�����Ɋg��
*/
void CEditDoc::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const int	nMaxWordLeng = 255;	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	const char*	pLine;
	int			nLineLen = 0;//: 2002/2/3 aroka �x���΍�F������
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	char		szWord[256];		// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	int			nWordIdx = 0;
//	int			nMaxWordLeng = 70;	// Aug 7, 2003 little YOSHI  �萔�ɕύX��
	int			nMode;
	char		szFuncName[256];	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	int			nFuncLine;
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// �N���X���W���[���t���O
	bool		bProcedure;		// �v���V�[�W���t���O�i�v���V�[�W�����ł�True�j
	bool		bDQuote;		// �_�u���N�H�[�e�[�V�����t���O�i�_�u���N�H�[�e�[�V������������True�j

	// ���ׂ�t�@�C�����N���X���W���[���̂Ƃ���Type�AConst�̋������قȂ�̂Ńt���O�𗧂Ă�
	bClass	= false;
	int filelen = _tcslen(GetFilePath());
	if ( 4 < filelen ) {
		if ( 0 == my_stricmp((GetFilePath() + filelen - 4), ".cls") ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	pLine = NULL;
	bProcedure	= false;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( '_' != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		nFuncId = 0;
		bDQuote	= false;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Public") ) {
						// �p�u���b�N�錾���������I
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Private") ) {
						// �v���C�x�[�g�錾���������I
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Friend") ) {
						// �t�����h�錾���������I
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Static") ) {
						// �X�^�e�B�b�N�錾���������I
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Function" ) ){
						if ( 0 == my_stricmp( szWordPrev, "End" ) ){
							// �v���V�[�W���t���O���N���A
							bProcedure	= false;
						}else
						if( 0 != my_stricmp( szWordPrev, "Exit" ) ){
							if( 0 == my_stricmp( szWordPrev, "Declare" ) ){
								nFuncId |= 0x200;	// DLL�Q�Ɛ錾
							}else{
								bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
							}
							nFuncId |= 0x01;		// �֐�
							nParseCnt = 1;
							nFuncLine = nLineCount + 1;
						}
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Sub" ) ){
						if ( 0 == my_stricmp( szWordPrev, "End" ) ){
							// �v���V�[�W���t���O���N���A
							bProcedure	= false;
						}else
						if( 0 != my_stricmp( szWordPrev, "Exit" ) ){
							if( 0 == my_stricmp( szWordPrev, "Declare" ) ){
								nFuncId |= 0x200;	// DLL�Q�Ɛ錾
							}else{
								bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
							}
							nFuncId |= 0x02;		// �֐�
							nParseCnt = 1;
							nFuncLine = nLineCount + 1;
						}
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Get" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId	|= 0x03;		// �v���p�e�B�擾
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Let" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId |= 0x04;		// �v���p�e�B�ݒ�
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Set" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId |= 0x05;		// �v���p�e�B�Q��
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Const" )
					 && 0 != my_stricmp( szWordPrev, "#" )
					){
						if ( bClass || bProcedure || 0 == ((nFuncId >> 4) & 0x0f) ) {
							// �N���X���W���[���ł͋����I��Private
							// �v���V�[�W�����ł͋����I��Private
							// Public�̎w�肪�Ȃ��Ƃ��A�f�t�H���g��Private�ɂȂ�
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x06;		// �萔
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Enum" )
					){
						nFuncId	|= 0x207;		// �񋓌^�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Type" )
					){
						if ( bClass ) {
							// �N���X���W���[���ł͋����I��Private
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x208;		// ���[�U��`�^�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Event" )
					){
						nFuncId	|= 0x209;		// �C�x���g�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Property" )
					 && 0 == my_stricmp( szWordPrev, "End")
					){
						bProcedure	= false;	// �v���V�[�W���t���O���N���A
					}
					else if( 1 == nParseCnt ){
						strcpy( szFuncName, szWord );
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  �� ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.LogicToLayout(	0, nFuncLine - 1, &nPosX, &nPosY );
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
						nFuncId	= 0;	// Jul 10, 2003  little YOSHI  �_���a���g�p���邽�߁A�K��������
					}

					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}
			/* �L����ǂݍ��ݒ� */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// �u#Const�v�ƁuConst�v����ʂ��邽�߂ɁA�u#�v�����ʂ���悤�ɕύX
				if( '_' == pLine[i] ||
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
					'/' == pLine[i]	||
					'-' == pLine[i] ||
					'#' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen && '\'' == pLine[i] ){
					break;
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* �e�L�X�g������܂œǂݔ�΂� */	// Aug 7, 2003 little YOSHI  �ǉ�
			if (nMode == 3) {
				// �A������_�u���N�H�[�e�[�V�����͖�������
				if (1 == nCharChars && '"' == pLine[i]) {
					// �_�u���N�H�[�e�[�V���������ꂽ��t���O�𔽓]����
					bDQuote	= !bDQuote;
				} else if (bDQuote) {
					// �_�u���N�H�[�e�[�V�����̎���
					// �_�u���N�H�[�e�[�V�����ȊO�̕��������ꂽ��m�[�}�����[�h�Ɉڍs
					--i;
					nMode	= 0;
					bDQuote	= false;
					continue;
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}
//	To Here June 23, 2001 N.Nakatani

/*[EOF]*/
