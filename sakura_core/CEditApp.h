/*
	Copyright (C) 2007, kobake

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

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_
class CEditWnd;
class CSMacroMgr;

#include <windows.h>
#include "CShareData.h"
#include "CImageListMgr.h" // 2002/2/10 aroka

//!	�풓���̊Ǘ�
/*!
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class CEditApp
{
public:
	static CEditApp* getInstance(){
		static CEditApp instance;

		return &instance;
	}

	void Create(HINSTANCE hInst, HWND hwndParent, int nGroupId);

private:
	// �O�����点�Ȃ��B
	CEditApp(){}
	CEditApp(CEditApp const&);
	void operator=(CEditApp const&);

public:
	~CEditApp();

	//�E�B���h�E
	CEditWnd*			m_pcEditWnd;

	CSMacroMgr*			m_pcSMacroMgr;			//�}�N���Ǘ�
	//GUI�I�u�W�F�N�g
	CImageListMgr		m_cIcons;					//!< Image List
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */


/*[EOF]*/
