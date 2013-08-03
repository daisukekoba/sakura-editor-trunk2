/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "COpeBlk.h"
#include "Debug.h"



COpeBlk::COpeBlk()
{
	m_refCount = 0;
}

COpeBlk::~COpeBlk()
{
	int		i;
	/* ����̔z����폜���� */
	int size = (int)m_ppCOpeArr.size();
	for( i = 0; i < size; ++i ){
		delete m_ppCOpeArr[i];
	}
	m_ppCOpeArr.clear();
}


/* ����̒ǉ� */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_ptCaretPos_PHY_Before.x	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_Before.y	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_After.x		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_After.y		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("�o�O\n")
			_T("pcOpe->m_ptCaretPos_PHY_Before.x = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_Before.y = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_After.x = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_After.y = %d\n"),
			pcOpe->m_ptCaretPos_PHY_Before.x,
			pcOpe->m_ptCaretPos_PHY_Before.y,
			pcOpe->m_ptCaretPos_PHY_After.x,
			pcOpe->m_ptCaretPos_PHY_After.y
		);
	}

	/* �z��̃������T�C�Y�𒲐� */
	m_ppCOpeArr.push_back(pcOpe);
	return true;
}


/* �����Ԃ� */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( GetNum() <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}


/* �ҏW����v�f�u���b�N�̃_���v */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	int size = GetNum();
	for( i = 0; i < size; ++i ){
		MYTRACE( _T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}


/*[EOF]*/
