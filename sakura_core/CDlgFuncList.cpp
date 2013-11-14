/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani Visual Basic�̃A�E�g���C�����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, JEPRO, genta, hor
	Copyright (C) 2002, MIK, aroka, hor, genta, YAZAKI, Moca, frozen
	Copyright (C) 2003, zenryaku, Moca, naoh, little YOSHI, genta,
	Copyright (C) 2004, zenryaku, Moca, novice
	Copyright (C) 2005, genta, zenryaku, ������, D.S.Koba
	Copyright (C) 2006, genta, aroka, ryoji, Moca
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDlgFuncList.h"
#include "CFuncInfo.h"
#include "CFuncInfoArr.h"// 2002/2/3 aroka
#include "etc_uty.h"
#include "os.h"
#include "Debug.h"
#include "CEditView.h"
#include "Funccode.h"		//Stonee, 2001/03/12
#include "mymessage.h"// 2002/2/3 aroka
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "CEditWnd.h"	//	2006/2/11 aroka �ǉ�
#include "sakura_rc.h"
#include "sakura.hh"

//�A�E�g���C����� CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,					HIDC_FL_BUTTON_COPY,	//�R�s�[
	IDOK,								HIDOK_FL,				//�W�����v
	IDCANCEL,							HIDCANCEL_FL,			//�L�����Z��
	IDC_BUTTON_HELP,					HIDC_FL_BUTTON_HELP,	//�w���v
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	//�����I�ɕ���
	IDC_LIST_FL,						HIDC_FL_LIST1,			//�g�s�b�N���X�g	IDC_LIST1->IDC_LIST_FL	2008/7/3 Uchi
	IDC_TREE_FL,						HIDC_FL_TREE1,			//�g�s�b�N�c���[	IDC_TREE1->IDC_TREE_FL	2008/7/3 Uchi
	IDC_CHECK_bFunclistSetFocusOnJump,	HIDC_FL_CHECK_bFunclistSetFocusOnJump,	//�W�����v�Ńt�H�[�J�X�ړ�����
	IDC_CHECK_bMarkUpBlankLineEnable,	HIDC_FL_CHECK_bMarkUpBlankLineEnable,	//��s�𖳎�����
	IDC_COMBO_nSortType,				HIDC_COMBO_nSortType,	//����
	IDC_BUTTON_WINSIZE,					HIDC_FL_BUTTON_WINSIZE,	//�E�B���h�E�ʒu�ۑ�	// 2006.08.06 ryoji
//	IDC_STATIC,							-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

//�֐����X�g�̗�
enum EFuncListCol {
	FL_COL_ROW		= 0,	//�s
	FL_COL_COL		= 1,	//��
	FL_COL_NAME		= 2,	//�֐���
	FL_COL_REMARK	= 3		//���l
};

/*! �\�[�g��r�p�v���V�[�W�� */
int CALLBACK _CompareFunc_( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	CFuncInfo*		pcFuncInfo1;
	CFuncInfo*		pcFuncInfo2;
	CDlgFuncList*	pcDlgFuncList;
	pcDlgFuncList = (CDlgFuncList*)lParamSort;

	pcFuncInfo1 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam1 );
	if( NULL == pcFuncInfo1 ){
		return -1;
	}
	pcFuncInfo2 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam2 );
	if( NULL == pcFuncInfo2 ){
		return -1;
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if( FL_COL_NAME == pcDlgFuncList->m_nSortCol){	/* ���O�Ń\�[�g */
		return strcmp( pcFuncInfo1->m_cmemFuncName.GetStringPtr(), pcFuncInfo2->m_cmemFuncName.GetStringPtr() );
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if( FL_COL_ROW == pcDlgFuncList->m_nSortCol){	/* �s�i�{���j�Ń\�[�g */
		if( pcFuncInfo1->m_nFuncLineCRLF < pcFuncInfo2->m_nFuncLineCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncLineCRLF == pcFuncInfo2->m_nFuncLineCRLF ){
			if( pcFuncInfo1->m_nFuncColCRLF < pcFuncInfo2->m_nFuncColCRLF ){
				return -1;
			}else
			if( pcFuncInfo1->m_nFuncColCRLF == pcFuncInfo2->m_nFuncColCRLF ){
				return 0;
			}else{
				return 1;
			}
		}else{
			return 1;
		}
	}
	if( FL_COL_COL == pcDlgFuncList->m_nSortCol){	/* ���Ń\�[�g */
		if( pcFuncInfo1->m_nFuncColCRLF < pcFuncInfo2->m_nFuncColCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncColCRLF == pcFuncInfo2->m_nFuncColCRLF ){
			return 0;
		}else{
			return 1;
		}
	}
	// From Here 2001.12.07 hor
	if( FL_COL_REMARK == pcDlgFuncList->m_nSortCol){	/* ���l�Ń\�[�g */
		if( pcFuncInfo1->m_nInfo < pcFuncInfo2->m_nInfo ){
			return -1;
		}else
		if( pcFuncInfo1->m_nInfo == pcFuncInfo2->m_nInfo ){
			return 0;
		}else{
			return 1;
		}
	}
	// To Here 2001.12.07 hor
	return -1;
}

CDlgFuncList::CDlgFuncList()
{
	m_pcFuncInfoArr = NULL;		/* �֐����z�� */
	m_nCurLine = CLayoutInt(0);				/* ���ݍs */
	//	Apr. 23, 2005 genta �s�ԍ������[��
	m_nSortCol = 0;				/* �\�[�g�����ԍ� 2004.04.06 zenryaku �W���͍s�ԍ�(1���) */
	m_bLineNumIsCRLF = false;	/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
	m_bWaitTreeProcess = false;	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 2/4
	m_nSortType = 0;
	m_cFuncInfo = NULL;			/* ���݂̊֐���� */
}


/*!
	�W���ȊO�̃��b�Z�[�W��ߑ�����

	@date 2007.11.07 ryoji �V�K
*/
INT_PTR CDlgFuncList::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	switch( wMsg ){
	case WM_ACTIVATEAPP:
		// �������ŏ��ɃA�N�e�B�u�����ꂽ�ꍇ�͈�U�ҏW�E�B���h�E���A�N�e�B�u�����Ė߂�
		//
		// Note. ���̃_�C�A���O�͑��Ƃ͈قȂ�E�B���h�E�X�^�C���̂��ߕ����Ƃ��̋������قȂ�D
		// ���̓X���b�h���ŋ߃A�N�e�B�u�ȃE�B���h�E���A�N�e�B�u�ɂȂ邪�C���̃_�C�A���O�ł�
		// �Z�b�V�������S�̂ł̍ŋ߃A�N�e�B�u�E�B���h�E���A�N�e�B�u�ɂȂ��Ă��܂��D
		// ����ł͓s���������̂ŁC���ʂɈȉ��̏������s���đ��Ɠ��l�ȋ�����������悤�ɂ���D
		if( (BOOL)wParam ){
			CEditView* pcEditView = (CEditView*)m_lParam;
			CEditWnd* pcEditWnd = pcEditView->m_pcEditWnd;
			if( ::GetActiveWindow() == m_hWnd ){
				::SetActiveWindow( pcEditWnd->m_hWnd );
				BlockingHook( NULL );	// �L���[���ɗ��܂��Ă��郁�b�Z�[�W������
				::SetActiveWindow( m_hWnd );
				return 0L;
			}
		}
		break;
	}

	return CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
}


/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgFuncList::DoModeless(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	CFuncInfoArr*	pcFuncInfoArr,
	CLayoutInt		nCurLine,
	CLayoutInt		nCurCol,
	int				nListType,
	bool			bLineNumIsCRLF		/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
)
{
	m_pcFuncInfoArr = pcFuncInfoArr;	/* �֐����z�� */
	m_nCurLine = nCurLine;				/* ���ݍs */
	m_nCurCol = nCurCol;				/* ���݌� */
	m_nListType = nListType;			/* �ꗗ�̎�� */
	m_bLineNumIsCRLF = bLineNumIsCRLF;	/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */

	// 2007.04.18 genta : �u�t�H�[�J�X���ڂ��v�Ɓu�����I�ɕ���v���`�F�b�N����Ă���ꍇ��
	// �_�u���N���b�N���s���ƁCtrue�̂܂܎c���Ă��܂��̂ŁC�E�B���h�E���J�����Ƃ��Ƀ��Z�b�g����D
	m_bWaitTreeProcess = false;
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FUNCLIST, lParam, SW_SHOW );
}

/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
void CDlgFuncList::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

/*! �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFuncList::SetData()
{
	int				i;
	TCHAR			szText[2048];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	HWND			hwndTree;
	int				bSelected;
	CLayoutInt		nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_FL );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE_FL );

	//2002.02.08 hor �B���Ƃ��ăA�C�e���폜�����Ƃŕ\��
	::ShowWindow( hwndList, SW_HIDE );
	::ShowWindow( hwndTree, SW_HIDE );
	ListView_DeleteAllItems( hwndList );
	TreeView_DeleteAllItems( hwndTree );

	m_cmemClipText.SetString( "" );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */

	if( OUTLINE_CPP == m_nListType ){	/* C++���\�b�h���X�g */
		m_nViewType = 1;
		SetTreeJava( m_hWnd, TRUE ); //	Jan. 04, 2002 genta Java Method Tree�ɓ���
		::SetWindowText( m_hWnd, _T("C++ ���\�b�h�c���[") );
	}
	else if( OUTLINE_FILE == m_nListType ){	//@@@ 2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C������
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, _T("���[���t�@�C��") );
	}
	else if( OUTLINE_WZTXT == m_nListType ){ //@@@ 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, _T("WZ�K�w�t�e�L�X�g") ); //	2003.06.22 Moca ���O�ύX
	}
	else if( OUTLINE_HTML == m_nListType ){ //@@@ 2003.05.20 zenryaku HTML�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, _T("HTML") );
	}
	else if( OUTLINE_TEX == m_nListType ){ //@@@ 2003.07.20 naoh TeX�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, _T("TeX") );
	}
	else if( OUTLINE_TEXT == m_nListType ){ /* �e�L�X�g�E�g�s�b�N���X�g */
		m_nViewType = 1;
		SetTree();	//@@@ 2002.04.01 YAZAKI �e�L�X�g�g�s�b�N�c���[���A�ėpSetTree���ĂԂ悤�ɕύX�B
		::SetWindowText( m_hWnd, _T("�e�L�X�g �g�s�b�N�c���[") );
	}
	else if( OUTLINE_JAVA == m_nListType ){ /* Java���\�b�h�c���[ */
		m_nViewType = 1;
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, _T("Java ���\�b�h�c���[") );
	}
	//	2007.02.08 genta Python�ǉ�
	else if( OUTLINE_PYTHON == m_nListType ){ /* Python ���\�b�h�c���[ */
		m_nViewType = 1;
		SetTree( true );
		::SetWindowText( m_hWnd, _T("Python ���\�b�h�c���[") );
	}
	else if( OUTLINE_COBOL == m_nListType ){ /* COBOL �A�E�g���C�� */
		m_nViewType = 1;
		SetTreeJava( m_hWnd, FALSE );
		::SetWindowText( m_hWnd, _T("COBOL �A�E�g���C��") );
	}
	else if( OUTLINE_VB == m_nListType ){	/* VisualBasic �A�E�g���C�� */
		m_nViewType = 0;
		SetListVB();
		::SetWindowText( m_hWnd, _T("Visual Basic �A�E�g���C��") );
	}
	else if( OUTLINE_TREE == m_nListType ){ /* �ėp�c���[ */
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, _T("") );
	}
	else if( OUTLINE_CLSTREE == m_nListType ){ /* �ėp�N���X�c���[ */
		m_nViewType = 1;
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, _T("") );
	}
	else{
		m_nViewType = 0;
		switch( m_nListType ){
		case OUTLINE_C:
			::SetWindowText( m_hWnd, _T("C �֐��ꗗ") );
			break;
		case OUTLINE_PLSQL:
			::SetWindowText( m_hWnd, _T("PL/SQL �֐��ꗗ") );
			break;
		case OUTLINE_ASM:
			::SetWindowText( m_hWnd, _T("�A�Z���u�� �A�E�g���C��") );
			break;
		case OUTLINE_PERL:	//	Sep. 8, 2000 genta
			::SetWindowText( m_hWnd, _T("Perl �֐��ꗗ") );
			break;
// Jul 10, 2003  little YOSHI  ��Ɉړ����܂���--->>
//		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText( m_hWnd, "Visual Basic �A�E�g���C��" );
//			break;
// <<---�����܂�
		case OUTLINE_ERLANG:	//	2009.08.11 genta
			::SetWindowText( m_hWnd, _T("Erlang �֐��ꗗ") );
			break;
		case OUTLINE_BOOKMARK:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = const_cast<TCHAR*>(_T("�e�L�X�g"));
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta �s�ԍ������[��
			ListView_SetColumn( hwndList, FL_COL_NAME, &col );
			::SetWindowText( m_hWnd, _T("�u�b�N�}�[�N") );
			break;
		case OUTLINE_LIST:	// �ėp���X�g 2010.03.28 syat
			::SetWindowText( m_hWnd, _T("") );
			break;
//		case OUTLINE_COBOL:
//			::SetWindowText( m_hWnd, "COBOL�A�E�g���C��" );
//			break;
		}
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow( hwndTree );
//		::ShowWindow( hwndTree, SW_HIDE );
		::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
		nFuncLineOld = CLayoutInt(0);
		bSelected = FALSE;
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
			if( !bSelected ){
				if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i;
				}
				else if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i - 1;
				}
			}
			nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		}
		if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
			bSelected = TRUE;
			nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
		}
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			/* ���݂̉�͌��ʗv�f */
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

			//	From Here Apr. 23, 2005 genta �s�ԍ������[��
			/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineCRLF );
			}else{
				wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = szText;
			item.iItem = i;
			item.lParam	= i;
			item.iSubItem = FL_COL_ROW;
			ListView_InsertItem( hwndList, &item);

			// 2010.03.17 syat ���ǉ�
			/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColCRLF );
			}else{
				wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColLAYOUT );
			}
			item.mask = LVIF_TEXT;
			item.pszText = szText;
			item.iItem = i;
			item.iSubItem = FL_COL_COL;
			ListView_SetItem( hwndList, &item);

			item.mask = LVIF_TEXT;
			item.pszText = pcFuncInfo->m_cmemFuncName.GetStringPtr();
			item.iItem = i;
			item.iSubItem = FL_COL_NAME;
			ListView_SetItem( hwndList, &item);
			//	To Here Apr. 23, 2005 genta �s�ԍ������[��

			item.mask = LVIF_TEXT;
			if(  1 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("�錾"));}else
			if( 10 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("�֐��錾"));}else
			if( 20 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("�v���V�[�W���錾"));}else
			if( 11 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("�֐�"));}else
			if( 21 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("�v���V�[�W��"));}else
			if( 31 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("���p�b�P�[�W�d�l��"));}else
			if( 41 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("���p�b�P�[�W�{�̕�"));}else
			if( 50 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("PROC"));}else
			if( 51 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("���x��"));}else
			if( 52 == pcFuncInfo->m_nInfo ){item.pszText = const_cast<TCHAR*>(_T("ENDP"));}else{
				// Jul 10, 2003  little YOSHI
				// �����ɂ�����VB�֌W�̏�����SetListVB()���\�b�h�Ɉړ����܂����B

				item.pszText = const_cast<TCHAR*>(_T(""));
			}
			item.iItem = i;
			item.iSubItem = FL_COL_REMARK;
			ListView_SetItem( hwndList, &item);

			/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
			if(item.pszText[0] != _T('\0')){
				// ���o���ʂ̎��(�֐�,,,)������Ƃ�
				wsprintf(
					szText,
					_T("%s(%d,%d): "),
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
				);
				m_cmemClipText.AppendString(szText);
				// "%ts(%ts)\r\n"
				m_cmemClipText.AppendString(pcFuncInfo->m_cmemFuncName.GetStringPtr());
				m_cmemClipText.AppendString("(");
				m_cmemClipText.AppendString(item.pszText);
				m_cmemClipText.AppendString(")\r\n");
			}else{
				// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
				wsprintf(
					szText,
					_T("%s(%d,%d): "),
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
				);
				m_cmemClipText.AppendString(szText);
				m_cmemClipText.AppendString(pcFuncInfo->m_cmemFuncName.GetStringPtr());
				m_cmemClipText.AppendString("\r\n");
			}
		}
		//2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
		::ShowWindow( hwndList, SW_SHOW );
		/* ��̕����f�[�^�ɍ��킹�Ē��� */
		ListView_SetColumnWidth( hwndList, FL_COL_ROW, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_COL, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_NAME, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, FL_COL_ROW, ListView_GetColumnWidth( hwndList, FL_COL_ROW ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_COL, ListView_GetColumnWidth( hwndList, FL_COL_COL ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_NAME, ListView_GetColumnWidth( hwndList, FL_COL_NAME ) + 16 );
		ListView_SetColumnWidth( hwndList, FL_COL_REMARK, ListView_GetColumnWidth( hwndList, FL_COL_REMARK ) + 16 );

		// 2005.07.05 ������
		DWORD dwExStyle  = ListView_GetExtendedListViewStyle( hwndList );
		dwExStyle |= LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle( hwndList, dwExStyle );

		if( bSelected ){
			ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
			ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
			ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		}
	}
	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList );
	/* �A�E�g���C�� �u�b�N�}�[�N�ꗗ�ŋ�s�𖳎����� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable, m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable );
	/* �A�E�g���C�� �W�����v������t�H�[�J�X���ڂ� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump, m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump );

	/* �A�E�g���C�� ���ʒu�ƃT�C�Y���L������ */ // 20060201 aroka
	::CheckDlgButton( m_hWnd, IDC_BUTTON_WINSIZE, m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos );

	/* �_�C�A���O�������I�ɕ���Ȃ�t�H�[�J�X�ړ��I�v�V�����͊֌W�Ȃ� */
	if(m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
	}

	//2002.02.08 hor
	//�iIDC_LIST_FL��IDC_TREE_FL����ɑ��݂��Ă��āAm_nViewType�ɂ���āA�ǂ����\�����邩��I��ł���j
	if(m_nViewType){
		::ShowWindow( hwndTree, SW_SHOW );
		::SetFocus( hwndTree );
	}else{
//		::ShowWindow( hwndList, SW_SHOW );
		::SetFocus( hwndList );
	}
	//2002.02.08 hor
	//��s���ǂ��������̃`�F�b�N�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̂Ƃ������\������
	if(OUTLINE_BOOKMARK == m_nListType){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), TRUE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_SHOW );
	}else{
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_HIDE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), FALSE );
	}
	// 2002/11/1 frozen ���ڂ̃\�[�g���ݒ肷��R���{�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̈ȊO�̎��ɕ\������
	// Nov. 5, 2002 genta �c���[�\���̎������\�[�g��R���{�{�b�N�X��\��
	if( m_nViewType == 1 ){
		HWND hWnd_Combo_Sort = ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType );
		::EnableWindow( hWnd_Combo_Sort , TRUE );
		::ShowWindow( hWnd_Combo_Sort , SW_SHOW );
		::SendMessage( hWnd_Combo_Sort , CB_RESETCONTENT, 0, 0 ); // 2002.11.10 Moca �ǉ�
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("�f�t�H���g")));
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("�A���t�@�x�b�g��")));
		::SendMessage( hWnd_Combo_Sort , CB_SETCURSEL, m_nSortType, 0L);
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_SHOW );
		// 2002.11.10 Moca �ǉ� �\�[�g����
		if( 1 == m_nSortType ){
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE_FL),TVI_ROOT);
		}
	}
	else {
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), FALSE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), SW_HIDE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_HIDE );
		//ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );  // 2005.04.05 zenryaku �\�[�g��Ԃ�ێ�
		SortListView( hwndList, m_nSortCol );	// 2005.04.23 genta �֐���(�w�b�_���������̂���)
	}
}




/* �_�C�A���O�f�[�^�̎擾 */
/* 0==����������   0���傫��==����   0��菬����==���̓G���[ */
int CDlgFuncList::GetData( void )
{
	HWND			hwndList;
	HWND			hwndTree;
	int				nItem;
	LV_ITEM			item;
	HTREEITEM		htiItem;
	TV_ITEM			tvi;
	TCHAR			szLabel[32];

	m_cFuncInfo = NULL;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_FL );
	if( m_nViewType == 0 ){
		//	List
		nItem = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nItem ){
			return -1;
		}
		item.mask = LVIF_PARAM;
		item.iItem = nItem;
		item.iSubItem = 0;
		ListView_GetItem( hwndList, &item );
		m_cFuncInfo = m_pcFuncInfoArr->GetAt( item.lParam );
	}else{
		hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE_FL );
		if( NULL != hwndTree ){
			htiItem = TreeView_GetSelection( hwndTree );

			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.hItem = htiItem;
			tvi.pszText = szLabel;
			tvi.cchTextMax = _countof( szLabel );
			if( TreeView_GetItem( hwndTree, &tvi ) ){
				// lParam��-1�ȉ��� pcFuncInfoArr�ɂ͊܂܂�Ȃ�����
				if( 0 <= tvi.lParam ){
					m_cFuncInfo = m_pcFuncInfoArr->GetAt( tvi.lParam );
				}
			}
		}
	}
	return 1;
}

/* Java/C++���\�b�h�c���[�̍ő�l�X�g�[�� */
#define MAX_JAVA_TREE_NEST 16

/*! �c���[�R���g���[���̏������FJava���\�b�h�c���[

	Java Method Tree�̍\�z: �֐����X�g������TreeControl������������B

	@date 2002.01.04 genta C++�c���[�𓝍�
*/
void CDlgFuncList::SetTreeJava( HWND hwndDlg, BOOL bAddClass )
{
	int				i;
	CFuncInfo*		pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	CLayoutInt		nFuncLineOld;
	CLayoutInt		nFuncColOld;
	TV_INSERTSTRUCT	tvis;
	const TCHAR*	pPos;
    TCHAR           szLabel[64+6];  // Jan. 07, 2001 genta �N���X���G���A�̊g��
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++�Ɠ���
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
	int				nClassNest;
	int				nDummylParam = -64000;	// 2002.11.10 Moca �N���X���̃_�~�[lParam �\�[�g�̂���
	TCHAR			szClassArr[MAX_JAVA_TREE_NEST][64];	// Jan. 04, 2001 genta �N���X���G���A�̊g�� //2009.9.21 syat �l�X�g���[������ۂ�BOF�΍�

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE_FL );

	nFuncLineOld = CLayoutInt(0);
	nFuncColOld = CLayoutInt(0);
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		const TCHAR*		pWork;
		pWork = pcFuncInfo->m_cmemFuncName.GetStringPtr();
		/* �N���X��::���\�b�h�̏ꍇ */
		if( NULL != ( pPos = strstr( pWork, _T("::") ) ) ){
			/* �C���i�[�N���X�̃l�X�g���x���𒲂ׂ� */
			int	k, m;
			int	nWorkLen;
			int	nCharChars;
			nClassNest = 0;
			m = 0;
			nWorkLen = _tcslen( pWork );
			for( k = 0; k < nWorkLen; ++k ){
				//2009.9.21 syat �l�X�g���[������ۂ�BOF�΍�
				if( nClassNest == MAX_JAVA_TREE_NEST ){
					k = nWorkLen;
					break;
				}
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pWork, nWorkLen, k );
				if( 1 == nCharChars && _T(':') == pWork[k] ){
					//	Jan. 04, 2001 genta
					//	C++�̓����̂��߁A\�ɉ�����::���N���X��؂�Ƃ݂Ȃ��悤��
					if( k < nWorkLen - 1 && _T(':') == pWork[k+1] ){
						memcpy( szClassArr[nClassNest], &pWork[m], k - m );
						szClassArr[nClassNest][k - m] = _T('\0');
						++nClassNest;
						m = k + 2;
						++k;
					}
					else 
						break;
				}
				else if( 1 == nCharChars && _T('\\') == pWork[k] ){
					memcpy( szClassArr[nClassNest], &pWork[m], k - m );
					szClassArr[nClassNest][k - m] = _T('\0');
					++nClassNest;
					m = k + 1;
				}
				if( 2 == nCharChars ){
					++k;
				}
			}

			//	Jan. 04, 2001 genta
			//	�֐��擪�̃Z�b�g(�c���[�\�z�Ŏg��)
			pWork = pWork + m; // 2 == lstrlen( "::" );

			/* �N���X���̃A�C�e�����o�^����Ă��邩 */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	�ǉ��������S�p�ɂ����̂Ń����������ꂾ���K�v
				//	6 == strlen( "�N���X" ), 1 == strlen( '\0' )

				// 2002/10/30 frozen
				// bAddClass == true �̏ꍇ�̎d�l�ύX
				// �����̍��ڂ́@�u(�N���X��)(���p�X�y�[�X���)(�ǉ�������)�v
				// �ƂȂ��Ă���Ƃ݂Ȃ��AszClassArr[k] �� �u�N���X���v�ƈ�v����΁A�����e�m�[�h�ɐݒ�B
				// �������A��v���鍀�ڂ���������ꍇ�͍ŏ��̍��ڂ�e�m�[�h�ɂ���B
				// ��v���Ȃ��ꍇ�́u(�N���X��)(���p�X�y�[�X���)�N���X�v�̃m�[�h���쐬����B
				size_t nClassNameLen = _tcslen( szClassArr[k] );
				for( ; NULL != htiClass ; htiClass = TreeView_GetNextSibling( hwndTree, htiClass ))
				{
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = _countof(szLabel);
					if( TreeView_GetItem( hwndTree, &tvi ) )
					{
						if( 0 == strncmp( szClassArr[k],szLabel,nClassNameLen) )
						{
							if( _countof(szLabel) < (nClassNameLen +1) )
								break;// �o�b�t�@�s���ł͖������Ƀ}�b�`����
							else
							{
								if(bAddClass)
								{
									if(szLabel[nClassNameLen]==' ')
										break;
								}
								else
								{
									if(szLabel[nClassNameLen]=='\0')
										break;
								}
							}
						}
					}
				}

				/* �N���X���̃A�C�e�����o�^����Ă��Ȃ��̂œo�^ */
				if( NULL == htiClass ){
					// 2002/10/28 frozen �ォ�炱���ֈړ�
					TCHAR*	pClassName;
					pClassName = new TCHAR[ _tcslen( szClassArr[k] ) + 1 + 9 ]; // 2002/10/28 frozen +9�͒ǉ����镶����̍ő咷�i" ���O���"���ő�j
					_tcscpy( pClassName, szClassArr[k] );

					tvis.item.lParam = -1;
					if( bAddClass )
					{
						if( pcFuncInfo->m_nInfo == 7 )
						{
							_tcscat( pClassName, _T(" ���O���") );
							tvis.item.lParam = i;
						}
						else
							_tcscat( pClassName, _T(" �N���X") );
							tvis.item.lParam = nDummylParam;
							nDummylParam++;
					}

					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = pClassName;

					htiClass = TreeView_InsertItem( hwndTree, &tvis );
					//	Jan. 04, 2001 genta
					//	�s�v�ɂȂ����炳�����ƍ폜
					delete [] pClassName; // 2002/10/28 frozen �����炱���ֈړ�

				}else{
					//none
				}
				htiParent = htiClass;
				//if( k + 1 >= nClassNest ){
				//	break;
				//}
				htiClass = TreeView_GetChild( hwndTree, htiClass );
			}
			htiClass = htiParent;
		}else{
			//	Jan. 04, 2001 genta
			//	Global��Ԃ̏ꍇ (C++�̂�)

			// 2002/10/27 frozen ��������
			// 2007.05.26 genta "__interface" ���N���X�ɗނ��鈵���ɂ���
			if( 3 <= pcFuncInfo->m_nInfo  && pcFuncInfo->m_nInfo <= 8 )
				htiClass = TVI_ROOT;
			else
			{
			// 2002/10/27 frozen �����܂�
				if( htiGlobal == NULL ){
					TV_INSERTSTRUCT	tvg;

					::ZeroMemory( &tvg, sizeof(tvg));
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvg.item.pszText = const_cast<TCHAR*>(_T("�O���[�o��"));
//					tvg.item.lParam = -1;
					tvg.item.lParam = nDummylParam;
					htiGlobal = TreeView_InsertItem( hwndTree, &tvg );
					nDummylParam++;
				}
				htiClass = htiGlobal;
			}
		}
		TCHAR*		pFuncName;
		pFuncName = new TCHAR[ _tcslen(pWork) + 1 + 18 ];	// +16 �͒ǉ����镶����̍ő咷
		_tcscpy( pFuncName, pWork );

		// 2002/10/27 frozen �ǉ�������̎�ނ𑝂₵��
		switch(pcFuncInfo->m_nInfo)
		{// case 4�ȏ�̊e�ǉ�������̍ŏ��ɂ��锼�p�X�y�[�X���ȗ����邱�Ƃ͂ł��Ȃ��B
		case 1: _tcscat( pFuncName, _T("(�錾)") );break;
		case 3: _tcscat( pFuncName, _T(" �N���X") );break;
		case 4: _tcscat( pFuncName, _T(" �\����") );break;
		case 5: _tcscat( pFuncName, _T(" �񋓑�") );break;
		case 6: _tcscat( pFuncName, _T(" ���p��") );break;
//		case 7: _tcscat( pFuncName, _T(" ���O���") );break;
		
		case 8: _tcscat( pFuncName, _T(" �C���^�[�t�F�[�X") );break; // 2007.05.26 genta : "__interface"
		}
//		}
		/* �Y���N���X���̃A�C�e���̎q�Ƃ��āA���\�b�h�̃A�C�e����o�^ */
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = pFuncName;
		tvis.item.lParam = i;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
		char szText[2048];
		wsprintf(
			szText,
			"%s(%d,%d): ",
			m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
			pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
			pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
		);
		m_cmemClipText.AppendString( (const char *)szText ); /* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		// "%ts%ls\r\n"
		m_cmemClipText.AppendString(pcFuncInfo->m_cmemFuncName.GetStringPtr());
		m_cmemClipText.AppendString(1 == pcFuncInfo->m_nInfo ? " (�錾)\r\n" : "\r\n"); 	//	Jan. 04, 2001 genta C++�Ŏg�p
		delete [] pFuncName;

		/* ���݃J�[�\���ʒu�̃��\�b�h���ǂ������ׂ� */
		if( !bSelected ){
			if( i == 0 &&
				( m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT
				|| ( m_nCurLine == pcFuncInfo->m_nFuncLineLAYOUT && m_nCurCol < pcFuncInfo->m_nFuncColLAYOUT ) ) ){
				bSelected = TRUE;
				htiSelected = htiItem;
			}else
			if( i > 0 &&
				( nFuncLineOld < m_nCurLine || ( nFuncLineOld == m_nCurLine && nFuncColOld <= m_nCurCol ) ) &&
				( m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT
				|| ( m_nCurLine == pcFuncInfo->m_nFuncLineLAYOUT && m_nCurCol < pcFuncInfo->m_nFuncColLAYOUT ) ) ){
				bSelected = TRUE;
				htiSelected = htiItemOld;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
		htiItemOld = htiItem;
		//	Jan. 04, 2001 genta
		//	delete�͂��̓s�x�s���̂ł����ł͕s�v
		}
	/* �\�[�g�A�m�[�h�̓W�J������ */
//	TreeView_SortChildren( hwndTree, TVI_ROOT, 0 );
	htiClass = TreeView_GetFirstVisible( hwndTree );
	while( NULL != htiClass ){
//		TreeView_SortChildren( hwndTree, htiClass, 0 );
		TreeView_Expand( hwndTree, htiClass, TVE_EXPAND );
		htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
	}
	/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
	if( bSelected ){
		TreeView_SelectItem( hwndTree, htiSelected );
	}else{
		if( NULL != htiItemOld ){
			TreeView_SelectItem( hwndTree, htiItemOld );
		}
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	return;
}


/*! ���X�g�r���[�R���g���[���̏������FVisualBasic

  �����Ȃ����̂œƗ������܂����B

  @date Jul 10, 2003  little YOSHI
*/
void CDlgFuncList::SetListVB (void)
{
	int				i;
	TCHAR			szType[64];
	TCHAR			szOption[64];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	int				bSelected;
	CLayoutInt		nFuncLineOld;
	CLayoutInt		nFuncColOld;
	int				nSelectedLine;
	RECT			rc;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_FL );

	nFuncLineOld = CLayoutInt(0);
	nFuncColOld = CLayoutInt(0);
	bSelected = FALSE;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		if( !bSelected ){
			if( i == 0 &&
				( m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT
				|| ( m_nCurLine == pcFuncInfo->m_nFuncLineLAYOUT && m_nCurCol < pcFuncInfo->m_nFuncColLAYOUT ) ) ){
				bSelected = TRUE;
				nSelectedLine = i;
			} else
			if( i > 0 &&
				( nFuncLineOld < m_nCurLine || ( nFuncLineOld == m_nCurLine && nFuncColOld <= m_nCurCol ) ) &&
				( m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT
				|| ( m_nCurLine == pcFuncInfo->m_nFuncLineLAYOUT && m_nCurCol < pcFuncInfo->m_nFuncColLAYOUT ) ) ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		nFuncColOld = pcFuncInfo->m_nFuncColLAYOUT;
	}
	if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
		bSelected = TRUE;
		nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
	}

	TCHAR			szText[2048];
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		/* ���݂̉�͌��ʗv�f */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		//	From Here Apr. 23, 2005 genta �s�ԍ������[��
		/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineCRLF );
		}else{
			wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = FL_COL_ROW;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		// 2010.03.17 syat ���ǉ�
		/* �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P�� */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColCRLF );
		}else{
			wsprintf( szText, _T("%d"), pcFuncInfo->m_nFuncColLAYOUT );
		}
		item.mask = LVIF_TEXT;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = FL_COL_COL;
		ListView_SetItem( hwndList, &item);

		item.mask = LVIF_TEXT;
		item.pszText = pcFuncInfo->m_cmemFuncName.GetStringPtr();
		item.iItem = i;
		item.iSubItem = FL_COL_NAME;
		ListView_SetItem( hwndList, &item);
		//	To Here Apr. 23, 2005 genta �s�ԍ������[��

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta ���p���ȁ��S�p��
		memset(szText, _T('\0'), _countof(szText));
		memset(szType, _T('\0'), _countof(szType));
		memset(szOption, _T('\0'), _countof(szOption));
		if( 1 == ((pcFuncInfo->m_nInfo >> 8) & 0x01) ){
			// �X�^�e�B�b�N�錾(Static)
			// 2006.12.12 Moca �����ɃX�y�[�X�ǉ�
			_tcscpy(szOption, _T("�ÓI "));
		}
		switch ((pcFuncInfo->m_nInfo >> 4) & 0x0f) {
			case 2  :	// �v���C�x�[�g(Private)
				_tcsncat(szOption, _T("�v���C�x�[�g"), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
				break;

			case 3  :	// �t�����h(Friend)
				_tcsncat(szOption, _T("�t�����h"), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
				break;

			default :	// �p�u���b�N(Public)
				_tcsncat(szOption, _T("�p�u���b�N"), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
		}
		switch (pcFuncInfo->m_nInfo & 0x0f) {
			case 1:		// �֐�(Function)
				_tcscpy(szType, _T("�֐�"));
				break;

			// 2006.12.12 Moca �X�e�[�^�X���v���V�[�W���ɕύX
			case 2:		// �v���V�[�W��(Sub)
				_tcscpy(szType, _T("�v���V�[�W��"));
				break;

			case 3:		// �v���p�e�B �擾(Property Get)
				_tcscpy(szType, _T("�v���p�e�B �擾"));
				break;

			case 4:		// �v���p�e�B �ݒ�(Property Let)
				_tcscpy(szType, _T("�v���p�e�B �ݒ�"));
				break;

			case 5:		// �v���p�e�B �Q��(Property Set)
				_tcscpy(szType, _T("�v���p�e�B �Q��"));
				break;

			case 6:		// �萔(Const)
				_tcscpy(szType, _T("�萔"));
				break;

			case 7:		// �񋓌^(Enum)
				_tcscpy(szType, _T("�񋓌^"));
				break;

			case 8:		// ���[�U��`�^(Type)
				_tcscpy(szType, _T("���[�U��`�^"));
				break;

			case 9:		// �C�x���g(Event)
				_tcscpy(szType, _T("�C�x���g"));
				break;

			default:	// ����`�Ȃ̂ŃN���A
				pcFuncInfo->m_nInfo	= 0;

		}
		if ( 2 == ((pcFuncInfo->m_nInfo >> 8) & 0x02) ) {
			// �錾(Declare�Ȃ�)
			_tcsncat(szType, _T("�錾"), _countof(szType) - _tcslen(szType));
		}

		TCHAR szTypeOption[256]; // 2006.12.12 Moca wsprintf�̓��o�͂œ���ϐ����g��Ȃ����߂̍�Ɨ̈�ǉ�
		if ( 0 == pcFuncInfo->m_nInfo ) {
			szTypeOption[0] = _T('\0');	//	2006.12.17 genta �S�̂�0�Ŗ��߂�K�v�͂Ȃ�
		} else
		if ( szOption[0] == _T('\0') ) {
			wsprintf(szTypeOption, _T("%s"), szType);
		} else {
			wsprintf(szTypeOption, _T("%s�i%s�j"), szType, szOption);
		}
		item.pszText = szTypeOption;
		item.iItem = i;
		item.iSubItem = FL_COL_REMARK;
		ListView_SetItem( hwndList, &item);

		/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
		if(item.pszText[0] != _T('\0')){
			// ���o���ʂ̎��(�֐�,,,)������Ƃ�
			// 2006.12.12 Moca szText ���������g�ɃR�s�[���Ă����o�O���C��
			wsprintf(
				szText,
				_T("%s(%d,%d): "),
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
			);
			m_cmemClipText.AppendString(szText);
			// "%ts(%ts)\r\n"
			m_cmemClipText.AppendString(pcFuncInfo->m_cmemFuncName.GetStringPtr());
			m_cmemClipText.AppendString("(");
			m_cmemClipText.AppendString(item.pszText);
			m_cmemClipText.AppendString(")\r\n");
		}else{
			// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
			wsprintf(
				szText,
				_T("%s(%d,%d): "),
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
			);
			m_cmemClipText.AppendString(szText);
			// "%ts\r\n"
			m_cmemClipText.AppendString(pcFuncInfo->m_cmemFuncName.GetStringPtr());
			m_cmemClipText.AppendString("\r\n");
		}
	}

	//2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
	::ShowWindow( hwndList, SW_SHOW );
	/* ��̕����f�[�^�ɍ��킹�Ē��� */
	ListView_SetColumnWidth( hwndList, FL_COL_ROW, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_COL, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_NAME, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, FL_COL_ROW, ListView_GetColumnWidth( hwndList, FL_COL_ROW ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_COL, ListView_GetColumnWidth( hwndList, FL_COL_COL ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_NAME, ListView_GetColumnWidth( hwndList, FL_COL_NAME ) + 16 );
	ListView_SetColumnWidth( hwndList, FL_COL_REMARK, ListView_GetColumnWidth( hwndList, FL_COL_REMARK ) + 16 );
	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	return;
}

/*! �ėp�c���[�R���g���[���̏������FCFuncInfo::m_nDepth�𗘗p���Đe�q��ݒ�

	@param[in] tagjump �^�O�W�����v�`���ŏo�͂���

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca �K�w�̐������Ȃ�����
	@date 2007.02.25 genta �N���b�v�{�[�h�o�͂��^�u�W�����v�\�ȏ����ɕύX
	@date 2007.03.04 genta �^�u�W�����v�\�ȏ����ɕύX���邩�ǂ����̃t���O��ǉ�
*/
void CDlgFuncList::SetTree(bool tagjump)
{
	HTREEITEM hItemSelected = NULL;
	HWND hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE_FL );

	int i;
	int nFuncInfoArrNum = m_pcFuncInfoArr->GetNum();
	int nStackPointer = 0;
	int nStackDepth = 32; // phParentStack �̊m�ۂ��Ă��鐔
	HTREEITEM* phParentStack;
	phParentStack = (HTREEITEM*)malloc( nStackDepth * sizeof( HTREEITEM ) );
	phParentStack[ nStackPointer ] = TVI_ROOT;

	for (i = 0; i < nFuncInfoArrNum; i++){
		CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);

		/*	�V�����A�C�e�����쐬
			���݂̐e�̉��ɂԂ牺����`�ŁA�Ō�ɒǉ�����B
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		cTVInsertStruct.hInsertAfter = TVI_LAST;	//	�K���Ō�ɒǉ��B
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pcFuncInfo->m_cmemFuncName.GetStringPtr();
		cTVInsertStruct.item.lParam = i;	//	���Ƃł��̐��l�i��m_pcFuncInfoArr�̉��Ԗڂ̃A�C�e�����j�����āA�ړI�n�ɃW�����v���邺!!�B

		/*	�e�q�֌W���`�F�b�N
		*/
		if (nStackPointer != pcFuncInfo->m_nDepth){
			//	���x�����ς��܂���!!
			//	�����A2�i�K�[���Ȃ邱�Ƃ͍l�����Ă��Ȃ��̂Œ��ӁB
			//	�@�������A2�i�K�ȏ�󂭂Ȃ邱�Ƃ͍l���ς݁B

			// 2002.11.10 Moca �ǉ� �m�ۂ����T�C�Y�ł͑���Ȃ��Ȃ����B�Ċm��
			if( nStackDepth <= pcFuncInfo->m_nDepth + 1 ){
				nStackDepth = pcFuncInfo->m_nDepth + 4; // ���߂Ɋm�ۂ��Ă���
				HTREEITEM* phTi;
				phTi = (HTREEITEM*)realloc( phParentStack, nStackDepth * sizeof( HTREEITEM ) );
				if( NULL != phTi ){
					phParentStack = phTi;
				}else{
					goto end_of_func;
				}
			}
			nStackPointer = pcFuncInfo->m_nDepth;
			cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		}
		hItem = TreeView_InsertItem( hwndTree, &cTVInsertStruct );
		phParentStack[ nStackPointer+1 ] = hItem;

		/*	pcFuncInfo�ɓo�^����Ă���s���A�����m�F���āA�I������A�C�e�����l����
		*/
		if ( pcFuncInfo->m_nFuncLineLAYOUT < m_nCurLine
			|| ( pcFuncInfo->m_nFuncLineLAYOUT == m_nCurLine && pcFuncInfo->m_nFuncColLAYOUT <= m_nCurCol ) ){
			hItemSelected = hItem;
		}

		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g���쐬���� */
		//	2003.06.22 Moca dummy�v�f�̓c���[�ɓ���邪TAGJUMP�ɂ͉����Ȃ�
		if( pcFuncInfo->IsAddClipText() ){
			CMemory text;

			if( tagjump ){

				text.AllocStringBuffer(
					pcFuncInfo->m_cmemFuncName.GetStringLength()
					+ nStackPointer * 2 + 1
					+ _tcslen( m_pcFuncInfoArr->m_szFilePath )
					+ 20
				);

				//	2007.03.04 genta �^�O�W�����v�ł���`���ŏ�������
				text.AppendString( m_pcFuncInfoArr->m_szFilePath );
				
				TCHAR linenum[32];
				int len = wsprintf( linenum, _T("(%d,%d): "),
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_nFuncColCRLF					/* ���o���ԍ� */
				);
				text.AppendString( linenum );
			}
			else {
				//	��ɏ\���ȃT�C�Y�̗̈������Ă���
				text.AllocStringBuffer(
					pcFuncInfo->m_cmemFuncName.GetStringLength() + nStackPointer * 2 + 1 + 5
				);
			}

			for( int cnt = 0; cnt < nStackPointer; cnt++ ){
				text.AppendString(_T("  "));
			}
			text.AppendString(_T(" "));
			
			text.AppendNativeData( pcFuncInfo->m_cmemFuncName );
			text.AppendString( _T("\r\n") );
			m_cmemClipText.AppendNativeData( text );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}
	}

end_of_func:;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	if( NULL != hItemSelected ){
		/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
		TreeView_SelectItem( hwndTree, hItemSelected );
	}

	free( phParentStack );
}



BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[] = { 0, 10, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_FL );
	// 2005.10.21 zenryaku 1�s�I��
	ListView_SetExtendedListViewStyle(hwndList,
		ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT);

	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] + nColWidthArr[3] ) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = const_cast<TCHAR*>(_T("�s *"));
	col.iSubItem = FL_COL_ROW;
	ListView_InsertColumn( hwndList, FL_COL_ROW, &col);

	// 2010.03.17 syat ���ǉ�
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_COL];
	col.pszText = const_cast<TCHAR*>(_T("��"));
	col.iSubItem = FL_COL_COL;
	ListView_InsertColumn( hwndList, FL_COL_COL, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_NAME];
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = const_cast<TCHAR*>(_T("�֐���"));
	col.iSubItem = FL_COL_NAME;
	ListView_InsertColumn( hwndList, FL_COL_NAME, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_REMARK];
	col.pszText = const_cast<TCHAR*>(_T(" "));
	col.iSubItem = FL_COL_REMARK;
	ListView_InsertColumn( hwndList, FL_COL_REMARK, &col);

	/* �A�E�g���C���ʒu�ƃT�C�Y������������ */ // 20060201 aroka
	CEditView* pcEditView=(CEditView*)m_lParam;
	if( pcEditView != NULL ){
		if( m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ){
			WINDOWPLACEMENT cWindowPlacement;
			cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
			if (::GetWindowPlacement( pcEditView->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
				/* �E�B���h�E�ʒu�E�T�C�Y��-1�ȊO�̒l�ɂ��Ă����ƁACDialog�Ŏg�p�����D */
				m_xPos = m_pShareData->m_Common.m_sOutline.m_xOutlineWindowPos + cWindowPlacement.rcNormalPosition.left;
				m_yPos = m_pShareData->m_Common.m_sOutline.m_yOutlineWindowPos + cWindowPlacement.rcNormalPosition.top;
				m_nWidth =  m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow;
				m_nHeight = m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow;
			}
		}
	}
	/* ���N���X�����o */
	CreateSizeBox();

	LONG_PTR lStyle = ::GetWindowLongPtr( m_hWnd, GWL_STYLE );
	::SetWindowLongPtr( m_hWnd, GWL_STYLE, lStyle | WS_THICKFRAME );
	::SetWindowPos( m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );

	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�A�E�g���C����́v�̃w���v */
		//Apr. 5, 2001 JEPRO �C���R���ǉ� (Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���)
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if( m_bModal ){		/* ���[�_�� �_�C�A���O�� */
			::EndDialog( m_hWnd, 0 );
		}else{
			::DestroyWindow( m_hWnd );
		}
		return TRUE;
	case IDC_BUTTON_COPY:
		// Windows�N���b�v�{�[�h�ɃR�s�[ 
		// 2004.02.17 Moca �֐���
		SetClipboardText( m_hWnd, m_cmemClipText.GetStringPtr(), m_cmemClipText.GetStringLength() );
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// �E�B���h�E�̈ʒu�ƃT�C�Y���L�� // 20060201 aroka
			m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos = ::IsDlgButtonChecked( m_hWnd, IDC_BUTTON_WINSIZE );
		}
		return TRUE;
	//2002.02.08 �I�v�V�����ؑ֌�List/Tree�Ƀt�H�[�J�X�ړ�
	case IDC_CHECK_bAutoCloseDlgFuncList:
	case IDC_CHECK_bMarkUpBlankLineEnable:
	case IDC_CHECK_bFunclistSetFocusOnJump:
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );
		m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable );
		m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump );
		if(m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
		}
		if(wID==IDC_CHECK_bMarkUpBlankLineEnable&&m_nListType==OUTLINE_BOOKMARK){
			CEditView* pcEditView=(CEditView*)m_lParam;
			pcEditView->HandleCommand( F_BOOKMARK_VIEW, true, TRUE, 0, 0, 0 );
			m_nCurLine=pcEditView->m_ptCaretPos.y + CLayoutInt(1);
			SetData();
		}else
		if(m_nViewType){
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_TREE_FL ) );
		}else{
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_LIST_FL ) );
		}
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFuncList::OnNotify( WPARAM wParam, LPARAM lParam )
{
//	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
//	int				nLineTo;

//	idCtrl = (int) wParam;
	pnmh = (LPNMHDR) lParam;
	pnlv = (NM_LISTVIEW*)lParam;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_FL );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE_FL );

	if( hwndTree == pnmh->hwndFrom ){
		pnmtv = (NM_TREEVIEW *) lParam;
		switch( pnmtv->hdr.code ){
		case NM_DBLCLK:
			// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 3/4
			OnJump();
			m_bWaitTreeProcess=true;
			return TRUE;
			//return OnJump();
		case TVN_KEYDOWN:
			Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		case TVN_SELCHANGED:
			// 2005.11.04 zenryaku ��ʊO�̑I�����ڂ�ǂ�
			::SendMessage(hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)((NMTREEVIEW*)lParam)->itemNew.hItem);
			break;
		case NM_KILLFOCUS:
			// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 4/4
			if(m_bWaitTreeProcess){
				if(m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump){
					::SetFocus(::GetParent( ::GetParent( m_hwndParent ) ));
				}
				m_bWaitTreeProcess=false;
			}
			return TRUE;
		}
	}else
	if( hwndList == pnmh->hwndFrom ){
		switch( pnmh->code ){
		case LVN_COLUMNCLICK:
//			MYTRACE( _T("LVN_COLUMNCLICK\n") );
			m_nSortCol =  pnlv->iSubItem;
			//	Apr. 23, 2005 genta �֐��Ƃ��ēƗ�������
			SortListView( hwndList, m_nSortCol );
			return TRUE;
		case NM_DBLCLK:
			return OnJump();
		case LVN_KEYDOWN:
			Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		}
	}
	return FALSE;
}
/*!
	�w�肳�ꂽ�J�����Ń��X�g�r���[���\�[�g����D
	�����Ƀw�b�_������������D

	�\�[�g��̓t�H�[�J�X����ʓ��Ɍ����悤�ɕ\���ʒu�𒲐�����D

	@par �\���ʒu�����̏��Z
	EnsureVisible�̌��ʂ́C��X�N���[���̏ꍇ�͏�[�ɁC���X�N���[���̏ꍇ��
	���[�ɖړI�̍��ڂ������D�[���班�����������ꍇ�̓I�t�Z�b�g��^����K�v��
	���邪�C�X�N���[���������킩��Ȃ��Ɓ}���킩��Ȃ�
	���̂��ߍŏ��Ɉ�ԉ��Ɉ��X�N���[�������邱�Ƃ�EnsureVisible�ł�
	���Ȃ炸��X�N���[���ɂȂ�悤�ɂ��邱�ƂŁC�\�[�g��̕\���ʒu��
	�Œ肷��

	@param[in] hwndList	���X�g�r���[�̃E�B���h�E�n���h��
	@param[in] sortcol	�\�[�g����J�����ԍ�(0-2)

	@date 2005.04.23 genta �֐��Ƃ��ēƗ�������
	@date 2005.04.29 genta �\�[�g��̕\���ʒu����
	@date 2010.03.17 syat ���ǉ�
*/
void CDlgFuncList::SortListView(HWND hwndList, int sortcol)
{
	LV_COLUMN		col;
	int col_no;

	//	Apr. 23, 2005 genta �s�ԍ������[��

//	if( sortcol == 1 ){
	{
		col_no = FL_COL_NAME;
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = _T("�֐��� *");
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = const_cast<TCHAR*>( sortcol == col_no ? _T("�e�L�X�g *") : _T("�e�L�X�g") );
		}else{
			col.pszText = const_cast<TCHAR*>( sortcol == col_no ? _T("�֐��� *") : _T("�֐���") );
		}
	// To Here 2001.12.03 hor
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		col_no = FL_COL_ROW;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? _T("�s *") : _T("�s") );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		// 2010.03.17 syat ���ǉ�
		col_no = FL_COL_COL;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? _T("�� *") : _T("��") );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );

		col_no = FL_COL_REMARK;
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>( sortcol == col_no ? _T("*") : _T("") );
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, col_no, &col );
	// To Here 2001.12.07 hor

		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	}
	//	2005.04.23 zenryaku �I�����ꂽ���ڂ�������悤�ɂ���

	//	Apr. 29, 2005 genta ��U��ԉ��ɃX�N���[��������
	ListView_EnsureVisible( hwndList,
		ListView_GetItemCount(hwndList) - 1,
		FALSE );
	
	//	Jan.  9, 2006 genta �擪����1�ڂ�2�ڂ̊֐���
	//	�I�����ꂽ�ꍇ�ɃX�N���[������Ȃ�����
	int keypos = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED) - 2;
	ListView_EnsureVisible( hwndList,
		keypos >= 0 ? keypos : 0,
		FALSE );
}

/*!	�E�B���h�E�T�C�Y���ύX���ꂽ

	@date 2003.06.22 Moca �R�[�h�̐���(�R���g���[���̏������@���e�[�u���Ɏ�������)
	@date 2003.08.16 genta �z���static��(���ʂȏ��������s��Ȃ�����)
*/
BOOL CDlgFuncList::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	static const int Controls[][2] = {
		{IDC_CHECK_bFunclistSetFocusOnJump, 1},
		{IDC_CHECK_bMarkUpBlankLineEnable , 1},
		{IDC_CHECK_bAutoCloseDlgFuncList, 1},
		{IDC_BUTTON_WINSIZE, 2}, // 20060201 aroka
		{IDC_BUTTON_COPY, 2},
		{IDOK, 2},
		{IDCANCEL, 2},
		{IDC_BUTTON_HELP, 2},
		{IDC_LIST_FL, 3},
		{IDC_TREE_FL, 3},
	};
	int		nControls = _countof( Controls );
//	int		fwSizeType;
	int		nWidth;
	int		nHeight;
	int		i;
	int		nHeightCheckBox;
	int		nHeightButton;
	const int	nHeightMargin = 3;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;

	nWidth = LOWORD(lParam);	// width of client area
	nHeight = HIWORD(lParam);	// height of client area


	::GetWindowRect( ::GetDlgItem( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList ), &rc );
	nHeightCheckBox = rc.bottom -  rc.top;
	::GetWindowRect( ::GetDlgItem( m_hWnd, IDOK ), &rc );	
	nHeightButton = rc.bottom - rc.top;

	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( m_hWnd, Controls[i][0] );
		::GetWindowRect( hwndCtrl, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( m_hWnd, &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( m_hWnd, &po );
		rc.right = po.x;
		rc.bottom  = po.y;
		//	2003.06.22 Moca �e�[�u����̎�ʂɂ���ď������@��ς���
		switch( Controls[i][1] ){
		case 1:
			::SetWindowPos( hwndCtrl, NULL, 
				rc.left,
				nHeight - nHeightCheckBox - nHeightMargin,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
// 2002/11/1 frozen ��������
		case 2:
			::SetWindowPos( hwndCtrl, NULL,
				rc.left,
				nHeight - nHeightCheckBox - nHeightButton - nHeightMargin * 2,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		case 3:
			::SetWindowPos( hwndCtrl, NULL, 0, 0, 
				nWidth - 2 * rc.left,
				nHeight - rc.top - nHeightCheckBox - nHeightButton - 3 * nHeightMargin,
				SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		}
// 2002/11/1 frozen �����܂�
		::InvalidateRect( hwndCtrl, NULL, TRUE );
	}
	return TRUE;
}

int CALLBACK Compare_by_ItemData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if( lParam1< lParam2 )
		return -1;
	if( lParam1 > lParam2 )
		return 1;
	else
		return 0;
}

BOOL CDlgFuncList::OnDestroy( void )
{
	CDialog::OnDestroy();

	/* �A�E�g���C�� ���ʒu�ƃT�C�Y���L������ */ // 20060201 aroka
	// �O������Fm_lParam �� CDialog::OnDestroy �ŃN���A����Ȃ�����
	CEditView* pcEditView=(CEditView*)m_lParam;
	if( m_pShareData->m_Common.m_sOutline.m_bRememberOutlineWindowPos ){
		/* �e�̃E�B���h�E�ʒu�E�T�C�Y���L�� */
		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
		if (::GetWindowPlacement( pcEditView->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
			/* �E�B���h�E�ʒu�E�T�C�Y���L�� */
			m_pShareData->m_Common.m_sOutline.m_xOutlineWindowPos = m_xPos - cWindowPlacement.rcNormalPosition.left;
			m_pShareData->m_Common.m_sOutline.m_yOutlineWindowPos = m_yPos - cWindowPlacement.rcNormalPosition.top;
			m_pShareData->m_Common.m_sOutline.m_widthOutlineWindow = m_nWidth;
			m_pShareData->m_Common.m_sOutline.m_heightOutlineWindow = m_nHeight;
		}

	}
	return TRUE;
}


BOOL CDlgFuncList::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int nSelect = ::SendMessage(hwndCtl,CB_GETCURSEL, 0, 0L);
	switch(wID)
	{
	case IDC_COMBO_nSortType:
		if( m_nSortType != nSelect )
		{
			m_nSortType = nSelect;
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE_FL),TVI_ROOT);
		}
		return TRUE;
	}
	return FALSE;

}
void  CDlgFuncList::SortTree(HWND hWndTree,HTREEITEM htiParent)
{
	if( m_nSortType == 1 )
		TreeView_SortChildren(hWndTree,htiParent,TRUE);
	else
	{
		TVSORTCB sort;
		sort.hParent =  htiParent;
		sort.lpfnCompare = Compare_by_ItemData;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , TRUE);
	}
	for(HTREEITEM htiItem = TreeView_GetChild( hWndTree, htiParent ); NULL != htiItem ; htiItem = TreeView_GetNextSibling( hWndTree, htiItem ))
		SortTree(hWndTree,htiItem);
}



BOOL CDlgFuncList::OnJump( bool bCheckAutoClose )	//2002.02.08 hor �����ǉ�
{
	int				nLineTo;
	int				nColTo;
	/* �_�C�A���O�f�[�^�̎擾 */
	if( 0 < GetData() && m_cFuncInfo != NULL ){
		nLineTo = m_cFuncInfo->m_nFuncLineCRLF;
		nColTo = m_cFuncInfo->m_nFuncColCRLF;
		if( m_bModal ){		/* ���[�_�� �_�C�A���O�� */
			//���[�_���\������ꍇ�́Am_cFuncInfo���擾����A�N�Z�T���������Č��ʎ擾���邱�ƁB
			::EndDialog( m_hWnd, 1 );
		}else{
			/* �J�[�\�����ړ������� */
			POINT	poCaret;
			poCaret.x = nColTo - 1;
			poCaret.y = nLineTo - 1;

			memcpy( m_pShareData->m_sWorkBuffer.m_szWork, (void*)&poCaret, sizeof(poCaret) );

			//	2006.07.09 genta �ړ����ɑI����Ԃ�ێ�����悤��
			::SendMessage( ::GetParent( ::GetParent( m_hwndParent ) ),
				MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			if( bCheckAutoClose ){
				/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList ){
					::DestroyWindow( m_hWnd );
				}
				else if( m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump ){
					::SetFocus( ::GetParent( ::GetParent( m_hwndParent ) ) );
				}
			}
		}
	}
	return TRUE;
}


//@@@ 2002.01.18 add start
LPVOID CDlgFuncList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


/*!	�L�[������R�}���h�ɕϊ�����w���p�[�֐�
	
*/
void CDlgFuncList::Key2Command(WORD KeyCode)
{
	CEditView*	pcEditView;
// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	int nIdx = getCtrlKeyState();
	int nFuncCode=CKeyBind::GetFuncCode(
			((WORD)(((BYTE)(KeyCode)) | ((WORD)((BYTE)(nIdx))) << 8)),
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);
	switch( nFuncCode ){
	case F_REDRAW:
		nFuncCode=(m_nListType==OUTLINE_BOOKMARK)?F_BOOKMARK_VIEW:F_OUTLINE;
		/*FALLTHROUGH*/
	case F_OUTLINE:
	case F_OUTLINE_TOGGLE: // 20060201 aroka �t�H�[�J�X������Ƃ��̓����[�h
	case F_BOOKMARK_VIEW:
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, true, SHOW_RELOAD, 0, 0, 0 ); // �����̕ύX 20060201 aroka

		break;
	case F_BOOKMARK_SET:
		OnJump( false );
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, true, 0, 0, 0, 0 );

		break;
	case F_COPY:
	case F_CUT:
		OnBnClicked( IDC_BUTTON_COPY );
		break;
	}
}

/*!
	@date 2002.10.05 genta
*/
void CDlgFuncList::Redraw( int nOutLineType, CFuncInfoArr* pcFuncInfoArr, CLayoutInt nCurLine, CLayoutInt nCurCol )
{
	m_nListType = nOutLineType;
	m_pcFuncInfoArr = pcFuncInfoArr;	/* �֐����z�� */
	m_nCurLine = nCurLine;				/* ���ݍs */
	m_nCurCol = nCurCol;				/* ���݌� */
	SetData();
}

/*[EOF]*/
