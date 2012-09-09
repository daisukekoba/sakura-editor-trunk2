/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 2002, Yazaki
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CBLOCKCOMMENT_H_
#define _CBLOCKCOMMENT_H_

//	sakura
#include "global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*! �u���b�N�R�����g�f���~�^���Ǘ�����

	@note CBlockComment�́A���L������STypeConfig�Ɋ܂܂��̂ŁA�����o�ϐ��͏�Ɏ��̂������Ă��Ȃ���΂Ȃ�Ȃ��B
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

//	2005.11.10 Moca �A�N�Z�X�֐��ǉ�
class SAKURA_CORE_API CBlockComment{
public:
	//�����Ɣj��
	CBlockComment();

	//�ݒ�
	void SetBlockCommentRule( const TCHAR* pszFrom, const TCHAR* pszTo );	//	�s�R�����g�f���~�^���R�s�[����

	//����
	bool Match_CommentFrom( int nPos, int nLineLen, const char* pLine ) const;	//	�s�R�����g�ɒl���邩�m�F����	
	int Match_CommentTo( int nPos, int nLineLen, const char* pLine ) const;	//	�s�R�����g�ɒl���邩�m�F����

	//�擾
	const TCHAR* getBlockCommentFrom() const{ return m_szBlockCommentFrom; }
	const TCHAR* getBlockCommentTo() const{ return m_szBlockCommentTo; }
	int getBlockFromLen() const { return m_nBlockFromLen; }
	int getBlockToLen() const { return m_nBlockToLen; }

private:
	TCHAR	m_szBlockCommentFrom[BLOCKCOMMENT_BUFFERSIZE];	//!< �u���b�N�R�����g�f���~�^(From)
	TCHAR	m_szBlockCommentTo[BLOCKCOMMENT_BUFFERSIZE];	//!< �u���b�N�R�����g�f���~�^(To)
	int		m_nBlockFromLen;
	int		m_nBlockToLen;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */


/*[EOF]*/
