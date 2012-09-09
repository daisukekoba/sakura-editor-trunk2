/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CBlockComment.h"
#include "CMemory.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

CBlockComment::CBlockComment()
{
	m_szBlockCommentFrom[ 0 ] = '\0';
	m_szBlockCommentTo[ 0 ] = '\0';
	m_nBlockFromLen = 0;
	m_nBlockToLen = 0;
}

/*!
	�u���b�N�R�����g�f���~�^���R�s�[����
*/
void CBlockComment::SetBlockCommentRule(
	const TCHAR*	pszFrom,	//!< [in] �R�����g�J�n������
	const TCHAR*	pszTo		//!< [in] �R�����g�I��������
)
{
	int nStrLen = _tcslen( pszFrom );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentFrom, pszFrom );
		m_nBlockFromLen = nStrLen;
	}
	else {
		m_szBlockCommentFrom[0] = '\0';
		m_nBlockFromLen = 0;
	}
	nStrLen = _tcslen( pszTo );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentTo, pszTo );
		m_nBlockToLen = nStrLen;
	}
	else {
		m_szBlockCommentTo[0] = '\0';
		m_nBlockToLen = 0;
	}
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́AnPos����̕����񂪊J�n������(From)�ɓ��Ă͂܂邩�m�F����B

	@retval true  ��v����
	@retval false ��v���Ȃ�����
*/
bool CBlockComment::Match_CommentFrom(
	int				nPos,		//!< [in] �T���J�n�ʒu
	int				nLineLen,	//!< [in] pLine�̒���
	const TCHAR*	pLine		//!< [in] �T���s�̐擪�D�T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
) const
{
	if (
		_T('\0') != m_szBlockCommentFrom[0] &&
		_T('\0') != m_szBlockCommentTo[0]  &&
		nPos <= nLineLen - m_nBlockFromLen &&	/* �u���b�N�R�����g�f���~�^(From) */
		0 == my_memicmp( &pLine[nPos], m_szBlockCommentFrom, m_nBlockFromLen )
	){
		return true;
	}
	return false;
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́A���(To)�ɓ��Ă͂܂镶�����nPos�ȍ~����T��

	@return ���Ă͂܂����ʒu��Ԃ����A���Ă͂܂�Ȃ������Ƃ��́AnLineLen�����̂܂ܕԂ��B
*/
int CBlockComment::Match_CommentTo(
	int				nPos,		//!< [in] �T���J�n�ʒu
	int				nLineLen,	//!< [in] pLine�̒���
	const TCHAR*	pLine		//!< [in] �T���s�̐擪�D�T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
) const
{
	int i;
	for( i = nPos; i <= nLineLen - m_nBlockToLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars_2 = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		if( 0 == nCharChars_2 ){
			nCharChars_2 = 1;
		}
		if( 0 == my_memicmp( &pLine[i], m_szBlockCommentTo, m_nBlockToLen ) ){
			return i + m_nBlockToLen;
		}
		if( 2 == nCharChars_2 ){
			++i;
		}
	}
	return nLineLen;
}
/*[EOF]*/
