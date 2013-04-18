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
	for( i = 0; i < (int)m_ppCOpeArr.size(); ++i ){
		delete m_ppCOpeArr[i];
	}
	m_ppCOpeArr.clear();
}


/* ����̒ǉ� */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("�o�O\n")
			_T("pcOpe->m_nCaretPosX_PHY_Before = %d\n")
			_T("pcOpe->m_nCaretPosY_PHY_Before = %d\n")
			_T("pcOpe->m_nCaretPosX_PHY_After = %d\n")
			_T("pcOpe->m_nCaretPosY_PHY_After = %d\n"),
			pcOpe->m_nCaretPosX_PHY_Before,
			pcOpe->m_nCaretPosY_PHY_Before,
			pcOpe->m_nCaretPosX_PHY_After,
			pcOpe->m_nCaretPosY_PHY_After
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
	for( i = 0; i < GetNum(); ++i ){
		MYTRACE( _T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}


/*[EOF]*/
