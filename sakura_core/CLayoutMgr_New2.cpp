/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "Debug.h"
#include <commctrl.h>
#include <stdlib.h>
#include "CLayout.h" // 2002/2/10 aroka
#include "CDocLineMgr.h" // 2002/2/10 aroka
#include "CEditDoc.h"		// 2009.08.28 nasukoji



/* ������u�� */
void CLayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg*	pArg
)
{
	int	nWork_nLines = m_nLines;	//�ύX�O�̑S�s���̕ۑ�	@@@ 2002.04.19 MIK

	/* �u���擪�ʒu�̃��C�A�E�g��� */
	EColorIndexType nCurrentLineType = COLORIDX_DEFAULT;
	int nLineWork = pArg->sDelRange.m_ptFrom.y;

	CLayout* pLayoutWork = (CLayout*)SearchLineByLayoutY( pArg->sDelRange.m_ptFrom.y );
	if( pLayoutWork ){
		while( 0 != pLayoutWork->m_ptLogicPos.x ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		nCurrentLineType = pLayoutWork->m_nTypePrev;
	}


	/*
	||  �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu) ��
	||  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptFrom;
	CLogicPoint ptTo;
	LayoutToLogic( pArg->sDelRange.m_ptFrom.x, pArg->sDelRange.m_ptFrom.y, &ptFrom.x, &ptFrom.y );
	LayoutToLogic( pArg->sDelRange.m_ptTo.x, pArg->sDelRange.m_ptTo.y, &ptTo.x, &ptTo.y );

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.sDelRange.m_ptFrom = ptFrom;			// �폜�͈�From 0�J�n)
	DLRArg.sDelRange.m_ptTo = ptTo;				// �폜�͈�To   0�J�n)
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	// �폜���ꂽ�f�[�^��ۑ�
	DLRArg.pInsData = pArg->pInsData;			// �}������f�[�^
	DLRArg.nInsDataLen = pArg->nInsDataLen;		// �}������f�[�^�̒���
	m_pcDocLineMgr->ReplaceData(
		&DLRArg
	);


	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */

	int	nModifyLayoutLinesOld = 0;
	CLayout* pLayoutPrev;
	int nWork;
	nWork = t_max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );


	if( pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			ptFrom.y,
			ptFrom.y + nWork,
			ptFrom.y, ptFrom.x,
			&nModifyLayoutLinesOld
		);

		/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
		/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
		/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
		if( 0 != DLRArg.nInsLineNum - DLRArg.nDeletedLineNum ){
			ShiftLogicalLineNum(
				pLayoutPrev,
				DLRArg.nInsLineNum - DLRArg.nDeletedLineNum
			);
		}
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int	nRowNum;
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_ptLogicPos.y;
		}
	}
	else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_ptLogicPos.y -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout     = pArg->sDelRange.m_ptFrom;									// �ҏW�J�n�ʒu
	ctwArg.nDelLines    = pArg->sDelRange.m_ptTo.y - pArg->sDelRange.m_ptFrom.y;	// �폜�s�Ȃ�
	ctwArg.nAllLinesOld = nWork_nLines;												// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = pArg->nInsDataLen ? TRUE : FALSE;							// �ǉ�������̗L��

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		ptFrom.y, ptFrom.x,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = nWork_nLines - m_nLines;	//�ύX��̑S�s���Ƃ̍���	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
	pArg->nModLineFrom = pArg->sDelRange.m_ptFrom.y;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom */
	pArg->nModLineTo += ( pArg->nModLineFrom - 1 ) ;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo */

	//2007.10.18 kobake LayoutReplaceArg::ptLayoutNew�͂����ŎZ�o����̂�������
	LogicToLayout( DLRArg.ptNewPos.x, DLRArg.ptNewPos.y, &pArg->ptLayoutNew.x, &pArg->ptLayoutNew.y );
}


/*[EOF]*/
