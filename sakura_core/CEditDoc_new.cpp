/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	
	@date aroka �x���΍�ŕϐ�����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, hor, genta, aroka, Moca, MIK, ai
	Copyright (C) 2003, MIK, zenryaku, genta, little YOSHI
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, maru, Moca, genta
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditDoc.h"
#include "charcode.h"
#include "CFuncInfoArr.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CEditWnd.h"
#include "Debug.h"
#include "etc_uty.h"
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "mymessage.h"	//	Oct. 9, 2004 genta
#include "CControlTray.h"

//	From Here Aug. 21, 2000 genta
//
//	�����ۑ����s�����ǂ����̃`�F�b�N
//
void CEditDoc::CheckAutoSave(void)
{
	if( m_cAutoSave.CheckAction() ){
		//	�㏑���ۑ�

		if( !IsModified() )	//	�ύX�����Ȃ牽�����Ȃ�
			return;				//	�����ł́C�u���ύX�ł��ۑ��v�͖�������

		//	2003.10.09 zenryaku �ۑ����s�G���[�̗}��
		if( !IsValidPath() )	//	�܂��t�@�C�������ݒ肳��Ă��Ȃ���Εۑ����Ȃ�
			return;

		bool en = m_cAutoSave.IsEnabled();
		m_cAutoSave.Enable(false);	//	2�d�Ăяo����h������
		SaveFile( GetFilePath() );	//	�ۑ��im_nCharCode, m_cSaveLineCode��ύX���Ȃ��j
		m_cAutoSave.Enable(en);
	}
}

//
//	�ݒ�ύX�������ۑ�����ɔ��f����
//
void CEditDoc::ReloadAutoSaveParam(void)
{
	m_cAutoSave.SetInterval( m_pShareData->m_Common.m_sBackup.GetAutoBackupInterval() );
	m_cAutoSave.Enable( m_pShareData->m_Common.m_sBackup.IsAutoBackupEnabled() );
}


//	�t�@�C���̕ۑ��@�\��EditView����ړ�
//
bool CEditDoc::SaveFile( const char* pszPath )
{
	// 2006.09.01 ryoji �ۑ��O�������s�}�N�������s����
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnSave, pszPath );

	if( FileWrite( pszPath, m_cSaveLineCode ) ){
		SetModified(false,true);	//	Jan. 22, 2002 genta

		/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
		m_cOpeBuf.SetNoModified();
		return true;
	}
	return false;
}

//	To Here Aug. 21, 2000 genta

/*! �u�b�N�}�[�N���X�g�쐬�i������I�j

	@date 2001.12.03 hor   �V�K�쐬
	@date 2002.01.19 aroka ��s���}�[�N�Ώۂɂ���t���O bMarkUpBlankLineEnable �𓱓����܂����B
	@date 2005.10.11 ryoji "��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ�
	@date 2005.11.03 genta �����񒷏C���D�E�[�̃S�~������
*/
void CEditDoc::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int		nLineLen;
	int		nLineCount;
	int		leftspace, pos_wo_space, k;
	BOOL	bMarkUpBlankLineEnable = m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! ��s���}�[�N�Ώۂɂ���t���O 20020119 aroka
	int		nNewLineLen	= m_cNewLineCode.GetLen();
	int		nLineLast	= m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = 0; nLineCount <  nLineLast; ++nLineCount ){
		if(!m_cDocLineMgr.GetLine(nLineCount)->IsBookmarked())continue;
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( pLine[leftspace] == ' ' ||
				pLine[leftspace] == '\t'){
				continue;
			}else if( (unsigned char)pLine[leftspace] == (unsigned char)0x81
				&& (unsigned char)pLine[leftspace + 1] == (unsigned char)0x40 ){
				++leftspace;
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(pLine[k] == CR ||
						pLine[k] == LF ||
						pLine[k] == SPACE ||
						pLine[k] == TAB ||
						pLine[k] == '\0') )
					pos_wo_space = k + nCharChars;
			}
			else if( 2 == nCharChars ){
				if( !((unsigned char)pLine[k] == (unsigned char)0x81 && (unsigned char)pLine[k + 1] == (unsigned char)0x40) )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta �����񒷌v�Z���̏C��
		std::string strText( &pLine[leftspace], pos_wo_space - leftspace );

		CLayoutPoint ptXY;
		m_cLayoutMgr.LogicToLayout(	0, nLineCount, &ptXY.x, &ptXY.y );
		pcFuncInfoArr->AppendData( nLineCount+1, ptXY.y+1 , strText.c_str(), 0 );
	}
	return;
}

// From Here Jan. 22, 2002 genta
/*! �ύX�t���O�̐ݒ�

	@param flag [in] �ݒ肷��l�Dtrue: �ύX�L�� / false: �ύX����
	@param redraw [in] true: �^�C�g���̍ĕ`����s�� / false: �s��Ȃ�
	
	@author genta
	@date 2002.01.22 �V�K�쐬
*/
void CEditDoc::SetModified( bool flag, bool redraw)
{
	if( m_bIsModified == flag )	//	�ύX���Ȃ���Ή������Ȃ�
		return;

	m_bIsModified = flag;
	if( redraw )
		m_pcEditWnd->UpdateCaption();
}
// From Here Jan. 22, 2002 genta

/*! �t�@�C����(�p�X�Ȃ�)���擾����
	@author Moca
	@date 2002.10.13
*/
const char * CEditDoc::GetFileName( void ) const
{
	const char *p, *pszName;
	pszName = p = GetFilePath();
	while( *p != '\0'  ){
		if( _IS_SJIS_1( (unsigned char)*p ) && _IS_SJIS_2( (unsigned char)p[1] ) ){
			p+=2;
		}else if( *p == '\\' ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return pszName;
}

/*!
	�A�C�R���̐ݒ�
	
	�^�C�v�ʐݒ�ɉ����ăE�B���h�E�A�C�R�����t�@�C���Ɋ֘A�Â���ꂽ���C
	�܂��͕W���̂��̂ɐݒ肷��D
	
	@author genta
	@date 2002.09.10
*/
void CEditDoc::SetDocumentIcon(void)
{
	HICON	hIconBig, hIconSmall;
	
	if( m_bGrepMode )	// Grep���[�h�̎��̓A�C�R����ύX���Ȃ�
		return;
	
	if( GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcEditWnd->GetRelatedIcon( GetFilePath(), &hIconBig, &hIconSmall );
	else
		m_pcEditWnd->GetDefaultIcon( &hIconBig, &hIconSmall );

	m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}

/*!
	�J�����g�t�@�C����MRU�ɓo�^����B
	�u�b�N�}�[�N���ꏏ�ɓo�^����B

	@date 2003.03.30 genta �쐬

*/
void CEditDoc::AddToMRU(void)
{
	EditInfo	fi;
	CMRUFile	cMRU;

	GetEditInfo( &fi );
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );
}


/*!	@brief �w�肳�ꂽ�t�@�C�����J��

	���݂̕ҏW�󋵂ɉ����āC���݂̃E�B���h�E�ɓǂݍ��ނ��C�V�����E�B���h�E���J����
	���邢�͊��ɊJ����Ă���E�B���h�E�Ƀt�H�[�J�X���ڂ������ɂ��邩�����肵�C
	���s����D

	�Ώۃt�@�C���C�G�f�B�^�E�B���h�E�̏󋵂Ɉ˂炸�V�����t�@�C�����J�������ꍇ��
	�g�p����D

	@date 2003.03.30 genta �u���ĊJ���v���痘�p���邽�߂Ɉ����ǉ�
	@date 2004.10.09 CEditView���ړ�
	@date 2007.03.12 maru �d���R�[�h(���d�I�[�v�����������Ȃ�)��CShareData::IsPathOpened�Ɉړ�
*/
void CEditDoc::OpenFile( const char *filename, ECodeType nCharCode, bool bReadOnly )
{
	char		pszPath[_MAX_PATH];
	BOOL		bOpened;
	HWND		hWndOwner;

	/* �u�t�@�C�����J���v�_�C�A���O */
	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_pcEditWnd->m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}
	else {
		//	2007.10.01 genta ���΃p�X���΃p�X�ɕϊ�
		//	�ϊ����Ȃ���IsPathOpened�Ő��������ʂ�����ꂸ�C
		//	����t�@�C���𕡐��J�����Ƃ�����D
		if( ! GetLongFileName( filename, pszPath )){
			//	�t�@�C�����̕ϊ��Ɏ��s
			OkMessage( m_pcEditWnd->m_hWnd,
				_T("�t�@�C�����̕ϊ��Ɏ��s���܂��� [%s]"), filename );
			return;
		}
	}
	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pszPath, &hWndOwner, nCharCode) ){		// �J���Ă���΃A�N�e�B�u�ɂ���
		/* 2007.03.12 maru �J���Ă����Ƃ��̏�����CShareData::IsPathOpened�Ɉړ� */
	}else{
		/* �t�@�C�����J����Ă��Ȃ� */
		/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
//@@@ 2001.12.26 YAZAKI Grep���ʂŖ����ꍇ���܂߂�B
		if( IsFileOpenInThisWindow()
		){
			BOOL bRet;
			/* �t�@�C���ǂݍ��� */
			//	Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
			bRet = FileRead( pszPath, &bOpened, nCharCode, bReadOnly,
							m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange );

			// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
			// 2007.06.27 maru ���łɕҏW�E�B���h�E�͊J���Ă���̂ŁAFileRead���L�����Z�����ꂽ�ꍇ�͊J���}�N���͎��s�s�v
			if( FALSE!=bRet ) RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );
		}else{
			if( strchr( pszPath, ' ' ) ){
				char	szFile2[_MAX_PATH + 3];
				wsprintf( szFile2, "\"%s\"", pszPath );
				strcpy( pszPath, szFile2 );
			}
			/* �V���ȕҏW�E�B���h�E���N�� */
			CControlTray::OpenNewEditor( m_hInstance, m_pcEditWnd->m_hWnd, pszPath, nCharCode, bReadOnly );
		}
	}
	return;
}

/* ����(����)

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
void CEditDoc::FileClose( void )
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	if( !OnFileClose() ){
		return;
	}
	/* �����f�[�^�̃N���A */
	InitDoc();

	/* �S�r���[�̏����� */
	InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption();

	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );

	return;
}

/* ���ĊJ��

	@param filename	[in] �t�@�C����
	@param nCharCode	[in] �����R�[�h
	@param bReadOnly	[in] �ǂݎ���p��

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
void CEditDoc::FileCloseOpen( const char *filename, ECodeType nCharCode, bool bReadOnly )
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	if( !OnFileClose() ){
		return;
	}

	// Mar. 30, 2003 genta
	char	pszPath[_MAX_PATH];

	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_pcEditWnd->m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}

	/* �����f�[�^�̃N���A */
	InitDoc();

	/* �S�r���[�̏����� */
	InitAllView();

	if( !IsValidPath() ){
		CShareData::getInstance()->GetNoNameNumber( m_pcEditWnd->m_hWnd );
	}

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditWnd->UpdateCaption();

	/* �t�@�C�����J�� */
	// Mar. 30, 2003 genta
	// Oct.  9, 2004 genta CEditDoc�ֈړ��������Ƃɂ��ύX
	OpenFile(( filename ? filename : pszPath ), nCharCode, bReadOnly );
}

/*! �㏑���ۑ�

	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�

	@date 2006.12.30 ryoji CEditView::Command_FILESAVE()���珈���{�̂�؂�o��
*/
BOOL CEditDoc::FileSave( bool warnbeep, bool askname )
{

	/* ���ύX�ł��㏑�����邩 */
	if( !m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite
	 && !IsModified()	// �ύX�t���O
	 ){
	 	//	Feb. 28, 2004 genta
	 	//	�ۑ��s�v�ł��x�������o���ė~�����Ȃ��ꍇ������
	 	if( warnbeep ){
			ErrorBeep();
		}
		return TRUE;
	}

	if( !IsValidPath() ){
		if( ! askname ){
			return FALSE;
		}
		//	Feb. 28, 2004 genta SAVEAS�̌��ʂ��������Ԃ���Ă��Ȃ�����
		//	���̏����Ƒg�ݍ��킹��Ƃ��ɖ�肪������
		//return Command_FILESAVEAS_DIALOG();
		FileSaveAs_Dialog();
	}
	else {
		//	Jun.  5, 2004 genta
		//	�ǂݎ���p�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			if( warnbeep ){
				ErrorBeep();
				TopErrorMessage(
					m_pcEditWnd->m_hWnd,
					_T("%s\n\n�͓ǂݎ���p���[�h�ŊJ���Ă��܂��B �㏑���ۑ��͂ł��܂���B\n\n")
					_T("���O��t���ĕۑ�������΂����Ǝv���܂��B"),
					IsValidPath() ? GetFilePath() : _T("(����)")
				);
			}
			return FALSE;
		}

		if( SaveFile( GetFilePath() ) ){	//	m_nCharCode, m_cSaveLineCode��ύX�����ɕۑ�
			/* �L�����b�g�̍s���ʒu��\������ */
			m_pcEditWnd->GetActiveView().ShowCaretPosInfo();
			return TRUE;
		}
	}
	return FALSE;
}

/*! ���O��t���ĕۑ��_�C�A���O

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()���珈���{�̂�؂�o��
*/
BOOL CEditDoc::FileSaveAs_Dialog( void )
{
	//	Aug. 16, 2000 genta
	//	���݂̃t�@�C�����������l�ŗ^���Ȃ�
	//	May 18, 2001 genta
	//	���݂̃t�@�C������^���Ȃ��̂͏㏑���֎~�̎��̂�
	//	�����łȂ��ꍇ�ɂ͌��݂̃t�@�C�����������l�Ƃ��Đݒ肷��B
	char szPath[_MAX_PATH + 1];
	if( IsReadOnly() )
		szPath[0] = '\0';
	else
		strcpy( szPath, GetFilePath() );

	//	Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOM�̗L����^����p�����[�^
	if( SaveFileDialog( szPath, &m_nCharCode, &m_cSaveLineCode, &m_bBomExist ) ){
		//	Jun.  5, 2004 genta
		//	�ǂݎ���p�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
		//	�����ŏ㏑�������̂�h��
		if( m_bReadOnly && strcmp( szPath, GetFilePath()) == 0 ){
			ErrorBeep();
			TopErrorMessage(
				m_pcEditWnd->m_hWnd,
				_T("�ǂݎ���p���[�h�ł͓���t�@�C���ւ̏㏑���ۑ��͂ł��܂���B")
			);
		}
		else {
			//Command_FILESAVEAS( szPath );
			FileSaveAs( szPath );
			m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji ���s�R�[�h�͈����p���Ȃ��i�㏑���ۑ��ł͏�Ɂu�ϊ��Ȃ��v�j
			return TRUE;
		}
	}
	m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji ���s�R�[�h�͈����p���Ȃ��i�㏑���ۑ��ł͏�Ɂu�ϊ��Ȃ��v�j
	return FALSE;
}

/* ���O��t���ĕۑ�

	@param filename	[in] �t�@�C����

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
BOOL CEditDoc::FileSaveAs( const char *filename )
{
	if( SaveFile( filename ) ){
		/* �L�����b�g�̍s���ʒu��\������ */
		m_pcEditWnd->GetActiveView().ShowCaretPosInfo();
		OnChangeSetting();	//	�^�C�v�ʐݒ�̕ύX���w���B
		//	�ăI�[�v��
		//	Jul. 26, 2003 ryoji ���݊J���Ă���̂Ɠ����R�[�h�ŊJ������
		ReloadCurrentFile( m_nCharCode, false );
		return TRUE;
	}
	return FALSE;
}

/*[EOF]*/
