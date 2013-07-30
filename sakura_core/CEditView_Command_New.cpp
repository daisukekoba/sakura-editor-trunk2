/*!	@file
	@brief CEditView�N���X�̃R�}���h�����n�֐��Q

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, asa-o, hor
	Copyright (C) 2002, YAZAKI, hor, genta. aroka, MIK, minfu, KK, �����
	Copyright (C) 2003, MIK, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, ryoji, genta, D.S.Koba
	Copyright (C) 2006, genta, Moca, fon, maru
	Copyright (C) 2007, ryoji, maru, genta
	Copyright (C) 2008, nasukoji, ryoji, genta
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include <string>///
#include <vector> ///	2002/2/3 aroka
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "CLayout.h"///
#include "CDocLine.h"///
#include "mymessage.h"///
#include "Debug.h"///
#include "etc_uty.h"///
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "CDlgFavorite.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "CEditWnd.h"
#include "CFileLoad.h"	// 2006.12.09 maru
#include "CDlgCancel.h"	// 2006.12.09 maru
#include "CFileWrite.h"	// 2006.12.09 maru

using namespace std; // 2002/2/3 aroka

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
/*!	���݈ʒu�Ƀf�[�^��}�� Ver0

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::InsertData_CEditView(
	int			nX,
	int			nY,
	const char*	pData,
	int			nDataLen,
	int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int*		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe,				/* �ҏW����v�f COpe */
	bool		bRedraw
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nInsLineNum;		/* �}���ɂ���đ��������C�A�E�g�s�̐� */
	PAINTSTRUCT ps;
	int			nLineAllColLen;
	CMemory		cMem;
	int			i;
	const CLayout*	pcLayout;
	bool			bHintPrev = false;	// �X�V���O�s����ɂȂ�\�������邱�Ƃ���������
	bool			bHintNext = false;	// �X�V�����s����ɂȂ�\�������邱�Ƃ���������
	bool			bKinsoku;			// �֑��̗L��

	*pnNewLine = 0;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	*pnNewPos = 0;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		DeleteData( bRedraw );
		nX = m_ptCaretPos.x;
		nY = m_ptCaretPos.y;
	}

	//�֑�������ꍇ��1�s�O����ĕ`����s��	@@@ 2002.04.19 MIK
	bKinsoku = ( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
			 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto );	//@@@ 2002.04.19 MIK

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen, &pcLayout );
	bool bLineModifiedChange = (pLine)? !pcLayout->m_pCDocLine->IsModifyed(): true;

	nIdxFrom = 0;
	cMem.SetString( "" );
	if( pLine ){
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̋�Ǔ_���͂őO�̍s�������X�V�����ꍇ������
		// ���}���ʒu�͍s�r���ł���Ǔ_���́{���[�h���b�v�őO�̕����񂩂瑱���đO�s�ɉ�荞�ޏꍇ������
		if (pcLayout->m_ptLogicPos.x && bKinsoku){	// �܂�Ԃ����C�A�E�g�s���H
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}

		// �X�V�����s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̕������͂╶����\��t���Ō��ݍs�͍X�V���ꂸ���s�Ȍオ�X�V�����ꍇ������
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdxFrom = LineColmnToIndex2( pcLayout, nX, nLineAllColLen );
		// �s�I�[���E�ɑ}�����悤�Ƃ���
		if( nLineAllColLen > 0 ){
			// �I�[���O����}���ʒu�܂ŋ󔒂𖄂߂�ׂ̏���
			// �s�I�[�����炩�̉��s�R�[�h��?
			if( EOL_NONE != pcLayout->m_cEol ){
				nIdxFrom = nLineLen - 1;
				for( i = 0; i < nX - nLineAllColLen + 1; ++i ){
					cMem += ' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
			else{
				nIdxFrom = nLineLen;
				for( i = 0; i < nX - nLineAllColLen; ++i ){
					cMem += ' ';
				}
				cMem.AppendString( pData, nDataLen );
				bHintNext = true;	// �X�V�����s����ɂȂ�\��������
			}
		}
		else{
			cMem.AppendString( pData, nDataLen );
		}
	}
	else{
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if( pcLayoutWk && pcLayoutWk->m_cEol == EOL_NONE && bKinsoku ){	// �܂�Ԃ����C�A�E�g�s���H�i�O�s�̏I�[�Œ����j
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}

		nLineAllColLen = nX;
		for( i = 0; i < nX - nIdxFrom; ++i ){
			cMem += ' ';
		}
		cMem.AppendString( pData, nDataLen );
	}


	if( !m_bDoing_UndoRedo && pcOpe ){	// �A���h�D�E���h�D�̎��s����
		if( pLine ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				LineIndexToColmn( pcLayout, nIdxFrom ),
				nY,
				&pcOpe->m_ptCaretPos_PHY_Before.x,
				&pcOpe->m_ptCaretPos_PHY_Before.y
			);
		}
		else{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				0,
				nY,
				&pcOpe->m_ptCaretPos_PHY_Before.x,
				&pcOpe->m_ptCaretPos_PHY_Before.y
			);
		}
	}


	// ������}��
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		nY,
		nIdxFrom,
		cMem.GetStringPtr(),
		cMem.GetStringLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		pnNewPos			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	);

	// ���������Ċm�ۂ���ăA�h���X�������ɂȂ�̂ŁA�ēx�A�s�f�[�^�����߂�
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2, &pcLayout );
	if( pLine2 ){
		*pnNewPos = LineIndexToColmn( pcLayout, *pnNewPos );
	}

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( *pnNewPos >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
		if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet
		 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
		{
			if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( *pnNewLine, *pnNewPos ) )	//@@@ 2002.04.18
			{
				*pnNewPos = 0;
				(*pnNewLine)++;
			}
		}
		else
		{
			// Oct. 7, 2002 YAZAKI
			*pnNewPos = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
			(*pnNewLine)++;
		}
	}

	// ��ԑJ��
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	// �ĕ`��
	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		// �L�����b�g�̕\���E�X�V
		ShowEditCaret();
	}
	else{

		if( bRedraw ){
			int nStartLine = nY;
			if( 0 < nInsLineNum ){
				// �X�N���[���o�[�̏�Ԃ��X�V����
				AdjustScrollBars();

				// �`��J�n�s�ʒu�𒲐�����	// 2009.02.17 ryoji
				if( bHintPrev ){	// �X�V���O�s����ɂȂ�\��������
					nStartLine--;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nStartLine - m_nViewTopLine);
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}
			else{
				if( nModifyLayoutLinesOld < 1 ){
					nModifyLayoutLinesOld = 1;
				}

				// �`��J�n�s�ʒu�ƕ`��s���𒲐�����	// 2009.02.17 ryoji
				if( bHintPrev ){	// �X�V���O�s����ɂȂ�\��������
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
				if( bHintNext ){	// �X�V�����s����ɂȂ�\��������
					nModifyLayoutLinesOld++;
				}

	//			ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				// 2002.02.25 Mod By KK ���s (nY - m_nViewTopLine - 1); => (nY - m_nViewTopLine);
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nStartLine - m_nViewTopLine);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nModifyLayoutLinesOld);

				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			HDC hdc = ::GetDC( m_hWnd );
			OnPaint( hdc, &ps, FALSE );
			::ReleaseDC( m_hWnd, hdc );

			// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
			if( bLineModifiedChange ){	// ���ύX�������s���ύX���ꂽ
				const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nStartLine );
				if( pcLayoutWk && pcLayoutWk->m_ptLogicPos.x ){	// �܂�Ԃ����C�A�E�g�s���H
					RedrawLineNumber();
				}
			}
		}
	}

	if( !m_bDoing_UndoRedo && pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nOpe = OPE_INSERT;				/* ������ */
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);

		pcOpe->m_nDataLen = cMem.GetStringLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = NULL;				/* ����Ɋ֘A����f�[�^ */
	}

#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
}


/*!	�w��ʒu�̎w�蒷�f�[�^�폜

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2�폜�i���FALSE������j
*/
void CEditView::DeleteData2(
	int			nCaretX,
	int			nCaretY,
	int			nDelLen,
	CMemory*	pcMem,
	COpe*		pcOpe		/* �ҏW����v�f COpe */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	const char*	pLine;
	int			nLineLen;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nModifyLayoutLinesNew;
	int			nDeleteLayoutLines;
	int			bLastLine;

	/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
	if( nCaretY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
		bLastLine = 1;
	}else{
		bLastLine = 0;
	}

	const CLayout* pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCaretY, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		goto end_of_func;
	}
	nIdxFrom = LineColmnToIndex( pcLayout, nCaretX );
	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nOpe = OPE_DELETE;				/* ������ */
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LineIndexToColmn( pcLayout, nIdxFrom + nDelLen ),
			nCaretY,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);
	}

	/* �f�[�^�폜 */
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		nCaretY, nIdxFrom, nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		pcMem
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nDataLen = pcMem->GetStringLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = pcMem;				/* ����Ɋ֘A����f�[�^ */
	}

	/* �I���G���A�̐擪�փJ�[�\�����ړ� */
	MoveCursor( nCaretX, nCaretY, false );
	m_nCaretPosX_Prev = m_ptCaretPos.x;


end_of_func:;
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
}





/*!	�J�[�\���ʒu�܂��͑I���G���A���폜

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::DeleteData(
	bool	bRedraw
//	BOOL	bUndo	/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(2)" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nLineNum;
	int			nCurIdx;
	int			nNxtIdx;
	int			nNxtPos;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nIdxFrom;
	int			nIdxTo;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	RECT		rcSel;
	int			bLastLine;
	CMemory*	pcMemDeleted;
	COpe*		pcOpe = NULL;
	int			nCaretPosXOld;
	int			nCaretPosYOld;
	int			i;
	const CLayout*	pcLayout;
	int			nSelectColmFrom_Old;
	int			nSelectLineFrom_Old;

	// �e�L�X�g�̑��݂��Ȃ��G���A�̍폜�́A�I��͈͂̃L�����Z���ƃJ�[�\���ړ��݂̂Ƃ���	// 2008.08.05 ryoji
	if( IsTextSelected() ){		// �e�L�X�g���I������Ă��邩
		if( IsEmptyArea( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, TRUE, m_bBeginBoxSelect ) ){
			// �J�[�\����I��͈͂̍���Ɉړ�
			MoveCursor( ( m_sSelect.m_ptFrom.x < m_sSelect.m_ptTo.x ) ? m_sSelect.m_ptFrom.x : m_sSelect.m_ptTo.x,
						( m_sSelect.m_ptFrom.y < m_sSelect.m_ptTo.y ) ? m_sSelect.m_ptFrom.y : m_sSelect.m_ptTo.y, bRedraw );
			m_nCaretPosX_Prev = m_ptCaretPos.x;
			DisableSelectArea( bRedraw );
			return;
		}
	}else{
		if( IsEmptyArea( m_ptCaretPos.x, m_ptCaretPos.y ) ){
			return;
		}
	}

	nCaretPosXOld = m_ptCaretPos.x;
	nCaretPosYOld = m_ptCaretPos.y;

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		CWaitCursor cWaitCursor( m_hWnd );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* ����O�̃L�����b�g�ʒu�x */

			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			m_pcEditDoc->SetModified(true,bRedraw);	//	2002/06/04 YAZAKI ��`�I�����폜�����Ƃ��ɕύX�}�[�N�����Ȃ��B

			m_bDrawSWITCH = false;	// 2002.01.25 hor
			nSelectColmFrom_Old = m_sSelect.m_ptFrom.x;
			nSelectLineFrom_Old = m_sSelect.m_ptFrom.y;

			/* �I��͈͂̃f�[�^���擾 */
			/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
			/* �Q�_��Ίp�Ƃ����`�����߂� */
			TwoPointToRect(
				&rcSel,
				m_sSelect.m_ptFrom.y,		// �͈͑I���J�n�s
				m_sSelect.m_ptFrom.x,		// �͈͑I���J�n��
				m_sSelect.m_ptTo.y,		// �͈͑I���I���s
				m_sSelect.m_ptTo.x			// �͈͑I���I����
			);
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( bRedraw );

			nIdxFrom = 0;
			nIdxTo = 0;
			for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
				nDelPosNext = nIdxFrom;
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pcLayout, rcSel.right );

					for( i = nIdxFrom; i <= nIdxTo; ++i ){
						if( pLine[i] == CR || pLine[i] == LF ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = 0;
					nIdxTo	 = 0;
				}
				nDelPos = nDelPosNext;
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						pcOpe = new COpe;
						pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							LineIndexToColmn( pcLayout, nDelPos ),
							nLineNum + 1,
							&pcOpe->m_ptCaretPos_PHY_Before.x,
							&pcOpe->m_ptCaretPos_PHY_Before.y
						);

					}else{
						pcOpe = NULL;
					}
					pcMemDeleted = new CMemory;
					// �w��ʒu�̎w�蒷�f�[�^�폜
					DeleteData2(
						rcSel.left,
						nLineNum + 1,
						nDelLen,
						pcMemDeleted,
						pcOpe				/* �ҏW����v�f COpe */
					);

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
							rcSel.left,
							nLineNum + 1,
							&pcOpe->m_ptCaretPos_PHY_After.x,
							&pcOpe->m_ptCaretPos_PHY_After.y
						);
						/* ����̒ǉ� */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}else{
						delete pcMemDeleted;
						pcMemDeleted = NULL;
					}
				}
			}
			m_bDrawSWITCH = true;	// 2002.01.25 hor

			/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
			if ( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
				/* �L�����b�g�̕\���E�X�V */
				ShowEditCaret();
			}
			if( bRedraw ){
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();

				/* �ĕ`�� */
				hdc = ::GetDC( m_hWnd );
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				OnPaint( hdc, &ps, FALSE );
				::ReleaseDC( m_hWnd, hdc );
			}
			/* �I���G���A�̐擪�փJ�[�\�����ړ� */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_ptCaretPos.x;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_ptCaretPos_PHY_Before.x,
					&pcOpe->m_ptCaretPos_PHY_Before.y
				);

				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* �����̃L�����b�g�ʒu�w */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* �����̃L�����b�g�ʒu�x */
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				m_sSelect.m_ptFrom.y,		// �͈͑I���J�n�s
				m_sSelect.m_ptFrom.x,		// �͈͑I���J�n��
				m_sSelect.m_ptTo.y,		// �͈͑I���I���s
				m_sSelect.m_ptTo.x,		// �͈͑I���I����
				NULL,					// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
				"",						// �}������f�[�^
				0,						// �}������f�[�^�̒���
				bRedraw
			);
		}
	}else{
		/* ���ݍs�̃f�[�^���擾 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_ptCaretPos.y, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
		if( m_ptCaretPos.y == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nCurIdx = LineColmnToIndex( pcLayout, m_ptCaretPos.x );
//		MYTRACE(_T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* �S�e�L�X�g�̍Ō� */
			goto end_of_func;
//			return;
		}

		/* �w�肳�ꂽ���̕����̃o�C�g���𒲂ׂ� */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* ���s */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_ptCaretPos.x + pcLayout->m_cEol.GetLen();
		}
		else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
			nNxtPos = LineIndexToColmn( pcLayout, nNxtIdx );
		}


		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			m_ptCaretPos.y,		// �͈͑I���J�n�s
			m_ptCaretPos.x,		// �͈͑I���J�n��
			m_ptCaretPos.y,		// �͈͑I���I���s
			nNxtPos,			// �͈͑I���I����
			NULL,				// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
			"",					// �}������f�[�^
			0,					// �}������f�[�^�̒���
			bRedraw
		);
	}

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_ptCaretPos.y > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* ���ݍs�̃f�[�^���擾 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
			}
			/* ���s�ŏI����Ă��邩 */
			if( ( EOL_NONE != pcLayout->m_cEol ) ){
				goto end_of_func;
			}
			/*�t�@�C���̍Ō�Ɉړ� */
			Command_GOFILEEND( false );
		}
	}
end_of_func:;

	return;
}











/* Undo ���ɖ߂� */
void CEditView::Command_UNDO( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( !m_pcEditDoc->IsEnableUndo() ){	/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
		return;
	}

	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_UNDO()" );

	COpe*		pcOpe = NULL;

	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	bool		bIsModified;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
	int			nCaretPosX_After;
	int			nCaretPosY_After;

	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_bDrawSWITCH = false;	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After.x,
				pcOpe->m_ptCaretPos_PHY_After.y,
				&nCaretPosX_After,
				&nCaretPosY_After
			);
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);


			/* �J�[�\�����ړ� */
			MoveCursor( nCaretPosX_After, nCaretPosY_After, false );

			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				{
					CMemory* pcMem = new CMemory;

					/* �I��͈͂̕ύX */
					m_sSelectBgn.m_ptFrom.y = nCaretPosY_Before;	// �͈͑I���J�n�s(���_)
					m_sSelectBgn.m_ptFrom.x = nCaretPosX_Before;	// �͈͑I���J�n��(���_)
					m_sSelectBgn.m_ptTo.y = m_sSelectBgn.m_ptFrom.y;	// �͈͑I���J�n�s(���_)
					m_sSelectBgn.m_ptTo.x = m_sSelectBgn.m_ptFrom.x;	// �͈͑I���J�n��(���_)
					m_sSelect.m_ptFrom.y = nCaretPosY_Before;
					m_sSelect.m_ptFrom.x = nCaretPosX_Before;
					m_sSelect.m_ptTo.y = nCaretPosY_After;
					m_sSelect.m_ptTo.x = nCaretPosX_After;

					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					ReplaceData_CEditView(
						m_sSelect.m_ptFrom.y,		// �͈͑I���J�n�s
						m_sSelect.m_ptFrom.x,		// �͈͑I���J�n��
						m_sSelect.m_ptTo.y,		// �͈͑I���I���s
						m_sSelect.m_ptTo.x,		// �͈͑I���I����
						pcMem,					// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						"",						// �}������f�[�^
						0,						// �}������f�[�^�̒���
						false					// �ĕ`�悷�邩�ۂ�
					);

					/* �I��͈͂̕ύX */
					m_sSelectBgn.m_ptFrom.y = -1;	// �͈͑I���J�n�s(���_)
					m_sSelectBgn.m_ptFrom.x = -1;	// �͈͑I���J�n��(���_)
					m_sSelectBgn.m_ptTo.y = -1;	// �͈͑I���J�n�s(���_)
					m_sSelectBgn.m_ptTo.x = -1;	// �͈͑I���J�n��(���_)
					m_sSelect.m_ptFrom.y = -1;
					m_sSelect.m_ptFrom.x = -1;
					m_sSelect.m_ptTo.y = -1;
					m_sSelect.m_ptTo.x = -1;

					pcOpe->m_pcmemData = pcMem;
				}
				break;
			case OPE_DELETE:
				// 2010.08.25 �������[���[�N�C��
				if( 0 < pcOpe->m_pcmemData->GetStringLength() ){
					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					ReplaceData_CEditView(
						nCaretPosY_Before,					// �͈͑I���J�n�s
						nCaretPosX_Before,					// �͈͑I���J�n��
						nCaretPosY_Before,					// �͈͑I���I���s
						nCaretPosX_Before,					// �͈͑I���I����
						NULL,								// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						pcOpe->m_pcmemData->GetStringPtr(),	// �}������f�[�^
						pcOpe->m_nDataLen,					// �}������f�[�^�̒���
						false								// �ĕ`�悷�邩�ۂ�
					);

				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_MOVECARET:
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, false/*true 2002.02.16 hor */ );
				break;
			}

			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);
			if( i == 0 ){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, false );
			}
		}
		m_bDrawSWITCH = true;	//	hor
		AdjustScrollBars(); // 2007.07.22 ryoji

		/* Undo��̕ύX�t���O */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnPaint( hdc, &ps, FALSE );
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}
		DrawCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V

	}

	m_nCaretPosX_Prev = m_ptCaretPos.x;	// 2007.10.11 ryoji �ǉ�
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}





/* Redo ��蒼�� */
void CEditView::Command_REDO( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}


	if( !m_pcEditDoc->IsEnableRedo() ){	/* Redo(��蒼��)�\�ȏ�Ԃ��H */
		return;
	}
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_REDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	bool		bIsModified;
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
	int			nCaretPosX_To;
	int			nCaretPosY_To;
	int			nCaretPosX_After;
	int			nCaretPosY_After;


	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		m_bDrawSWITCH = false;	// 2007.07.22 ryoji
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before.x,
				pcOpe->m_ptCaretPos_PHY_Before.y,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);

			if( i == 0 ){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, false );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				// 2010.08.25 �������[���[�N�̏C��
				if( 0 < pcOpe->m_pcmemData->GetStringLength() ){
					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					ReplaceData_CEditView(
						nCaretPosY_Before,					// �͈͑I���J�n�s
						nCaretPosX_Before,					// �͈͑I���J�n��
						nCaretPosY_Before,					// �͈͑I���I���s
						nCaretPosX_Before,					// �͈͑I���I����
						NULL,								// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						pcOpe->m_pcmemData->GetStringPtr(),	// �}������f�[�^
						pcOpe->m_pcmemData->GetStringLength(),	// �}������f�[�^�̒���
						false								//�ĕ`�悷�邩�ۂ�
					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_DELETE:
				{
				CMemory* pcMem = new CMemory;

				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
					pcOpe->m_ptCaretPos_PHY_To.x,
					pcOpe->m_ptCaretPos_PHY_To.y,
					&nCaretPosX_To,
					&nCaretPosY_To
				);

				/* �f�[�^�u�� �폜&�}���ɂ��g���� */
				ReplaceData_CEditView(
					nCaretPosY_Before,	// �͈͑I���J�n�s
					nCaretPosX_Before,	// �͈͑I���J�n��
					nCaretPosY_To,		// �͈͑I���I���s
					nCaretPosX_To,		// �͈͑I���I����
					pcMem,				// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
					"",					// �}������f�[�^
					0,					// �}������f�[�^�̒���
					false
				);

				pcOpe->m_pcmemData = pcMem;
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After.x,
				pcOpe->m_ptCaretPos_PHY_After.y,
				&nCaretPosX_After,
				&nCaretPosY_After
			);

			if( i == nOpeBlkNum - 1	){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, true );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, false );
			}
		}
		m_bDrawSWITCH = true; // 2007.07.22 ryoji
		AdjustScrollBars(); // 2007.07.22 ryoji

		/* Redo��̕ύX�t���O */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnPaint( hdc, &ps, FALSE );
		DispRuler( hdc );	// 2007.10.19 ryoji
		::ReleaseDC( m_hWnd, hdc );

		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}
		DrawCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !m_pcEditDoc->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			m_pcEditDoc->RedrawAllViews( this );	//	���̃y�C���̕\�����X�V
	}

	m_nCaretPosX_Prev = m_ptCaretPos.x;	// 2007.10.11 ryoji �ǉ�
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */
}




/* �f�[�^�u�� �폜&�}���ɂ��g���� */
void CEditView::ReplaceData_CEditView(
	int			nDelLineFrom,			// �폜�͈͍s  From ���C�A�E�g�s�ԍ�
	int			nDelColmFrom,			// �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu
	int			nDelLineTo,				// �폜�͈͍s  To   ���C�A�E�g�s�ԍ�
	int			nDelColmTo,				// �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu
	CMemory*	pcmemCopyOfDeleted,		// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	const char*	pInsData,				// �}������f�[�^
	int			nInsDataLen,			// �}������f�[�^�̒���
	bool		bRedraw
)
{
	bool bLineModifiedChange;

	{
		//	Jun 23, 2000 genta
		//	�ϐ��������������Y��Ă����̂��C��

		//	Jun. 1, 2000 genta
		//	DeleteData����ړ�����

		//	May. 29, 2000 genta
		//	From Here
		//	�s�̌�낪�I������Ă����Ƃ��̕s���������邽�߁C
		//	�I��̈悩��s���ȍ~�̕�������菜���D

		//	�擪
		int len, pos;
		const char *line;
		const CLayout* pcLayout;
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineFrom, &len, &pcLayout );
		bLineModifiedChange = (line)? !pcLayout->m_pCDocLine->IsModifyed(): true;
		//	Jun. 1, 2000 genta
		//	������NULL�`�F�b�N���܂��傤
		if( line ){
			pos = LineColmnToIndex( pcLayout, nDelColmFrom );
			//	Jun. 1, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ���l������

			//	Aug. 22, 2000 genta
			//	�J�n�ʒu��EOF�̌��̂Ƃ��͎��s�ɑ��鏈�����s��Ȃ�
			//	���������Ă��܂��Ƒ��݂��Ȃ��s��Point���ė�����D
			if( nDelLineFrom < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( nDelLineFrom == nDelLineTo  ){
					//	m_sSelect.m_ptFrom.y <= m_sSelect.m_ptTo.y �̓`�F�b�N���Ȃ�
					++nDelLineFrom;
					nDelLineTo = nDelLineFrom;
					nDelColmTo = nDelColmFrom = 0;
				}
				else {
					nDelLineFrom++;
					nDelColmFrom = 0;
				}
			}
		}

		//	����
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineTo, &len, &pcLayout );
		if( line ){
			pos = LineIndexToColmn( pcLayout, len );

			if( nDelColmTo > pos ){
				nDelColmTo = pos;
			}
		}
		//	To Here
	}

	int nDelLineFrom_PHY;
	int nDelColmFrom_PHY;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(	// 2009.07.18 ryoji PHY�ŋL������
		nDelColmFrom,
		nDelLineFrom,
		&nDelColmFrom_PHY,
		&nDelLineFrom_PHY
	);

	COpe* pcOpe = NULL;		/* �ҏW����v�f COpe */
	CMemory* pcMemDeleted;
	int	nCaretPosXOld;
	int	nCaretPosYOld;
	int	nCaretPosX_PHY_Old;
	int	nCaretPosY_PHY_Old;

	nCaretPosXOld = m_ptCaretPos.x;
	nCaretPosYOld = m_ptCaretPos.y;
	nCaretPosX_PHY_Old = m_ptCaretPos_PHY.x;
	nCaretPosY_PHY_Old = m_ptCaretPos_PHY.y;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* ������ */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* ����O�̃L�����b�g�ʒu�x */


		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_DELETE;				/* ������ */

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			nDelColmFrom/*m_ptCaretPos.x*/,
			nDelLineFrom/*m_ptCaretPos.y*/,
			&pcOpe->m_ptCaretPos_PHY_Before.x,
			&pcOpe->m_ptCaretPos_PHY_Before.y
		);

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			nDelColmTo,
			nDelLineTo,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);



		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
	}else{
		pcOpe = NULL;
	}
	pcMemDeleted = new CMemory;

	// Feb. 08, 2008 genta �폜�o�b�t�@�̊m�ۂ�CDocLineMgr::ReplaceData�ōs���̂ō폜

	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	// 2009.07.18 ryoji �u���と�u���O�Ɉʒu��ύX�i�u���ゾ�Ɣ��]���s���ɂȂ��ĉ��� Wiki BugReport/43�j
	DisableSelectArea( bRedraw );

	/* ������u�� */
	LayoutReplaceArg	LRArg;
	LRArg.sDelRange.m_ptFrom.y = nDelLineFrom;	// �폜�͈͍s  From ���C�A�E�g�s�ԍ�
	LRArg.sDelRange.m_ptFrom.x = nDelColmFrom;	// �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu
	LRArg.sDelRange.m_ptTo.y   = nDelLineTo;	// �폜�͈͍s  To   ���C�A�E�g�s�ԍ�
	LRArg.sDelRange.m_ptTo.x   = nDelColmTo;	// �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu
	LRArg.pcmemDeleted = pcMemDeleted;	// �폜���ꂽ�f�[�^
	LRArg.pInsData = pInsData;			// �}������f�[�^
	LRArg.nInsDataLen = nInsDataLen;	// �}������f�[�^�̒���
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
	);

	//	Jan. 30, 2001 genta
	//	�ĕ`��̎��_�Ńt�@�C���X�V�t���O���K�؂ɂȂ��Ă��Ȃ��Ƃ����Ȃ��̂�
	//	�֐��̖������炱���ֈړ�
	/* ��ԑJ�� */
	if( FALSE == m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();
	}
	else{

		if( bRedraw ){
		/* �ĕ`�� */
			HDC	hdc;
			PAINTSTRUCT ps;
			hdc = ::GetDC( m_hWnd );
			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
			//	Jan. 30, 2001 genta	�\��t���ōs��������ꍇ�̍l���������Ă���
			if( 0 != LRArg.nAddLineNum ){
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_ptCaretPos.y - m_nViewTopLine); // 2002.02.25 Del By KK ���ŏ㏑������Ă��邽�ߖ��g�p�B
				//ps.rcPaint.top = 0/*m_nViewAlignTop*/;			// 2002.02.25 Del By KK
				ps.rcPaint.top = m_nViewAlignTop - m_nTopYohaku;	// ���[���[��͈͂Ɋ܂߂Ȃ��B2002.02.25 Add By KK
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}
			else{
				// �����������s�Ȃ�������ɕω�������				// 2009.11.11 ryoji
				// EOF�̂ݍs���ǉ��ɂȂ�̂ŁA1�s�]���ɕ`�悷��B
				// �i�����������s���聨�Ȃ��ɕω�����ꍇ�̖���EOF�����͕`��֐����ōs����j
				int nAddLine = ( LRArg.nNewLine > LRArg.sDelRange.m_ptTo.y )? 1: 0;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
				ps.rcPaint.top = m_nViewAlignTop + (LRArg.nModLineFrom - m_nViewTopLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = m_nViewAlignTop + (LRArg.nModLineTo - m_nViewTopLine + 1 + nAddLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}

			}
			OnPaint( hdc, &ps, FALSE );
			::ReleaseDC( m_hWnd, hdc );

			// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
			if( bLineModifiedChange ){	// ���ύX�������s���ύX���ꂽ
				const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
				if( pcLayoutWk && pcLayoutWk->m_ptLogicPos.x ){	// �܂�Ԃ����C�A�E�g�s���H
					RedrawLineNumber();
				}
			}
		}
	}

	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	if( pcmemCopyOfDeleted && 0 < pcMemDeleted->GetStringLength() ){
		*pcmemCopyOfDeleted = *pcMemDeleted;
	}

	if( !m_bDoing_UndoRedo && 0 < pcMemDeleted->GetStringLength() ){
		pcOpe->m_nDataLen = pcMemDeleted->GetStringLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = pcMemDeleted;						/* ����Ɋ֘A����f�[�^ */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		delete pcMemDeleted;
		pcMemDeleted = NULL;
	}


	if( !m_bDoing_UndoRedo && 0 < nInsDataLen ){
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_INSERT;				/* ������ */

		// 2009.07.18 ryoji ���C�A�E�g�͕ω�����̂ɈȑO��nDelColmFrom,nDelLineFrom����LayoutToLogic�Ōv�Z���Ă����o�O���C��
		pcOpe->m_ptCaretPos_PHY_Before.x = nDelColmFrom_PHY;
		pcOpe->m_ptCaretPos_PHY_Before.y = nDelLineFrom_PHY;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LRArg.nNewPos,
			LRArg.nNewLine,
			&pcOpe->m_ptCaretPos_PHY_To.x,
			&pcOpe->m_ptCaretPos_PHY_To.y
		);
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_To.x;
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_To.y;


		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		pcOpe = NULL;
	}


	// �}������ʒu�փJ�[�\�����ړ�
	MoveCursor(
		LRArg.nNewPos,		// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu)
		LRArg.nNewLine,		// �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s)
		bRedraw
	);
	m_nCaretPosX_Prev = m_ptCaretPos.x;


	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
		pcOpe->m_ptCaretPos_PHY_Before.x = nCaretPosX_PHY_Old;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_Before.y = nCaretPosY_PHY_Old;	/* �����̃L�����b�g�ʒu�x */

		pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	//	Jan. 30, 2001 genta
	//	�t�@�C���S�̂̍X�V�t���O�������Ă��Ȃ��Ɗe�s�̍X�V��Ԃ��\������Ȃ��̂�
	//	�t���O�X�V�������ĕ`����O�Ɉړ�����
}




/* C/C++�X�}�[�g�C���f���g���� */
void CEditView::SmartIndent_CPP( char cChar )
{
	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			k;
	int			m;
	const char*	pLine2;
	int			nLineLen2;
	int			nLevel;

	/* �����ɂ���Ēu�������ӏ� */
	int			nXFm = -1;
	int			nYFm = -1;
	int			nXTo = -1;
	int			nYTo = -1;

	char*		pszData = NULL;
	int			nDataLen;

	int			nCPX;
	int			nCPY;
	COpe*		pcOpe = NULL;
	int			nWork;
	CDocLine*	pCDocLine = NULL;
	int			nCharChars;
	int			nSrcLen;
	char		pszSrc[1024];
	BOOL		bChange;

	int			nCaretPosX_PHY;


	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':
		break;
	default:
		return;
	}
	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':

		nCaretPosX_PHY = m_ptCaretPos_PHY.x;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( m_ptCaretPos_PHY.y, &nLineLen );
		if( NULL == pLine ){
			if( CR != cChar ){
				return;
			}
			/* �����ɂ���Ēu�������ӏ� */
			nXFm = 0;
			nYFm = m_ptCaretPos_PHY.y;
			nXTo = 0;
			nYTo = m_ptCaretPos_PHY.y;
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y );


			//	nWork�ɏ����̊���ʒu��ݒ肷��
			if( CR != cChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CR�����͂��ꂽ���A�J�[�\������̎��ʎq���C���f���g����B
				|| �J�[�\������̎��ʎq��'}'��')'�Ȃ��
				|| '}'��')'�����͂��ꂽ���Ɠ�������������
				*/

				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( TAB != pLine[i] && SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars && ( pLine[i] == ')' || pLine[i] == '}' ) ){
						cChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			for( i = 0; i < nWork; i++ ){
				if( TAB != pLine[i] && SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( ':' == cChar
					 && (
							0 == strncmp( &pLine[i], "case", 4 )
						 || 0 == strncmp( &pLine[i], "default:", 8 )
						 || 0 == strncmp( &pLine[i], "public:", 7 )
						 || 0 == strncmp( &pLine[i], "private:", 8 )
						 || 0 == strncmp( &pLine[i], "protected:", 10 )
						)
					)
					//	Sep. 18, 2002 �����
					|| (( '{' == cChar ) && ( '#' != pLine[i] ))
					|| (( '(' == cChar ) && ( '#' != pLine[i] ))
				){

				}else{
					return;
				}
			}else{
				if( ':' == cChar ){
					return;
				}
			}
			/* �����ɂ���Ēu�������ӏ� */
			nXFm = 0;
			nYFm = m_ptCaretPos_PHY.y;
			nXTo = i;
			nYTo = m_ptCaretPos_PHY.y;
		}


		/* �Ή����銇�ʂ������� */
		nLevel = 0;	/* {}�̓���q���x�� */


		nDataLen = 0;
		for( j = m_ptCaretPos_PHY.y; j >= 0; --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 );
			if( j == m_ptCaretPos_PHY.y ){
				// 2005.10.11 ryoji EOF �݂̂̍s���X�}�[�g�C���f���g�̑Ώۂɂ���
				if( NULL == pLine2 ){
					if( m_ptCaretPos_PHY.y == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF �݂̂̍s
					break;
				}
				nCharChars = &pLine2[nWork] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nWork] );
				k = nWork - nCharChars;
			}else{
				if( NULL == pLine2 )
					break;
				nCharChars = &pLine2[nLineLen2] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] );
				k = nLineLen2 - nCharChars;
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( '}' == pLine2[k] || ')' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( _T("��[%s]\n"), pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == m_ptCaretPos_PHY.y ){
							if( '{' == cChar && '}' == pLine2[k] ){
								cChar = '}';
								nLevel--;	/* {}�̓���q���x�� */
							}
							if( '(' == cChar && ')' == pLine2[k] ){
								cChar = ')';
								nLevel--;	/* {}�̓���q���x�� */
							}
						}

						nLevel++;	/* {}�̓���q���x�� */
					}
				}
				if( 1 == nCharChars && ( '{' == pLine2[k] || '(' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( _T("��[%s]\n"), pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == m_ptCaretPos_PHY.y ){
							if( '{' == cChar && '{' == pLine2[k] ){
								return;
							}
							if( '(' == cChar && '(' == pLine2[k] ){
								return;
							}
						}
						if( 0 == nLevel ){
							break;
						}else{
							nLevel--;	/* {}�̓���q���x�� */
						}

					}
				}
				nCharChars = &pLine2[k] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[k] );
				if( 0 == nCharChars ){
					nCharChars = 1;
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* ���̍s�ɂ͂Ȃ� */
				continue;
			}

			for( m = 0; m < nLineLen2; m++ ){
				if( TAB != pLine2[m] && SPACE != pLine2[m] ){
					break;
				}
			}


			nDataLen = m;
			nCharChars = (m_pcEditDoc->GetDocumentAttribute().m_bInsSpace)? m_pcEditDoc->m_cLayoutMgr.GetTabSpace(): 1;
			pszData = new char[nDataLen + nCharChars + 1];
			memcpy( pszData, pLine2, nDataLen );
			if( CR  == cChar || '{' == cChar || '(' == cChar ){
				// 2005.10.11 ryoji TAB�L�[��SPACE�}���̐ݒ�Ȃ�ǉ��C���f���g��SPACE�ɂ���
				//	����������̉E�[�̕\���ʒu�����߂���ő}������X�y�[�X�̐������肷��
				if( m_pcEditDoc->GetDocumentAttribute().m_bInsSpace ){	// SPACE�}���ݒ�
					i = m = 0;
					while( i < nDataLen ){
						nCharChars = CMemory::GetSizeOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && TAB == pszData[i] )
							m += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
						else
							m += nCharChars;
						i += nCharChars;
					}
					nCharChars = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
					for( i = 0; i < nCharChars; i++ )
						pszData[nDataLen + i] = SPACE;
					pszData[nDataLen + nCharChars] = '\0';
					nDataLen += nCharChars;
				}else{
					pszData[nDataLen] = TAB;
					pszData[nDataLen + 1] = '\0';
					++nDataLen;
				}
			}else{
				pszData[nDataLen] = '\0';

			}
			break;
		}
		if( j < 0 ){
			/* �Ή����銇�ʂ�������Ȃ����� */
			if( CR == cChar ){
				return;
			}else{
				nDataLen = 0;
				pszData = new char[nDataLen + 1];
				pszData[nDataLen] = '\0';
			}
		}

		/* ������̃J�[�\���ʒu���v�Z���Ă��� */
		nCPX = nCaretPosX_PHY - nXTo + nDataLen;
		nCPY = m_ptCaretPos_PHY.y;

		nSrcLen = nXTo - nXFm;
		if( nSrcLen >= _countof( pszSrc ) - 1 ){
			//	Sep. 18, 2002 genta ���������[�N�΍�
			delete [] pszData;
			return;
		}
		if( NULL == pLine ){
			pszSrc[0] = '\0';
		}else{
			memcpy( pszSrc, &pLine[nXFm], nSrcLen );
			pszSrc[nSrcLen] = '\0';
		}


		/* �����ɂ���Ēu�������ӏ� */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nXFm, nYFm, &nXFm, &nYFm );
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nXTo, nYTo, &nXTo, &nYTo );

		if( ( 0 == nDataLen && nYFm == nYTo && nXFm == nXTo )
		 || ( nDataLen == nSrcLen && 0 == memcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;

			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				nYFm,		/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
				nXFm,		/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
				nYTo,		/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
				nXTo,		/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
				NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				pszData,	/* �}������f�[�^ */
				nDataLen,	/* �}������f�[�^�̒��� */
				true
			);
		}


		/* �J�[�\���ʒu���� */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nCPX, nCPY, &nCPX, &nCPY );
		/* �I���G���A�̐擪�փJ�[�\�����ړ� */
		MoveCursor( nCPX, nCPY, true );
		m_nCaretPosX_Prev = m_ptCaretPos.x;


		if( bChange && !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		break;
	}
	if( NULL != pszData ){
		delete [] pszData;
		pszData = NULL;
	}
}


/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */
void CEditView::RTrimPrevLine( void )
{
	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			nXFm;
	int			nYFm;
	int			nXTo;
	int			nYTo;
	int			nCPX;
	int			nCPY;
	int			nCharChars;
	int			nCaretPosX_PHY;
	int			nCaretPosY_PHY;
	COpe*		pcOpe = NULL;

	nCaretPosX_PHY = m_ptCaretPos_PHY.x;
	nCaretPosY_PHY = m_ptCaretPos_PHY.y;

	if( m_ptCaretPos_PHY.y > 0 ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( m_ptCaretPos_PHY.y - 1, &nLineLen );
		if( NULL != pLine && nLineLen > 0 ){
			i = j = 0;
			while( i < nLineLen ){
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
				if( 1 == nCharChars ){
					if( TAB != pLine[i] && SPACE != pLine[i])
						j = i + nCharChars;
				}
				else if( 2 == nCharChars ){
					if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
						j = i + nCharChars;
				}
				i += nCharChars;
			}
			if( j < nLineLen ){
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( j, m_ptCaretPos_PHY.y - 1, &nXFm, &nYFm );
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nLineLen, m_ptCaretPos_PHY.y - 1, &nXTo, &nYTo );
				if( !( nXFm >= nXTo && nYFm == nYTo) ){
					ReplaceData_CEditView(
						nYFm,		/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
						nXFm,		/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
						nYTo,		/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
						nXTo,		/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
						NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						NULL,		/* �}������f�[�^ */
						0,			/* �}������f�[�^�̒��� */
						true
					);
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( nCaretPosX_PHY, nCaretPosY_PHY, &nCPX, &nCPY );
					MoveCursor( nCPX, nCPY, true );

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						pcOpe = new COpe;
						pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
						pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* ����O�̃L�����b�g�ʒu�w */
						pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* ����O�̃L�����b�g�ʒu�x */
						pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
						pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
						/* ����̒ǉ� */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}
				}
			}
		}
	}
}


// 2001/06/20 Start by asa-o

// �e�L�X�g���P�s���փX�N���[��
void CEditView::Command_WndScrollDown( void )
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	nCaretMarginY += 2;

	if( m_ptCaretPos.y > m_nViewRowNum + m_nViewTopLine - (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( true );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	SyncScrollV( ScrollAtV(m_nViewTopLine - 1));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( m_ptCaretPos.y > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
			if( m_ptCaretPos.y > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - m_ptCaretPos.y, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( true );
}

// �e�L�X�g���P�s��փX�N���[��
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_ptCaretPos.y < m_nViewTopLine + (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( true );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	SyncScrollV( ScrollAtV( m_nViewTopLine + 1 ));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( m_ptCaretPos.y < m_nViewTopLine + nCaretMarginY )
		{
			if( m_nViewTopLine == 1 )
				Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				Cursor_UPDOWN( 1, FALSE );
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( true );
}

// 2001/06/20 End



/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CEditView::Command_GONEXTPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF�s�ł����B
		return;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			}
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* �ړ��O�̕����ʒu */
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* �ړ���̕����ʒu */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CEditView::Command_GOPREVPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			}
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* �ړ��O�̕����ʒu */
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* �ړ���̕����ʒu */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		m_ptCaretPos_PHY.x, m_ptCaretPos_PHY.y + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
}

// From Here 2001.12.03 hor

//! �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void CEditView::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
	int			nYfrom,nYto;
	if( IsTextSelected() && m_sSelect.m_ptFrom.y<m_sSelect.m_ptTo.y ){
		nYfrom=m_sSelect.m_ptFrom.y;
		nYto  =m_sSelect.m_ptTo.y;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nX,nYfrom,&nX,&nYfrom);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nX,nYto,&nX,&nYto);
		for(nY=nYfrom;nY<=nYto;nY++){
			pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLine( nY );
			if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookmarked());
		}
	}else{
		pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLine( m_ptCaretPos_PHY.y );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookmarked());
	}
	// 2002.01.16 hor ���������r���[���X�V
	m_pcEditDoc->Views_Redraw();
	return;
}



//! ���̃u�b�N�}�[�N��T���C����������ړ�����
void CEditView::Command_BOOKMARK_NEXT(void)
{
	int			nX=0;
	int			nY;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	nY=m_ptCaretPos_PHY.y;
	nYOld=nY;						// hor

re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, SEARCH_FORWARD, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nX,nY,&nX,&nY);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo			// �ŏ��̌���
		){
			nY=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if(bFound){
		if(nYOld >= nY)SendStatusMessage(_T("���擪����Č������܂���"));
	}else{
		SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_hWnd, _T("�O��(��) �Ƀu�b�N�}�[�N��������܂���B"));
	}
	return;
}



//! �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void CEditView::Command_BOOKMARK_PREV(void)
{
	int			nX=0;
	int			nY;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	nY=m_ptCaretPos_PHY.y;
	nYOld=nY;						// hor

re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, SEARCH_BACKWARD, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nX,nY,&nX,&nY);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			nY=m_pcEditDoc->m_cLayoutMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if(nYOld <= nY)SendStatusMessage(_T("����������Č������܂���"));
	}else{
		SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_hWnd, _T("���(��) �Ƀu�b�N�}�[�N��������܂���B") );
	}
	return;
}



//! �u�b�N�}�[�N���N���A����
void CEditView::Command_BOOKMARK_RESET(void)
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	m_pcEditDoc->Views_Redraw();
}


//�w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
//�L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void CEditView::Command_BOOKMARK_PATTERN( void )
{
	//����or�u���_�C�A���O����Ăяo���ꂽ
	if( !ChangeCurRegexp(false) ) return;

	m_pcEditDoc->m_cDocLineMgr.MarkSearchWord(
		m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0],		// ��������
		m_pShareData->m_Common.m_sSearch.m_sSearchOption,	// ��������
		&m_CurRegexp							// ���K�\���R���p�C���f�[�^
	);
	// 2002.01.16 hor ���������r���[���X�V
	m_pcEditDoc->Views_Redraw();
	return;
}



/*! TRIM Step1
	��I�����̓J�����g�s��I������ ConvSelectedArea �� ConvMemory ��
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CEditView::Command_TRIM(
	BOOL bLeft	//!< [in] FALSE: �ETRIM / ����ȊO: ��TRIM
)
{
	bool bBeDisableSelectArea = false;
	if(!IsTextSelected()){	//	��I�����͍s�I���ɕύX
		m_sSelect.m_ptFrom.y = m_ptCaretPos.y;
		m_sSelect.m_ptFrom.x = 0;
		m_sSelect.m_ptTo.y   = m_ptCaretPos.y; 
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		m_sSelect.m_ptTo.x   = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		bBeDisableSelectArea = true;
	}

	if(bLeft){
		ConvSelectedArea( F_LTRIM );
	}
	else{
		ConvSelectedArea( F_RTRIM );
	}
	if (bBeDisableSelectArea) DisableSelectArea( true );
}

/*! TRIM Step2
	ConvMemory ���� �߂��Ă�����̏����D
	CMemory.cpp�̂Ȃ��ɒu���Ȃ��ق����ǂ����ȂƎv���Ă�����ɒu���܂����D
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CEditView::Command_TRIM2( CMemory* pCMemory , BOOL bLeft )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i,j;
	int			nPosDes;
	CEol		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ� */
	while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	if( bLeft ){
	// LTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				for( i = 0; i <= nLineLen; ++i ){
					if( pLine[i] ==' ' ||
						pLine[i] =='\t'){
						continue;
					}else if( (unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40 ){
						++i;
						continue;
					}else{
						break;
					}
				}
				if(nLineLen-i>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[i], nLineLen );
					nPosDes+=nLineLen-i;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}else{
	// RTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetStringPtr(), pCMemory->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
				i = j = 0;
				while( i < nLineLen ){
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars ){
						if( TAB != pLine[i] && SPACE != pLine[i])
							j = i + nCharChars;
					}
					else if( 2 == nCharChars ){
						if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
							j = i + nCharChars;
					}
					i += nCharChars;
				}
				if(j>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[0], j );
					nPosDes+=j;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	pDes[nPosDes] = '\0';

	pCMemory->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
}

/*!	�����s�̃\�[�g�Ɏg���\����*/
typedef struct _SORTTABLE {
	string sKey1;
	string sKey2;
} SORTDATA, *SORTTABLE;

/*!	�����s�̃\�[�g�Ɏg���֐�(����) */
bool SortByKeyAsc (SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1<pst2->sKey1);}

/*!	�����s�̃\�[�g�Ɏg���֐�(�~��) */
bool SortByKeyDesc(SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1>pst2->sKey1);}

/*!	@brief �����s�̃\�[�g

	��I�����͉������s���Ȃ��D��`�I�����́A���͈̔͂��L�[�ɂ��ĕ����s���\�[�g�D
	
	@note �Ƃ肠�������s�R�[�h���܂ރf�[�^���\�[�g���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CEditView::Command_SORT(BOOL bAsc)	//bAsc:TRUE=����,FALSE=�~��
{
	int			nLFO , nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nCFO , nSelectColFromOld ; 	/* �͈͑I���J�n�� */
	int			nLTO , nSelectLineToOld  ;	/* �͈͑I���I���s */
	int			nCTO , nSelectColToOld   ;	/* �͈͑I���I���� */
	int			nColmFrom,nColmTo;
	int			nCF,nCT;
	int			nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	const char*	pLine;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	std::vector<SORTTABLE> sta;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}

	if( m_bBeginBoxSelect ){
		nLFO = m_sSelect.m_ptFrom.y;
		nCFO = m_sSelect.m_ptFrom.x;
		nLTO = m_sSelect.m_ptTo.y;
		nCTO = m_sSelect.m_ptTo.x;
		if( m_sSelect.m_ptFrom.x==m_sSelect.m_ptTo.x ){
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			m_sSelect.m_ptTo.x=m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		}
		if(m_sSelect.m_ptFrom.x<m_sSelect.m_ptTo.x){
			nCF=m_sSelect.m_ptFrom.x;
			nCT=m_sSelect.m_ptTo.x;
		}else{
			nCF=m_sSelect.m_ptTo.x;
			nCT=m_sSelect.m_ptFrom.x;
		}
	}
	bBeginBoxSelectOld=m_bBeginBoxSelect;
	nCaretPosYOLD=m_ptCaretPos.y;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptFrom.x,m_sSelect.m_ptFrom.y,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptTo.x,m_sSelect.m_ptTo.y,
		&nSelectColToOld,&nSelectLineToOld
	);

	if( bBeginBoxSelectOld ){
		++nSelectLineToOld;
	}
	else{
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if ( nSelectColToOld > 0 ) {
			// 2006.03.31 Moca nSelectLineToOld�́A�����s�Ȃ̂�Layout�n����DocLine�n�ɏC��
			const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( nSelectLineToOld );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->m_cEol ){
				++nSelectLineToOld;
			}
		}
	}
	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//�s�I������ĂȂ�
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( i );
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
				// 2006.03.31 genta std::string::assign���g���Ĉꎞ�ϐ��폜
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}
			else if(nColmFrom<nLineLen){	// BOX�I��͈͂̉E�[���s�����E�ɂ͂ݏo���Ă���ꍇ
				pst->sKey1=&pLine[nColmFrom];
			}
			pst->sKey2=pLine;
		}else{
			pst->sKey1=pLine;
		}
		sta.push_back(pst);
	}
	if(bAsc){
		std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
	}else{
		std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
	}
	cmemBuf.SetString( "" );
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey2.c_str() ); 
	}else{
		for (i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey1.c_str() );
	}

	//sta.clear(); �����ꂶ�Ⴞ�߂݂���
	for (i=0; i<j; i++) delete sta[i];

	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false
	);

	//	�I���G���A�̕���
	if(bBeginBoxSelectOld){
		m_bBeginBoxSelect=bBeginBoxSelectOld;
		m_sSelect.m_ptFrom.y=nLFO;
		m_sSelect.m_ptFrom.x=nCFO;
		m_sSelect.m_ptTo.y  =nLTO;
		m_sSelect.m_ptTo.x  =nCTO;
	}else{
		m_sSelect.m_ptFrom.y=nSelectLineFromOld;
		m_sSelect.m_ptFrom.x=nSelectColFromOld;
		m_sSelect.m_ptTo.y  =nSelectLineToOld;
		m_sSelect.m_ptTo.x  =nSelectColToOld;
	}
	if(nCaretPosYOLD==m_sSelect.m_ptFrom.y || m_bBeginBoxSelect ) {
		MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );
	}else{
		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, true );
	}
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();
}


/*! @brief �����s�̃}�[�W

	�A�����镨���s�œ��e������̕���1�s�ɂ܂Ƃ߂܂��D
	
	��`�I�����͂Ȃɂ����s���܂���D
	
	@note ���s�R�[�h���܂ރf�[�^���r���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CEditView::Command_MERGE(void)
{
	int			nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nSelectColFromOld ; /* �͈͑I���J�n�� */
	int			nSelectLineToOld  ;	/* �͈͑I���I���s */
	int			nSelectColToOld   ;	/* �͈͑I���I���� */
	int			nCaretPosYOLD;
	const char*	pLine;
	const char*	pLinew;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	if( m_bBeginBoxSelect ){
		return;
	}

	nCaretPosYOLD=m_ptCaretPos.y;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptFrom.x,m_sSelect.m_ptFrom.y,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_sSelect.m_ptTo.x,m_sSelect.m_ptTo.y,
		&nSelectColToOld,&nSelectLineToOld
	);

	// 2001.12.21 hor
	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if ( nSelectColToOld > 0 ) {
		const CLayout* pcLayout=m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(nSelectLineToOld);
		if( NULL != pcLayout && EOL_NONE != pcLayout->m_cEol ){
			++nSelectLineToOld;
		}
	}

	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//�s�I������ĂȂ�
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetString( "" );
	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		if( NULL == pLinew || strcmp(pLine,pLinew) ){
			cmemBuf.AppendString( pLine );
		}
		pLinew=pLine;
	}
	j=m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false
	);
	j-=m_pcEditDoc->m_cDocLineMgr.GetLineCount();

	//	�I���G���A�̕���
	m_sSelect.m_ptFrom.y=nSelectLineFromOld;
	m_sSelect.m_ptFrom.x=nSelectColFromOld;
	m_sSelect.m_ptTo.y  =nSelectLineToOld-j;
	m_sSelect.m_ptTo.x  =nSelectColToOld;
	if(nCaretPosYOLD==m_sSelect.m_ptFrom.y){
		MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );
	}else{
		MoveCursor( m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, true );
	}
	m_nCaretPosX_Prev = m_ptCaretPos.x;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* �����̃L�����b�g�ʒu�x */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();

	if(j){
		TopOkMessage( m_hWnd, _T("%d�s���}�[�W���܂����B"), j);
	}else{
		InfoMessage( m_hWnd, _T("�}�[�W�\�ȍs���݂���܂���ł����B") );
	}
}



// To Here 2001.12.03 hor
	
/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfo�̃J�v�Z���������܂��傤�B
*/
void CEditView::Command_Reconvert(void)
{
	//�T�C�Y���擾
	int nSize = SetReconvertStruct(NULL,false);
	if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
		return ;
	
	bool bUseUnicodeATOK = false;
	//�o�[�W�����`�F�b�N
	if( !OsSupportReconvert() ){
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(m_hWnd);
		if (SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_hWnd);
			return ;
		}

		// ATOK���g���邩�ǂ���
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0),sz,_countof(sz));
		if ( (strncmp(sz,_T("ATOK"),4) == 0) && (NULL != m_AT_ImmSetReconvertString) ){
			bUseUnicodeATOK = true;
		}else{
			//�Ή�IME�Ȃ�
			return;
		}
	}else{
		//���݂�IME���Ή����Ă��邩�ǂ���
		//IME�̃v���p�e�B
		if ( !(ImmGetProperty(GetKeyboardLayout(0),IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING) ){
			//�Ή�IME�Ȃ�			
			return ;
		}
	}
	
	//IME�̃R���e�L�X�g�擾
	HIMC hIMC = ::ImmGetContext( m_hWnd );
	
	//�̈�m��
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	//�\���̐ݒ�
	SetReconvertStruct( pReconv, bUseUnicodeATOK);
	
	//�ϊ��͈͂̒���
	if(bUseUnicodeATOK){
		(*m_AT_ImmSetReconvertString)(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//���������ϊ��͈͂�I������
	SetSelectionFromReonvert(pReconv, bUseUnicodeATOK);
	
	//�ĕϊ����s
	if(bUseUnicodeATOK){
		(*m_AT_ImmSetReconvertString)(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//�̈���
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_hWnd, hIMC);
}

/*!	�R���g���[���R�[�h�̓���(�_�C�A���O)
	@author	MIK
	@date	2002/06/02
*/
void CEditView::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//�R���g���[���R�[�h���̓_�C�A���O��\������
	if( cDlgCtrlCode.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		//�R���g���[���R�[�h����͂���
		Command_CHAR( cDlgCtrlCode.GetCharCode() );
	}
}

/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void CEditView::Command_JUMP_SRCHSTARTPOS(void)
{
	if( 0 <= m_ptSrchStartPos_PHY.x && 0 <= m_ptSrchStartPos_PHY.y )
	{
		int x, y;
		/* �͈͑I�𒆂� */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			m_ptSrchStartPos_PHY.x,
			m_ptSrchStartPos_PHY.y,
			&x, &y );
		//	2006.07.09 genta �I����Ԃ�ۂ�
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
	else
	{
		ErrorBeep();
	}
	return;
}

/*!	�����̊Ǘ�(�_�C�A���O)
	@author	MIK
	@date	2003/04/07
*/
void CEditView::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//�_�C�A���O��\������
	if( !cDlgFavorite.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	return;
}

/*! ���͂�����s�R�[�h��ݒ�

	@author moca
	@date 2003.06.23 �V�K�쐬
*/
void CEditView::Command_CHGMOD_EOL( EEolType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		m_pcEditDoc->SetNewLineCode( e );
		// �X�e�[�^�X�o�[���X�V���邽��
		// �L�����b�g�̍s���ʒu��\������֐����Ăяo��
		DrawCaretPosInfo();
	}
}

/*! ��Ɏ�O�ɕ\��
	@date 2004.09.21 Moca
*/
void CEditView::Command_WINTOPMOST( LPARAM lparam )
{
	m_pcEditDoc->m_pcEditWnd->WindowTopMost( int(lparam) );
}

/*!	@brief ���p���̐ݒ�
	@date Jan. 29, 2005 genta �V�K�쐬
*/
void CEditView::Command_SET_QUOTESTRING( const char* quotestr )
{
	if( quotestr == NULL )
		return;

	strncpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( m_pShareData->m_Common.m_sFormat.m_szInyouKigou ));
	
	m_pShareData->m_Common.m_sFormat.m_szInyouKigou[ _countof( m_pShareData->m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = '\0';
}

/*!	@brief �E�B���h�E�ꗗ�|�b�v�A�b�v�\�������i�t�@�C�����̂݁j
	@date  2006.03.23 fon �V�K�쐬
	@date  2006.05.19 genta �R�}���h���s�v����\�������ǉ�
	@date  2007.07.07 genta �R�}���h���s�v���̒l��ύX
*/
void CEditView::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;

	//�E�B���h�E�ꗗ���|�b�v�A�b�v�\������
	pCEditWnd->PopupWinList(( nCommandFrom & FA_FROMKEYBOARD ) != FA_FROMKEYBOARD );
	// 2007.02.27 ryoji �A�N�Z�����[�^�L�[����łȂ���΃}�E�X�ʒu��

}


/*!	@brief �}�N���p�A�E�g�v�b�g�E�C���h�E�ɕ\��
	@date 2006.04.26 maru �V�K�쐬
*/
void CEditView::Command_TRACEOUT( const char* outputstr, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameter�ɂ�镶����W�J�L��
	if (nFlgOpt & 0x01) {
		char Buffer[2048];
		m_pcEditDoc->ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOut( "%s", Buffer );
	} else {
		CShareData::getInstance()->TraceOut( "%s", outputstr );
	}

	// 0x02 ���s�R�[�h�̗L��
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOut( "\r\n" );

}

/*!	@brief �ҏW���̓��e��ʖ��ۑ�

	��ɕҏW���̈ꎞ�t�@�C���o�͂Ȃǂ̖ړI�Ɏg�p����D
	���݊J���Ă���t�@�C��(m_szFilePath)�ɂ͉e�����Ȃ��D

	@param[in] filename �o�̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:���݂̕����R�[�h���ێ�
	@param[in] nFlgOpt ����I�v�V����
		@li	0x01:�I��͈͂��o�� (��I����Ԃł���t�@�C�����o�͂���)

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���쐬�Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CEditView::Command_PUTFILE( const char* filename, const ECodeType nCharCode, int nFlgOpt )
{
	BOOL	bResult = TRUE;
	ECodeType	nSaveCharCode;
	nSaveCharCode = nCharCode;
	if(filename[0] == '\0') {
		return FALSE;
	}
	
	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = m_pcEditDoc->m_nCharCode;
	
	//	2007.09.08 genta CEditDoc::FileWrite()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_hWnd );
	
	if(nFlgOpt & 0x01)
	{	/* �I��͈͂��o�� */
		try
		{
			CFileWrite cfw(filename);
			if ( m_pcEditDoc->m_bBomExist) {
				switch( nSaveCharCode ){
				case CODE_UNICODE:
					cfw.Write("\xff\xfe",sizeof(char)*2);
					break;
				case CODE_UNICODEBE:
					cfw.Write( "\xfe\xff", sizeof(char) * 2 );
					break;
				case CODE_UTF8: // 2003.05.04 Moca BOM�̊ԈႢ�����
					cfw.Write( "\xef\xbb\xbf", sizeof(char) * 3 );
					break;
				default:
					//	genta �����ɗ���̂̓o�O��
					//	2007.09.08 genta �ǉ�
					PleaseReportToAuthor( NULL, _T("CEditView::Command_PUTFILE/BOM Error\nSaveCharCode=%d"), nSaveCharCode );
				}
			}

			/* �I��͈͂̎擾 */
			CMemory cMem;
			GetSelectedData(&cMem, FALSE, NULL, FALSE, false);

			/* �������ݎ��̃R�[�h�ϊ� */
			switch( nSaveCharCode ){
				case CODE_UNICODE:	cMem.SJISToUnicode();break;	/* SJIS��Unicode�R�[�h�ϊ� */
				case CODE_UTF8:		cMem.SJISToUTF8();break;	/* SJIS��UTF-8�R�[�h�ϊ� */
				case CODE_UTF7:		cMem.SJISToUTF7();break;	/* SJIS��UTF-7�R�[�h�ϊ� */
				case CODE_EUC:		cMem.SJISToEUC();break;		/* SJIS��EUC�R�[�h�ϊ� */
				case CODE_JIS:		cMem.SJIStoJIS();break;		/* SJIS��JIS�R�[�h�ϊ� */
				case CODE_UNICODEBE:	cMem.SJISToUnicodeBE();break;	/* SJIS��UnicodeBE�R�[�h�ϊ� */
				case CODE_SJIS:		/* NO BREAK */
				default:			break;
			}
			if( 0 < cMem.GetStringLength() ) cfw.Write(cMem.GetStringPtr(),sizeof(char)*cMem.GetStringLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage( NULL,
				_T("\'%s\'\n�t�@�C����ۑ��ł��܂���B\n�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				filename);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage( NULL, _T("�t�@�C���̏������ݒ��ɃG���[���������܂����B") );
			bResult = FALSE;
		}
	}
	else {	/* �t�@�C���S�̂��o�� */
		CFileTime	cfiletime;
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;
		
		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_hwndProgressBar;
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		bResult = (BOOL)m_pcEditDoc->m_cDocLineMgr.WriteFile(	// �ꎞ�t�@�C���o��
					filename, m_pcEditDoc->m_hWnd, hwndProgress, nSaveCharCode,
					&cfiletime, EOL_NONE , m_pcEditDoc->m_bBomExist );

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}

/*!	@brief �J�[�\���ʒu�Ƀt�@�C����}��

	���݂̃J�[�\���ʒu�Ɏw��̃t�@�C����ǂݍ��ށD

	@param[in] filename ���̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:�O�񕶎��R�[�h�������͎������ʂ̌��ʂɂ��
	@param[in] nFlgOpt ����I�v�V�����i���݂͖���`�D0���w��̂��Ɓj

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���I�[�v���Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CEditView::Command_INSFILE( const char* filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl;
	const char*	pLine;
	CEol cEol;
	int			nLineLen;
	int			nLineNum = 0;
	ECodeType	nSaveCharCode;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	BOOL		bResult = TRUE;

	if(filename[0] == '\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileRead()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_hWnd );

	// �͈͑I�𒆂Ȃ�}������I����Ԃɂ��邽��	/* 2007.04.29 maru */
	int	nLineFrom, nColmFrom;
	BOOL	bBeforeTextSelected = IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = m_sSelect.m_ptFrom.y;
		nColmFrom = m_sSelect.m_ptFrom.x;
	}


	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo		fi;
		const CMRUFile		cMRU;
		if ( cMRU.GetEditInfo( filename, &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = m_pcEditDoc->m_nCharCode;
		}
	}
	
	/* �����܂ł��ĕ����R�[�h�����肵�Ȃ��Ȃ�ǂ����������� */
	if( !IsValidCodeType(nSaveCharCode) ) nSaveCharCode = CODE_SJIS;
	
	try{
		// �t�@�C�����J��
		cfl.FileOpen( filename, nSaveCharCode, 0 );

		/* �t�@�C���T�C�Y��65KB���z������i���_�C�A���O�\�� */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
				::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
			}
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw CError_FileRead �𓊂��܂�
		while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
			++nLineNum;
			Command_INSTEXT( false, pLine, nLineLen, TRUE);

			/* �i���_�C�A���O�L�� */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				break;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				::PostMessage( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
				Redraw();
			}
		}
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, _T("file open error [%s]"), filename );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, _T("�t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B") );
		bResult = FALSE;
	} // ��O�����I���

	delete pcDlgCancel;

	if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
		SetSelectArea( nLineFrom, nColmFrom, m_ptCaretPos.y, m_ptCaretPos.x );
		DrawSelectArea();
	}
	Redraw();
	return bResult;
}

/*!
	@brief �e�L�X�g�̐܂�Ԃ����@��ύX����
	
	@param[in] nWrapMethod �܂�Ԃ����@
		WRAP_NO_TEXT_WRAP  : �܂�Ԃ��Ȃ�
		WRAP_SETTING_WIDTH ; �w�茅�Ő܂�Ԃ�
		WRAP_WINDOW_WIDTH  ; �E�[�Ő܂�Ԃ�
	
	@note �E�B���h�E�����E�ɕ�������Ă���ꍇ�A�����̃E�B���h�E����܂�Ԃ����Ƃ���B
	
	@date 2008.05.31 nasukoji	�V�K�쐬
	@date 2009.08.28 nasukoji	�e�L�X�g�̍ő啝���Z�o����
*/
void CEditView::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	// ���݂̐ݒ�l�Ɠ����Ȃ牽�����Ȃ�
	if( m_pcEditDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	int nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// �܂�Ԃ��Ȃ�
		nWidth = MAXLINEKETAS;	// �A�v���P�[�V�����̍ő啝�Ő܂�Ԃ�
		break;

	case WRAP_SETTING_WIDTH:	// �w�茅�Ő܂�Ԃ�
		nWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// �E�[�Ő܂�Ԃ�
		// �E�B���h�E�����E�ɕ�������Ă���ꍇ�͍����̃E�B���h�E�����g�p����
		nWidth = ViewColNumToWrapColNum( m_pcEditDoc->m_pcEditViewArr[0]->m_nViewColNum );
		break;

	default:
		return;		// �s���Ȓl�̎��͉������Ȃ�
	}

	m_pcEditDoc->m_nTextWrapMethodCur = nWrapMethod;	// �ݒ���L��

	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�^�ꎞ�ݒ�K�p����	// 2008.06.08 ryoji
	m_pcEditDoc->m_bTextWrapMethodCurTemp = !( m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// �܂�Ԃ��ʒu��ύX
	m_pcEditDoc->ChangeLayoutParam( false, m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), nWidth );

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		m_pcEditDoc->m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
		m_pcEditDoc->RedrawAllViews( NULL );				// �X�N���[���o�[�̍X�V���K�v�Ȃ̂ōĕ\�������s����
	}else{
		m_pcEditDoc->m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}

/*!
	@brief �w��ʒu�܂��͎w��͈͂��e�L�X�g�̑��݂��Ȃ��G���A���`�F�b�N����

	@param[in] nColmFrom  �w��ʒu�܂��͎w��͈͊J�n�J����
	@param[in] nLineFrom  �w��ʒu�܂��͎w��͈͊J�n���C��
	@param[in] nColmTo    �w��͈͏I���J����
	@param[in] nLineTo    �w��͈͏I�����C��
	@param[in] bSelect    �͈͎w��
	@param[in] bBoxSelect ��`�I��
	
	@retval TRUE  �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g�����݂��Ȃ�
			FALSE �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g�����݂���

	@date 2008.08.03 nasukoji	�V�K�쐬
*/
BOOL CEditView::IsEmptyArea( int nColmFrom, int nLineFrom, int nColmTo, int nLineTo, BOOL bSelect, BOOL bBoxSelect )
{
	BOOL result;

	if( bSelect && !bBoxSelect && nLineFrom != nLineTo ){	// �����s�͈͎̔w��
		// �����s�ʏ�I�������ꍇ�A�K���e�L�X�g���܂�
		result = FALSE;
	}else{
		if( bSelect ){
			int nTemp;

			// �͈͂̒���
			if( nLineFrom > nLineTo ){
				nTemp = nLineFrom;
				nLineFrom = nLineTo;
				nLineTo = nTemp;
			}

			if( nColmFrom > nColmTo ){
				nTemp = nColmFrom;
				nColmFrom = nColmTo;
				nColmTo = nTemp;
			}
		}else{
			nLineTo = nLineFrom;
		}

		const CLayout*	pcLayout;
		int nLineLen;

		result = TRUE;
		for( int nLineNum = nLineFrom; nLineNum <= nLineTo; nLineNum++ ){
			if( m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout ) ){
				// �w��ʒu�ɑΉ�����s�̃f�[�^���̈ʒu
				LineColmnToIndex2( pcLayout, nColmFrom, nLineLen );
				if( nLineLen == 0 ){	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					result = FALSE;		// �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g������
					break;
				}
			}
		}
	}

	return result;
}

/*[EOF]*/
