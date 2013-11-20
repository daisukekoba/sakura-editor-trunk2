/*
	Copyright (C) 2008, kobake

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
#include "CPropertyManager.h"
#include "CShareData.h"
#include "mymessage.h"

void CPropertyManager::Create( HINSTANCE hInstance, HWND hwndOwner, CImageListMgr* pImageList, CMenuDrawer* pMenuDrawer )
{
	m_hInstance = hInstance;
	m_hwndOwner = hwndOwner;
	m_pImageList = pImageList;
	m_pMenuDrawer = pMenuDrawer;

	m_nPropComPageNum = -1;
	m_nPropTypePageNum = -1;
}

/*! ���ʐݒ� �v���p�e�B�V�[�g */
bool CPropertyManager::OpenPropertySheet( HWND hWnd, int nPageNum )
{
	bool bRet;
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	CPropCommon* pcPropCommon = new CPropCommon();
	pcPropCommon->Create( m_hInstance, m_hwndOwner, m_pImageList, m_pMenuDrawer );

	// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	pcPropCommon->InitData();

	if( nPageNum != -1 ){
		m_nPropComPageNum = nPageNum;
	}
	
	/* �v���p�e�B�V�[�g�̍쐬 */
	if( pcPropCommon->DoPropertySheet( m_nPropComPageNum ) ){

		// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
		// ShareData �� �ݒ��K�p�E�R�s�[����
		// 2007.06.20 ryoji �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		BOOL bGroup = (pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin);

		pcPropCommon->ApplyData();
		// note: ��{�I�ɂ����œK�p���Ȃ��ŁAMYWM_CHANGESETTING���炽�ǂ��ēK�p���Ă��������B
		// ���E�B���h�E�ɂ͍Ō�ɒʒm����܂��B���́AOnChangeSetting �ɂ���܂��B
		// �����ł����K�p���Ȃ��ƁA�ق��̃E�B���h�E���ύX����܂���B
		
		if( bGroup != (pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( pShareData->m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		bRet = true;
	}else{
		bRet = false;
	}

	// �Ō�ɃA�N�Z�X�����V�[�g���o���Ă���
	m_nPropComPageNum = pcPropCommon->GetPageNum();

	delete pcPropCommon;

	return bRet;
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
bool CPropertyManager::OpenPropertySheetTypes( HWND hWnd, int nPageNum, int nSettingType )
{
	bool bRet;
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	CPropTypes* pcPropTypes = new CPropTypes();
	pcPropTypes->Create( m_hInstance, m_hwndOwner );

	STypeConfig& types = pShareData->m_Types[nSettingType];
	pcPropTypes->SetTypeData( types );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��

	if( nPageNum != -1 ){
		m_nPropTypePageNum = nPageNum;
	}

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( pcPropTypes->DoPropertySheet( m_nPropTypePageNum ) ){

		pcPropTypes->GetTypeData( types );

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( pShareData->m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		bRet = true;
	}else{
		bRet = false;
	}

	// �Ō�ɃA�N�Z�X�����V�[�g���o���Ă���
	m_nPropTypePageNum = pcPropTypes->GetPageNum();

	delete pcPropTypes;

	return bRet;
}

