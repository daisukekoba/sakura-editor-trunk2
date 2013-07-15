/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "COpe.h"
#include "Debug.h"
#include "CMemory.h"// 2002/2/10 aroka


// COpe�N���X�\�z
COpe::COpe()
{
	m_nOpe = OPE_UNKNOWN;					// ������

	m_ptCaretPos_PHY_To.x = 0;		// ����O�̃L�����b�g�ʒu�w To ���s�P�ʍs�̍s�ԍ��i�O�J�n�j
	m_ptCaretPos_PHY_To.y = 0;		// ����O�̃L�����b�g�ʒu�x To ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j
	m_ptCaretPos_PHY_Before.x = -1;	// �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j
	m_ptCaretPos_PHY_Before.y = -1;	// �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j
	m_ptCaretPos_PHY_After.x = -1;	// �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j
	m_ptCaretPos_PHY_After.y = -1;	// �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j
	m_nDataLen = 0;					// ����Ɋ֘A����f�[�^�̃T�C�Y 
	m_pcmemData = NULL;				// ����Ɋ֘A����f�[�^ 
}




/* COpe�N���X���� */
COpe::~COpe()
{
	if( NULL != m_pcmemData ){	/* ����Ɋ֘A����f�[�^ */
		delete m_pcmemData;
		m_pcmemData = NULL;
	}
	return;
}

/* �ҏW����v�f�̃_���v */
void COpe::DUMP( void )
{
	DEBUG_TRACE( _T("\t\tm_nOpe                  = [%d]\n"), m_nOpe               );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_Before.x = [%d]\n"), m_ptCaretPos_PHY_Before.x   );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_Before.y = [%d]\n"), m_ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_After.x; = [%d]\n"), m_ptCaretPos_PHY_After.x   );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_After.y; = [%d]\n"), m_ptCaretPos_PHY_After.y   );
	DEBUG_TRACE( _T("\t\tm_nDataLen              = [%d]\n"), m_nDataLen           );
	if( NULL == m_pcmemData ){
		DEBUG_TRACE( _T("\t\tm_pcmemData         = [NULL]\n") );
	}else{
		DEBUG_TRACE( _T("\t\tm_pcmemData         = [%s]\n"), m_pcmemData->GetStringPtr() );
	}
	return;
}


/*[EOF]*/
