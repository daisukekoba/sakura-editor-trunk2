/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�J�X�^�����j���[�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropCommon.h"
#include "CDlgInput1.h"
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "Debug.h" // 2002/2/10 aroka
#include "sakura.hh"

//	From Here Sept. 5, 2000 JEPRO ���p�J�^�J�i�̑S�p���ɔ�����������ύX(27��46)
#define STR_CUSTMENU_HEAD_LEN 	46
#define STR_CUSTMENU_HEAD		"�e�L�X�g�G�f�B�^ �J�X�^�����j���[�ݒ�t�@�C��\x1a"
//	To Here Sept. 5, 2000


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE,				//���j���[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT,				//���j���[�֋@�\�}��
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD,				//���j���[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_UP,					//���j���[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN,				//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,				//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,				//�G�N�X�|�[�g
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,			//�@�\�̎��
	IDC_COMBO_MENU,					HIDC_COMBO_MENU,				//���j���[�̎��
	IDC_EDIT_MENUNAME,				HIDC_EDIT_MENUNAME,				//���j���[��		// 2009.02.20 ryoji
	IDC_BUTTON_MENUNAME,			HIDC_BUTTON_MENUNAME,			//���j���[���ݒ�	// 2009.02.20 ryoji
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,					//�@�\�ꗗ
	IDC_LIST_RES,					HIDC_LIST_RES,					//���j���[�ꗗ
//	IDC_LABEL_MENUFUNCKIND,			-1,
//	IDC_LABEL_MENUCHOICE,			-1,
//	IDC_LABEL_MENUFUNC,				-1,
//	IDC_LABEL_MENU,					-1,
//	IDC_LABEL_MENUKEYCHANGE,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_CUSTMENU(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p8, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* Custom menu ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_p8(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	static HWND	hwndCOMBO_FUNCKIND;
	static HWND	hwndLIST_FUNC;
	static HWND	hwndCOMBO_MENU;
	static HWND	hwndLIST_RES;

	int			i;

	int			nIdx1;
	int			nIdx2;
	int			nNum2;
	int			nIdx3;
	int			nIdx4;
	char		szLabel[300];
	char		szLabel2[300];
	CDlgInput1	cDlgInput1;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Custom menu */
		SetData_p8( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
		hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

		/* �L�[�I�����̏��� */
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );

		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_CUSTMENU );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE_A( "Custom menu PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 Custom menu */
			GetData_p8( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_CUSTMENU;

			// �\�����X�V����i�}�N���ݒ��ʂł̃}�N�����ύX�𔽉f�j	// 2007.11.02 ryoji
			nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
			nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
			nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
			nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
			if( nIdx1 != CB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_MENU );
				if( nIdx2 != LB_ERR ){
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );
				}
			}
			if( nIdx3 != CB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );
				if( nIdx4 != LB_ERR ){
					::SendMessage( hwndLIST_FUNC, LB_SETCURSEL, nIdx4, 0 );
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
				/* �J�X�^�����j���[�ݒ���C���|�[�g���� */
				p8_Import_CustMenuSetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
				/* �J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
				p8_Export_CustMenuSetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_MENUNAME:
				//	���j���[������̐ݒ�
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}
				::GetDlgItemText( hwndDlg, IDC_EDIT_MENUNAME,
					m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1], MAX_CUSTOM_MENU_NAME_LEN );
				//	Combo Box���ύX �폜���ēo�^
				::SendMessage( hwndCOMBO_MENU, CB_DELETESTRING, nIdx1, 0 );
				::SendMessage( hwndCOMBO_MENU, CB_INSERTSTRING, nIdx1,
					(LPARAM)m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				// �폜����ƑI�������������̂ŁC���ɖ߂�
				::SendMessage( hwndCOMBO_MENU, CB_SETCURSEL, nIdx1, 0 );
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}

		if( hwndCOMBO_MENU == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}
				/* ���j���[���ڈꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
				::SendMessage( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] ){
						strcpy( szLabel2, " ��������������������������" );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
					}else{
						//	Oct. 3, 2001 genta
						m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i], szLabel, 256 );
						/* �L�[ */
						if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] ){
							strcpy( szLabel2, szLabel );
						}else{
							wsprintf( szLabel2, "%s(%c)", szLabel, m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] );
						}
					}
					::SendMessage( hwndLIST_RES, LB_ADDSTRING, 0, (LPARAM)szLabel2 );
				}
				//	Oct. 15, 2001 genta ���j���[����ݒ�
				::SetDlgItemText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				
				break;	/* CBN_SELCHANGE */
			}
		}else
		if( hwndLIST_RES == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_DBLCLK:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}
				nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
					break;
				}

				if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] ){
					break;
				}

//			idListBox = (int) LOWORD(wParam);	// identifier of list box
//			hwndListBox = (HWND) lParam;		// handle of list box
				TCHAR		szKey[2];
				wsprintf( szKey, _T("%c"), m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				{
					BOOL bDlgInputResult = cDlgInput1.DoModal(
						m_hInstance,
						hwndDlg,
						_T("���j���[�A�C�e���̃A�N�Z�X�L�[�ݒ�"),
						_T("�L�[����͂��Ă��������B"),
						1,
						szKey
					);
					if( !bDlgInputResult ){
						return TRUE;
					}
				}
				//	Oct. 3, 2001 genta
				m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );
				//::LoadString( m_hInstance, m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = szKey[0];
//@@@ 2002.01.08 YAZAKI �J�X�^�����j���[�ŃA�N�Z�X�L�[�����������A���J�b�R ( �����j���[���ڂɈ��c��o�O�C��
				if (m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]){
					wsprintf( szLabel2, "%s(%c)", szLabel, m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				}
				else {
					wsprintf( szLabel2, "%s", szLabel );
				}

				::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel2 );
				::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2 + 1, 0 );

				break;
			case LBN_SELCHANGE:
				nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}

				if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
					break;
				}

				nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
					break;
				}

				/* �L�[ */
				if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ||
					' '  == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ){
				}else{
				}
				break;	/* LBN_SELCHANGE */
			}
		}
		else if( hwndCOMBO_FUNCKIND == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );

				/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
//	Oct. 14, 2000 jepro note: ������for�u���b�N�Ŏ��ۂɃ��X�g�������Ă���悤�ł���
				// Oct. 3, 2001 genta
				// ��p���[�`���ɒu������
				m_cLookup.SetListItem( hwndLIST_FUNC, nIdx3 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)" ��������������������������" );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
					if( nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );


					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i--){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = F_0;
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText( hwndEDIT_KEY, "" );
					break;
				case IDC_BUTTON_DELETE:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					if( nNum2 == LB_ERR ){
						break;
					}

					for( i = nIdx2; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i + 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]--;

					if( nNum2 > 0 ){
						if( nNum2 <= nIdx2 ){
							nIdx2 = nNum2 - 1;
						}
						nIdx2 = ::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					}else{
					}
					break;


				case IDC_BUTTON_INSERT:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					::SendMessage( hwndLIST_FUNC, LB_GETTEXT, nIdx4, (LPARAM)szLabel );

					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i-- ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					break;


				case IDC_BUTTON_ADD:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					//	Oct. 3, 2001 genta
					if( m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0 )
						break;

					::SendMessage( hwndLIST_FUNC, LB_GETTEXT, nIdx4, (LPARAM)szLabel );

					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nNum2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_ADDSTRING, nIdx2, (LPARAM)szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					break;

				case IDC_BUTTON_UP:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					if( 0 == nIdx2 ){
						break;
					}

					{
						int nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
						char key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}

					::SendMessage( hwndLIST_RES, LB_GETTEXT, nIdx2, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 - 1, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2 - 1, 0 );
					break;

				case IDC_BUTTON_DOWN:
					nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						break;
					}
					if( nNum2 - 1 <= nIdx2 ){
						break;
					}

					{
						int nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
						char key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}
					::SendMessage( hwndLIST_RES, LB_GETTEXT, nIdx2, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 + 1, (LPARAM)szLabel );
					::SendMessage( hwndLIST_RES, LB_SETCURSEL, nIdx2 + 1, 0 );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdx1 = ::SendMessage( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
		nIdx2 = ::SendMessage( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
		nIdx3 = ::SendMessage( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
		nIdx4 = ::SendMessage( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
		i = ::SendMessage( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
		if( LB_ERR == nIdx2	){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			if( nIdx2 <= 0 ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
			}
			if( nIdx2 + 1 >= i ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
			}
		}
		if( LB_ERR == nIdx2 || LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
		}
		if( LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERTSEPARATOR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		if( CB_ERR != nIdx3 && LB_ERR != nIdx4 &&
		 	m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0
		){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		break;
	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}




/* �_�C�A���O�f�[�^�̐ݒ� Custom menu */
void CPropCommon::SetData_p8( HWND hwndDlg )
{
	HWND		hwndCOMBO_MENU;
	HWND		hwndCombo;
	HWND		hwndLIST_RES;
	int			i;
	int			nIdx;
	char		szLabel[300];
	char		szLabel2[300];

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 3, 2001 genta

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* ���j���[�ꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		::SendMessage( hwndCOMBO_MENU, CB_ADDSTRING, 0, (LPARAM)m_Common.m_sCustomMenu.m_szCustMenuNameArr[i] );
	}
	/* ���j���[�ꗗ�̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessage( hwndCOMBO_MENU, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

	/* ���j���[���ڈꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//	hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );
	::SendMessage( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
	nIdx = 0;
	for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx]; ++i ){
		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i] ){
			strcpy( szLabel, " ��������������������������" );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
		}else{
			//	Oct. 3, 2001 genta
			m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
			//::LoadString( m_hInstance, m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
		}
		/* �L�[ */
		if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i] ){
			strcpy( szLabel2, szLabel );
		}else{
			wsprintf( szLabel2, "%s(%c)",
				szLabel,
				m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i]
			);
		}
		::SendMessage( hwndLIST_RES, LB_ADDSTRING, 0, (LPARAM)szLabel2 );
	}
	
	//	Oct. 15, 2001 genta ���j���[����ݒ�
	::SetDlgItemText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[0] );
	
//	/* �J�X�^�����j���[�̐擪�̍��ڂ�I���i���X�g�{�b�N�X�j*/	//Oct. 8, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
	::SendMessage( hwndLIST_RES, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
	return;
}



/* �_�C�A���O�f�[�^�̎擾 Custom menu */
int CPropCommon::GetData_p8( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_CUSTMENU;

	return TRUE;
}





/* �J�X�^�����j���[�ݒ���C���|�[�g���� */
void CPropCommon::p8_Import_CustMenuSetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;

	char			pHeader[STR_CUSTMENU_HEAD_LEN + 1];
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.mnu"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("�t�@�C�����J���܂���ł����B\n\n%s"), szPath
		);
		return;
	}
	if( STR_CUSTMENU_HEAD_LEN						!= _lread( hFile, (LPVOID)pHeader, STR_CUSTMENU_HEAD_LEN ) ||
		sizeof( m_Common.m_sCustomMenu.m_szCustMenuNameArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_sCustomMenu.m_szCustMenuNameArr   , sizeof( m_Common.m_sCustomMenu.m_szCustMenuNameArr    ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemNumArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_sCustomMenu.m_nCustMenuItemNumArr , sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemNumArr  ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr, sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr	)	!= _lread( hFile, (LPVOID)&m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr , sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr  ) ) ||
		0 != memcmp( pHeader, STR_CUSTMENU_HEAD, STR_CUSTMENU_HEAD_LEN )
	){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("�J�X�^�����j���[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%s"), szPath
		);
		return;
	}
	_lclose( hFile );

	// ��ʍX�V
	HWND	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCtrl );
}

/* �J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
void CPropCommon::p8_Export_CustMenuSetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.mnu"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	hFile = _lcreat( szPath, 0 );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("�t�@�C�����J���܂���ł����B\n\n%s"), szPath
		);
		return;
	}

	if( STR_CUSTMENU_HEAD_LEN						!= _lwrite( hFile, (LPCSTR)STR_CUSTMENU_HEAD, STR_CUSTMENU_HEAD_LEN ) ||
		sizeof( m_Common.m_sCustomMenu.m_szCustMenuNameArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_sCustomMenu.m_szCustMenuNameArr   , sizeof( m_Common.m_sCustomMenu.m_szCustMenuNameArr    ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemNumArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_sCustomMenu.m_nCustMenuItemNumArr , sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemNumArr  ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr, sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr ) ) ||
		sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr	)	!= _lwrite( hFile, (LPCSTR)&m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr , sizeof( m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr  ) )
	){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s"), szPath
		);
		return;
	}
	_lclose( hFile );
}


/*[EOF]*/
