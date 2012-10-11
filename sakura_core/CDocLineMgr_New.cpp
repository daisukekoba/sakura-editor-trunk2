/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1999/12/17  �V�K�쐬
	@date 2001/12/03  hor Bookmark�@�\�ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta
	Copyright (C) 2003, Moca, genta
	Copyright (C) 2005, �����
	Copyright (C) 2008, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "Debug.h"
#include "charcode.h"
#include <io.h>
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"
#include "CDocLine.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka
#include "CBregexp.h"// 2002/2/10 aroka
#include "sakura_rc.h"// 2002/2/10 aroka
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void CDocLineMgr::ReplaceData( DocLineReplaceArg* pArg )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReplaceData()" );

	/* �}���ɂ���đ������s�̐� */
	pArg->nInsLineNum = 0;
	/* �폜�����s�̑��� */
	pArg->nDeletedLineNum = 0;
	/* �폜���ꂽ�f�[�^ */
	pArg->pcmemDeleted->SetString( "" );

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	CDocLine* pCDocLineNew;
	int nWorkPos;
	int nWorkLen;
	const char* pLine;
	int nLineLen;
	int i;
	int			nBgn;
	int			nPos;
	int			nAllLinesOld;
	int			nCount;
	int			nProgress;
	CMemory		cmemPrevLine;
	CMemory		cmemCurLine;
	CMemory		cmemNextLine;
	//	May 15, 2000
	CEol cEOLType;
	CEol cEOLTypeNext;
	CDlgCancel*	pCDlgCancel = NULL;
	HWND		hwndCancel = NULL;	//	������
	HWND		hwndProgress = NULL;	//	������

	pArg->nNewLine = pArg->nDelLineFrom;
	pArg->nNewPos =  pArg->nDelPosFrom;

	/* ��ʂ̃f�[�^�𑀍삷��Ƃ� */
	if( 3000 < pArg->nDelLineTo - pArg->nDelLineFrom || 1024000 < pArg->nInsDataLen ){
		/* �i���_�C�A���O�̕\�� */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		}
	}

	// From Here Feb. 08, 2008 genta �폜�o�b�t�@�����炩���ߊm�ۂ���
	pCDocLine = GetLine( pArg->nDelLineFrom );
	nWorkLen = 0;
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		nWorkLen += pCDocLine->m_pLine->GetStringLength();
		pCDocLine = pCDocLine->m_pNext;
	}
	pArg->pcmemDeleted->AllocStringBuffer( nWorkLen );
	// To Here Feb. 08, 2008 genta

	// �폜�f�[�^�̎擾�̃��[�v
	/* �O���珈�����Ă��� */
	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLine( pArg->nDelLineFrom );
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_pLine->GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_pLine->GetStringLength(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->nDelLineTo ){
			nWorkLen = pArg->nDelPosTo - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			PleaseReportToAuthor(
				NULL,
				_T("CDocLineMgr::ReplaceData()\n")
				_T("\n")
				_T("0 > nWorkLen\nnWorkLen=%d\n")
				_T("i=%d\n")
				_T("pArg->nDelLineTo=%d"),
				nWorkLen, i, pArg->nDelLineTo
			);
		}

		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		/* �폜���ꂽ�f�[�^��ۑ� */
		// 2002/2/10 aroka from here CMemory�ύX �O�̂��߁B
		if( pLine != pCDocLine->m_pLine->GetStringPtr() ){
			PleaseReportToAuthor(
				NULL,
				_T("CDocLineMgr::ReplaceData()\n")
				_T("\n")
				_T("pLine != pCDocLine->m_pLine->GetPtr() =%d\n")
				_T("i=%d\n")
				_T("pArg->nDelLineTo=%d"),
				pLine,
				i,
				pArg->nDelLineTo
			);
		}

		if( NULL == pArg->pcmemDeleted->AppendString( &pLine[nWorkPos], nWorkLen ) ){
			/* �������m�ۂɎ��s���� */
			pArg->pcmemDeleted->SetString( "" );
			break;
		}

next_line:;
		/* ���̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLineNext;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		++m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->nDelLineFrom) && ( 0 == ((i - pArg->nDelLineFrom) % 32)) ){
				nProgress = (i - pArg->nDelLineFrom) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );

			}
		}
	} // �폜�f�[�^�̎擾�̃��[�v


	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLine( pArg->nDelLineTo );
	i = pArg->nDelLineTo;
	if( 0 < pArg->nDelLineTo && NULL == pCDocLine ){
		pCDocLine = GetLine( pArg->nDelLineTo - 1 );
		i--;
	}
	/* ��납�珈�����Ă��� */
	for( ; i >= pArg->nDelLineFrom && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->m_pLine->GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_pLine->GetStringLength(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->nDelLineTo ){
			nWorkLen = pArg->nDelPosTo - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}

		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto prev_line;
		}
		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}


		/* �s�S�̂̍폜 */
		if( nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX
			/* �폜�����s�̑��� */
			++(pArg->nDeletedLineNum);
			/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
			DeleteNode( pCDocLine );
			pCDocLine = NULL;
		}
		/* ���̍s�ƘA������悤�ȍ폜 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX

			/* �s���f�[�^�폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				char* pWork = new char[nWorkPos + 1];
				memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
				pCDocLine->m_pLine->SetString( pWork, nWorkPos );
				delete [] pWork;
			}

			/* ���̍s������ */
			if( pCDocLineNext ){
				/* ���s�R�[�h�̏����X�V (���̍s������炤) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* ���s�R�[�h�̎�� */
				pCDocLine->m_pLine->AppendNativeData( pCDocLineNext->m_pLine );

				/* ���̍s �s�I�u�W�F�N�g�̍폜 */
				DeleteNode( pCDocLineNext );
				pCDocLineNext = NULL;

				/* �폜�����s�̑��� */
				++(pArg->nDeletedLineNum);
			}else{
				/* ���s�R�[�h�̏����X�V */
				pCDocLine->m_cEol.SetType( EOL_NONE );
			}
			pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */
		}
		else{
		/* �s�������̍폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				// 2002/2/10 aroka CMemory�ύX ���x�� GetStringLength,GetPtr ����΂Ȃ��B
				int nLength = pCDocLine->m_pLine->GetStringLength();
				char* pWork = new char[nLength - nWorkLen + 1];
				memcpy( pWork, pLine, nWorkPos );

				memcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->m_pLine->SetString( pWork, nLength - nWorkLen );
				delete [] pWork;
			}
			pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */
		}

prev_line:;
		/* ���O�̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLinePrev;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		--m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->nDelLineTo - i) && ( 0 == ((pArg->nDelLineTo - i) % 32) ) ){
				nProgress = (pArg->nDelLineTo - i) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2 + 50;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
			}
		}
	}


	/* �f�[�^�}������ */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_nLines;
	pArg->nNewLine = pArg->nDelLineFrom;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->nNewPos  = 0;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	nCount = 0;
	pArg->nInsLineNum = 0;
	pCDocLine = GetLine( pArg->nDelLineFrom );



	if( NULL == pCDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetString( "" );
		cmemNextLine.SetString( "" );
		cEOLTypeNext.SetType( EOL_NONE );
	}else{
		pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */

		// 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
		pLine = pCDocLine->m_pLine->GetStringPtr( &nLineLen );
		cmemPrevLine.SetString( pLine, pArg->nDelPosFrom );
		cmemNextLine.SetString( &pLine[pArg->nDelPosFrom], nLineLen - pArg->nDelPosFrom );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.GetTypeFromString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				pCDocLineNew = new CDocLine;

				pCDocLineNew->m_pLine = new CMemory;
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					if( NULL == m_pDocLineTop ){
						m_pDocLineTop = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_pLine->SetNativeData( &cmemPrevLine );
					*(pCDocLineNew->m_pLine) += cmemCurLine;

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
				}
				else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_pLine->SetNativeData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
				}
				pCDocLine = NULL;
				++m_nLines;
			}
			else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pCDocLine->m_pLine->SetNativeData( &cmemPrevLine );
					*(pCDocLine->m_pLine) += cmemCurLine;

					pCDocLine->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
					pCDocLine = pCDocLine->m_pNext;
				}
				else{
					pCDocLineNew = new CDocLine;
					pCDocLineNew->m_pLine = new CMemory;
					pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
					pCDocLineNew->m_pNext = pCDocLine;
					pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
					pCDocLine->m_pPrev = pCDocLineNew;
					pCDocLineNew->m_pLine->SetNativeData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */

					++m_nLines;
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(pArg->nNewLine);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
	}
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			pCDocLineNew = new CDocLine;
			pCDocLineNew->m_pLine = new CMemory;
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				if( NULL == m_pDocLineTop ){
					m_pDocLineTop = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetNativeData( &cmemPrevLine );
				*(pCDocLineNew->m_pLine) += cmemCurLine;

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

			}
			else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetNativeData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
			}
			pCDocLine = NULL;
			++m_nLines;
			pArg->nNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}
		else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pCDocLine->m_pLine->SetNativeData( &cmemPrevLine );
				*(pCDocLine->m_pLine) += cmemCurLine;

				pCDocLine->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

				pCDocLine = pCDocLine->m_pNext;
				pArg->nNewPos = cmemPrevLine.GetStringLength() + nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
			else{
				pCDocLineNew = new CDocLine;
				pCDocLineNew->m_pLine = new CMemory;
				pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
				pCDocLineNew->m_pNext = pCDocLine;
				pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
				pCDocLine->m_pPrev = pCDocLineNew;
				pCDocLineNew->m_pLine->SetNativeData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

				++m_nLines;
				pArg->nNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		// �i���_�C�A���O��\�����Ȃ��ꍇ�Ɠ��������ɂȂ�悤�Ƀ_�C�A���O�͒x���j������
		// ������ pCDlgCancel �� delete ����� delete ����߂�܂ł̊Ԃ�
		// �_�C�A���O�j�� -> �ҏW��ʂփt�H�[�J�X�ړ� -> �L�����b�g�ʒu����
		// �܂ň�C�ɓ����̂Ŗ����ȃ��C�A�E�g���Q�Ƃňُ�I�����邱�Ƃ�����
		pCDlgCancel->DeleteAsync();	// �����j����x�����s����	// 2008.05.28 ryoji
	}
	return;
}


//!�u�b�N�}�[�N�̑S����
/*
	@date 2001.12.03 hor
*/
void CDocLineMgr::ResetAllBookMark( void )
{
	CDocLine* pDocLine = m_pDocLineTop;
	while( pDocLine ){
		pDocLine->SetBookMark(false);
		pDocLine = pDocLine->m_pNext;
	}
}


//! �u�b�N�}�[�N����
/*
	@date 2001.12.03 hor
*/
int CDocLineMgr::SearchBookMark(
	int					nLineNum,		/* �����J�n�s */
	ESearchDirection	bPrevOrNext,	/* 0==�O������ 1==������� */
	int*				pnLineNum 		/* �}�b�`�s */
)
{
	CDocLine*	pDocLine;
	int			nLinePos=nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD ){
		nLinePos--;
		pDocLine = GetLine( nLinePos );
		while( pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}
	//�������
	else{
		nLinePos++;
		pDocLine = GetLine( nLinePos );
		while( NULL != pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}

//! �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::SetBookMarks( char* pMarkLines )
{
	CDocLine*	pCDocLine;
	char *p;
	char delim[] = ", ";
	p = pMarkLines;
	while(strtok(p, delim) != NULL) {
		while(strchr(delim, *p) != NULL)p++;
		pCDocLine=GetLine( atol(p) );
		if(pCDocLine)pCDocLine->SetBookMark(true);
		p += strlen(p) + 1;
	}
}


//! �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
/*
	@date 2002.01.16 hor
*/
char* CDocLineMgr::GetBookMarks( void )
{
	CDocLine*	pCDocLine;
	static char szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta static��
	char szBuff[10];
	int	nLinePos=0;
	pCDocLine = GetLine( nLinePos );
	strcpy( szText, "" );
	while( pCDocLine ){
		if(pCDocLine->IsBookMarked()){
			wsprintf( szBuff, "%d,",nLinePos );
			if(lstrlen(szBuff)+lstrlen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			strcat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->m_pNext;
	}
	return szText; // Feb. 17, 2003 genta
}




//! ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::MarkSearchWord(
	const char*				pszPattern,		//!< ��������
	const SSearchOption&	sSearchOption,	//!< �����I�v�V����
	CBregexp*				pRegexp			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
)
{
	CDocLine*	pDocLine;
	const char*	pLine;
	int			nLineLen;
	char*		pszRes;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	lstrlen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = lstrlen( pszPattern );

	/* 1==���K�\�� */
	if( sSearchOption.bRegularExp ){
		pDocLine = GetLine( 0 );
		while( pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetStringPtr( &nLineLen );
				// 2005.03.19 ����� �O����v�T�|�[�g�̂��߂̃��\�b�h�ύX
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
	}
	/* 1==�P��̂݌��� */
	else if( sSearchOption.bWordOnly ){
		pDocLine = GetLine( 0 );
		int nLinePos = 0;
		int nNextWordFrom = 0;
		int nNextWordFrom2;
		int nNextWordTo2;
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked() &&
				WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const char* pData = pDocLine->m_pLine->GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( !sSearchOption.bLoHiCase && 0 == my_memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( sSearchOption.bLoHiCase && 0 == memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					pDocLine->SetBookMark(true);
				}
				else if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* ���̍s�����ɍs�� */
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
			nNextWordFrom = 0;
		}
	}
	else{
		/* ���������̏�� */
		pnKey_CharCharsArr = NULL;
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pszPattern,
			nPatternLen,
			&pnKey_CharCharsArr
		);
		pDocLine = GetLine( 0 );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetStringPtr( &nLineLen );
				pszRes = SearchString(
					(const unsigned char *)pLine,
					nLineLen,
					0,
					(const unsigned char *)pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
			pnKey_CharCharsArr = NULL;
		}
	}
	return;

}

/*!	�����\���̑S����
	@author	MIK
	@date	2002.05.25
*/
void CDocLineMgr::ResetAllDiffMark( void )
{
	CDocLine* pDocLine;

	pDocLine = m_pDocLineTop;
	while( pDocLine )
	{
		pDocLine->SetDiffMark( 0 );
		pDocLine = pDocLine->m_pNext;
	}

	m_bIsDiffUse = false;
}

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
int CDocLineMgr::SearchDiffMark(
	int					nLineNum,		//!< �����J�n�s
	ESearchDirection	bPrevOrNext,	//!< 0==�O������ 1==�������
	int*				pnLineNum 		//!< �}�b�`�s
)
{
	CDocLine*	pDocLine;
	int			nLinePos = nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		pDocLine = GetLine( nLinePos );
		while( pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}
	//�������
	else
	{
		nLinePos++;
		pDocLine = GetLine( nLinePos );
		while( pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}

/*!	���������s�͈͎w��œo�^����B
	@author	MIK
	@date	2002/05/25
*/
void CDocLineMgr::SetDiffMarkRange( int nMode, int nStartLine, int nEndLine )
{
	int	i;
	CDocLine	*pCDocLine;

	m_bIsDiffUse = true;

	if( nStartLine < 0 ) nStartLine = 0;

	//�ŏI�s����ɍ폜�s����
	int	nLines = GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - 1;
		pCDocLine = GetLine( nEndLine );
		if( pCDocLine ) pCDocLine->SetDiffMark( MARK_DIFF_DEL_EX );
	}

	//�s�͈͂Ƀ}�[�N������
	for( i = nStartLine; i <= nEndLine; i++ )
	{
		pCDocLine = GetLine( i );
		if( pCDocLine ) pCDocLine->SetDiffMark( nMode );
	}

	return;
}

/*[EOF]*/
