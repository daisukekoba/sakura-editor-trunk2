/*!	@file
	@brief �^�C�v�ʐݒ� - �x��

	@author Norio Nakatani
	@date 1998/12/24  �V�K�쐬
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, ������, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "Debug.h"
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "CDlgKeywordSelect.h"
#include "etc_uty.h"
#include "shell.h"
#include "file.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "Funccode.h"	//Stonee, 2001/05/18
#include "CDlgSameColor.h"	// 2006.04.26 ryoji
#include "CEditApp.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//�P��t�@�C����
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//���͕⊮ �P��t�@�C���Q��
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//���͕⊮�̉p�啶��������
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//���݂̃t�@�C��������͕⊮

	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//�O���w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//�O���w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//�O��HTML�w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//�O��HTML�w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//�r���[�A�𕡐��N�����Ȃ�	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//�f�t�H���g�����R�[�h
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//�f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//�f�t�H���gBOM	// 2011.01.24 ryoji
//	IDC_STATIC,						-1,
	0, 0
};

static const TCHAR* aszCodeStr[] = {
	_T("SJIS"),
	_T("EUC"),
	_T("UTF-8"),
	_T("UTF-16"),
	_T("UTF-16BE")
};
static const ECodeType aeCodeType[] = {
	CODE_SJIS,
	CODE_EUC,
	CODE_UTF8,
	CODE_UNICODE,
	CODE_UNICODEBE
};
static const BOOL abBomEnable[] = {
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE
};
static const TCHAR* aszEolStr[] = {
	_T("CR+LF"),
	_T("LF (UNIX)"),
	_T("CR (Mac)"),
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

/* ���b�Z�[�W���� */
INT_PTR CPropTypesSupport::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p2 */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���͕⊮ �P��t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
					i = ::SendMessage((HWND) lParam, CB_GETCURSEL, 0L, 0L);
					if( CB_ERR != i ){
						int nCheck = BST_UNCHECKED;
						if( abBomEnable[i] ){
							if( (aeCodeType[i] == CODE_UNICODE || aeCodeType[i] == CODE_UNICODEBE) )
								nCheck = BST_CHECKED;
						}
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, nCheck );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );
					}
					break;
				}
			}
			break;

		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* ���͕⊮ �P��t�@�C���́u�Q��...�v�{�^�� */
				{
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_HOKANFILE), _T("*.kwd"), true, true);
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
				{
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTHELP), _T("*.hlp;*.chm;*.col"), true, false);
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
				{
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP), _T("*.chm;*.col"), true, false);
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:	//Jul. 03, 2001 JEPRO �x���^�u�̃w���v��L����
			OnHelp( hwndDlg, IDD_PROP_SUPPORT );
			return TRUE;
		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PROPTYPE_PAGENUM_SUPPORT;
			return TRUE;
		}
		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropTypesSupport::SetData( HWND hwndDlg )
{
	/* ���͕⊮ �P��t�@�C�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

//	2001/06/19 asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase );

	// 2003.06.25 Moca �t�@�C������̕⊮�@�\
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED );

	//@@@ 2002.2.2 YAZAKI
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED );

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;

		// �f�t�H���g�R�[�h�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		for( i = 0; i < _countof(aszCodeStr); ++i ){
			::SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)aszCodeStr[i] );
		}
		for( i = 0; i < _countof(aeCodeType); ++i ){
			if( m_Types.m_encoding.m_eDefaultCodetype == aeCodeType[i] ){
				break;
			}
		}
		if( i == _countof(aeCodeType) ){
			i = 0;
		}
		::SendMessage(hCombo, CB_SETCURSEL, (WPARAM)i, 0L);

		// BOM �`�F�b�N�{�b�N�X�ݒ�
		if( !abBomEnable[i] )
			m_Types.m_encoding.m_bDefaultBom = false;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (m_Types.m_encoding.m_bDefaultBom ? BST_CHECKED : BST_UNCHECKED) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );

		// �f�t�H���g���s�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			::SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_encoding.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		::SendMessage(hCombo, CB_SETCURSEL, (WPARAM)i, 0L);
	}
}

/* �_�C�A���O�f�[�^�̎擾 */
int CPropTypesSupport::GetData( HWND hwndDlg )
{
//	2001/06/19	asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE ) != 0;

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE ) != 0;

	/* ���͕⊮ �P��t�@�C�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, sizeof( m_Types.m_szHokanFile ));

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;

		// m_Types.eDefaultCodetype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = ::SendMessage(hCombo, CB_GETCURSEL, 0L, 0L);
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultCodetype = aeCodeType[i];
		}

		// m_Types.m_bDefaultBom ��ݒ�
		m_Types.m_encoding.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM ) != 0;

		// m_Types.eDefaultEoltype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = ::SendMessage(hCombo, CB_GETCURSEL, 0L, 0L);
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultEoltype = aeEolType[i];
		}
	}

	//@@@ 2002.2.2 YAZAKI
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, _countof( m_Types.m_szExtHelp ));
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, _countof( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE ) != 0;

	return TRUE;
}

// 2001/06/13 End

/*[EOF]*/
