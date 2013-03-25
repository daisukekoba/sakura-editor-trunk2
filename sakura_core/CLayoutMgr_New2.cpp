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
	int	nxFrom;
	int	nyFrom;
	int	nxTo;
	int	nyTo;
	int nLineWork;

	int	nWork_nLines = m_nLines;	//�ύX�O�̑S�s���̕ۑ�	@@@ 2002.04.19 MIK

	/* �u���擪�ʒu�̃��C�A�E�g��� */
	CLayout* pLayout = (CLayout*)SearchLineByLayoutY( pArg->nDelLineFrom );
	EColorIndexType nCurrentLineType = COLORIDX_DEFAULT;
	CLayout* pLayoutWork = pLayout;
	nLineWork = pArg->nDelLineFrom;

	if( pLayoutWork ){
		while( 0 != pLayoutWork->m_nOffset ){
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
	LayoutToLogic( pArg->nDelColmFrom, pArg->nDelLineFrom, &nxFrom, &nyFrom );
	LayoutToLogic( pArg->nDelColmTo, pArg->nDelLineTo, &nxTo, &nyTo );

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.nDelLineFrom = nyFrom;			// �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n)
	DLRArg.nDelPosFrom = nxFrom;			// �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n)
	DLRArg.nDelLineTo = nyTo;				// �폜�͈͍s  To   ���s�P�ʂ̍s�ԍ� 0�J�n)
	DLRArg.nDelPosTo = nxTo;				// �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n)
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	// �폜���ꂽ�f�[�^��ۑ�
	DLRArg.pInsData = pArg->pInsData;			// �}������f�[�^
	DLRArg.nInsDataLen = pArg->nInsDataLen;		// �}������f�[�^�̒���
	m_pcDocLineMgr->ReplaceData(
		&DLRArg
	);
	pArg->nNewLine = DLRArg.nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->nNewPos = DLRArg.nNewPos;				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */


	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */

	int nAllLinesOld = m_nLines;
	int	nModifyLayoutLinesOld = 0;
	CLayout* pLayoutPrev;
	int nWork;
	nWork = __max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );


	if( pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			nyFrom,
			nyFrom + nWork,
			nyFrom, nxFrom,
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
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}
	else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = pArg->nDelLineFrom;						// �ҏW�J�n��
	ctwArg.nColmFrom    = pArg->nDelColmFrom;						// �ҏW�J�n��
	ctwArg.nDelLines    = pArg->nDelLineTo - pArg->nDelLineFrom;	// �폜�s�Ȃ�
	ctwArg.nAllLinesOld = nWork_nLines;								// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = pArg->nInsDataLen ? TRUE : FALSE;			// �ǉ�������̗L��

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nyFrom, nxFrom,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = nWork_nLines - m_nLines;	//�ύX��̑S�s���Ƃ̍���	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
	pArg->nModLineFrom = pArg->nDelLineFrom;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom */
	pArg->nModLineTo += ( pArg->nModLineFrom - 1 ) ;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo */

	/* ���C�A�E�g�ʒu�ւ̕ϊ� */
	LogicToLayout( pArg->nNewPos, pArg->nNewLine, &pArg->nNewPos, &pArg->nNewLine );
}


/*[EOF]*/
