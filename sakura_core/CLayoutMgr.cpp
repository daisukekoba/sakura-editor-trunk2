/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, YAZAKI, genta, aroka
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2006, genta, Moca
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayoutMgr.h"
#include "CShareData.h" // 2005.11.20 Moca
#include "charcode.h"
#include "Debug.h"
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"/// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "etc_uty.h" // Oct. 5, 2002 genta
#include "CMemoryIterator.h" // 2006.07.29 genta
#include "CEditDoc.h"		// 2009.08.28 nasukoji

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
{
	m_pcDocLineMgr = NULL;
	m_sTypeConfig.m_bWordWrap = true;				// �p�����[�h���b�v������
	m_sTypeConfig.m_nTabSpace = 8;					// TAB�����X�y�[�X
	m_sTypeConfig.m_nStringType = 0;				// �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""]['']
	m_sTypeConfig.m_bKinsokuHead = false;			// �s���֑�				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuTail = false;			// �s���֑�				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuRet  = false;			// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	m_sTypeConfig.m_bKinsokuKuto = false;			// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	m_pszKinsokuHead_1 = NULL;						// �s���֑�				//@@@ 2002.04.08 MIK
	m_pszKinsokuHead_2 = NULL;						// �s���֑�				//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1 = NULL;						// �s���֑�				//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_2 = NULL;						// �s���֑�				//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1 = NULL;						// ��Ǔ_�Ԃ炳��		//@@@ 2002.04.17 MIK
	m_pszKinsokuKuto_2 = NULL;						// ��Ǔ_�Ԃ炳��		//@@@ 2002.04.17 MIK

	// 2005.11.21 Moca �F�����t���O�������o�Ŏ���
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false; 
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;

	m_nTextWidth = 0;			// �e�L�X�g�ő啝�̋L��		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = 0;	// �ő啝�̃��C�A�E�g�s		// 2009.08.28 nasukoji

	Init();
}

CLayoutMgr::~CLayoutMgr()
{
	_Empty();

	/* �s���֑� */	//@@@ 2002.04.08 MIK
	delete [] m_pszKinsokuHead_1;
	m_pszKinsokuHead_1 = NULL;
	delete [] m_pszKinsokuHead_2;
	m_pszKinsokuHead_2 = NULL;
	/* �s���֑� */	//@@@ 2002.04.08 MIK
	delete [] m_pszKinsokuTail_1;
	m_pszKinsokuTail_1 = NULL;
	delete [] m_pszKinsokuTail_2;
	m_pszKinsokuTail_2 = NULL;
	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
	delete [] m_pszKinsokuKuto_1;
	m_pszKinsokuKuto_1 = NULL;
	delete [] m_pszKinsokuKuto_2;
	m_pszKinsokuKuto_2 = NULL;
}


/*
||
|| �s�f�[�^�Ǘ��N���X�̃|�C���^�����������܂�
||
*/
void CLayoutMgr::Create( CEditDoc* pcEditDoc, CDocLineMgr* pcDocLineMgr )
{
	Init();
	//	Jun. 20, 2003 genta EditDoc�ւ̃|�C���^�ǉ�
	m_pcEditDoc = pcEditDoc;
	m_pcDocLineMgr = pcDocLineMgr;
}

void CLayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
	m_nLines = 0;			/* �S�����s�� */

	// EOF���C�A�E�g�ʒu�L��	//2006.10.07 Moca
	m_nEOFLine = -1;
	m_nEOFColumn = -1;
}



void CLayoutMgr::_Empty()
{
	CLayout* pLayout = m_pLayoutTop;
	while( pLayout ){
		CLayout* pLayoutNext = pLayout->m_pNext;
		delete pLayout;
		pLayout = pLayoutNext;
	}
}




/*! ���C�A�E�g���̕ύX
	@param bDoRayout [in] ���C�A�E�g���̍č쐬
	@param refType [in] �^�C�v�ʐݒ�
*/
void CLayoutMgr::SetLayoutInfo(
	bool				bDoRayout,
	HWND				hwndProgress,
	const STypeConfig&	refType
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::SetLayoutInfo" );

	//�^�C�v�ʐݒ�
	m_sTypeConfig = refType;

	//	Oct. 1, 2002 genta �^�C�v�ɂ���ď����֐���ύX����
	//	���������Ă�����e�[�u���ɂ��ׂ�
	switch ( refType.m_nIndentLayout ){	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */	//@@@ 2002.09.29 YAZAKI
	case 1:
		//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Normal;
		break;
	}

	{	//@@@ 2002.04.08 MIK start
		unsigned char	*p, *q1, *q2;
		int	length;

		//��Ǔ_�̂Ԃ炳��
		m_sTypeConfig.m_bKinsokuKuto = refType.m_bKinsokuKuto;	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
		delete [] m_pszKinsokuKuto_1;
		m_pszKinsokuKuto_1 = NULL;
		delete [] m_pszKinsokuKuto_2;
		m_pszKinsokuKuto_2 = NULL;
		length = strlen( refType.m_szKinsokuKuto ) + 1;
		m_pszKinsokuKuto_1 = new char[ length ];
		m_pszKinsokuKuto_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuKuto_1;
		q2 = (unsigned char *)m_pszKinsokuKuto_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		if( m_sTypeConfig.m_bKinsokuKuto )	// 2009.08.06 ryoji m_bKinsokuKuto�ŐU�蕪����(Fix)
		{
			for( p = (unsigned char *)refType.m_szKinsokuKuto; *p; p++ )
			{
				if( _IS_SJIS_1( *p ) )
				{
					*q2 = *p; q2++; p++;
					*q2 = *p; q2++;
					*q2 = 0;
				}
				else
				{
					*q1 = *p; q1++;
					*q1 = 0;
				}
			}
		}

		//�s���֑�������1,2�o�C�g�����𕪂��ĊǗ�����B
		m_sTypeConfig.m_bKinsokuHead = refType.m_bKinsokuHead;
		delete [] m_pszKinsokuHead_1;
		delete [] m_pszKinsokuHead_2;
		length = strlen( refType.m_szKinsokuHead ) + 1;
		m_pszKinsokuHead_1 = new char[ length ];
		m_pszKinsokuHead_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuHead_1;
		q2 = (unsigned char *)m_pszKinsokuHead_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		for( p = (unsigned char *)refType.m_szKinsokuHead; *p; p++ )
		{
			if( _IS_SJIS_1( *p ) )
			{
				if( IsKinsokuKuto( (char*)p, 2 ) )	//��Ǔ_�͕ʊǗ�
				{
					p++;
					continue;
				}
				else{
					*q2 = *p; q2++; p++;
					*q2 = *p; q2++;
					*q2 = 0;
				}
			}
			else
			{
				if( IsKinsokuKuto( (char*)p, 1 ) )	//��Ǔ_�͕ʊǗ�
				{
					continue;
				}
				else
				{
					*q1 = *p; q1++;
					*q1 = 0;
				}
			}
		}

		//�s���֑�������1,2�o�C�g�����𕪂��ĊǗ�����B
		m_sTypeConfig.m_bKinsokuTail = refType.m_bKinsokuTail;
		delete [] m_pszKinsokuTail_1;
		delete [] m_pszKinsokuTail_2;
		length = strlen( refType.m_szKinsokuTail ) + 1;
		m_pszKinsokuTail_1 = new char[ length ];
		m_pszKinsokuTail_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuTail_1;
		q2 = (unsigned char *)m_pszKinsokuTail_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		for( p = (unsigned char *)refType.m_szKinsokuTail; *p; p++ )
		{
			if( _IS_SJIS_1( *p ) )
			{
				*q2 = *p; q2++; p++;
				*q2 = *p; q2++;
				*q2 = 0;
			}
			else
			{
				*q1 = *p; q1++;
				*q1 = 0;
			}
		}

		m_sTypeConfig.m_bKinsokuRet = refType.m_bKinsokuRet;	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	}	//@@@ 2002.04.08 MIK end

	if( bDoRayout ){
		_DoLayout( hwndProgress );
	}
}




/*!
	@brief �w�肳�ꂽ�����s�̃��C�A�E�g�����擾

	@param nLineNum [in] �����s�ԍ� (0�`)
*/
CLayout* CLayoutMgr::SearchLineByLayoutY(
	int nLineNum
)
{
	CLayout*	pLayout;
	int			nCount;
	if( 0 == m_nLines ){
		return NULL;
	}

	//	Mar. 19, 2003 Moca nLineNum�����̏ꍇ�̃`�F�b�N��ǉ�
	if( 0 > nLineNum || nLineNum >= m_nLines ){
		if( 0 > nLineNum ){
			DEBUG_TRACE( _T("CLayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum );
		}
		return NULL;
	}
//	/*+++++++ �ᑬ�� +++++++++*/
//	if( nLineNum < (m_nLines / 2) ){
//		nCount = 0;
//		pLayout = m_pLayoutTop;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->m_pNext;
//			nCount++;
//		}
//	}else{
//		nCount = m_nLines - 1;
//		pLayout = m_pLayoutBot;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->m_pPrev;
//			nCount--;
//		}
//	}


	/*+++++++�킸���ɍ�����+++++++*/
	// 2004.03.28 Moca m_pLayoutPrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	int nPrevToLineNumDiff = abs( m_nPrevReferLine - nLineNum );
	if( m_pLayoutPrevRefer == NULL
	  || nLineNum < nPrevToLineNumDiff
	  || m_nLines - nLineNum < nPrevToLineNumDiff
	){
		if( nLineNum < (m_nLines / 2) ){
			nCount = 0;
			pLayout = m_pLayoutTop;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}else{
			nCount = m_nLines - 1;
			pLayout = m_pLayoutBot;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}else{
		if( nLineNum == m_nPrevReferLine ){
			return m_pLayoutPrevRefer;
		}
		else if( nLineNum > m_nPrevReferLine ){
			nCount = m_nPrevReferLine + 1;
			pLayout = m_pLayoutPrevRefer->m_pNext;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}
		else{
			nCount = m_nPrevReferLine - 1;
			pLayout = m_pLayoutPrevRefer->m_pPrev;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}
	return NULL;
}


//@@@ 2002.09.23 YAZAKI CLayout*���쐬����Ƃ���͕������āAInsertLineNext()�Ƌ��ʉ�
void CLayoutMgr::AddLineBottom( CLayout* pLayout )
{
	if(	0 == m_nLines ){
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
	}else{
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		m_pLayoutBot = pLayout;
	}
	pLayout->m_pNext = NULL;
	m_nLines++;
	return;
}

//@@@ 2002.09.23 YAZAKI CLayout*���쐬����Ƃ���͕������āAAddLineBottom()�Ƌ��ʉ�
CLayout* CLayoutMgr::InsertLineNext( CLayout* pLayoutPrev, CLayout* pLayout )
{
	CLayout* pLayoutNext;

	if(	0 == m_nLines ){
		/* �� */
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}
	else if( NULL == pLayoutPrev ){
		/* �擪�ɑ}�� */
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if( NULL == pLayoutPrev->m_pNext ){
		/* �Ō�ɑ}�� */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		/* �r���ɑ}�� */
		pLayoutNext = pLayoutPrev->m_pNext;
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
	m_nLines++;
	return pLayout;
}

/* CLayout���쐬����
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	���C�A�E�g���������ɒǉ�
*/
CLayout* CLayoutMgr::CreateLayout(
	CDocLine*		pCDocLine,
	CLogicPoint		ptLogicPos,
	int				nLength,
	EColorIndexType	nTypePrev,
	int				nIndent,
	int				nPosX
)
{
	CLayout* pLayout = new CLayout;
	pLayout->m_pCDocLine = pCDocLine;

	pLayout->m_ptLogicPos = ptLogicPos;
	pLayout->m_nLength = nLength;
	pLayout->m_nTypePrev = nTypePrev;
	pLayout->m_nIndent = nIndent;

	if( EOL_NONE == pCDocLine->m_cEol ){
		pLayout->m_cEol.SetType( EOL_NONE );/* ���s�R�[�h�̎�� */
	}else{
		if( pLayout->m_ptLogicPos.x + pLayout->m_nLength >
			pCDocLine->m_cLine.GetStringLength() - pCDocLine->m_cEol.GetLen()
		){
			pLayout->m_cEol = pCDocLine->m_cEol;/* ���s�R�[�h�̎�� */
		}else{
			pLayout->m_cEol = EOL_NONE;/* ���s�R�[�h�̎�� */
		}
	}

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̂݃��C�A�E�g�����L������
	// �u�܂�Ԃ��Ȃ��v�ȊO�Ōv�Z���Ȃ��̂̓p�t�H�[�}���X�ቺ��h���ړI�Ȃ̂ŁA
	// �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�S�Ă̐܂�Ԃ����@�Ōv�Z����
	// �悤�ɂ��Ă��ǂ��Ǝv���B
	// �i���̏ꍇCLayoutMgr::CalculateTextWidth()�̌Ăяo���ӏ����`�F�b�N�j
	pLayout->m_nLayoutWidth = ( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ) ? nPosX : 0;

	return pLayout;
}


/*
|| �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver0

	@date 2002/2/10 aroka CMemory�ύX
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CLayout* pLayout;
	if( NULL == ( pLayout = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = pLayout->m_nLength;
	return pLayout->m_pCDocLine->m_cLine.GetStringPtr() + pLayout->m_ptLogicPos.x;
}

/*!	�w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver1
	@date 2002/03/24 YAZAKI GetLineStr( int nLine, int* pnLineLen )�Ɠ�������ɕύX�B
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen, const CLayout** ppcLayoutDes )
{
	if( NULL == ( (*ppcLayoutDes) = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->m_nLength;
	return (*ppcLayoutDes)->m_pCDocLine->m_cLine.GetStringPtr() + (*ppcLayoutDes)->m_ptLogicPos.x;
}

/*
|| �w�肳�ꂽ�ʒu�����C�A�E�g�s�̓r���̍s�����ǂ������ׂ�

	@date 2002/4/27 MIK
*/
bool CLayoutMgr::IsEndOfLine(
	const CLayoutPoint& ptLinePos
)
{
	const CLayout* pLayout;

	if( NULL == ( pLayout = SearchLineByLayoutY( ptLinePos.y )	) )
	{
		return false;
	}

	if( EOL_NONE == pLayout->m_cEol.GetType() )
	{	/* ���̍s�ɉ��s�͂Ȃ� */
		/* ���̍s�̍Ōォ�H */
		if( ptLinePos.x == pLayout->m_nLength ) return true;
	}

	return false;
}

/*!	@brief �t�@�C�������̃��C�A�E�g�ʒu���擾����

	�t�@�C�������܂őI������ꍇ�ɐ��m�Ȉʒu����^���邽��

	�����̊֐��ł͕����s���烌�C�A�E�g�ʒu��ϊ�����K�v������C
	�����ɖ��ʂ��������߁C��p�֐����쐬
	
	@date 2006.07.29 genta
	@date 2006.10.01 Moca �����o�ŕێ�����悤�ɁB�f�[�^�ύX���ɂ́A_DoLayout/DoLayout_Range�Ŗ����ɂ���B
*/
void CLayoutMgr::GetEndLayoutPos(
	CLayoutPoint* ptLayoutEnd //[out]
)
{
	if( -1 != m_nEOFLine ){
		ptLayoutEnd->x = m_nEOFColumn;
		ptLayoutEnd->y = m_nEOFLine;
		return;
	}

	if( 0 == m_nLines || m_pLayoutBot == NULL ){
		// �f�[�^����
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = 0;
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	CLayout *btm = m_pLayoutBot;
	if( btm->m_cEol != EOL_NONE ){
		//	�����ɉ��s������
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = GetLineCount();
	}
	else {
		CMemoryIterator<CLayout> it( btm, GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->x = it.getColumn();
		ptLayoutEnd->y = GetLineCount() - 1;
		// 2006.10.01 Moca Start [EOF]�݂̂̃��C�A�E�g�s�����������Ă����o�O���C��
		if( GetMaxLineKetas() <= ptLayoutEnd->x ){
			ptLayoutEnd->x = 0;
			ptLayoutEnd->y++;
		}
		// 2006.10.01 Moca End
	}
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
}


/*!	�s�������폜

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2009/08/28 nasukoji	�e�L�X�g�ő啝�̎Z�o�ɑΉ�
*/
void CLayoutMgr::DeleteData_CLayoutMgr(
	int			nLineNum,
	int			nDelPos,
	int			nDelLen,
	int*		pnModifyLayoutLinesOld,
	int*		pnModifyLayoutLinesNew,
	int*		pnDeleteLayoutLines,
	CMemory*	cmemDeleted			//!< [out] �폜���ꂽ�f�[�^
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CLayoutMgr::DeleteData_CLayoutMgr" );
#endif
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
	CLayout*	pLayoutWork;
	int			nModLineOldFrom;	/* �e���̂������ύX�O�̍s(from) */
	int			nModLineOldTo;		/* �e���̂������ύX�O�̍s(to) */
	int			nDelLineOldFrom;	/* �폜���ꂽ�ύX�O�_���s(from) */
	int			nDelLineOldNum;		/* �폜���ꂽ�s�� */
	int			nRowNum;
	int			nDelStartLogicalLine;
	int			nDelStartLogicalPos;
	EColorIndexType	nCurrentLineType;
	int			nLineWork;

	/* ���ݍs�̃f�[�^���擾 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		return;
	}
	pLayout = m_pLayoutPrevRefer;
	nDelStartLogicalLine = pLayout->m_ptLogicPos.y;
	nDelStartLogicalPos  = nDelPos + pLayout->m_ptLogicPos.x;

	pLayoutWork = pLayout;
	nLineWork = nLineNum;
	while( 0 != pLayoutWork->m_ptLogicPos.x ){
		pLayoutWork = pLayoutWork->m_pPrev;
		--nLineWork;
	}
	nCurrentLineType = pLayoutWork->m_nTypePrev;

	/* �e�L�X�g�̃f�[�^���폜 */
	m_pcDocLineMgr->DeleteData_CDocLineMgr(
		nDelStartLogicalLine,
		nDelStartLogicalPos,
		nDelLen,
		&nModLineOldFrom,
		&nModLineOldTo,
		&nDelLineOldFrom,
		&nDelLineOldNum,
		cmemDeleted
	);

//	DUMP();

	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
	int		nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,
		nModLineOldFrom,
		nModLineOldTo,
		nDelStartLogicalLine,
		nDelStartLogicalPos,
		pnModifyLayoutLinesOld
	);

	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
	/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
	/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
	ShiftLogicalLineNum( pLayoutPrev, nDelLineOldNum * (-1) );

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_ptLogicPos.y;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_ptLogicPos.y -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout.y   = nLineNum;			// �ҏW�J�n�s
	ctwArg.ptLayout.x   = nDelPos;			// �ҏW�J�n��
	ctwArg.nDelLines    = 0;				// �폜�s��1�s
	ctwArg.nAllLinesOld = nAllLinesOld;		// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = FALSE;			// �ǉ�������Ȃ�

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int	nAddInsLineNum;
	*pnModifyLayoutLinesNew = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nDelStartLogicalLine, nDelStartLogicalPos,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnDeleteLayoutLines = nAllLinesOld - m_nLines + nAddInsLineNum;
	return;
}






/*!	������}��

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CLayoutMgr::InsertData_CLayoutMgr(
	int			nLineNum,
	int			nInsPos,
	const char*	pInsData,
	int			nInsDataLen,
	int*		pnModifyLayoutLinesOld,
	int*		pnInsLineNum,		// �}���ɂ���đ��������C�A�E�g�s�̐�
	int*		pnNewLine,			// �}�����ꂽ�����̎��̈ʒu�̍s
	int*		pnNewPos			// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
)
{
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
	CLayout*	pLayoutWork;
	int			nInsStartLogicalLine;
	int			nInsStartLogicalPos;
	int			nInsLineNum;
	int			nRowNum;
	EColorIndexType	nCurrentLineType;
	int			nLineWork;


	// ���ݍs�̃f�[�^���擾
	int			nLineLen;
	const char* pLine = GetLineStr( nLineNum, &nLineLen );
	if( !pLine ){
		/*
			2004.04.02 FILE / Moca �J�[�\���ʒu�s���̂��߁A��e�L�X�g��
			nLineNum��0�łȂ��Ƃ��ɗ�����΍�D�f�[�^����ł��邱�Ƃ�
			�J�[�\���ʒu�ł͂Ȃ����s���Ŕ��肷�邱�Ƃł��m���ɁD
		*/
		if( m_nLines == 0 )
		{
			/* ��̃e�L�X�g�̐擪�ɍs�����ꍇ */
			pLayout = NULL;
			nLineWork = 0;
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = 0;
			nCurrentLineType = COLORIDX_DEFAULT;
		}
		else{
			pLine = GetLineStr( m_nLines - 1, &nLineLen );
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				// ��łȂ��e�L�X�g�̍Ō�ɍs�����ꍇ
				pLayout = NULL;
				nLineWork = 0;
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = 0;
				nCurrentLineType = m_nLineTypeBot;
			}
			else{
				/* ��łȂ��e�L�X�g�̍Ō�̍s��ύX����ꍇ */
				nLineNum = m_nLines	- 1;
				nInsPos = nLineLen;
				pLayout = m_pLayoutPrevRefer;
				nLineWork = m_nPrevReferLine;


				nInsStartLogicalLine = pLayout->m_ptLogicPos.y;
				nInsStartLogicalPos  = nInsPos + pLayout->m_ptLogicPos.x;
				nCurrentLineType = pLayout->m_nTypePrev;
			}
		}
	}else{
		pLayout = m_pLayoutPrevRefer;
		nLineWork = m_nPrevReferLine;


		nInsStartLogicalLine = pLayout->m_ptLogicPos.y;
		nInsStartLogicalPos  = nInsPos + pLayout->m_ptLogicPos.x;
		nCurrentLineType = pLayout->m_nTypePrev;
	}

	if( pLayout ){
		pLayoutWork = pLayout;
		while( pLayoutWork != NULL && 0 != pLayoutWork->m_ptLogicPos.x ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		if( NULL != pLayoutWork ){
			nCurrentLineType = pLayoutWork->m_nTypePrev;
		}else{
			nCurrentLineType = COLORIDX_DEFAULT;
		}
	}


	// �f�[�^�̑}��
	int			nNewLine;	//�}�����ꂽ�����̎��̍s
	int			nNewPos;	//�}�����ꂽ�����̎��̃f�[�^�ʒu
	m_pcDocLineMgr->InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData,
		nInsDataLen,
		&nInsLineNum,
		&nNewLine,
		&nNewPos
	);


	//--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---
	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜����
	// �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ�
	int	nAllLinesOld = m_nLines;
	if( NULL != pLayout ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			nInsStartLogicalLine, nInsStartLogicalLine,
			nInsStartLogicalLine, nInsStartLogicalPos,
			pnModifyLayoutLinesOld
		);
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����
	// �_���s���폜���ꂽ�ꍇ�͂O��菬�����s��
	// �_���s���}�����ꂽ�ꍇ�͂O���傫���s��
	if( pLine ){
		ShiftLogicalLineNum( pLayoutPrev, nInsLineNum );
	}

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_ptLogicPos.y;
		}
	}
	else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_ptLogicPos.y -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout.y   = nLineNum;			// �ҏW�J�n�s
	ctwArg.ptLayout.x   = nInsPos;			// �ҏW�J�n��
	ctwArg.nDelLines    = -1;				// �폜�s�Ȃ�
	ctwArg.nAllLinesOld = nAllLinesOld;		// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = TRUE;				// �ǉ������񂠂�

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	int			nAddInsLineNum;
	DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nInsStartLogicalLine, nInsStartLogicalPos,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnInsLineNum = m_nLines - nAllLinesOld + nAddInsLineNum;

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	pLayout = SearchLineByLayoutY( nNewLine );
	XYLogicalToLayout( pLayout, nNewLine, nNewLine, nNewPos, pnNewLine, pnNewPos );
	return;
}






/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
CLayout* CLayoutMgr::DeleteLayoutAsLogical(
	CLayout*	pLayoutInThisArea,
	int			nLineOf_pLayoutInThisArea,
	int			nLineFrom,
	int			nLineTo,
	int			nDelLogicalLineFrom,
	int			nDelLogicalColFrom,
	int*		pnDeleteLines
)
{
	CLayout* pLayout;
	CLayout* pLayoutWork;
	CLayout* pLayoutNext;

	*pnDeleteLines = 0;
	if( 0 == m_nLines){	/* �S�����s�� */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->m_pPrev;
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - 1;


	/* �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�` */
	pLayoutWork = pLayoutInThisArea->m_pPrev;
	while( NULL != pLayoutWork && nLineFrom <= pLayoutWork->m_ptLogicPos.y){
		pLayoutWork = pLayoutWork->m_pPrev;
	}



	if( NULL == pLayoutWork ){
		pLayout	= m_pLayoutTop;
	}else{
		pLayout = pLayoutWork->m_pNext;
	}
	while( NULL != pLayout ){
		if( pLayout->m_ptLogicPos.y > nLineTo ){
			break;
		}
		pLayoutNext = pLayout->m_pNext;
		if( NULL == pLayoutWork ){
			/* �擪�s�̏��� */
			m_pLayoutTop = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else{
			pLayoutWork->m_pNext = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if( m_pLayoutPrevRefer == pLayout ){
//			// 1999.12.22 �O�ɂ��炷�����ł悢�̂ł�
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;
//		}

		if( ( nDelLogicalLineFrom == pLayout->m_ptLogicPos.y &&
			  nDelLogicalColFrom < pLayout->m_ptLogicPos.x + pLayout->m_nLength ) ||
			( nDelLogicalLineFrom < pLayout->m_ptLogicPos.y )
		){
			(*pnDeleteLines)++;
		}

		if( m_pLayoutPrevRefer == pLayout ){
			DEBUG_TRACE( _T("�o�O�o�O\n") );
		}

		delete pLayout;

		m_nLines--;	/* �S�����s�� */
		if( NULL == pLayoutNext ){
			m_pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE( _T("(*pnDeleteLines)=%d\n"), (*pnDeleteLines) );

	return pLayoutWork;
}




/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
void CLayoutMgr::ShiftLogicalLineNum( CLayout* pLayoutPrev, int nShiftLines )
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::ShiftLogicalLineNum" );

	CLayout* pLayout;
//	CLayout* pLayoutNext;
	if( 0 == nShiftLines ){
		return;
	}
	if( NULL == pLayoutPrev ){
		pLayout = m_pLayoutTop;
	}else{
		pLayout = pLayoutPrev->m_pNext;
	}
	/* ���C�A�E�g���S�̂��X�V����(�ȂȁA�Ȃ��!!!) */
	while( NULL != pLayout ){
		pLayout->m_ptLogicPos.y += nShiftLines;	/* �Ή�����_���s�ԍ� */
		pLayout = pLayout->m_pNext;
	}
	return;
}


bool CLayoutMgr::ChangeLayoutParam(
	HWND hwndProgress,
	int nTabSize,
	int nMaxLineKetas
)
{
	if( nTabSize < 1 || nTabSize > 64 ) { return false; }
	if( nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS ){ return false; }

	m_sTypeConfig.m_nTabSpace = nTabSize;
	m_sTypeConfig.m_nMaxLineKetas = nMaxLineKetas;

	_DoLayout( hwndProgress );

	return true;
}





/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
bool CLayoutMgr::WhereCurrentWord(
	int				nLineNum,
	int				nIdx,
	CLayoutRange*	pSelect,		//!< [out]
	CMemory*		pcmcmWord,		//!< [out]
	CMemory*		pcmcmWordLeft	//!< [out]
)
{
	CLayout* pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return false;
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	int nFromX;
	int nToX;
	bool nRetCode = m_pcDocLineMgr->WhereCurrentWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_ptLogicPos.y, nFromX, &pSelect->m_ptFrom.y, &pSelect->m_ptFrom.x );
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_ptLogicPos.y, nToX, &pSelect->m_ptTo.y, &pSelect->m_ptTo.x );
	}
	return nRetCode;

}





/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
int CLayoutMgr::PrevOrNextWord(
	int				nLineNum,
	int				nIdx,
	CLayoutPoint*	pptLayoutNew,
	BOOL			bLEFT,
	BOOL			bStopsBothEnds
)
{
	CLayout*  pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}

	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	int nRetCode = m_pcDocLineMgr->PrevOrNextWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		&pptLayoutNew->x,
		bLEFT,
		bStopsBothEnds
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		XYLogicalToLayout(
			pLayout,
			nLineNum,
			pLayout->m_ptLogicPos.y,
			pptLayoutNew->x,
			&pptLayoutNew->y,
			&pptLayoutNew->x
		);
	}
	return nRetCode;
}





//! �P�ꌟ��
/*
	@retval 0 ������Ȃ�
*/
int CLayoutMgr::SearchWord(
	int						nLineNum, 			//!< [in] �����J�n�s
	int						nIdx, 				//!< [in] �����J�n�ʒu
	const char*				pszPattern,			//!< [in] ��������
	ESearchDirection		eSearchDirection,	//!< [in] ��������
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	int*					pnLineFrom, 		//!< [out] �}�b�`���C�A�E�g�sfrom
	int*					pnIdxFrom, 			//!< [out] �}�b�`���C�A�E�g�ʒufrom
	int*					pnLineTo, 			//!< [out] �}�b�`���C�A�E�g�sto
	int*					pnIdxTo,  			//!< [out] �}�b�`���C�A�E�g�ʒuto
	CBregexp*				pRegexp				//!< [in] ���K�\���R���p�C���f�[�^	Jun. 26, 2001 genta
)
{
	int			nRetCode;
	int			nLogLine;
	CLayout*	pLayout;
	pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}

	// �P�ꌟ��
	nRetCode = m_pcDocLineMgr->SearchWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		pszPattern,
		eSearchDirection,
		sSearchOption,
		pnLineFrom,
		pnIdxFrom,
		pnIdxTo,
		pRegexp			/* ���K�\���R���p�C���f�[�^ */
	);

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	if( nRetCode ){
		nLogLine = *pnLineFrom;
		LogicToLayout(
			*pnIdxFrom,
			nLogLine,
			pnIdxFrom,
			pnLineFrom
		);
		LogicToLayout(
			*pnIdxTo,
			nLogLine,
			pnIdxTo,
			pnLineTo
		);
	}
	return nRetCode;
}





/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
void CLayoutMgr::XYLogicalToLayout(
		CLayout*	pLayoutInThisArea,
		int			nLayoutLineOfThisArea,
		int			nLogLine,
		int			nLogIdx,
		int*		pnLayLine,
		int*		pnLayIdx
)
{
	CLayout*	pLayout;
	int			nCurLayLine;
	*pnLayLine = 0;
	*pnLayIdx = 0;

	if( pLayoutInThisArea == NULL ){
		pLayoutInThisArea = m_pLayoutBot;
		nLayoutLineOfThisArea = m_nLines - 1;
	}
	if( pLayoutInThisArea == NULL ){
		*pnLayLine = m_nLines;
		*pnLayIdx = 0;
		return;
	}
	/* �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�` */
	if( (pLayoutInThisArea->m_ptLogicPos.y > nLogLine) ||
		(pLayoutInThisArea->m_ptLogicPos.y == nLogLine &&
		 pLayoutInThisArea->m_ptLogicPos.x >	nLogIdx)
	){
		/* ���݈ʒu���O���Ɍ������ăT�[�` */
		pLayout =  pLayoutInThisArea->m_pPrev;
		nCurLayLine = nLayoutLineOfThisArea - 1;
		while( pLayout != NULL ){
			if( pLayout->m_ptLogicPos.y == nLogLine &&
				pLayout->m_ptLogicPos.x <= nLogIdx &&
				nLogIdx <= pLayout->m_ptLogicPos.x + pLayout->m_nLength
			){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
				return;
			}
			if( NULL == pLayout->m_pPrev ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = 0;
				return;
			}
			pLayout = pLayout->m_pPrev;
			nCurLayLine--;
		}
	}else{
		/* ���݈ʒu���܂ތ���Ɍ������ăT�[�` */
		pLayout =  pLayoutInThisArea;
		nCurLayLine = nLayoutLineOfThisArea;
		while( pLayout != NULL ){
			if( pLayout->m_ptLogicPos.y == nLogLine &&
				pLayout->m_ptLogicPos.x <= nLogIdx  &&
				nLogIdx <= pLayout->m_ptLogicPos.x + pLayout->m_nLength
			 ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
				return;
			}
			if( NULL == pLayout->m_pNext ){
//				if( nCurLayLine == nLogLine ){
//					*pnLayLine = nCurLayLine;
//					*pnLayIdx = pLayout->m_nLength;
//				}else{
					*pnLayLine = nCurLayLine + 1;
					*pnLayIdx = 0;
//				}
				return;
			}else
			if( pLayout->m_pNext->m_ptLogicPos.y > nLogLine ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
				return;
			}
			pLayout = pLayout->m_pNext;
			nCurLayLine++;
		}
	}
	return;
}


/*!
	@brief �J�[�\���ʒu�ϊ� ���������C�A�E�g

	�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	�����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)

	@date 2004.06.16 Moca �C���f���g�\���̍ۂ�TAB���܂ލs�̍��W����C��
	@date 2007.09.06 kobake �֐�����CaretPos_Phys2Log����LogicToLayout�ɕύX
*/
void CLayoutMgr::LogicToLayout(
	int		nX,				//!< [in] �����ʒuX
	int		nY,				//!< [in] �����ʒuY
	int*	pnCaretPosX,	//!< [out] �_���ʒuX
	int*	pnCaretPosY		//!< [out] �_���ʒuY
)
{
	int				nCaretPosX = 0;
	int				nCaretPosY;
	const CLayout*	pLayout;
	int				nCharChars;

	*pnCaretPosX = 0;
	*pnCaretPosY = 0;

	// ���s�P�ʍs <= �܂�Ԃ��P�ʍs �����藧����A
	// �T�[�`�J�n�n�_���ł��邾���ړI�n�֋߂Â���
	pLayout = SearchLineByLayoutY( nY );
	if( !pLayout ){
		if( 0 < m_nLines ){
			*pnCaretPosY = m_nLines;
		}
		return;
	}
	nCaretPosY = nY;

	//	Layout���P����ɐi�߂Ȃ���nY�������s�Ɉ�v����Layout��T��
	do{
		if( nY == pLayout->m_ptLogicPos.y ){
			//	2004.06.16 Moca �C���f���g�\���̍ۂɈʒu�������(TAB�ʒu����ɂ��)
			//	TAB���𐳊m�Ɍv�Z����ɂ͓�������C���f���g���������Ă����K�v������D
			nCaretPosX = pLayout->GetIndent();
			const char*		pData;
			pData = pLayout->m_pCDocLine->m_cLine.GetStringPtr() + pLayout->m_ptLogicPos.x; // 2002/2/10 aroka CMemory�ύX
			int	nDataLen = pLayout->m_nLength;

			int i;
			for( i = 0; i < nDataLen; ++i ){
				if( pLayout->m_ptLogicPos.x + i >= nX ){
					break;
				}
				if( pData[i] ==	TAB ){
					// Sep. 23, 2002 genta �����o�[�֐����g���悤��
					nCharChars = GetActualTabSpace( nCaretPosX );
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pData, nDataLen, i );
				}
				if( nCharChars == 0 ){
					nCharChars = 1;
				}

				nCaretPosX += nCharChars;

				if( pData[i] ==	TAB ){
					nCharChars = 1;
				}
				i += nCharChars - 1;
			}
			if( i < nDataLen ){
				//	nX, nY�����̍s�̒��Ɍ��������烋�[�v�ł��؂�
				break;
			}

			if( !pLayout->m_pNext ){
				//	���Y�ʒu�ɒB���Ă��Ȃ��Ă��C���C�A�E�g�����Ȃ�f�[�^������Ԃ��D
				nCaretPosX += ( nDataLen - i );
				break;
			}

			if( nY < pLayout->m_pNext->m_ptLogicPos.y ){
				//	����Layout�����Y�����s���߂��Ă��܂��ꍇ�̓f�[�^������Ԃ��D
				nCaretPosX += ( nDataLen - i );
				break;
			}
		}
		if( nY < pLayout->m_ptLogicPos.y ){
			//	�ӂ��͂����ɂ͗��Ȃ��Ǝv����... (genta)
			//	Layout�̎w�������s���T���Ă���s������w���Ă�����ł��؂�
			break;
		}

		//	���̍s�֐i��
		nCaretPosY++;
		pLayout = pLayout->m_pNext;
	}
	while( pLayout );

	//	2004.06.16 Moca �C���f���g�\���̍ۂ̈ʒu����C��
	*pnCaretPosX = pLayout ? nCaretPosX : 0;
	*pnCaretPosY = nCaretPosY;
}

/*!
	@brief �J�[�\���ʒu�ϊ�  ���C�A�E�g������

	���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	�������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)

	@date 2007.09.06 kobake �֐�����CaretPos_Log2Phys��LayoutToLogic�ɕύX
*/
void CLayoutMgr::LayoutToLogic(
	int		nCaretPosX,		//!< [in] �_���ʒuX
	int		nCaretPosY,		//!< [in] �_���ʒuY
	int*	pnX,			//!< [out] �����ʒuX
	int*	pnY				//!< [out] �����ʒuY
)
{
	int				nCharKetas;

	*pnX = 0;
	*pnY = 0;
	if( nCaretPosY > m_nLines ){
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}

	int				nDataLen;
	const char*		pData;

	BOOL			bEOF = FALSE;
	int				nX;
	const CLayout*	pcLayout = SearchLineByLayoutY( nCaretPosY );
	if( !pcLayout ){
		if( 0 < nCaretPosY ){
			pcLayout = SearchLineByLayoutY( nCaretPosY - 1 );
			if( NULL == pcLayout ){
				*pnX = 0;
				*pnY = m_pcDocLineMgr->GetLineCount(); // 2002/2/10 aroka CDocLineMgr�ύX
				return;
			}
			else{
				pData = GetLineStr( nCaretPosY - 1, &nDataLen );
				if( pData[nDataLen - 1] == '\r' || pData[nDataLen - 1] == '\n' ){
					*pnX = 0;
					*pnY = m_pcDocLineMgr->GetLineCount(); // 2002/2/10 aroka CDocLineMgr�ύX
					return;
				}
				else{
					*pnY = m_pcDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka CDocLineMgr�ύX
					bEOF = TRUE;
					nX = 999999;
					goto checkloop;

				}
			}
		}
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}
	else{
		*pnY = pcLayout->m_ptLogicPos.y;
	}

	pData = GetLineStr( nCaretPosY, &nDataLen );
	nX = pcLayout ? pcLayout->GetIndent() : 0;
checkloop:;
	int			i;
	for( i = 0; i < nDataLen; ++i )
	{
		if( pData[i] ==	TAB ){
			// Sep. 23, 2002 genta �����o�[�֐����g���悤��
			nCharKetas = GetActualTabSpace( nX );
		}
		else{
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharKetas = CMemory::GetSizeOfChar( pData, nDataLen, i );
		}

		if( nCharKetas == 0 ){
			nCharKetas = 1;
		}

		nX += nCharKetas;
		if( nX > nCaretPosX && !bEOF ){
			break;
		}

		if( pData[i] ==	TAB ){
			nCharKetas = 1;
		}
		i += nCharKetas - 1;
	}
	i += pcLayout->m_ptLogicPos.x;
	*pnX = i;
	return;
}


/* �e�X�g�p�Ƀ��C�A�E�g�����_���v */
void CLayoutMgr::DUMP()
{
#ifdef _DEBUG
	const char* pData;
	int nDataLen;
	MYTRACE( _T("------------------------\n") );
	MYTRACE( _T("m_nLines=%d\n"), m_nLines );
	MYTRACE( _T("m_pLayoutTop=%08lxh\n"), m_pLayoutTop );
	MYTRACE( _T("m_pLayoutBot=%08lxh\n"), m_pLayoutBot );
	MYTRACE( _T("m_nMaxLineKetas=%d\n"), m_sTypeConfig.m_nMaxLineKetas );

	MYTRACE( _T("m_nTabSpace=%d\n"), m_sTypeConfig.m_nTabSpace );
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->m_pNext;
		MYTRACE( _T("\t-------\n") );
		MYTRACE( _T("\tthis=%08lxh\n"), pLayout );
		MYTRACE( _T("\tm_pPrev =%08lxh\n"),		pLayout->m_pPrev );
		MYTRACE( _T("\tm_pNext =%08lxh\n"),		pLayout->m_pNext );
		MYTRACE( _T("\tm_ptLogicPos.y=%d\n"),	pLayout->m_ptLogicPos.y );
		MYTRACE( _T("\tm_ptLogicPos.x=%d\n"),	pLayout->m_ptLogicPos.x );
		MYTRACE( _T("\tm_nLength=%d\n"),		pLayout->m_nLength );
		MYTRACE( _T("\tm_enumEOLType =%s\n"),	pLayout->m_cEol.GetName() );
		MYTRACE( _T("\tm_nEOLLen =%d\n"),		pLayout->m_cEol.GetLen() );
		MYTRACE( _T("\tm_nTypePrev=%d\n"),		pLayout->m_nTypePrev );
		pData = m_pcDocLineMgr->GetLineStr( pLayout->m_ptLogicPos.y, &nDataLen );
		MYTRACE( _T("\t[%s]\n"), pData );
		pLayout = pLayoutNext;
	}
	MYTRACE( _T("------------------------\n") );
#endif
	return;
}


/*[EOF]*/
