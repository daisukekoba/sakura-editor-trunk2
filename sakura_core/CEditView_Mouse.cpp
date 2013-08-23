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
#include <process.h> // _beginthreadex
#include "CEditView.h"
#include "Debug.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "os.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpeBlk.h"///
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/* �}�E�X���{�^������ */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearch���L�����Z������
	if (m_nISearchMode > 0 ){
		ISearchExit();
	}

	int			nCaretPosY_Old;
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;

	CLayoutRange sRange;

	int			nIdx;

	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	�g���v���N���b�N�ł��邱�Ƃ�����
	int			nFuncID = 0;				// 2007.11.30 nasukoji	�}�E�X���N���b�N�ɑΉ�����@�\�R�[�h

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //�t�H�[�J�X���Ȃ��Ƃ�
		return;
	}
	nCaretPosY_Old = m_ptCaretPos.y;

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}
	else{
		m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
	}

	// 2007.11.30 nasukoji	�g���v���N���b�N���`�F�b�N
	tripleClickMode = CheckTripleClick(xPos, yPos);

	if(tripleClickMode){
		// �}�E�X���g���v���N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[5]�ɓ����Ă���
		nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// ���蓖�ċ@�\�����̎��̓g���v���N���b�N OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}

	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );

	// OLE�ɂ��h���b�O & �h���b�v���g��
	// 2007.11.30 nasukoji	�g���v���N���b�N���̓h���b�O���J�n���Ȃ�
	if( !tripleClickMode && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){
		if( m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource ){		/* OLE�ɂ��h���b�O���ɂ��邩 */
			/* �s�I���G���A���h���b�O���� */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// �J�[�\���ʒuX
				nNewY		// �J�[�\���ʒuY
				)
			){
				POINT ptWk = {xPos, yPos};
				::ClientToScreen(m_hWnd, &ptWk);
				if( !::DragDetect(m_hWnd, ptWk) ){
					// �h���b�O�J�n�����𖞂����Ȃ������̂ŃN���b�N�ʒu�ɃJ�[�\���ړ�����
					if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						DisableSelectArea( true );
					}
//@@@ 2002.01.08 YAZAKI �t���[�J�[�\��OFF�ŕ����s�I�����A�s�̌����N���b�N����Ƃ����ɃL�����b�g���u����Ă��܂��o�O�C��
					/* �J�[�\���ړ��B */
					if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
						if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
							MoveCursorToPoint( xPos, yPos );
						}else
						if( xPos < m_nViewAlignLeft ){
							MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
						}
					}
					return;
				}
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					DWORD dwEffectsSrc = (
							m_pcEditDoc->IsReadOnly()	// �ǂݎ���p
							|| ( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )	// �㏑���֎~
						)? DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pcEditDoc->m_cOpeBuf.GetCurrentPointer();
					m_pcEditDoc->SetDragSourceView( this );
					CDataObject data( cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), m_bBeginBoxSelect );	// 2008.03.26 ryoji �e�L�X�g���A��`�̎w���ǉ�
					dwEffects = data.DragDrop( TRUE, dwEffectsSrc );
					m_pcEditDoc->SetDragSourceView( NULL );
//					MYTRACE( _T("dwEffects=%d\n"), dwEffects );
					if( m_pcEditDoc->m_cOpeBuf.GetCurrentPointer() == nOpe ){	// �h�L�������g�ύX�Ȃ����H	// 2007.12.09 ryoji
						m_pcEditDoc->SetActivePane( m_nMyIndex );
						if( DROPEFFECT_MOVE == (dwEffectsSrc & dwEffects) ){
							// �ړ��͈͂��폜����
							// �h���b�v�悪�ړ����������������h�L�������g�ɂ����܂ŕύX������
							// ���h���b�v��͊O���̃E�B���h�E�ł���
							if( NULL == m_pcOpeBlk ){
								m_pcOpeBlk = new COpeBlk;
							}

							// �I��͈͂��폜
							DeleteData( true );

							// �A���h�D�o�b�t�@�̏���
							SetUndoBuffer();
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALT�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.10.10 nasukoji	�g���v���N���b�N�Ή�
	if( GetKeyState_Alt() &&( ! tripleClickMode)){
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		// �}�E�X�͈͑I��O��ʒu(X���W)
		m_nMouseRollPosYOld = yPos;		// �}�E�X�͈͑I��O��ʒu(Y���W)
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = true;			/* �͈͑I�� */
		m_bBeginBoxSelect = true;		/* ��`�͈͑I�� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */

		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
		m_cUnderLine.CaretUnderLineOFF( true );
		m_cUnderLine.Lock();
		if( xPos < m_nViewAlignLeft ){
			/* �J�[�\�����ړ� */
			Command_DOWN( true, false );
		}
	}
	else{
		/* �J�[�\���ړ� */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}
		else if( yPos < m_nViewAlignTop ){
			//	���[���N���b�N
			return;
		}
		else {
			return;
		}

		/* �}�E�X�̃L���v�`���Ȃ� */
		m_nMouseRollPosXOld = xPos;		/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;		/* �}�E�X�͈͑I��O��ʒu(Y���W) */
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = true;			/* �͈͑I�� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */
		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice


		if(tripleClickMode){		// 2007.10.10 nasukoji	�g���v���N���b�N����������
			// 1�s�I���łȂ��ꍇ�͑I�𕶎��������
			// 2007.11.05 nasukoji	�g���v���N���b�N��1�s�I���łȂ��Ă��N�A�h���v���N���b�N��L���Ƃ���
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, xPos, yPos );	// �����ō��{�^���A�b�v�������Ƃɂ���

				if( IsTextSelected() )		// �e�L�X�g���I������Ă��邩
					DisableSelectArea( true );		// ���݂̑I��͈͂��I����Ԃɖ߂�
			}

			// 2007.10.10 nasukoji	�P��̓r���Ő܂�Ԃ���Ă���Ɖ��̍s���I������Ă��܂����Ƃւ̑Ώ�
			MoveCursorToPoint( xPos, yPos );	// �J�[�\���ړ�

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1�s�I���łȂ��ꍇ�͂����Ŕ�����i���̑I���R�}���h�̎����ƂȂ邩���j
			if(F_SELECTLINE != nFuncID)
				return;

			// 2007.10.13 nasukoji	�I��������̂������i[EOF]�݂̂̍s�j���͒ʏ�N���b�N�Ɠ�������
			if(( ! IsTextSelected() )&&( m_ptCaretPos_PHY.y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() )){
				BeginSelectArea();				// ���݂̃J�[�\���ʒu����I�����J�n����
				m_bBeginLineSelect = false;		// �s�P�ʑI�� OFF
			}
		}else
		/* �I���J�n���� */
		/* SHIFT�L�[��������Ă����� */
		if(GetKeyState_Shift()){
			if( IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
				if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( true );
					/* ���݂̃J�[�\���ʒu����I�����J�n���� */
					BeginSelectArea( );
				}
				else{
				}
			}
			else{
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				BeginSelectArea( );
			}

			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}
		else{
			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( true );
			}
			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea( );
		}


		/******* ���̎��_�ŕK�� true == IsTextSelected() �̏�ԂɂȂ� ****:*/
		if( !IsTextSelected() ){
			WarningMessage( m_hWnd, _T("�o�O���Ă�") );
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_ptCaretPos.x,	// �J�[�\���ʒuX
			m_ptCaretPos.y	// �J�[�\���ʒuY
		);


		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );


		// CTRL�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.10.10 nasukoji	�g���v���N���b�N�Ή�
		if( GetKeyState_Control() &&( ! tripleClickMode)){
			m_bBeginWordSelect = true;		/* �P��P�ʑI�� */
			if( !IsTextSelected() ){
				/* ���݈ʒu�̒P��I�� */
				if ( Command_SELECTWORD() ){
					m_sSelectBgn.m_ptFrom.y = m_sSelect.m_ptFrom.y;	/* �͈͑I���J�n�s(���_) */
					m_sSelectBgn.m_ptFrom.x = m_sSelect.m_ptFrom.x;	/* �͈͑I���J�n��(���_) */
					m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;		/* �͈͑I���J�n�s(���_) */
					m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;		/* �͈͑I���J�n��(���_) */
				}
			}else{

				/* �I��̈�`�� */
				DrawSelectArea();


				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptFrom.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptFrom.x );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptFrom.y, nIdx, &sRange, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
						sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
						sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );


						nWork = IsCurrentPositionSelected(
							sRange.m_ptFrom.x,	// �J�[�\���ʒuX
							sRange.m_ptFrom.y	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptFrom = sRange.m_ptFrom;
							if( 1 == nWorkRel ){
								m_sSelectBgn = sRange;
							}
						}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.m_ptTo.y, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_sSelect.m_ptTo.x );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_sSelect.m_ptTo.y, nIdx,
						&sRange, NULL, NULL )
					){
						// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
						sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
						sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );

						nWork = IsCurrentPositionSelected(
							sRange.m_ptFrom.x,	// �J�[�\���ʒuX
							sRange.m_ptFrom.y	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_sSelect.m_ptTo = sRange.m_ptFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							sRange.m_ptTo.x,	// �J�[�\���ʒuX
							sRange.m_ptTo.y		// �J�[�\���ʒuY
						) ){
							m_sSelect.m_ptTo = sRange.m_ptTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_sSelectBgn = sRange;
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* �I��̈�`�� */
				DrawSelectArea();
			}
		}
		// �s�ԍ��G���A���N���b�N����
		// 2007.12.08 nasukoji	�V�t�g�L�[�������Ă���ꍇ�͍s���N���b�N�Ƃ��Ĉ���
		if(( xPos < m_nViewAlignLeft )&& !GetKeyState_Shift() ){
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_bBeginLineSelect = true;

			// 2002.10.07 YAZAKI �܂�Ԃ��s���C���f���g���Ă���Ƃ��ɑI�������������o�O�̑΍�
			// �P�s����ʕ����������ƍ��E�ɃX�N���[�����Ă�������������Ȃ�̂Ō�őS�̂��ĕ`��	// 2008.05.20 ryoji
			bool bDrawSwitchOld = m_bDrawSWITCH;
			BOOL bDrawAfter = FALSE;
			if( bDrawSwitchOld ){
				const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_ptCaretPos.y );
				if( pcLayout ){
					int nColumn = LineIndexToColmn( pcLayout, pcLayout->GetLengthWithoutEOL() );
					bDrawAfter = (nColumn + SCROLLMARGIN_RIGHT >= m_nViewColNum);
					if( bDrawAfter ){
						m_bDrawSWITCH = false;
					}
				}
			}
			Command_GOLINEEND( true, 0 );
			Command_RIGHT( true, false, false );
			if( bDrawSwitchOld && bDrawAfter ){
				m_bDrawSWITCH = true;
				Redraw();
			}

			//	Apr. 14, 2003 genta
			//	�s�ԍ��̉����N���b�N���ăh���b�O���J�n����Ƃ��������Ȃ�̂��C��
			//	�s�ԍ����N���b�N�����ꍇ�ɂ�ChangeSelectAreaByCurrentCursor()�ɂ�
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.y��-1���ݒ肳��邪�A���
			//	Command_GOLINEEND(), Command_RIGHT()�ɂ���čs�I�����s����B
			//	�������L�����b�g�������ɂ���ꍇ�ɂ̓L�����b�g���ړ����Ȃ��̂�
			//	m_sSelect.m_ptTo.x/m_sSelect.m_ptTo.y��-1�̂܂܎c���Ă��܂��A���ꂪ
			//	���_�ɐݒ肳��邽�߂ɂ��������Ȃ��Ă����B
			//	�Ȃ̂ŁA�͈͑I�����s���Ă��Ȃ��ꍇ�͋N�_�����̐ݒ���s��Ȃ��悤�ɂ���
			if( IsTextSelected() ){
				m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;	/* �͈͑I���J�n�s(���_) */
				m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;	/* �͈͑I���J�n��(���_) */
			}
		}
		else{
			/* URL���N���b�N���ꂽ��I�����邩 */
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL ){

				int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
				int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				int			nUrlLen;	// URL�̒���(�o�C�g��)
				// �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
				bool bIsUrl = IsCurrentPositionURL(
					m_ptCaretPos.x,	// �J�[�\���ʒuX
					m_ptCaretPos.y,	// �J�[�\���ʒuY
					&nUrlLine,		// URL�̍s(���s�P��)
					&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
					&nUrlLen,		// URL�̒���(�o�C�g��)
					NULL			// URL�󂯎���
				);
				if( bIsUrl ){
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( true );

					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						2002/04/08 YAZAKI �����ł��킩��₷���B
					*/
					CLayoutRange sRangeB;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn          , nUrlLine, &sRangeB.m_ptFrom.x, &sRangeB.m_ptFrom.y );
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nUrlIdxBgn + nUrlLen, nUrlLine, &sRangeB.m_ptTo.x,   &sRangeB.m_ptTo.y );

					m_sSelectBgn = sRangeB;
					m_sSelect = sRangeB;

					/* �I��̈�`�� */
					DrawSelectArea();
				}
			}
		}
	}
}


/*!	�g���v���N���b�N�̃`�F�b�N
	@brief �g���v���N���b�N�𔻒肷��
	
	2��ڂ̃N���b�N����3��ڂ̃N���b�N�܂ł̎��Ԃ��_�u���N���b�N���Ԉȓ��ŁA
	�����̎��̃N���b�N�ʒu�̂��ꂪ�V�X�e�����g���b�N�iX:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK�j�̒l�i�s�N�Z���j�ȉ��̎��g���v���N���b�N�Ƃ���B
	
	@param[in] xPos		�}�E�X�N���b�NX���W
	@param[in] yPos		�}�E�X�N���b�NY���W
	@return		�g���v���N���b�N�̎���TRUE��Ԃ�
	�g���v���N���b�N�łȂ�����FALSE��Ԃ�

	@note	m_dwTripleClickCheck��0�łȂ����Ƀ`�F�b�N���[�h�Ɣ��肷�邪�APC��
			�A���ғ����Ă���ꍇ49.7�����ɃJ�E���^��0�ɂȂ�ׁA�킸���ȉ\��
			�ł��邪�g���v���N���b�N������ł��Ȃ���������B
	
	@date 2007.10.02 nasukoji	�V�K�쐬
	@date 2007.10.11 nasukoji	���s������荶���łȂ��ƃg���v���N���b�N���F������Ȃ��s����C��
	@date 2007.10.13 nasukoji	�s�ԍ��\���G���A�̃g���v���N���b�N�͒ʏ�N���b�N�Ƃ��Ĉ���
*/
BOOL CEditView::CheckTripleClick( int xPos, int yPos )
{

	// �g���v���N���b�N�`�F�b�N�L���łȂ��i�������Z�b�g����Ă��Ȃ��j
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// �O��N���b�N�Ƃ̃N���b�N�ʒu�̂�����Z�o
	int dxpos = m_nMouseRollPosXOld - xPos;
	int dypos = m_nMouseRollPosYOld - yPos;

	if(dxpos < 0)
		dxpos = -dxpos;		// ��Βl��

	if(dypos < 0)
		dypos = -dypos;		// ��Βl��

	// �s�ԍ��\���G���A�łȂ��A���N���b�N�v���X����_�u���N���b�N���Ԉȓ��A
	// ���_�u���N���b�N�̋��e����s�N�Z���ȉ��̂���̎��g���v���N���b�N�Ƃ���
	//	2007.10.12 genta/dskoba �V�X�e���̃_�u���N���b�N���x�C���ꋖ�e�ʂ��擾
	if( (xPos >= m_nViewAlignLeft)&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dxpos <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dypos <= GetSystemMetrics(SM_CYDOUBLECLK)))
	{
		result = TRUE;
	}else{
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}
	
	return result;
}

/* �}�E�X�E�{�^������ */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// �J�[�\���ʒuX
		nNewY		// �J�[�\���ʒuY
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;
}

/* �}�E�X�E�{�^���J�� */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[1]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* �E�N���b�N���j���[ */
//	Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 �}�E�X���{�^���Ή�
/*!
	@brief �}�E�X���{�^�����������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.11 novice �V�K�쐬
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X���{�^�����J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	
	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnMBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_CENTER &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[������
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X���{�^���ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[2]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
}


// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	@brief �}�E�X�T�C�h�{�^��1���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X�T�C�h�{�^��1���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnXLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_LEFTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�T�C�h�{�^��1�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[3]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.12 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.12 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void CEditView::OnXRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsPageScrollByWheel() )
	{
		// �z�C�[������ɂ��y�[�W�X�N���[�������OFF
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( FALSE );
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel == MOUSEFUNCTION_RIGHTSIDE &&
	    m_pcEditDoc->m_pcEditWnd->IsHScrollByWheel() )
	{
		// �z�C�[������ɂ�鉡�X�N���[�������OFF
		m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( FALSE );
		return;
	}

	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	/* �}�E�X�T�C�h�{�^��2�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[4]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}

/* �}�E�X�ړ��̃��b�Z�[�W���� */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	CLayoutRange	sSelect_Old = m_sSelect;

	if( !m_bBeginSelect ){	/* �͈͑I�� */
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: �⊮�E�B���h�E���\������Ă��Ȃ�
		if(!m_bHokan){
			/* ����Tip���N������Ă��� */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* ����Tip������ */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
			}
		}
		/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
		int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		int			nUrlLen;	// URL�̒���(�o�C�g��)


		/* �I���e�L�X�g�̃h���b�O���� */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
				/* ���W�w��ɂ��J�[�\���ړ� */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}
		else{
			/* �s�I���G���A? */
			if( xPos < m_nViewAlignLeft || yPos < m_nViewAlignTop ){	//	2002/2/10 aroka
				/* ���J�[�\�� */
				if( yPos >= m_nViewAlignTop )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
			 && TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource /* OLE�ɂ��h���b�O���ɂ��邩 */
			 && 0 == IsCurrentPositionSelected(						/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
				nNewX,	// �J�[�\���ʒuX
				nNewY	// �J�[�\���ʒuY
				)
			){
				/* ���J�[�\�� */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}
			/* �J�[�\���ʒu��URL���L��ꍇ */
			else if(
				IsCurrentPositionURL(
				nNewX,			// �J�[�\���ʒuX
				nNewY,			// �J�[�\���ʒuY
				&nUrlLine,		// URL�̍s(���s�P��)
				&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				&nUrlLen,		// URL�̒���(�o�C�g��)
				NULL			// URL�󂯎���
			) ){
				/* ��J�[�\�� */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* �A�C�r�[�� */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
		/* ���W�w��ɂ��J�[�\���ړ� */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	}
	else{
		/* ���W�w��ɂ��J�[�\���ړ� */
		if(( xPos < m_nViewAlignLeft || m_dwTripleClickCheck )&& m_bBeginLineSelect ){		// 2007.10.02 nasukoji	�s�P�ʑI��
			// 2007.10.13 nasukoji	������̍s�I�������}�E�X�J�[�\���̈ʒu�̍s���I�������悤�ɂ���
			int nNewY = yPos;
			int nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;		// 1�s�̍���

			// �I���J�n�s�ȉ��ւ̃h���b�O����1�s���ɃJ�[�\�����ړ�����
			if( m_nViewTopLine + (yPos - m_nViewAlignTop) / nLineHeight >= m_sSelectBgn.m_ptTo.y )
				nNewY += nLineHeight;

			// �J�[�\�����ړ�
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , nNewY );

			// 2007.10.13 nasukoji	2.5�N���b�N�ɂ��s�P�ʂ̃h���b�O
			if( m_dwTripleClickCheck ){
				int nSelectStartLine = m_sSelectBgn.m_ptTo.y;

				// �I���J�n�s�ȏ�Ƀh���b�O����
				if( m_ptCaretPos.y <= nSelectStartLine ){
					Command_GOLINETOP( true, 0x09 );	// ���s�P�ʂ̍s���ֈړ�
				}else{
					int nCaretPosX;
					int nCaretPosY;

					int nCaretPrevPosX_PHY;
					int nCaretPrevPosY_PHY = m_ptCaretPos_PHY.y;

					// �I���J�n�s��艺�ɃJ�[�\�������鎞��1�s�O�ƕ����s�ԍ��̈Ⴂ���`�F�b�N����
					// �I���J�n�s�ɃJ�[�\�������鎞�̓`�F�b�N�s�v
					if(m_ptCaretPos.y > nSelectStartLine){
						// 1�s�O�̕����s���擾����
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							0, m_ptCaretPos.y - 1, &nCaretPrevPosX_PHY, &nCaretPrevPosY_PHY
						);
					}

					// �O�̍s�Ɠ��������s
					if( nCaretPrevPosY_PHY == m_ptCaretPos_PHY.y ){
						// 1�s��̕����s���烌�C�A�E�g�s�����߂�
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
							0, m_ptCaretPos_PHY.y + 1, &nCaretPosX, &nCaretPosY
						);

						// �J�[�\�������̕����s���ֈړ�����
						nScrollRowNum = MoveCursor( nCaretPosX, nCaretPosY, true );
					}
				}
			}
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */

		if( !m_bBeginWordSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
		}else{
			CLayoutRange sSelect;

			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_ptCaretPos.x,
				(int)m_ptCaretPos.y,
				&sSelect
			);
			/* �I��͈͂ɕύX�Ȃ� */
			if( sSelect_Old.m_ptFrom.y == sSelect.m_ptFrom.y
			 && sSelect_Old.m_ptFrom.x == sSelect.m_ptFrom.x
			 && sSelect_Old.m_ptTo.y == sSelect.m_ptTo.y
			 && sSelect_Old.m_ptTo.x == sSelect.m_ptTo.x
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_ptCaretPos.x,
					(int)m_ptCaretPos.y
				);
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
				CLayoutRange sRange;

				/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_ptCaretPos.y, nIdx, &sRange, NULL, NULL )
				){
					/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptFrom.y, &nLineLen, &pcLayout );
					sRange.m_ptFrom.x = LineIndexToColmn( pcLayout, sRange.m_ptFrom.x );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sRange.m_ptTo.y, &nLineLen, &pcLayout );
					sRange.m_ptTo.x = LineIndexToColmn( pcLayout, sRange.m_ptTo.x );

					nWorkF = IsCurrentPositionSelectedTEST(
						sRange.m_ptFrom.x,	// �J�[�\���ʒuX
						sRange.m_ptFrom.y,	// �J�[�\���ʒuY
						sSelect
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						sRange.m_ptTo.x,	// �J�[�\���ʒuX
						sRange.m_ptTo.y,	// �J�[�\���ʒuY
						sSelect
					);
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* �n�_���O���Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptFrom.x, sRange.m_ptFrom.y );
					}
					else if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* �I�_������Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptTo.x, sRange.m_ptTo.y );
					}
					else if( sSelect_Old.m_ptFrom.y == sSelect.m_ptFrom.y
					 && sSelect_Old.m_ptFrom.x == sSelect.m_ptFrom.x
					){
						/* �n�_�����ύX���O���ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptTo.x, sRange.m_ptTo.y );
					}
					else if( sSelect_Old.m_ptTo.y == sSelect.m_ptTo.y
					 && sSelect_Old.m_ptTo.x == sSelect.m_ptTo.x
					){
						/* �I�_�����ύX������ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( sRange.m_ptFrom.x, sRange.m_ptFrom.y );
					}
				}else{
					/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
					ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
				}
			}else{
				/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
				ChangeSelectAreaByCurrentCursor( m_ptCaretPos.x, m_ptCaretPos.y );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* �}�E�X�z�C�[���̃��b�Z�[�W����
*/
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE( _T("CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos );

	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	// 2009.01.12 nasukoji	�z�C�[���X�N���[���𗘗p�����y�[�W�X�N���[���E���X�N���[���Ή�
	if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel ) ){				// �y�[�W�X�N���[���H
		if( IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel ) ){		// ���X�N���[���H
			int line = m_nViewLeftCol + (( nScrollCode == SB_LINEUP ) ? -m_nViewColNum : m_nViewColNum );
			SyncScrollH( ScrollAtH( line ) );

			// �z�C�[������ɂ�鉡�X�N���[������
			m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
		}else{
			int line = m_nViewTopLine + (( nScrollCode == SB_LINEUP ) ? -m_nViewRowNum : m_nViewRowNum );
			SyncScrollV( ScrollAtV( line ) );
		}
		// �z�C�[������ɂ��y�[�W�X�N���[������
		m_pcEditDoc->m_pcEditWnd->SetPageScrollByWheel( TRUE );
	}else{
		/* �}�E�X�z�C�[���ɂ��X�N���[���s�������W�X�g������擾 */
		nRollLineNum = 6;
		/* ���W�X�g���̑��݃`�F�b�N */
		// 2006.06.03 Moca ReadRegistry �ɏ�������
		unsigned int uDataLen;	// size of value data
		char szValStr[256];
		uDataLen = sizeof(szValStr) - 1;
		if( ReadRegistry( HKEY_CURRENT_USER, "Control Panel\\desktop", "WheelScrollLines", szValStr, uDataLen ) ){
			nRollLineNum = ::atoi( szValStr );
		}
		if( -1 == nRollLineNum ){/* �u1��ʕ��X�N���[������v */
			nRollLineNum = m_nViewRowNum;	// �\����̍s��
		}else{
			if( nRollLineNum < 1 ){
				nRollLineNum = 1;
			}
			if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10��30�ցB
				nRollLineNum = 30;
			}
		}

		// 2009.01.12 nasukoji	�L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������
		bool bHorizontal = IsSpecialScrollMode( m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel );
		int nCount = ( nScrollCode == SB_LINEUP ) ? -1 : 1;		// �X�N���[����

		for( i = 0; i < nRollLineNum; ++i ){

			//	Sep. 11, 2004 genta �����X�N���[���s��
			if( bHorizontal ){
				SyncScrollH( ScrollAtH( m_nViewLeftCol + nCount ) );

				// �z�C�[������ɂ�鉡�X�N���[������
				m_pcEditDoc->m_pcEditWnd->SetHScrollByWheel( TRUE );
			}else{
				SyncScrollV( ScrollAtV( m_nViewTopLine + nCount ) );
			}
		}
	}
	return 0;
}

/*!
	@brief �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷��

	�}�E�X�z�C�[�����A�s�X�N���[�����ׂ����y�[�W�X�N���[���E���X�N���[��
	���ׂ����𔻒肷��B
	���݂̃L�[�܂��̓}�E�X��Ԃ������Ŏw�肳�ꂽ�g�ݍ��킹�ɍ��v����ꍇ
	true��Ԃ��B

	@param nSelect	[in] �L�[�E�}�E�X�{�^���̑g�ݍ��킹�w��ԍ�

	@return �y�[�W�X�N���[���܂��͉��X�N���[�����ׂ���Ԃ̎�true��Ԃ�
	        �ʏ�̍s�X�N���[�����ׂ���Ԃ̎�false��Ԃ�

	@date 2009.01.12 nasukoji	�V�K�쐬
*/
bool CEditView::IsSpecialScrollMode( int nSelect )
{
	bool bSpecialScrollMode;

	switch( nSelect ){
	case 0:		// �w��̑g�ݍ��킹�Ȃ�
		bSpecialScrollMode = false;
		break;

	case MOUSEFUNCTION_CENTER:		// �}�E�X���{�^��
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_MBUTTON ) ) );
		break;

	case MOUSEFUNCTION_LEFTSIDE:	// �}�E�X�T�C�h�{�^��1
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON1 ) ) );
		break;

	case MOUSEFUNCTION_RIGHTSIDE:	// �}�E�X�T�C�h�{�^��2
		bSpecialScrollMode = ( 0 != ( 0x8000 & ::GetAsyncKeyState( VK_XBUTTON2 ) ) );
		break;

	case VK_CONTROL:	// Control�L�[
		bSpecialScrollMode = GetKeyState_Control();
		break;

	case VK_SHIFT:		// Shift�L�[
		bSpecialScrollMode = GetKeyState_Shift();
		break;

	default:	// ��L�ȊO�i�����ɂ͗��Ȃ��j
		bSpecialScrollMode = false;
		break;
	}

	return bSpecialScrollMode;
}







/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( _T("OnLBUTTONUP()\n") );

	/* �͈͑I���I�� & �}�E�X�L���v�`���[����� */
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X �L���v�`������� */
		::ReleaseCapture();
		ShowCaret_( m_hWnd ); // 2002/07/22 novice

		m_bBeginSelect = false;

		if( m_sSelect.m_ptFrom.y == m_sSelect.m_ptTo.y &&
			m_sSelect.m_ptFrom.x == m_sSelect.m_ptTo.x
		){
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( true );

			// �Ί��ʂ̋����\��	// 2007.10.18 ryoji
			DrawBracketPair( false );
			SetBracketPairPos( true );
			DrawBracketPair( true );
		}

		m_cUnderLine.UnLock();
	}
	return;
}


/* ShellExecute���Ăяo���v���V�[�W�� */
/*   �Ăяo���O�� lpParameter �� new ���Ă������� */
static unsigned __stdcall ShellExecuteProc( LPVOID lpParameter )
{
	LPTSTR pszFile = (LPTSTR)lpParameter;
	::ShellExecute( NULL, _T("open"), pszFile, NULL, NULL, SW_SHOW );
	delete []pszFile;
	return 0;
}


/* �}�E�X���{�^���_�u���N���b�N */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
	int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
	int			nUrlLen;	// URL�̒���(�o�C�g��)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���̓`�F�b�N���Ȃ�
	if(! m_dwTripleClickCheck){
		/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
		if(
			IsCurrentPositionURL(
				m_ptCaretPos.x,	// �J�[�\���ʒuX
				m_ptCaretPos.y,	// �J�[�\���ʒuY
				&nUrlLine,		// URL�̍s(���s�P��)
				&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				&nUrlLen,		// URL�̒���(�o�C�g��)
				&pszURL			// URL�󂯎���
			)
		 ){
			char*		pszWork = NULL;
			char*		pszOPEN;

			/* URL���J�� */
			/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
			if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
				pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
				strcpy( pszWork, pszMailTo );
				strcat( pszWork, pszURL );
				pszOPEN = pszWork;
			}else{
				if( _tcsnicmp( pszURL, _T("ttp://"), 6 ) == 0 ){	//�}�~URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 1 + 1 ];
					_tcscpy( pszWork, _T("h") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else if( _tcsnicmp( pszURL, _T("tp://"), 5 ) == 0 ){	//�}�~URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 2 + 1 ];
					_tcscpy( pszWork, _T("ht") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else{
					pszOPEN = pszURL;
				}
			}
			{
				// URL���J��
				// 2009.05.21 syat UNC�p�X����1���ȏ㖳�����ɂȂ邱�Ƃ�����̂ŃX���b�h��
				CWaitCursor cWaitCursor( m_hWnd );	// �J�[�\���������v�ɂ���

				unsigned int nThreadId;
				LPCTSTR szUrl = pszOPEN;
				LPTSTR szUrlDup = new TCHAR[_tcslen( szUrl ) + 1];
				_tcscpy( szUrlDup, szUrl );
				HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, ShellExecuteProc, (LPVOID)szUrlDup, 0, &nThreadId );
				if( hThread != INVALID_HANDLE_VALUE ){
					// ���[�U�[��URL�N���w���ɔ��������ڈ�Ƃ��Ă�����Ƃ̎��Ԃ��������v�J�[�\����\�����Ă���
					// ShellExecute �͑����ɃG���[�I�����邱�Ƃ����傭���傭����̂� WaitForSingleObject �ł͂Ȃ� Sleep ���g�p�iex.���݂��Ȃ��p�X�̋N���j
					// �y�⑫�z������� API �ł��҂��𒷂߁i2�`3�b�j�ɂ���ƂȂ��� Web �u���E�U���N������̋N�����d���Ȃ�͗l�iPC�^�C�v, XP/Vista, IE/FireFox �Ɋ֌W�Ȃ��j
					::Sleep(200);
					::CloseHandle(hThread);
				}else{
					//�X���b�h�쐬���s
					delete[] szUrlDup;
				}
			}
			delete [] pszURL;
			delete [] pszWork;
			return;
		}

		/* GREP�o�̓��[�h�܂��̓f�o�b�O���[�h ���� �}�E�X���{�^���_�u���N���b�N�Ń^�O�W�����v �̏ꍇ */
		//	2004.09.20 naoh �O���R�}���h�̏o�͂���Tagjump�ł���悤��
		if( (m_pcEditDoc->m_bGrepMode || m_pcEditDoc->m_bDebugMode) && m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK ){
			/* �^�O�W�����v�@�\ */
			Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();

	/* �}�E�X���N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_sKeyBind.m_pKeyNameArr[?]�ɓ����Ă��� 2007.10.06 nasukoji */
	nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// ��I����Ԃɂ����㍶�N���b�N�������Ƃɂ���
		// ���ׂđI���̏ꍇ�́A3.5�N���b�N���̑I����ԕێ��ƃh���b�O�J�n����
		// �͈͕ύX�̂��߁B
		// �N�A�h���v���N���b�N�@�\�����蓖�Ă��Ă��Ȃ��ꍇ�́A�_�u���N���b�N
		// �Ƃ��ď������邽�߁B
		if( IsTextSelected() )		// �e�L�X�g���I������Ă��邩
			DisableSelectArea( true );		// ���݂̑I��͈͂��I����Ԃɖ߂�

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
			nFuncID = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, xPos , yPos );	// �J�[�\�����N���b�N�ʒu�ֈړ�����
		}
	}

	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���������Ŕ�����
	if(m_dwTripleClickCheck){
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF�i����͒ʏ�N���b�N�j
		return;
	}

	// 2007.11.06 nasukoji	�_�u���N���b�N���P��I���łȂ��Ă��g���v���N���b�N��L���Ƃ���
	// 2007.10.02 nasukoji	�g���v���N���b�N�`�F�b�N�p�Ɏ������擾
	m_dwTripleClickCheck = ::GetTickCount();

	// �_�u���N���b�N�ʒu�Ƃ��ċL��
	m_nMouseRollPosXOld = xPos;			// �}�E�X�͈͑I��O��ʒu(X���W)
	m_nMouseRollPosYOld = yPos;			// �}�E�X�͈͑I��O��ʒu(Y���W)

	/*	2007.07.09 maru �@�\�R�[�h�̔����ǉ�
		�_�u���N���b�N����̃h���b�O�ł͒P��P�ʂ͈̔͑I��(�G�f�B�^�̈�ʓI����)�ɂȂ邪
		���̓���́A�_�u���N���b�N���P��I����O��Ƃ������́B
		�L�[���蓖�Ă̕ύX�ɂ��A�_�u���N���b�N���P��I���̂Ƃ��ɂ� m_bBeginWordSelect = true
		�ɂ���ƁA�����̓��e�ɂ���Ă͕\�������������Ȃ�̂ŁA�����Ŕ�����悤�ɂ���B
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
	m_bBeginSelect = true;				/* �͈͑I�� */
	m_bBeginBoxSelect = false;			/* ��`�͈͑I�𒆂łȂ� */
	m_bBeginLineSelect = false;			/* �s�P�ʑI�� */
	m_bBeginWordSelect = true;			/* �P��P�ʑI�� */

	if( m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		/* ALT�L�[��������Ă����� */
		if(GetKeyState_Alt()){
			m_bBeginBoxSelect = true;	/* ��`�͈͑I�� */
		}
	}
	::SetCapture( m_hWnd );
	HideCaret_( m_hWnd ); // 2002/07/22 novice
	if( IsTextSelected() ){
		/* �펞�I��͈͈͂̔� */
		m_sSelectBgn.m_ptTo.y = m_sSelect.m_ptTo.y;
		m_sSelectBgn.m_ptTo.x = m_sSelect.m_ptTo.x;
	}else{
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
	}

	return;
}






/** DragEnter ����
	@date 2008.03.26 ryoji SAKURAClip�t�H�[�}�b�g�iNULL�������܂ރe�L�X�g�j�ւ̑Ή���ǉ�
	@date 2008.06.20 ryoji CF_HDROP�t�H�[�}�b�g�ւ̑Ή���ǉ�
*/
STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragEnter()\n") );

	if( TRUE == m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		//	Oct. 22, 2005 genta �㏑���֎~(�t�@�C�������b�N����Ă���)�ꍇ���s��
		 && !( SHAREMODE_NOT_EXCLUSIVE != m_pcEditDoc->m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_pcEditDoc->m_hLockedFile )
		 && !m_pcEditDoc->IsReadOnly() ){ // Mar. 30, 2003 �ǂݎ���p�̃t�@�C���ɂ̓h���b�v�����Ȃ�
	}else{
		return E_UNEXPECTED;	//	Moca E_INVALIDARG����ύX
	}

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	m_cfDragData = GetAvailableClipFormat( pDataObject );
	if( m_cfDragData == 0 )
		return E_INVALIDARG;
	else if( m_cfDragData == CF_HDROP ){
		// �E�{�^���œ����Ă����Ƃ������t�@�C�����r���[�Ŏ�舵��
		if( !(MK_RBUTTON & dwKeyState) )
			return E_INVALIDARG;
	}

	/* �������A�N�e�B�u�y�C���ɂ��� */
	m_pcEditDoc->SetActivePane( m_nMyIndex );

	// ���݂̃J�[�\���ʒu���L������	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter.x = m_ptCaretPos.x;
	m_ptCaretPos_DragEnter.y = m_ptCaretPos.y;
	m_nCaretPosX_Prev_DragEnter = m_nCaretPosX_Prev;

	// �h���b�O�f�[�^�͋�`��
	m_bDragBoxData = IsDataAvailable( pDataObject, ::RegisterClipboardFormat( _T("MSDEVColumnSelect") ) );

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = TRUE;

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::DragOver()\n") );

	/* �}�E�X�ړ��̃��b�Z�[�W���� */
	::ScreenToClient( m_hWnd, (LPPOINT)&pt );
	OnMOUSEMOVE( dwKeyState, pt.x , pt.y );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( m_cfDragData, dwKeyState, pt, *pdwEffect );

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

	// �h���b�O�������r���[�ŁA���̃r���[�̃J�[�\�����h���b�O���̑I��͈͓��̏ꍇ�͋֎~�}�[�N�ɂ���
	// �����r���[�̂Ƃ��͋֎~�}�[�N�ɂ��Ȃ��i���A�v���ł������͂����Ȃ��Ă���͗l�j	// 2009.06.09 ryoji
	if( pcDragSourceView && !IsDragSource() &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP CEditView::DragLeave( void )
{
	DEBUG_TRACE( _T("CEditView::DragLeave()\n") );
	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	// DragEnter���̃J�[�\���ʒu�𕜌�	// 2007.12.09 ryoji
	MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
	m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ���[���[�A�A���_�[���C���A�J�[�\���ʒu�\���X�V

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	return S_OK;
}

/** �h���b�v����
	@date 2008.03.26 ryoji �h���b�v�œ\��t�����͈͂�I����Ԃɂ���
	                       SAKURAClip�t�H�[�}�b�g�iNULL�������܂ރe�L�X�g�j�ւ̑Ή���ǉ�
	@date 2008.06.20 ryoji CF_HDROP�t�H�[�}�b�g�ւ̑Ή���ǉ�
*/
STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	DEBUG_TRACE( _T("CEditView::Drop()\n") );
	CMemory		cmemBuf;
	COpe*		pcOpe;
	BOOL		bBoxData;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	bool		bBeginBoxSelect_Old;
	CLayoutRange	sSelectBgn_Old;
	CLayoutRange	sSelect_Old;

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if( ::GetActiveWindow() == NULL )
		OnKillFocus();

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat( pDataObject );
	if( cf == 0 )
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect( cf, dwKeyState, pt, *pdwEffect );
	if( *pdwEffect == DROPEFFECT_NONE )
		return E_INVALIDARG;

	// �t�@�C���h���b�v�� PostMyDropFiles() �ŏ�������
	if( cf == CF_HDROP )
		return PostMyDropFiles( pDataObject );

	// �O������̃h���b�v�͈Ȍ�̏����ł̓R�s�[�Ɠ��l�Ɉ���
	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pcDragSourceView;
	bBoxData = m_bDragBoxData;

	// �J�[�\�����I��͈͓��ɂ���Ƃ��̓R�s�[�^�ړ����Ȃ�	// 2009.06.09 ryoji
	if( pcDragSourceView &&
		!pcDragSourceView->IsCurrentPositionSelected( m_ptCaretPos.x, m_ptCaretPos.y )
	){
		// DragEnter���̃J�[�\���ʒu�𕜌�
		// Note. �h���b�O�������r���[�ł��}�E�X�ړ��������ƋH�ɂ����ɂ���\�������肻��
		*pdwEffect = DROPEFFECT_NONE;
		MoveCursor( m_ptCaretPos_DragEnter.x, m_ptCaretPos_DragEnter.y, false );
		m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if( !IsDragSource() )	// �h���b�O���̏ꍇ�͂����ł͍ĕ`��s�v�iDragDrop�㏈����SetActivePane�ōĕ`�悳���j
			RedrawAll();	// ����ɈȌ�̔�A�N�e�B�u���ɔ����A���_�[���C�������̂��߂Ɉ�x�X�V���Đ������Ƃ�
		return S_OK;
	}

	// �h���b�v�f�[�^�̎擾
	HGLOBAL hData = GetGlobalData( pDataObject, cf );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );
	if( cf == ::RegisterClipboardFormat( _T("SAKURAClip") ) ){
		cmemBuf.SetString( (char*)pData + sizeof(int), *(int*)pData );
	}else{
		CMemory cmemTemp;
		cmemTemp.SetString( (char*)pData, nSize );	// ���S�̂��ߖ����� null ������t��
		cmemBuf.SetString( cmemTemp.GetStringPtr() );		// ������I�[�܂ŃR�s�[
	}

	// �A���h�D�o�b�t�@�̏���
	if( NULL == m_pcOpeBlk ){
		m_pcOpeBlk = new COpeBlk;
	}

	/* �ړ��̏ꍇ�A�ʒu�֌W���Z�o */
	if( bMove ){
		if( bBoxData ){
			/* 2�_��Ίp�Ƃ����`�����߂� */
			TwoPointToRect(
				&rcSel,
				pcDragSourceView->m_sSelect.m_ptFrom.y,		/* �͈͑I���J�n�s */
				pcDragSourceView->m_sSelect.m_ptFrom.x,		/* �͈͑I���J�n�� */
				pcDragSourceView->m_sSelect.m_ptTo.y,		/* �͈͑I���I���s */
				pcDragSourceView->m_sSelect.m_ptTo.x			/* �͈͑I���I���� */
			);
			++rcSel.bottom;
			if( m_ptCaretPos.y >= rcSel.bottom ){
				bMoveToPrev = FALSE;
			}else
			if( m_ptCaretPos.y + rcSel.bottom - rcSel.top < rcSel.top ){
				bMoveToPrev = TRUE;
			}else
			if( m_ptCaretPos.x < rcSel.left ){
				bMoveToPrev = TRUE;
			}else{
				bMoveToPrev = FALSE;
			}
		}else{
			if( pcDragSourceView->m_sSelect.m_ptFrom.y > m_ptCaretPos.y ){
				bMoveToPrev = TRUE;
			}else
			if( pcDragSourceView->m_sSelect.m_ptFrom.y == m_ptCaretPos.y ){
				if( pcDragSourceView->m_sSelect.m_ptFrom.x > m_ptCaretPos.x ){
					bMoveToPrev = TRUE;
				}else{
					bMoveToPrev = FALSE;
				}
			}else{
				bMoveToPrev = FALSE;
			}
		}
	}

	nCaretPosX_Old = m_ptCaretPos.x;
	nCaretPosY_Old = m_ptCaretPos.y;
	if( !bMove ){
		/* �R�s�[���[�h */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( true );
	}else{
		// �h���b�O���̑I��͈͂��L��
		bBeginBoxSelect_Old = pcDragSourceView->m_bBeginBoxSelect;
		sSelectBgn_Old = pcDragSourceView->m_sSelectBgn;
		sSelect_Old = pcDragSourceView->m_sSelect;

		if( bMoveToPrev ){
			/* �ړ����[�h & �O�Ɉړ� */
			/* �I���G���A���폜 */
			if( this != pcDragSourceView ){
				// �h���b�O���̑I��͈͂𕜌�
				pcDragSourceView->DisableSelectArea( true );
				DisableSelectArea( true );
				m_bBeginBoxSelect = bBeginBoxSelect_Old;
				m_sSelectBgn = sSelectBgn_Old;
				m_sSelect = sSelect_Old;
			}
			DeleteData( true );
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
		}else{
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			pcDragSourceView->DisableSelectArea( true );
			if( this != pcDragSourceView )
				DisableSelectArea( true );
		}
	}
	if( !bBoxData ){	/* ��`�f�[�^ */
		//	2004,05.14 Moca �����ɕ����񒷂�ǉ�

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// �s�I�[���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\�������������Ă���
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		Command_INSTEXT( true, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), FALSE );

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		CLayoutRange sRange;
		sRange.m_ptFrom = m_sSelect.m_ptFrom;
		sRange.m_ptTo = m_ptCaretPos;
		SetSelectArea( sRange );	// 2009.07.25 ryoji
	}else{
		// 2004.07.12 Moca �N���b�v�{�[�h�����������Ȃ��悤��
		// TRUE == bBoxData
		// false == m_bBeginBoxSelect
		/* �\��t���i�N���b�v�{�[�h����\��t���j*/
		Command_PASTEBOX( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() );
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if( bMove ){
		if( bMoveToPrev ){
		}else{
			/* �ړ����[�h & ���Ɉړ�*/

			// ���݂̑I��͈͂��L������	// 2008.03.26 ryoji
			CLogicPoint ptSelectFrom_PHY;
			CLogicPoint ptSelectTo_PHY;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y,
				&ptSelectFrom_PHY.x, &ptSelectFrom_PHY.y
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y,
				&ptSelectTo_PHY.x, &ptSelectTo_PHY.y
			);

			// �ȑO�̑I��͈͂��L������	// 2008.03.26 ryoji
			CLogicPoint ptSelectFrom_PHY_Old;
			CLogicPoint ptSelect_PHY_Old;
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				sSelect_Old.m_ptFrom.x, sSelect_Old.m_ptFrom.y,
				&ptSelectFrom_PHY_Old.x, &ptSelectFrom_PHY_Old.y
			);
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				sSelect_Old.m_ptTo.x, sSelect_Old.m_ptTo.y,
				&ptSelect_PHY_Old.x, &ptSelect_PHY_Old.y
			);

			// ���݂̍s�����L������	// 2008.03.26 ryoji
			int nLines_Old = m_pcEditDoc->m_cDocLineMgr.GetLineCount();

			// �ȑO�̑I��͈͂�I������
			m_bBeginBoxSelect = bBeginBoxSelect_Old;
			m_sSelectBgn = sSelectBgn_Old;	/* �͈͑I���J�n(���_) */
			m_sSelect = sSelect_Old;

			/* �I���G���A���폜 */
			DeleteData( true );

			// �폜�O�̑I��͈͂𕜌�����	// 2008.03.26 ryoji
			if( !bBoxData ){
				// �폜���ꂽ�͈͂��l�����đI��͈͂𒲐�����
				if( ptSelectFrom_PHY.y == ptSelect_PHY_Old.y ){	// �I���J�n���폜�����Ɠ���s
					ptSelectFrom_PHY.x -= (ptSelect_PHY_Old.x - ptSelectFrom_PHY_Old.x);
				}
				if( ptSelectTo_PHY.y == ptSelect_PHY_Old.y ){	// �I���I�����폜�����Ɠ���s
					ptSelectTo_PHY.x -= (ptSelect_PHY_Old.x - ptSelectFrom_PHY_Old.x);
				}
				// Note.
				// (ptSelect_PHY_Old.y - ptSelectFrom_PHY_Old.y) �͎��ۂ̍폜�s���Ɠ����ɂȂ�
				// ���Ƃ����邪�A�i�폜�s���|�P�j�ɂȂ邱�Ƃ�����D
				// ��j�t���[�J�[�\���ł̍s�ԍ��N���b�N���̂P�s�I��
				int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				ptSelectFrom_PHY.y -= (nLines_Old - nLines);
				ptSelectTo_PHY.y -= (nLines_Old - nLines);

				// ������̑I��͈͂�ݒ肷��
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					ptSelectFrom_PHY.x, ptSelectFrom_PHY.y,
					&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
				);
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					ptSelectTo_PHY.x, ptSelectTo_PHY.y,
					&m_sSelect.m_ptTo.x, &m_sSelect.m_ptTo.y
				);
				SetSelectArea( m_sSelect );	// 2009.07.25 ryoji
				nCaretPosX_Old = m_sSelect.m_ptTo.x;
				nCaretPosY_Old = m_sSelect.m_ptTo.y;
			}

			// �L�����b�g���ړ�����
			MoveCursor( nCaretPosX_Old, nCaretPosY_Old, true );
			m_nCaretPosX_Prev = m_ptCaretPos.x;

			// �폜�ʒu����ړ���ւ̃J�[�\���ړ����A���h�D����ɒǉ�����	// 2008.03.26 ryoji
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;
			pcOpe->m_ptCaretPos_PHY_Before = ptSelectFrom_PHY_Old;
			pcOpe->m_ptCaretPos_PHY_After = m_ptCaretPos_PHY;
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	DrawSelectArea();

	// �A���h�D�o�b�t�@�̏���
	SetUndoBuffer();

	::GlobalUnlock( hData );
	// 2004.07.12 fotomo/���� �������[���[�N�̏C��
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
		::GlobalFree( hData );
	}

	return S_OK;
}

/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g����
	@date 2008.06.20 ryoji �V�K�쐬
*/
STDMETHODIMP CEditView::PostMyDropFiles( LPDATAOBJECT pDataObject )
{
	HGLOBAL hData = GetGlobalData( pDataObject, CF_HDROP );
	if( hData == NULL )
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock( hData );
	SIZE_T nSize = ::GlobalSize( hData );

	// �h���b�v�f�[�^���R�s�[���Ă��ƂœƎ��̃h���b�v�t�@�C���������s��
	HGLOBAL hDrop = ::GlobalAlloc( GHND | GMEM_DDESHARE, nSize );
	memcpy( ::GlobalLock( hDrop ), pData, nSize );
	::GlobalUnlock( hDrop );
	::PostMessage(
		m_hWnd,
		MYWM_DROPFILES,
		(WPARAM)hDrop,
		0
	);

	::GlobalUnlock( hData );
	if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags( hData )) ){
		::GlobalFree( hData );
	}

	return S_OK;
}

/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W����
	@date 2008.06.20 ryoji �V�K�쐬
*/
void CEditView::OnMyDropFiles( HDROP hDrop )
{
	// ���ʂɃ��j���[���삪�ł���悤�ɓ��͏�Ԃ��t�H�A�O�����h�E�B���h�E�ɃA�^�b�`����
	int nTid2 = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL );
	int nTid1 = ::GetCurrentThreadId();
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, TRUE );

	// �_�~�[�� STATIC ������ăt�H�[�J�X�𓖂Ă�i�G�f�B�^���O�ʂɏo�Ȃ��悤�Ɂj
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, m_hInstance, NULL );
	::SetFocus(hwnd);

	// ���j���[���쐬����
	POINT pt;
	::GetCursorPos( &pt );
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, _T("�p�X���\��t��(&P)") );
	::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, _T("�t�@�C�����\��t��(&F)") );
	::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu( hMenu, 3, MF_BYPOSITION | MF_STRING, 110, _T("�t�@�C�����J��(&O)") );
	::InsertMenu( hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu( hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, _T("�L�����Z��") );
	int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
								0, hwnd, NULL);
	::DestroyMenu( hMenu );

	::DestroyWindow( hwnd );

	// ���͏�Ԃ��f�^�b�`����
	if( nTid1 != nTid2 ) ::AttachThreadInput( nTid1, nTid2, FALSE );

	// �I�����ꂽ���j���[�ɑΉ����鏈�������s����
	switch( nId ){
	case 110:	// �t�@�C�����J��
		// �ʏ�̃h���b�v�t�@�C���������s��
		::SendMessage( m_pcEditDoc->m_pcEditWnd->m_hWnd, WM_DROPFILES, (WPARAM)hDrop, 0 );
		break;

	case 100:	// �p�X����\��t����
	case 101:	// �t�@�C������\��t����
		CMemory cmemBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, (LPSTR) NULL, 0 );
		for( UINT i = 0; i < nFiles; i++ ){
			::DragQueryFile( hDrop, i, szPath, sizeof(szPath)/sizeof(TCHAR) );
			if( !::GetLongFileName( szPath, szWork ) )
				continue;
			if( nId == 100 ){	// �p�X��
				::lstrcpy( szPath, szWork );
			}else if( nId == 101 ){	// �t�@�C����
				_tsplitpath( szWork, NULL, NULL, szPath, szExt );
				::lstrcat( szPath, szExt );
			}
			cmemBuf.AppendString( szPath );
			if( nFiles > 1 ){
				cmemBuf.AppendString( m_pcEditDoc->GetNewLineCode().GetValue() );
			}
		}
		::DragFinish( hDrop );

		// �I��͈͂̑I������
		if( IsTextSelected() ){
			DisableSelectArea( true );
		}

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		int nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
		int nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
		const CLayout* pcLayout;
		int nLineLen;
		if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout ) ){
			LineColmnToIndex2( pcLayout, m_ptCaretPos.x, nLineLen );
			if( nLineLen > 0 ){	// �s�I�[���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\�������������Ă���
				nCaretPosX_PHY_Old += (m_ptCaretPos.x - nLineLen);
			}
		}

		// �e�L�X�g�}��
		HandleCommand( F_INSTEXT, true, (LPARAM)cmemBuf.GetStringPtr(), TRUE, 0, 0 );

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			nCaretPosX_PHY_Old, nCaretPosY_PHY_Old,
			&m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y
		);
		CLayoutRange sRange;
		sRange.m_ptFrom = m_sSelect.m_ptFrom;
		sRange.m_ptTo = m_ptCaretPos;
		SetSelectArea( sRange );	// 2009.07.25 ryoji
		DrawSelectArea();
		break;
	}

	// ���������
	::GlobalFree( hDrop );
}

CLIPFORMAT CEditView::GetAvailableClipFormat( LPDATAOBJECT pDataObject )
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = ::RegisterClipboardFormat( _T("SAKURAClip") );

	if( IsDataAvailable( pDataObject, cfSAKURAClip ) )
		cf = cfSAKURAClip;
	//else if( IsDataAvailable(pDataObject, CF_UNICODETEXT) )
	//	cf = CF_UNICODETEXT;
	else if( IsDataAvailable(pDataObject, CF_TEXT) )
		cf = CF_TEXT;
	else if( IsDataAvailable(pDataObject, CF_HDROP) )	// 2008.06.20 ryoji
		cf = CF_HDROP;

	return cf;
}

DWORD CEditView::TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect )
{
	if( cf == CF_HDROP )	// 2008.06.20 ryoji
		return DROPEFFECT_LINK;

	CEditView* pcDragSourceView = m_pcEditDoc->GetDragSourceView();

	// 2008.06.21 ryoji
	// Win 98/Me ���ł͊O������̃h���b�O���� GetKeyState() �ł̓L�[��Ԃ𐳂����擾�ł��Ȃ����߁A
	// Drag & Drop �C���^�[�t�F�[�X�œn����� dwKeyState ��p���Ĕ��肷��B
#if 1
	// �h���b�O�����O���E�B���h�E���ǂ����ɂ���Ď󂯕���ς���
	// ���ėp�e�L�X�g�G�f�B�^�ł͂����炪�嗬���ۂ�
	if( pcDragSourceView ){
#else
	// �h���b�O�����ړ����������ǂ����ɂ���Ď󂯕���ς���
	// ��MS ���i�iMS Office, Visual Studio�Ȃǁj�ł͂����炪�嗬���ۂ�
	if( dwEffect & DROPEFFECT_MOVE ){
#endif
		dwEffect &= (MK_CONTROL & dwKeyState)? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else{
		dwEffect &= (MK_SHIFT & dwKeyState)? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool CEditView::IsDragSource( void )
{
	return ( this == m_pcEditDoc->GetDragSourceView() );
}

/*[EOF]*/
