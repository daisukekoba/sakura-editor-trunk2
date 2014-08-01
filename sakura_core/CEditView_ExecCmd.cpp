/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date   2005/09/02 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, �����, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta, maru
	Copyright (C) 2007, ryoji, ���イ��, maru, genta, Moca, nasukoji, D.S.Koba
	Copyright (C) 2008, ryoji, nasukoji, bosagami, Moca, genta
	Copyright (C) 2009, nasukoji, ryoji, syat
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditView.h"
#include "Debug.h"
#include "charcode.h"
#include "CEditWnd.h"
#include "CDlgCancel.h"
#include "etc_uty.h"

/*!	@brief	�O���R�}���h�̎��s

	@param[in] pszCmd �R�}���h���C��
	@param[in] nFlgOpt �I�v�V����
		@li	0x01	�W���o�͂𓾂�
		@li	0x02	�W���o�͂̂�_�C���N�g��i����=�A�E�g�v�b�g�E�B���h�E / �L��=�ҏW���̃E�B���h�E�j
		@li	0x04	�ҏW���t�@�C����W�����͂�

	@note	�q�v���Z�X�̕W���o�͎擾�̓p�C�v���g�p����
	@note	�q�v���Z�X�̕W�����͂ւ̑��M�͈ꎞ�t�@�C�����g�p

	@author	N.Nakatani
	@date	2001/06/23
	@date	2001/06/30	GAE
	@date	2002/01/24	YAZAKI	1�o�C�g��肱�ڂ��\����������
	@date	2003/06/04	genta
	@date	2004/09/20	naoh	�����͌��₷���E�E�E
	@date	2004/01/23	genta
	@date	2004/01/28	Moca	���s�R�[�h�����������̂�h��
	@date	2007/03/18	maru	�I�v�V�����̊g��
	@date	2009/02/21	ryoji	�ҏW�֎~�̂Ƃ��͕ҏW���E�B���h�E�ւ͏o�͂��Ȃ��i�w�莞�̓A�E�g�v�b�g�ցj
*/
void CEditView::ExecCmd( const char* pszCmd, const int nFlgOpt )
{
	char				cmdline[1024];
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
	CDlgCancel				cDlgCancel;

	bool bEditable = m_pcEditDoc->IsEditable();	// �ҏW�֎~

	//	From Here 2006.12.03 maru �������g���̂���
	BOOL	bGetStdout;			//	�q�v���Z�X�̕W���o�͂𓾂�
	BOOL	bToEditWindow;		//	TRUE=�ҏW���̃E�B���h�E / FALSAE=�A�E�g�v�b�g�E�B���h�E
	BOOL	bSendStdin;			//	�ҏW���t�@�C�����q�v���Z�XSTDIN�ɓn��
	
	bGetStdout = nFlgOpt & 0x01 ? TRUE : FALSE;
	bToEditWindow = ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;
	bSendStdin = nFlgOpt & 0x04 ? TRUE : FALSE;
	//	To Here 2006.12.03 maru �������g���̂���

	// �ҏW���̃E�B���h�E�ɏo�͂���ꍇ�̑I��͈͏����p	/* 2007.04.29 maru */
	CLayoutPoint ptFrom = CLayoutPoint( 0, 0 );
	BOOL	bBeforeTextSelected = IsTextSelected();
	if (bBeforeTextSelected){
		ptFrom = m_sSelect.m_ptFrom;
	}
	
	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE ) {
		//�G���[�B�΍�����
		return;
	}
	//hStdOutRead�̂ق��͎q�v���Z�X�ł͎g�p����Ȃ��̂Ōp���s�\�ɂ���i�q�v���Z�X�̃��\�[�X�𖳑ʂɑ��₳�Ȃ��j
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), &hStdOutRead,					// �V�����p���s�\�n���h�����󂯎��	// 2007.01.31 ryoji
				0, FALSE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );	// ���̌p���\�n���h���� DUPLICATE_CLOSE_SOURCE �ŕ���	// 2007.01.31 ryoji

	// From Here 2007.03.18 maru �q�v���Z�X�̕W�����̓n���h��
	// CDocLineMgr::WriteFile�ȂǊ����̃t�@�C���o�͌n�̊֐��̂Ȃ��ɂ�
	// �t�@�C���n���h����Ԃ��^�C�v�̂��̂��Ȃ��̂ŁA��U�����o���Ă���
	// �ꎞ�t�@�C�������ŃI�[�v�����邱�ƂɁB
	hStdIn = NULL;
	if(bSendStdin){	/* ���ݕҏW���̃t�@�C�����q�v���Z�X�̕W�����͂� */
		TCHAR		szPathName[MAX_PATH];
		TCHAR		szTempFileName[MAX_PATH];
		int			nFlgOpt;

		GetTempPath( MAX_PATH, szPathName );
		GetTempFileName( szPathName, TEXT("skr_"), 0, szTempFileName );
		DEBUG_TRACE( _T("CEditView::ExecCmd() TempFilename=[%s]\n"), szTempFileName );
		
		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* �I��͈͂��o�� */
		
		if( FALSE == Command_PUTFILE( szTempFileName, CODE_SJIS, nFlgOpt) ){	// �ꎞ�t�@�C���o��
			hStdIn = NULL;
		} else {
			hStdIn = CreateFile( szTempFileName, GENERIC_READ,		// �q�v���Z�X�ւ̌p���p�Ƀt�@�C�����J��
					0, &sa, OPEN_EXISTING,
					FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
					NULL );
			if(hStdIn == INVALID_HANDLE_VALUE) hStdIn = NULL;
		}
	}
	
	if (hStdIn == NULL) {	/* �W�����͂𐧌䂵�Ȃ��ꍇ�A�܂��͈ꎞ�t�@�C���̐����Ɏ��s�����ꍇ */
		bSendStdin = FALSE;
		hStdIn = GetStdHandle( STD_INPUT_HANDLE );
	}
	// To Here 2007.03.18 maru �q�v���Z�X�̕W�����̓n���h��
	
	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb = sizeof(STARTUPINFO);
	if( bGetStdout || bSendStdin ) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle( STD_OUTPUT_HANDLE );
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle( STD_ERROR_HANDLE );
	}

	//�R�}���h���C�����s
	strcpy( cmdline, pszCmd );
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//���s�Ɏ��s�����ꍇ�A�R�}���h���C���x�[�X�̃A�v���P�[�V�����Ɣ��f����
		// command(9x) �� cmd(NT) ���Ăяo��

		// 2010.08.27 Moca �V�X�e���f�B���N�g���t��
		TCHAR szCmdDir[_MAX_PATH];
		if( IsWin32NT() ){
			::GetSystemDirectory(szCmdDir, sizeof(szCmdDir));
		}else{
			::GetWindowsDirectory(szCmdDir, sizeof(szCmdDir));
		}

		//�R�}���h���C��������쐬
		wsprintf( cmdline, "\"%s\\%s\" %s%s",
				szCmdDir,
				( IsWin32NT() ? "cmd.exe" : "command.com" ),
				( bGetStdout ? "/C " : "/K " ), pszCmd );
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, "�R�}���h���s�͎��s���܂����B", MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	// �t�@�C���S�̂ɑ΂���t�B���^����
	//	���ݕҏW���̃t�@�C������̃f�[�^������������уf�[�^��荞�݂�
	//	�w�肳��Ă��āC���͈͑I�����s���Ă��Ȃ��ꍇ��
	//	�u���ׂđI���v����Ă�����̂Ƃ��āC�ҏW�f�[�^�S�̂�
	//	�R�}���h�̏o�͌��ʂƒu��������D
	//	2007.05.20 maru
	if((FALSE == bBeforeTextSelected) && bSendStdin && bGetStdout && bToEditWindow){
		CLayoutRange sRange = CLayoutRange( CLayoutPoint( 0, 0 ), CLayoutPoint( 0, m_pcEditDoc->m_cLayoutMgr.GetLineCount()) );
		SetSelectArea( sRange );
		DeleteData( true );
	}

	// hStdOutWrite �� CreateProcess() �Ōp�������̂Őe�v���Z�X�ł͗p�ς�
	// hStdIn���e�v���Z�X�ł͎g�p���Ȃ����AWin9x�n�ł͎q�v���Z�X���I�����Ă���
	// �N���[�Y����悤�ɂ��Ȃ��ƈꎞ�t�@�C���������폜����Ȃ�
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta ��dclose��h��

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//���f�_�C�A���O�\��
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		//���s�����R�}���h���C����\��
		// 2004.09.20 naoh �����͌��₷���E�E�E
		if (FALSE==bToEditWindow) {	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CShareData::getInstance()->MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CShareData::getInstance()->MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			CShareData::getInstance()->TraceOut( "\r\n%s\r\n", "#============================================================" );
			CShareData::getInstance()->TraceOut( "#DateTime : %s %s\r\n", szTextDate, szTextTime );
			CShareData::getInstance()->TraceOut( "#CmdLine  : %s\r\n", pszCmd );
			CShareData::getInstance()->TraceOut( "#%s\r\n", "==============================" );
		}
		
		
		//���s���ʂ̎�荞��
		do {
			//�v���Z�X���I�����Ă��Ȃ����m�F
			// Jun. 04, 2003 genta CPU��������炷���߂�200msec�҂�
			// ���̊ԃ��b�Z�[�W�������؂�Ȃ��悤�ɑ҂�����WaitForSingleObject����
			// MsgWaitForMultipleObject�ɕύX
			// Jan. 23, 2004 genta
			// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
			// ��~���Ă��܂����߁C�҂����Ԃ�200ms����20ms�Ɍ��炷
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 20, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
					//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//�������̃��[�U�[������\�ɂ���
					if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
						break;
					}
					break;
				default:
					break;
			}
			//���f�{�^�������`�F�b�N
			if( cDlgCancel.IsCanceled() ){
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				if (FALSE==bToEditWindow) {	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
					//�Ō�Ƀe�L�X�g��ǉ�
					const char* pszText = "\r\n���f���܂����B\r\n";
					CShareData::getInstance()->TraceOut( "%s", pszText );
				}
				break;
			}
			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
				while( new_cnt > 0 ) {												//�ҋ@���̂��̂�����
					if( new_cnt >= sizeof(work)-2 ) {							//�p�C�v����ǂݏo���ʂ𒲐�
						new_cnt = sizeof(work)-2;
					}
					ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					read_cnt += bufidx;													//work���̎��ۂ̃T�C�Y�ɂ���
					
					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while�ǉ��̂��ߐ����ύX
						break;
					}
					//�ǂݏo������������`�F�b�N����
					// \r\n �� \r �����Ƃ������̑��o�C�g�������o�͂���̂�h���K�v������
					//@@@ 2002.1.24 YAZAKI 1�o�C�g��肱�ڂ��\�����������B
					//	Jan. 28, 2004 Moca �Ō�̕����͂��ƂŃ`�F�b�N����
					for( j=0; j<(int)read_cnt - 1; j++ ) {
						//	2007.09.10 ryoji
						if( CMemory::GetSizeOfChar(work, read_cnt, j) == 2 ) {
							j++;
						} else {
							if( work[j] == '\r' && work[j+1] == '\n' ) {
								j++;
							} else if( work[j] == '\n' && work[j+1] == '\r' ) {
								j++;
							}
						}
					}
					//	From Here Jan. 28, 2004 Moca
					//	���s�R�[�h�����������̂�h��
					if( j == read_cnt - 1 ){
						if( _IS_SJIS_1(work[j]) ) {
							j = read_cnt + 1; // �҂�����o�͂ł��Ȃ����Ƃ��咣
						}else if( work[j] == '\r' || work[j] == '\n' ) {
							// CRLF,LFCR�̈ꕔ�ł͂Ȃ����s�������ɂ���
							// ���̓ǂݍ��݂ŁACRLF,LFCR�̈ꕔ�ɂȂ�\��������
							j = read_cnt + 1;
						}else{
							j = read_cnt;
						}
					}
					//	To Here Jan. 28, 2004 Moca
					if( j == (int)read_cnt ) {	//�҂�����o�͂ł���ꍇ
						//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
						if (FALSE==bToEditWindow) {
							work[read_cnt] = '\0';
							CShareData::getInstance()->TraceOut( "%s", work );
						} else {
							Command_INSTEXT( false, work, read_cnt, TRUE);
						}
						bufidx = 0;
						//DEBUG_TRACE( _T("ExecCmd: No leap character\n"));
					} else {
						char tmp = work[read_cnt-1];
						//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
						if (FALSE==bToEditWindow) {
							work[read_cnt-1] = '\0';
							CShareData::getInstance()->TraceOut( "%s", work );
						} else {
							Command_INSTEXT( false, work, read_cnt-1, TRUE);
						}
						work[0] = tmp;
						bufidx = 1;
						DEBUG_TRACE( _T("ExecCmd: Carry last character [%d]\n"), tmp );
					}
					// Jan. 23, 2004 genta
					// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
					// ��~���Ă��܂����߁C�o�b�t�@����ɂȂ�܂łǂ�ǂ�ǂݏo���D
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0);
				}
			}
		} while( bLoopFlag || new_cnt > 0 );
		
		if (FALSE==bToEditWindow) {	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
			work[bufidx] = '\0';
			CShareData::getInstance()->TraceOut( "%s", work );	/* �Ō�̕����̏��� */
			//	Jun. 04, 2003 genta	�I���R�[�h�̎擾�Əo��
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			CShareData::getInstance()->TraceOut( "\r\n�I���R�[�h: %d\r\n", result );

			// 2004.09.20 naoh �I���R�[�h��1�ȏ�̎��̓A�E�g�v�b�g���A�N�e�B�u�ɂ���
			if(result > 0) ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
		}
		else {						//	2006.12.03 maru �ҏW���̃E�B���h�E�ɏo�͎��͍Ō�ɍĕ`��
			Command_INSTEXT( false, work, bufidx, TRUE);	/* �Ō�̕����̏��� */
			if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
				CLayoutRange sRange;
				sRange.m_ptFrom = ptFrom;
				sRange.m_ptTo = m_ptCaretPos;
				SetSelectArea( sRange );
				DrawSelectArea();
			}
			RedrawAll();
		}
	}


finish:
	//�I������
	if(bSendStdin) CloseHandle( hStdIn );	/* 2007.03.18 maru �W�����͂̐���̂��� */
	if(hStdOutWrite) CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread ) CloseHandle( pi.hThread );
}

/*[EOF]*/
