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
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include "COpeBlk.h"
#include "Debug.h"



COpeBlk::COpeBlk()
{
	m_nCOpeArrNum = 0;	/* ����̐� */
	m_ppCOpeArr = NULL;	/* ����̔z�� */
}

COpeBlk::~COpeBlk()
{
	int		i;
	/* ����̔z����폜���� */
	if( 0 < m_nCOpeArrNum && NULL != m_ppCOpeArr ){
		for( i = 0; i < m_nCOpeArrNum; ++i ){
			if( NULL != m_ppCOpeArr[i] ){
				delete m_ppCOpeArr[i];
				m_ppCOpeArr[i] = NULL;
			}
		}
		free( m_ppCOpeArr );
		m_ppCOpeArr = NULL;
		m_nCOpeArrNum = 0;
	}
}


/* ����̒ǉ� */
int COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	){
		MYMESSAGEBOX( NULL,
			MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
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
	if( 0 == m_nCOpeArrNum ){
		m_ppCOpeArr = (COpe**)malloc( sizeof( COpe* ) );
	}else{
		m_ppCOpeArr = (COpe**)realloc( (void*)m_ppCOpeArr,  sizeof( COpe* ) * (m_nCOpeArrNum + 1 ) );
	}
	if( NULL == m_ppCOpeArr ){
		MessageBox( 0, "COpeBlk::AppendOpe() error", "�������m�ۂɎ��s���܂����B\n���Ɋ댯�ȏ�Ԃł��B", MB_OK );
		return FALSE;
	}
	m_ppCOpeArr[m_nCOpeArrNum] = pcOpe;
	m_nCOpeArrNum++;
	return TRUE;
}


/* �����Ԃ� */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( m_nCOpeArrNum <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}


/* �ҏW����v�f�u���b�N�̃_���v */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	for( i = 0; i < m_nCOpeArrNum; ++i ){
		MYTRACE_A( "\tCOpeBlk.m_ppCOpeArr[%d]----\n", i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}


/*[EOF]*/
