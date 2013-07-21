/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayout.h"
#include "Debug.h"



CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
	DEBUG_TRACE( _T("\n\n��CLayout::DUMP()======================\n") );
	DEBUG_TRACE( _T("m_ptLogicPos.y=%d\t\t�Ή�����_���s�ԍ�\n"), m_ptLogicPos.y );
	DEBUG_TRACE( _T("m_ptLogicPos.x=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n"), m_ptLogicPos.x );
	DEBUG_TRACE( _T("m_nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n"), m_nLength );
	DEBUG_TRACE( _T("m_nTypePrev=%d\t\t�^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� \n"), m_nTypePrev );
	DEBUG_TRACE( _T("======================\n") );
	return;
}


/*[EOF]*/
