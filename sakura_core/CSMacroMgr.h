//	$Id$
/*!	@file
	�L�[�{�[�h�}�N��(���ڎ��s�p)

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CSMacroMgr;

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>
#include <vector>
#include "CMemory.h"
#include "CShareData.h"

using namespace std;

class CEditView;

//#define MAX_STRLEN			70
//#define MAX_SMacroNUM		10000
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	�f�[�^�̌^�錾
	typedef vector<KeyMacroData> KeyMacroList;

	/*! �}�N���P�̏����i�[���� */
	struct Macro1 {
		KeyMacroList m_mac; //!< �}�N���{��
		vector<string> m_strlist; //!< �⏕������
		bool m_flag; //!< �ǂݍ��ݍς݂��ǂ�����\���t���O
	
		Macro1() : m_flag(false){}
		
		void Reset(void){
			m_flag = false;
			m_strlist.clear();
			m_mac.clear();
		}
		
		bool IsReady(void) const { return m_flag; }
	};
	
public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll( void );	/* �L�[�}�N���̃o�b�t�@���N���A���� */

	/*! �L�[�{�[�h�}�N���̎��s */
	BOOL Exec( HINSTANCE hInstance, CEditView* pViewClass, int idx );
	
	//	�o�^�C���^�[�t�F�[�X
	//! �L���E�����̐؂�ւ�
	void Enable(int idx, bool state){
		if( idx < 0 || MAX_CUSTMACRO <= idx )
			m_pShareData->m_MacroTable[idx].Enable( state );
	}
	bool IsEnabled(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	�\�����閼�O�̐ݒ�
	const char* GetTitle(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
			( m_pShareData->m_MacroTable[idx].m_szName[0] == '\0' ?
				m_pShareData->m_MacroTable[idx].m_szFile : 
				m_pShareData->m_MacroTable[idx].m_szName)
			: NULL;
	}
	
	//!	�\�����̐ݒ�
	BOOL SetName(int idx, const char *);
	
	//!	�\�����̎擾
	const char* GetName(int idx) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	//!	�t�@�C�����̐ݒ�
	BOOL SetFile(int idx, const char *);
	
	//!	�t�@�C�����̎擾
	const char* GetFile(int idx, const char *) const {
		return ( idx < 0 || MAX_CUSTMACRO <= idx ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

protected:
	/*! �L�[�{�[�h�}�N���̓ǂݍ��� */
	BOOL Load( CSMacroMgr::Macro1& mbuf, HINSTANCE hInstance, const char* pszPath );
	
	/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	int Append( CSMacroMgr::Macro1& mbuf, int nFuncID, LPARAM lParam1 );

private:
	vector<Macro1> m_vMacro;

	DLLSHAREDATA*	m_pShareData;

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
