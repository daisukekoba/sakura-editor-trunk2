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

/*! HTML �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2004.04.19 zenryaku ��v�f�𔻒�
	@date 2004.04.20 Moca �R�����g�����ƁA�s���ȏI���^�O�𖳎����鏈����ǉ�
	@date 2008.08.15 aroka ���o���ƒi���̐[�������ǉ� 2008.09.07�C��
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const unsigned char*	pLineBuf;	//	pLineBuf �͍s�S�̂��w���A
	const unsigned char*	pLine;		//	pLine �͏������̕����ȍ~�̕������w���܂��B
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;
	bool					bEndTag;
	bool					bCommentTag = false;
	bool					bParaTag = false;	//	2008.08.15 aroka

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int				nMaxStack = 32;	//	�l�X�g�̍Ő[
	int						nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	char					pszStack[nMaxStack][32];
	char					szTitle[32];			//	�ꎞ�̈�
	char					szTag[32];				//	�ꎞ�̈�  �������ŕێ����č��������Ă��܂��B

	enum ELabelType {						//	�񋓑́F���x���̎��
		LT_DEFAULT,		LT_INLINE,		LT_IGNORE,		LT_EMPTY,
		LT_BLOCK,		LT_PARAGRAPH,	LT_HEADING
	};
	enum ELabelType	nLabelType;				// default, inlined, ignore, empty, block, p, heading
	/*	�������o���v�f�ihy�j�����ɏ�ʃ��x���̌��o��(hx)�������܂œ����[���ɂ��낦�܂��B
		���̂��߁A���o���̐[�����L�����Ă����܂��B
		���ʃ��x���̌��o���̐[���͌����܂ŕs��ŁA�O�̏͐߂ł̐[���͉e�����܂���B 2008.08.15 aroka
	*/
	int						nHeadDepth[6+1];		// [0]�� �󂯂Ă���
	for(k=0;k<=6;k++){
		nHeadDepth[k] = -1;
	}
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLineBuf = (const unsigned char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if( !pLineBuf )
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			pLine = &pLineBuf[i];
			// 2004.04.20 Moca �R�����g����������
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == memcmp( "-->", pLine, 3 ) )
				{
					bCommentTag = false;
					i += 2;
					pLine += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if( *pLine!='<' || nDepth>=nMaxStack )
			{
				continue;
			}
			bEndTag = false;
			pLine++; i++;
			if( *pLine=='/')
			{
				pLine++; i++;
				bEndTag = true;
			}
			for(j=0;i+j<nLineLen && j<sizeof(szTitle)-1; )
			{
				// �^�O����؂�o��
				// �X�y�[�X�A�^�u�A�u_:-.�p���v�ȊO�̔��p�����A�P�����ڂ́u-.�����v�͔F�߂Ȃ��B
				if( (pLine[j]==' ' || pLine[j]=='\t') ||
					(pLine[j]<0x80 && !strchr("_:-.",pLine[j]) && !isalnum(pLine[j])) ||
					(j==0 &&( (pLine[j]>='0' && pLine[j]<='9') || pLine[j]=='-' || pLine[j]=='.' )) )
				{
					break;
				}
				int nCharSize = CMemory::GetSizeOfChar((char*)pLine, nLineLen-i, j);
				memcpy(szTitle + j, pLine + j, nCharSize);
				j += nCharSize;
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here �R�����g����������
				if( i < nLineLen - 3 && 0 == memcmp( "!--", pLine, 3 ) )
				{
					bCommentTag = true;
					i += 3;
					pLine += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j] = '\0';
			/*	�^�O�̎�ނ��Ƃɏ�����ς���K�v�����邪�A
				�s�x��r����̂̓R�X�g�������̂ŁA�ŏ��ɕ��ނ��Ă����B 2008.08.15 aroka
				��r�̉񐔂��������߁A�������ɕϊ����Ă�����strcmp���g���B
			*/
			strcpy( szTag, szTitle );
			_strlwr( szTag );
			
			nLabelType = LT_DEFAULT;
			// �����v�f�i�����ڂ�ς��邽�߂̃^�O�j�͍\����͂��Ȃ��B
			if( !strcmp(szTag,"b") || !strcmp(szTag,"big") || !strcmp(szTag,"blink")
			 || !strcmp(szTag,"font") || !strcmp(szTag,"i") || !strcmp(szTag,"marquee")
			 || !strcmp(szTag,"nobr") || !strcmp(szTag,"s") || !strcmp(szTag,"small")
			 || !strcmp(szTag,"strike") || !strcmp(szTag,"tt") || !strcmp(szTag,"u")
			 || !strcmp(szTag,"bdo") || !strcmp(szTag,"sub") || !strcmp(szTag,"sup") )
			{
				nLabelType = LT_INLINE;
			}
			// �C�����C���e�L�X�g�v�f�i�e�L�X�g���C������^�O�j�͍\����͂��Ȃ�?
//			if( !strcmp(szTag,"abbr") || !strcmp(szTag,"acronym") || !strcmp(szTag,"dfn")
//			 || !strcmp(szTag,"em") || !strcmp(szTag,"strong") || !strcmp(szTag,"span")
//			 || !strcmp(szTag,"code") || !strcmp(szTag,"samp") || !strcmp(szTag,"kbd")
//			 || !strcmp(szTag,"var") || !strcmp(szTag,"cite") || !strcmp(szTag,"q") )
//			{
//				nLabelType = LT_INLINE;
//			}
			// ���r�v�f�iXHTML1.1�j�͍\����͂��Ȃ��B
			if( !strcmp(szTag,"rbc") || !strcmp(szTag,"rtc") || !strcmp(szTag,"ruby")
			 || !strcmp(szTag,"rb") || !strcmp(szTag,"rt") || !strcmp(szTag,"rp") )
			{
				nLabelType = LT_INLINE;
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W�Ȃ����͍̂\����͂��Ȃ��B
			if( !strcmp(szTag,"br") || !strcmp(szTag,"base") || !strcmp(szTag,"basefont")
			 || !strcmp(szTag,"frame") )
			{
				nLabelType = LT_IGNORE;
			}
			// ��v�f�i���e�������Ȃ��^�O�j�̂����\���Ɋ֌W������́B
			if( !strcmp(szTag,"area") || !strcmp(szTag,"hr") || !strcmp(szTag,"img")
			 || !strcmp(szTag,"input") || !strcmp(szTag,"link") || !strcmp(szTag,"meta")
			 || !strcmp(szTag,"param") )
			{
				nLabelType = LT_EMPTY;
			}
			if( !strcmp(szTag,"div") || !strcmp(szTag,"center")
			 || !strcmp(szTag,"address") || !strcmp(szTag,"blockquote")
			 || !strcmp(szTag,"noscript") || !strcmp(szTag,"noframes")
			 || !strcmp(szTag,"ol") || !strcmp(szTag,"ul") || !strcmp(szTag,"dl")
			 || !strcmp(szTag,"dir") || !strcmp(szTag,"menu")
			 || !strcmp(szTag,"pre") || !strcmp(szTag,"table")
			 || !strcmp(szTag,"form") || !strcmp(szTag,"fieldset") || !strcmp(szTag,"isindex") )
			{
				nLabelType = LT_BLOCK;
			}
			if( !strcmp(szTag,"p") )
			{
				nLabelType = LT_PARAGRAPH;
			}
			if( (szTag[0]=='h') && ('1'<=szTitle[1]&&szTitle[1]<='6') ){
				nLabelType = LT_HEADING;
			}

			// 2009.08.08 syat �u/>�v�ŏI���^�O�̔���̂��߁A�I���^�O�������J�n�^�O�����̌�ɂ����B
			//                  �i�J�n�^�O�����̒��ŁAbEndTag��true�ɂ��Ă��鏊������B�j

			if( ! bEndTag ) // �J�n�^�O
			{
				if( nLabelType!=LT_INLINE && nLabelType!=LT_IGNORE ){
					// p�̒��Ńu���b�N�v�f��������A�����I��p�����B 2008.09.07 aroka
					if( bParaTag ){
						if( nLabelType==LT_HEADING || nLabelType==LT_PARAGRAPH || nLabelType==LT_BLOCK ){
							nDepth--;
						}
					}
					if( nLabelType==LT_HEADING ){
						if( nHeadDepth[szTitle[1]-'0'] != -1 ) // �����o��:���o
						{
							nDepth = nHeadDepth[szTitle[1]-'0'];
							for(k=szTitle[1]-'0';k<=6;k++){
								nHeadDepth[k] = -1;
							}
							nHeadDepth[szTitle[1]-'0'] = nDepth;
							bParaTag = false;
						}
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = true;
					}
					if( nLabelType==LT_BLOCK ){
						bParaTag = false;
					}

					int		nPosX;
					int		nPosY;

					m_cLayoutMgr.LogicToLayout(
						i,
						nLineCount,
						&nPosX,
						&nPosY
					);

					if( nLabelType!=LT_EMPTY ){
						// �I���^�O�Ȃ��������S�Ẵ^�O�炵�����̂𔻒�
						strcpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<sizeof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if( pLine[j]=='/' && pLine[j+1]=='>' )
								{
									bEndTag = true;
									break;
								}
								else if( pLine[j]=='>' )
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	' ';
								for(j-=k-1;i+j+k<nLineLen && k<sizeof(szTitle)-1;k++)
								{
									if( pLine[j+k]=='<' || pLine[j+k]=='\r' || pLine[j+k]=='\n' )
									{
										break;
									}
									szTitle[k] = pLine[j+k];
								}
								j += k-1;
							}
						}
						szTitle[k]	=	'\0';
						pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1, szTitle, 0, nDepth++ );
					}
					else
					{
						for(;i+j<nLineLen && j<sizeof(szTitle)-1;j++)
						{
							if( pLine[j]=='>' )
							{
								break;
							}
							szTitle[j] = pLine[j];
						}
						szTitle[j]	=	'\0';
						pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle,0,nDepth);
					}
				}
			}
			if( bEndTag ) // �I���^�O
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca �ǉ�
				while(nDepth>0)
				{
					nDepth--;
					if(!my_stricmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca �c���[���ƈ�v���Ȃ��Ƃ��́A���̏I���^�O�͖���
				if( nDepth == 0 )
				{
					if(my_stricmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}else{
					if( nLabelType==LT_HEADING ){	//	���o���̏I���
						nHeadDepth[szTitle[1]-'0'] = nDepth;
						nDepth++;
					}
					if( nLabelType==LT_PARAGRAPH ){
						bParaTag = false;
					}
				}
			}
			i	+=	j;
		}
	}
}

/*[EOF]*/
