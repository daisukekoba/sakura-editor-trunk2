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

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

class CMemory;// 2002/2/10 aroka


// �A���h�D�o�b�t�@�p ����R�[�h
enum EOpeCode {
	OPE_UNKNOWN		= 0,
	OPE_INSERT		= 1,
	OPE_DELETE		= 2,
	OPE_MOVECARET	= 3,
};


/*!
	�ҏW����v�f
	
	Undo�̂��߂ɂɑ���菇���L�^���邽�߂ɗp����B
	1�I�u�W�F�N�g���P�̑����\���B
*/
class COpe {
public:
	COpe();		/* COpe�N���X�\�z */
	~COpe();	/* COpe�N���X���� */

	void DUMP( void );	/* �ҏW����v�f�̃_���v */

	EOpeCode	m_nOpe;						//!< ������

	CLogicPoint	m_ptCaretPos_PHY_Before;	//!< �J�[�\���ʒu
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< ����O�̃L�����b�g�ʒu
	CLogicPoint	m_ptCaretPos_PHY_After;		//!< �J�[�\���ʒu

	int			m_nDataLen;			//!< ����Ɋ֘A����f�[�^�̃T�C�Y
	CMemory*	m_pcmemData;		//!< ����Ɋ֘A����f�[�^

};



///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */


/*[EOF]*/
