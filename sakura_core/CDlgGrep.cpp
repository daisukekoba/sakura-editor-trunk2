/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, bosagami, genta
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CDlgGrep.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "Debug.h"
#include "etc_uty.h"
#include "global.h"
#include "sakura_rc.h"
#include "sakura.hh"

//GREP CDlgGrep.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12000
	IDC_BUTTON_FOLDER,				HIDC_GREP_BUTTON_FOLDER,			//�t�H���_
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_BUTTON_CURRENTFOLDER,		//���t�H���_
	IDOK,							HIDOK_GREP,							//����
	IDCANCEL,						HIDCANCEL_GREP,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				//�w���v
	IDC_CHK_WORD,					HIDC_GREP_CHK_WORD,					//�P��P��
	IDC_CHK_SUBFOLDER,				HIDC_GREP_CHK_SUBFOLDER,			//�T�u�t�H���_������
	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_CHK_FROMTHISTEXT,			//���̃t�@�C������
	IDC_CHK_LOHICASE,				HIDC_GREP_CHK_LOHICASE,				//�啶��������
	IDC_CHK_REGULAREXP,				HIDC_GREP_CHK_REGULAREXP,			//���K�\��
	IDC_COMBO_CHARSET,				HIDC_GREP_COMBO_CHARSET,			//�����R�[�h�Z�b�g
	IDC_COMBO_TEXT,					HIDC_GREP_COMBO_TEXT,				//����
	IDC_COMBO_FILE,					HIDC_GREP_COMBO_FILE,				//�t�@�C��
	IDC_COMBO_FOLDER,				HIDC_GREP_COMBO_FOLDER,				//�t�H���_
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_RADIO_OUTPUTLINE,			//���ʏo�́F�s�P��
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_RADIO_OUTPUTMARKED,		//���ʏo�́F�Y������
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_RADIO_OUTPUTSTYLE1,		//���ʏo�͌`���F�m�[�}��
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_RADIO_OUTPUTSTYLE2,		//���ʏo�͌`���F�t�@�C����
	IDC_STATIC_JRE32VER,			HIDC_GREP_STATIC_JRE32VER,			//���K�\���o�[�W����
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_CHK_DEFAULTFOLDER,		//�t�H���_�̏����l���J�����g�t�H���_�ɂ���
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgGrep::CDlgGrep()
{
	m_bSubFolder = FALSE;				// �T�u�t�H���_�������������
	m_bFromThisText = FALSE;			// ���̕ҏW���̃e�L�X�g���猟������
	m_bLoHiCase = FALSE;				// �p�啶���Ɖp����������ʂ���
	m_bRegularExp = FALSE;				// ���K�\��
	m_nGrepCharSet = CODE_SJIS;			// �����R�[�h�Z�b�g
	m_bGrepOutputLine = TRUE;			// �s���o�͂��邩�Y�����������o�͂��邩
	m_nGrepOutputStyle = 1;				// Grep: �o�͌`��

	_tcscpy( m_szText, m_pShareData->m_szSEARCHKEYArr[0] );		/* ���������� */
	_tcscpy( m_szFile, m_pShareData->m_szGREPFILEArr[0] );		/* �����t�@�C�� */
	_tcscpy( m_szFolder, m_pShareData->m_szGREPFOLDERArr[0] );	/* �����t�H���_ */
	m_szCurrentFilePath[0] = _T('\0');
	return;
}



/* ���[�_���_�C�A���O�̕\�� */
int CDlgGrep::DoModal( HINSTANCE hInstance, HWND hwndParent, const char* pszCurrentFilePath )
{
	m_bSubFolder = m_pShareData->m_Common.m_bGrepSubFolder;							// Grep: �T�u�t�H���_������
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;							// 1==���K�\��
	m_nGrepCharSet = (ECodeType)m_pShareData->m_Common.m_nGrepCharSet;				// �����R�[�h�Z�b�g
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;								// 1==�啶���������̋��
	m_bGrepOutputLine = m_pShareData->m_Common.m_bGrepOutputLine;					// �s���o�͂��邩�Y�����������o�͂��邩
	m_nGrepOutputStyle = m_pShareData->m_Common.m_nGrepOutputStyle;					// Grep: �o�͌`��

	//2001/06/23 N.Nakatani add
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;					/* �P��P�ʂŌ��� */

	if( pszCurrentFilePath ){	// 2010.01.10 ryoji
		_tcscpy(m_szCurrentFilePath, pszCurrentFilePath);
	}

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_GREP, NULL );
}

//	2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL CDlgGrep::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;

	/* ���[�U�[���R���{�{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

	/* �_�C�A���O�̃A�C�R�� */
//2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
	::SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

	// 2002/09/22 Moca Add
	int i;
	/* �����R�[�h�Z�b�g�I���R���{�{�b�N�X������ */
	for( i = 0; i < gm_nCodeComboNameArrNum; ++i ){
		int idx = ::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET ), CB_ADDSTRING,   0, (LPARAM)gm_pszCodeComboNameArr[i] );
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET ), CB_SETITEMDATA, idx, gm_nCodeComboValueArr[i] );
	}
	//	2007.02.09 bosagami
	HWND hFolder = ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER );
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);


	/* ���N���X�����o */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

/*! @brief �t�H���_�w��EditBox�̃R�[���o�b�N�֐�

	@date 2007.02.09 bosagami �V�K�쐬
	@date 2007.09.02 genta �f�B���N�g���`�F�b�N������
*/
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	if(msg == WM_DROPFILES) 
	do {
		//	From Here 2007.09.02 genta 
		TCHAR sPath[MAX_PATH + 1];
		if( DragQueryFile((HDROP)wparam, 0, NULL, 0 ) > sizeof(sPath) - 1 ){
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, sPath, sizeof(sPath) - 1);

		//�t�@�C���p�X�̉���
		ResolvePath(sPath);

		//	�t�@�C�����h���b�v���ꂽ�ꍇ�̓t�H���_��؂�o��
		//	�t�H���_�̏ꍇ�͍Ōオ������̂�split���Ă͂����Ȃ��D
		if( IsFileExists( sPath, true )){	//	��2������true���ƃf�B���N�g���͑ΏۊO
			TCHAR szWork[MAX_PATH + 1];
			SplitPath_FolderAndFile( sPath, szWork, NULL );
			_tcscpy( sPath, szWork );
		}

		SetWindowText(hwnd, sPath);
	}
	while(0);

	return  CallWindowProc((WNDPROC)g_pOnFolderProc,hwnd,msg,wparam,lparam);
}

BOOL CDlgGrep::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �uGrep�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_CHK_FROMTHISTEXT:	/* ���̕ҏW���̃e�L�X�g���猟������ */
		// 2010.05.30 �֐���
		SetDataFromThisText( 0 != ::IsDlgButtonChecked( m_hWnd, IDC_CHK_FROMTHISTEXT ) );
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	/* ���ݕҏW���̃t�@�C���̃t�H���_ */
		/* �t�@�C�����J���Ă��邩 */
		if( 0 < _tcslen( m_szCurrentFilePath ) ){
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
		}
		else{
			/* ���݂̃v���Z�X�̃J�����g�f�B���N�g�����擾���܂� */
			TCHAR	szWorkFolder[MAX_PATH];
			::GetCurrentDirectory( _countof( szWorkFolder ) - 1, szWorkFolder );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
		}
		return TRUE;


//	case IDC_CHK_LOHICASE:	/* �p�啶���Ɖp����������ʂ��� */
//		MYTRACE_A( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
	case IDC_CHK_REGULAREXP:	/* ���K�\�� */
//		MYTRACE_A( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
				//	To Here Jun. 26, 2001 genta
				/* �p�啶���Ɖp����������ʂ��� */
				//	���K�\���̂Ƃ����I���ł���悤�ɁB
//				::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				//2001/06/23 N.Nakatani
				/* �P��P�ʂŌ��� */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* �p�啶���Ɖp����������ʂ��� */
			//	���K�\���̂Ƃ����I���ł���悤�ɁB
//			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
//			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );


//2001/06/23 N.Nakatani
//�P��P�ʂ�grep���������ꂽ��R�����g���O���Ǝv���܂�
//2002/03/07�������Ă݂��B
			/* �P��P�ʂŌ��� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );

		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		/* �t�H���_�Q�ƃ{�^�� */
		{
			TCHAR	szFolder[MAX_PATH];
			/* �����t�H���_ */
			::GetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1 );
			if( 0 == _tcslen( szFolder ) ){
				::GetCurrentDirectory( _countof( szFolder ), szFolder );
			}
			if( SelectDir( m_hWnd, _T("��������t�H���_��I��ł�������"), szFolder, szFolder ) ){
				::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szFolder );
			}
		}

		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		/* �t�H���_�̏����l���J�����g�t�H���_�ɂ��� */
		{
			m_pShareData->m_Common.m_bGrepDefaultFolder = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_DEFAULTFOLDER );
		}
		return TRUE;
	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		if( GetData() ){
//			::EndDialog( hwndDlg, TRUE );
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog( hwndDlg, FALSE );
		CloseDialog( FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgGrep::SetData( void )
{
	int		i;

	/* ���������� */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	HWND	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}

	/* �����t�@�C�� */
	::SetDlgItemText( m_hWnd, IDC_COMBO_FILE, m_szFile );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_FILE );
	for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szGREPFILEArr[i] );
	}

	/* �����t�H���_ */
	::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, m_szFolder );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER );
	for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szGREPFOLDERArr[i] );
	}

	if((0 == _tcslen( m_pShareData->m_szGREPFOLDERArr[0] ) || m_pShareData->m_Common.m_bGrepDefaultFolder ) &&
		0 < _tcslen( m_szCurrentFilePath )
	){
		TCHAR	szWorkFolder[MAX_PATH];
		TCHAR	szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
		::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
	}

	/* �T�u�t�H���_������������� */
	::CheckDlgButton( m_hWnd, IDC_CHK_SUBFOLDER, m_bSubFolder );

	// ���̕ҏW���̃e�L�X�g���猟������
	::CheckDlgButton( m_hWnd, IDC_CHK_FROMTHISTEXT, m_bFromThisText );
	// 2010.05.30 �֐���
	SetDataFromThisText( m_bFromThisText != FALSE );

	/* �p�啶���Ɖp����������ʂ��� */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 N.Nakatani �����_�ł�Grep�ł͒P��P�ʂ̌����̓T�|�[�g�ł��Ă��܂���
	// 2002/03/07 �e�X�g�T�|�[�g
	/* ��v����P��̂݌������� */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );
//	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ) , false );	//�`�F�b�N�{�b�N�X���g�p�s�ɂ���


	/* �����R�[�h�������� */
//	::CheckDlgButton( m_hWnd, IDC_CHK_KANJICODEAUTODETECT, m_bKanjiCode_AutoDetect );

	// 2002/09/22 Moca Add
	/* �����R�[�h�Z�b�g */
	{
		int		nIdx, nCurIdx, nCharSet;
		HWND	hWndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET );
		nCurIdx = ::SendMessage( hWndCombo , CB_GETCURSEL, 0, 0 );
		for( nIdx = 0; nIdx < gm_nCodeComboNameArrNum; nIdx++ ){
			nCharSet = ::SendMessage( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
			if( nCharSet == m_nGrepCharSet ){
				nCurIdx = nIdx;
			}
		}
		::SendMessage( hWndCombo, CB_SETCURSEL, (WPARAM)nCurIdx, 0 );
	}

	/* �s���o�͂��邩�Y�����������o�͂��邩 */
	if( m_bGrepOutputLine ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTLINE, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTMARKED, TRUE );
	}

	/* Grep: �o�͌`�� */
	if( 1 == m_nGrepOutputStyle ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}else
	if( 2 == m_nGrepOutputStyle ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE2, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* �p�啶���Ɖp����������ʂ��� */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
		//	���K�\���̂Ƃ����I���ł���悤�ɁB
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

	if( 0 < _tcslen( m_szCurrentFilePath ) ){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_FROMTHISTEXT ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_FROMTHISTEXT ), FALSE );
	}

	// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	::CheckDlgButton( m_hWnd, IDC_CHK_DEFAULTFOLDER, m_pShareData->m_Common.m_bGrepDefaultFolder );
	if( m_pShareData->m_Common.m_bGrepDefaultFolder ) OnBnClicked( IDC_BUTTON_CURRENTFOLDER );

	return;
}


/*!
	���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ�
*/
void CDlgGrep::SetDataFromThisText( bool bChecked )
{
	BOOL bEnableControls = TRUE;
	if( 0 != m_szCurrentFilePath[0] && bChecked ){
		TCHAR	szWorkFolder[MAX_PATH];
		TCHAR	szWorkFile[MAX_PATH];
		// 2003.08.01 Moca �t�@�C�����̓X�y�[�X�Ȃǂ͋�؂�L���ɂȂ�̂ŁA""�ň͂��A�G�X�P�[�v����
		szWorkFile[0] = _T('"');
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile + 1 );
		_tcscat( szWorkFile, _T("\"") ); // 2003.08.01 Moca
		::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
		::SetDlgItemText( m_hWnd, IDC_COMBO_FILE, szWorkFile );

		::CheckDlgButton( m_hWnd, IDC_CHK_SUBFOLDER, BST_UNCHECKED );
		bEnableControls = FALSE;
	}
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), bEnableControls );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), bEnableControls );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_FOLDER ), bEnableControls );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_SUBFOLDER ), bEnableControls );
	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[  */
int CDlgGrep::GetData( void )
{

	/* �T�u�t�H���_�������������*/
	m_bSubFolder = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_SUBFOLDER );

	m_pShareData->m_Common.m_bGrepSubFolder = m_bSubFolder;		/* Grep�F�T�u�t�H���_������ */

	/* ���̕ҏW���̃e�L�X�g���猟������ */
	m_bFromThisText = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_FROMTHISTEXT );
	/* �p�啶���Ɖp����������ʂ��� */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	//2001/06/23 N.Nakatani
	/* �P��P�ʂŌ��� */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* ���K�\�� */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );

	/* �����R�[�h�������� */
//	m_bKanjiCode_AutoDetect = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_KANJICODEAUTODETECT );

	/* �����R�[�h�Z�b�g */
	{
		int		nIdx;
		HWND	hWndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET );
		nIdx = ::SendMessage( hWndCombo, CB_GETCURSEL, 0, 0 );
		m_nGrepCharSet = (ECodeType)::SendMessage( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
	}


	/* �s���o�͂��邩�Y�����������o�͂��邩 */
	m_bGrepOutputLine = ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTLINE );

	/* Grep: �o�͌`�� */
	if( TRUE == ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTSTYLE1 ) ){
		m_nGrepOutputStyle = 1;				/* Grep: �o�͌`�� */
	}
	if( TRUE == ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTSTYLE2 ) ){
		m_nGrepOutputStyle = 2;				/* Grep: �o�͌`�� */
	}



	/* ���������� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );
	/* �����t�@�C�� */
	::GetDlgItemText( m_hWnd, IDC_COMBO_FILE, m_szFile, _MAX_PATH - 1 );
	/* �����t�H���_ */
	::GetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, m_szFolder, _MAX_PATH - 1 );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;							// 1==���K�\��
	m_pShareData->m_Common.m_nGrepCharSet = m_nGrepCharSet;							// �����R�[�h��������
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;								// 1==�p�啶���������̋��
	m_pShareData->m_Common.m_bGrepOutputLine = m_bGrepOutputLine;					// �s���o�͂��邩�Y�����������o�͂��邩
	m_pShareData->m_Common.m_nGrepOutputStyle = m_nGrepOutputStyle;					// Grep: �o�͌`��
	//2001/06/23 N.Nakatani add
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;		/* 1==�P��̂݌��� */


//��߂܂���
//	if( 0 == _tcslen( m_szText ) ){
//		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("�����̃L�[���[�h���w�肵�Ă��������B") );
//		return FALSE;
//	}
	/* ���̕ҏW���̃e�L�X�g���猟������ */
	if( 0 == _tcslen( m_szFile ) ){
		//	Jun. 16, 2003 Moca
		//	�����p�^�[�����w�肳��Ă��Ȃ��ꍇ�̃��b�Z�[�W�\������߁A
		//	�u*.*�v���w�肳�ꂽ���̂ƌ��Ȃ��D
		_tcscpy( m_szFile, _T("*.*") );
	}
	if( 0 == _tcslen( m_szFolder ) ){
		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("�����Ώۃt�H���_���w�肵�Ă��������B") );
		return FALSE;
	}

	char szCurDirOld[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, szCurDirOld );
	// ���΃p�X����΃p�X
	if( 0 == ::SetCurrentDirectory( m_szFolder ) ){
		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"�����Ώۃt�H���_������������܂���B"
		);
		::SetCurrentDirectory( szCurDirOld );
		return FALSE;
	}
	::GetCurrentDirectory( MAX_PATH, m_szFolder );
	::SetCurrentDirectory( szCurDirOld );

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
	/* ���������� */
	if( 0 < _tcslen( m_szText ) ){
		// From Here Jun. 26, 2001 genta
		//	���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp  && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag) ){
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );
	}

	// ���̕ҏW���̃e�L�X�g���猟������ꍇ�A�����Ɏc���Ȃ�	Uchi 2008/5/23
	if (!m_bFromThisText) {
		/* �����t�@�C�� */
		CShareData::getInstance()->AddToGrepFileArr( m_szFile );

		/* �����t�H���_ */
		CShareData::getInstance()->AddToGrepFolderArr( m_szFolder );
	}

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgGrep::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
