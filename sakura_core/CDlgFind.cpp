/*!	@file
	@brief �����_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/12/12 �č쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor, YAZAKI, genta
	Copyright (C) 2005, zenryaku
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDlgFind.h"
#include "Funccode.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include "CEditView.h"
#include "etc_uty.h"	//Stonee, 2001/03/12
#include "Debug.h"// 2002/2/10 aroka �w�b�_����
#include "sakura_rc.h"
#include "sakura.hh"

//���� CDlgFind.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		//��������
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		//�O������
	IDCANCEL,						HIDCANCEL_FIND,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				//�w���v
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					//�P��P��
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				//�啶��������
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			//���K�\��
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		//������Ȃ��Ƃ��ɒʒm
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	//�����I�ɕ���
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				//����������
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			//���K�\���o�[�W����
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			//2002.01.16 hor �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			//2002.01.26 hor �擪�i�����j����Č���
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgFind::CDlgFind()
{
	m_bLoHiCase = FALSE;	/* �p�啶���Ɖp����������ʂ��� */
	m_bWordOnly = FALSE;	/* ��v����P��̂݌������� */
	m_bRegularExp = FALSE;	/* ���K�\�� */
	m_szText[0] = '\0';		/* ���������� */
	return;
}


/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			// 1==���K�\��
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				// 1==�p�啶���������̋��
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				// 1==�P��̂݌���
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	m_nEscCaretPosX_PHY = ((CEditView*)lParam)->m_nCaretPosX_PHY;	// �����J�n���̃J�[�\���ʒu�ޔ�	02/07/28 ai
	m_nEscCaretPosY_PHY = ((CEditView*)lParam)->m_nCaretPosY_PHY;	// �����J�n���̃J�[�\���ʒu�ޔ�	02/07/28 ai
	((CEditView*)lParam)->m_bSearch = TRUE;							// �����J�n�ʒu�̓o�^�L��		02/07/28 ai
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FIND, lParam, SW_SHOW );
}

/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
void CDlgFind::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFind::SetData( void )
{
//	MYTRACE( "CDlgFind::SetData()" );

	/*****************************
	*           ������           *
	*****************************/
	// Here Jun. 26, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌������ɂ��jre.dll������폜

	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	/* ���������� */
	// ���������񃊃X�g�̐ݒ�(�֐���)	2010/5/28 Uchi
	SetCombosList();

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
//���K�\����ON�ł��A�啶������������ʂ���^���Ȃ���I���ł���悤�ɁB
//		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* �P��P�ʂŒT�� */
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* �����_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFind, m_pShareData->m_Common.m_bAutoCloseDlgFind );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	return;
}


// ���������񃊃X�g�̐ݒ�
//	2010/5/28 Uchi
void CDlgFind::SetCombosList( void )
{
	int		i;
	HWND	hwndCombo;
	TCHAR	szBuff[_MAX_PATH+1];

	/* ���������� */
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	while (::SendMessage(hwndCombo, CB_GETCOUNT, 0L, 0L) > 0) {
		::SendMessage(hwndCombo, CB_DELETESTRING, 0L, 0L);
	}
	for (i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i) {
		::SendMessage( hwndCombo, CB_ADDSTRING, 0L, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}
	::GetWindowText( hwndCombo, szBuff, _MAX_PATH );
	if (_tcscmp( szBuff, m_szText ) != 0) {
		SetDlgItemText(m_hWnd, IDC_COMBO_TEXT, m_szText);
	}
}


/* �_�C�A���O�f�[�^�̎擾 */
int CDlgFind::GetData( void )
{
//	MYTRACE( "CDlgFind::GetData()" );

	/* �p�啶���Ɖp����������ʂ��� */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	// 2001/06/23 Norio Nakatani
	/* �P��P�ʂŌ��� */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* ��v����P��̂݌������� */
	/* ���K�\�� */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );

	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;			// 1==���K�\��
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;				// 1==�p�啶���������̋��
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;				// 1==�P��̂݌���
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	/* ���������� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _countof( m_szText ));

	/* �����_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFind );

	/* �擪�i�����j����Č��� 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* ���K�\���H */
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0x00;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�

		/* ���������� */
		//@@@ 2002.2.2 YAZAKI CShareData�Ɉړ�
		CShareData::getInstance()->AddToSearchKeyArr( (const char*)m_szText );
		if( !m_bModal ){
			/* �_�C�A���O�f�[�^�̐ݒ� */
			//SetData();
			SetCombosList();		//	�R���{�݂̂̏�����	2010/5/28 Uchi
		}
		return 1;
	}else{
		return 0;
	}
}



BOOL CDlgFind::OnBnClicked( int wID )
{
	int			nRet;
	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�����v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO �C���R���ǉ�	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		break;
	case IDC_CHK_REGULAREXP:	/* ���K�\�� */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){

			// From Here Jun. 26, 2001 genta
			//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* �p�啶���Ɖp����������ʂ��� */
				//	Jan. 31, 2002 genta
				//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
				//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 Norio Nakatani
				/* �P��P�ʂŌ��� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	�啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 Norio Nakatani
			/* �P��P�ʂŌ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	/* �㌟�� */	//Feb. 13, 2001 JEPRO �{�^������[IDC_BUTTON1]��[IDC_BUTTON_SERACHPREV]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 1 );
			}else{
				/* �O������ */
				pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

				/* �ĕ`�� 2005.04.06 zenryaku 0�������}�b�`�ŃL�����b�g��\�����邽�� */
				pcEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v	// HandleCommand(F_REDRAW) -> Redraw() ��}�b�`���Ɂu������Ȃ������v�X�e�[�^�X�o�[���b�Z�[�W�������Ȃ�

				// 02/06/26 ai Start
				// �����J�n�ʒu��o�^
				if( TRUE == pcEditView->m_bSearch ){
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX 02/07/28 ai start
					pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
					pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
					pcEditView->m_bSearch = FALSE;
					// 02/07/28 ai end
				}//  02/06/26 ai End

				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME, _T("�����������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		/* ������ */	//Feb. 13, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_SERACHNEXT]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}
			else{
				/* �������� */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

				/* �ĕ`�� 2005.04.06 zenryaku 0�������}�b�`�ŃL�����b�g��\�����邽�� */
				pcEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v	// HandleCommand(F_REDRAW) -> Redraw() ��}�b�`���Ɂu������Ȃ������v�X�e�[�^�X�o�[���b�Z�[�W�������Ȃ�

				// �����J�n�ʒu��o�^
				if( TRUE == pcEditView->m_bSearch ){
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX 02/07/28 ai start
					pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
					pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
					pcEditView->m_bSearch = FALSE;
					// 02/07/28 ai end
				}

				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME, _T("�����������w�肵�Ă��������B") );
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor �Y���s�}�[�N
		if( 0 < GetData() ){
			if( m_bModal ){		/* ���[�_���_�C�A���O�� */
				CloseDialog( 2 );
			}else{
				pcEditView->HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
				/* �����_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				else{
					::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgFind::OnActivate( WPARAM wParam, LPARAM lParam )
{
	// 0�������}�b�`�`���ON/OFF	// 2009.11.29 ryoji
	CEditView*	pcEditView = (CEditView*)m_lParam;
	if( pcEditView->IsTextSelected()
		&& pcEditView->m_nSelectLineFrom == pcEditView->m_nSelectLineTo
		&& pcEditView->m_nSelectColmFrom == pcEditView->m_nSelectColmTo
	){
		::InvalidateRect( pcEditView->m_hWnd, NULL, TRUE );	// �A�N�e�B�u���^��A�N�e�B�u�����������Ă���ĕ`��
	}

	return CDialog::OnActivate( wParam, lParam );
}

//@@@ 2002.01.18 add start
LPVOID CDlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
