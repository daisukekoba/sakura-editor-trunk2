/*!	@file
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <windows.h>
#include <commctrl.h>
#include "CDlgTypeList.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "etc_uty.h"
#include "Debug.h"
#include "Funccode.h"	//Stonee, 2001/03/12

//�����g�p�萔
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// �^�C�v�ʐݒ�ꗗ CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	//�ꎞ�K�p
	IDOK,					HIDOK_TL,					//�ݒ�
	IDCANCEL,				HIDCANCEL_TL,				//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		//�w���v
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			//���X�g
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, SResult* psResult )
{
	int	nRet;
	m_nSettingType = psResult->cDocumentType;
	m_bEnableTempChange = psResult->bTempChange;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, (LPARAM)NULL );
	if( -1 == nRet ){
		return FALSE;
	}
	else{
		//����
		psResult->cDocumentType = (nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
}


BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		//	Nov. 29, 2000	genta
		//	����ύX: �w��^�C�v�̐ݒ�_�C�A���O���ꎞ�I�ɕʂ̐ݒ��K�p
		::EndDialog(
			m_hWnd,
			::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�^�C�v�ʐݒ�ꗗ�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//	Nov. 29, 2000	From Here	genta
	//	�K�p����^�̈ꎞ�I�ύX
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			m_hWnd,
			::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( m_hWnd, ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, -1 );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );

}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	TCHAR	szText[64 + MAX_TYPES_EXTS + 10];
	int		nExtent = 0;
	HWND	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TYPES );
	HDC		hDC = ::GetDC( hwndList );
	HFONT	hFont = (HFONT)::SendMessage(hwndList, WM_GETFONT, 0, 0);
	HFONT	hFontOld = (HFONT)::SelectObject(hDC, hFont);

	::SendMessage(hwndList, LB_RESETCONTENT, 0L, 0L);	/* ���X�g����ɂ��� */
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		if( m_pShareData->m_Types[nIdx].m_szTypeExts[0] != '\0' ){		/* �^�C�v�����F�g���q���X�g */
			wsprintf( szText, "%s ( %s )",
				m_pShareData->m_Types[nIdx].m_szTypeName,	/* �^�C�v�����F���� */
				m_pShareData->m_Types[nIdx].m_szTypeExts	/* �^�C�v�����F�g���q���X�g */
			);
		}else{
			wsprintf( szText, "%s",
				m_pShareData->m_Types[nIdx].m_szTypeName	/* �^�C�v�����F�g�� */
			);
		}
		::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szText );

		SIZE sizeExtent;
		if( ::GetTextExtentPoint32( hDC, szText, _tcslen(szText), &sizeExtent) && sizeExtent.cx > nExtent ){
			nExtent = sizeExtent.cx;
		}
	}

	::SelectObject(hDC, hFontOld);
	::ReleaseDC( hwndList, hDC );
	::SendMessage( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)(nExtent + 8), 0L );
	::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)m_nSettingType, (LPARAM)0 );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_TEMPCHANGE ), m_bEnableTempChange );
	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
