//	$Id$
/*!	@file

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D

	@author genta
	@date Oct. 1, 2001
*/

#ifndef __CNAMELOOKUP_H__
#define __CNAMELOOKUP_H__

#include <windows.h>
#include "global.h"
#include "Funccode.h"
#include "CSMacroMgr.h"

/*!
	�@�\�C�@�\���ނƈʒu�C�@�\�ԍ��C������Ȃǂ̑Ή����W�񂷂�D
*/
class SAKURA_CORE_API CFuncLookup {

public:
	CFuncLookup( HINSTANCE hInst, CSMacroMgr* SMacroMgr )
		: m_pcSMacroMgr( SMacroMgr ), m_hInstance( hInst ) {}
	CFuncLookup() : m_pcSMacroMgr( NULL ), m_hInstance( NULL ) {}

	void Init( HINSTANCE hInst, CSMacroMgr* SMacroMgr ){
		m_pcSMacroMgr = SMacroMgr;
		m_hInstance = hInst;
	}

	int Pos2FuncCode( int category, int position );
	bool Pos2FuncName( int category, int position, char *ptr, int bufsize );
	bool Funccode2Name( int funccode, char *ptr, int bufsize );

	void SetCategory2Combo( HWND hComboBox );
	void SetListItem( HWND hListBox, int category );

private:
	HINSTANCE m_hInstance;	//!< �����񃊃\�[�X�����C���X�^���X
	CSMacroMgr* m_pcSMacroMgr;	//!< �}�N���Ǘ��N���X

};

#endif
/* [EOF] */
