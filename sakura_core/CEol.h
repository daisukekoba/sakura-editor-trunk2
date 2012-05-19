/*! @file
	@brief End of Line��ʂ̊Ǘ�

	@author genta
	@date 2000/5/15 �V�K�쐬
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _CEOL_H_
#define _CEOL_H_

#include "global.h"

// 2002/09/22 Moca EOL_CRLF_UNICODE��p�~
/* �s�I�[�q�̎�� */
SAKURA_CORE_API enum EEolType {
	EOL_NONE,			//!<
	EOL_CRLF,			//!< 0d0a
	EOL_LFCR,			//!< 0a0d
	EOL_LF,				//!< 0a
	EOL_CR,				//!< 0d
	EOL_CODEMAX,		//
	EOL_UNKNOWN = -1	//
};

/* �s�I�[�q�̃f�[�^�� */
SAKURA_CORE_API enum enumEOLLen {
	LEN_EOL_NONE			= 0,
	LEN_EOL_CRLF			= 2,
	LEN_EOL_LFCR			= 2,
	LEN_EOL_LF				= 1,
	LEN_EOL_CR				= 1
//	LEN_EOL_UNKNOWN		0
};

#define EOL_TYPE_NUM	5

/* �s�I�[�q�̔z�� */
SAKURA_CORE_API extern const EEolType gm_pnEolTypeArr[EOL_TYPE_NUM];


/*!
	@brief �s���̉��s�R�[�h���Ǘ�

	�Ǘ��Ƃ͌����Ă��I�u�W�F�N�g�����邱�Ƃň��S�ɐݒ���s������֘A���̎擾��
	�I�u�W�F�N�g�ɑ΂��郁�\�b�h�ōs���邾�������A�O���[�o���ϐ��ւ̎Q�Ƃ�
	�N���X�����ɕ����߂邱�Ƃ��ł���̂ł���Ȃ�ɈӖ��͂���Ǝv���B
*/
class SAKURA_CORE_API CEol
{
	static const char* gm_pszEolDataArr[EOL_TYPE_NUM];
	static const wchar_t* gm_pszEolUnicodeDataArr[EOL_TYPE_NUM];
	static const wchar_t* gm_pszEolUnicodeBEDataArr[EOL_TYPE_NUM];
	static const int gm_pnEolLenArr[EOL_TYPE_NUM];
	static const char* gm_pszEolNameArr[EOL_TYPE_NUM];
public:

	//	�ݒ�֐�
	void Init(void){
		m_enumEOLType = EOL_NONE;
	//	m_nEOLLen = 2; /* = CR+LF */
	}

	static EEolType GetEOLType( const char* pszData, int nDataLen );
	static EEolType GetEOLTypeUni( const wchar_t* pszData, int nDataLen );
	static EEolType GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen );
	bool SetType( EEolType t);	//	Type�̐ݒ�
	void GetTypeFromString( const char* pszData, int nDataLen )
		{	SetType( GetEOLType( pszData, nDataLen ) ); }

	//	�ǂݏo���֐�
	EEolType GetType(void) const { return m_enumEOLType; }	//!<	���݂�Type���擾
	int GetLen(void) const
		{ return gm_pnEolLenArr[ m_enumEOLType ]; }	//!<	���݂�EOL�����擾
	const char* GetName(void) const
		{ return gm_pszEolNameArr[ m_enumEOLType ]; }	//!<	���݂�EOL�̖��̎擾
	const char* GetValue(void) const
		{ return gm_pszEolDataArr[ m_enumEOLType ]; }	//!<	���݂�EOL������擪�ւ̃|�C���^���擾
	const char* GetUnicodeValue() const
		{ return reinterpret_cast<const char*>(gm_pszEolUnicodeDataArr[m_enumEOLType]);}	//!<	Unicode��GetValue	2002/5/9 Frozen
	const char* GetUnicodeBEValue(void) const
		{ return reinterpret_cast<const char*>(gm_pszEolUnicodeBEDataArr[m_enumEOLType]); }	//!<	UnicodeBE�ł�GetValue 2002.05.30 Moca

	//	���֐�����̂��߂�Overload
	bool operator==( EEolType t ) const { return GetType() == t; }
	bool operator!=( EEolType t ) const { return GetType() != t; }
	const CEol& operator=( const CEol& t )
		{ m_enumEOLType = t.m_enumEOLType; return *this; }
	operator EEolType(void) const { return GetType(); }

	//	constructor
	CEol(){ Init(); }
	CEol( EEolType t ){ SetType(t); }

private:
	EEolType		m_enumEOLType;	//!< ���s�R�[�h�̎��
	//int			m_nEOLLen;		/* ���s�R�[�h�̒��� */
};


#endif


/*[EOF]*/
