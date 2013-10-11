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

#include "StdAfx.h"
#include "CEditApp.h"
#include "CSMacroMgr.h"
#include "CEditWnd.h"

void CEditApp::Create(HINSTANCE hInst, HWND hwndParent, int nGroupId)
{
	//�w���p�쐬
	m_cIcons.Create( hInst );	//	CreateImage List

	//�}�N��
	m_pcSMacroMgr = new CSMacroMgr();

	//�E�B���h�E�̍쐬
	m_pcEditWnd = CEditWnd::getInstance();
	m_pcEditWnd->Create( hInst, hwndParent, &m_cIcons, nGroupId );
}

CEditApp::~CEditApp()
{
}


/*[EOF]*/
