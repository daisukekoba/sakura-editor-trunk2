/*!	@file
	@brief CDicMgr�N���X

	@author Norio Nakatani
	@date	1998/11/05 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, Moca
	Copyright (C) 2003, Moca
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdio.h>
#include "CDicMgr.h"
#include "CMemory.h" // 2002/2/10 aroka �w�b�_����
#include "CRunningTimer.h"
#include "etc_uty.h"
#include "file.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

CDicMgr::CDicMgr()
{
	return;
}




CDicMgr::~CDicMgr()
{
	return;
}




/*!
	�L�[���[�h�̌���
	�ŏ��Ɍ��������L�[���[�h�̈Ӗ���Ԃ�

	@date 2006.04.10 fon �����q�b�g�s��Ԃ�����pLine��ǉ�
*/
BOOL CDicMgr::Search(
	const char*			pszKey,				//!< �����L�[���[�h
	const int			nCmpLen,			//!< �����L�[���[�h�̒���
	CMemory**			ppcmemKey,			//!< ���������L�[���[�h�D�Ăяo�����̐ӔC�ŉ������D
	CMemory**			ppcmemMean,			//!< ���������L�[���[�h�ɑΉ����鎫�����e�D�Ăяo�����̐ӔC�ŉ������D
	const TCHAR*		pszKeyWordHelpFile,	//!< �L�[���[�h�w���v�t�@�C���̃p�X��
	int*				pLine				//!< ���������L�[���[�h�̃L�[���[�h�w���v�t�@�C�����ł̍s�ԍ�
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDicMgr::Search" );
#endif
	long	i;
	const char*	pszDelimit = " /// ";
	char*	pszWork;
	int		nRes;
	char*	pszToken;
	const char*	pszKeySeps = ",\0";


	/* �����t�@�C�� */
	if( pszKeyWordHelpFile[0] == _T('\0') ){
		return FALSE;
	}
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE* pFile = _tfopen_absini( pszKeyWordHelpFile, _T("r") );
	if(!pFile){
		return FALSE;
	}

	char	szLine[LINEREADBUFSIZE];
	for(int line=1 ;NULL != fgets( szLine, _countof(szLine), pFile ); line++ ){	// 2006.04.10 fon
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += _tcslen( pszDelimit );

			/* �ŏ��̃g�[�N�����擾���܂��B */
			pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
				nRes = my_strnicmp( pszKey, pszToken, nCmpLen );	// 2006.04.10 fon
				if( 0 == nRes ){
					int nLen = (int)_tcslen(pszWork);
					for( i = 0; i < nLen; ++i ){
						if( pszWork[i] == '\r' ||
							pszWork[i] == '\n' ){
							pszWork[i] = '\0';
							break;
						}
					}
					//�L�[���[�h�̃Z�b�g
					*ppcmemKey = new CMemory;	// 2006.04.10 fon
					(*ppcmemKey)->SetString( pszToken );
					//�Ӗ��̃Z�b�g
					*ppcmemMean = new CMemory;
					(*ppcmemMean)->SetString( pszWork );

					fclose( pFile );
					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return FALSE;
}





/*
||  ���͕⊮�L�[���[�h�̌���
||
||  �E���������������ׂĕԂ�(���s�ŋ�؂��ĕԂ�)
||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
||  �E������������Ԃ�
||
*/
int CDicMgr::HokanSearch(
	const char* 	pszKey,
	bool			bHokanLoHiCase,	//!< �p�啶���������𓯈ꎋ����
	CMemory**		ppcmemKouho,
	int				nMaxKouho,		//!< Max��␔(0==������)
	const TCHAR*	pszKeyWordFile
)
{
	int		nKeyLen;
	int		nKouhoNum;
	int		nRet;
	*ppcmemKouho = NULL;
	if( pszKeyWordFile[0] == _T('\0') ){
		return 0;
	}

	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE* pFile = _tfopen_absini( pszKeyWordFile, _T("r") );
	if(!pFile){
		return 0;
	}
	nKouhoNum = 0;
	nKeyLen = _tcslen( pszKey );
	char	szLine[1024];
	while( NULL != fgets( szLine, _countof(szLine), pFile ) ){
		if( nKeyLen > (int)lstrlen( szLine ) ){
			continue;
		}
		if( szLine[0] == ';' ){
			continue;
		}
		if( szLine[nKeyLen] == '\r' || szLine[nKeyLen] == '\n' ){
			continue;
		}

		if( bHokanLoHiCase ){	/* �p�啶���������𓯈ꎋ���� */
			nRet = my_memicmp( pszKey, szLine, nKeyLen );
		}else{
			nRet = memcmp( pszKey, szLine, nKeyLen );
		}
		if( 0 == nRet ){
			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CMemory;
				(*ppcmemKouho)->SetString( szLine );
			}else{
				(*ppcmemKouho)->AppendString( szLine );
			}
			++nKouhoNum;
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				break;
			}
		}
	}
	fclose( pFile );
	return nKouhoNum;
}


/*[EOF]*/
