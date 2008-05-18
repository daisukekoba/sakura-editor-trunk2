#include "stdafx.h"
#include "CTextDrawer.h"
#include <vector>
#include "CTextMetrics.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CEol.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"
#include "charset/charcode.h"
#include "doc/CLayout.h"
#include "view/colors/CColorStrategy.h"


const CTextArea* CTextDrawer::GetTextArea() const
{
	return &m_pEditView->GetTextArea();
}

using namespace std;

EColorIndexType CTextDrawer::_GetColorIdx(EColorIndexType nColorIdx, bool bSearchStringMode) const
{
	if(bSearchStringMode)return COLORIDX_SEARCH;						//�����q�b�g�F
	if(CTypeSupport(m_pEditView,nColorIdx).IsDisp())return nColorIdx;	//����F
	return COLORIDX_TEXT;												//�ʏ�F
}


/*
�e�L�X�g�\��
@@@ 2002.09.22 YAZAKI    const unsigned char* pData���Aconst char* pData�ɕύX
@@@ 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const
{
	if( 0 >= nLength ){
		return;
	}
	int x=pDispPos->GetDrawPos().x;
	int y=pDispPos->GetDrawPos().y;

	//�K�v�ȃC���^�[�t�F�[�X���擾
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//�����Ԋu�z��𐶐�
	vector<int> vDxArray;
	const int* pDxArray=pMetrics->GenerateDxArray(&vDxArray,pData,nLength,this->m_pEditView->GetTextMetrics().GetHankakuDx());

	//������̃s�N�Z����
	int nTextWidth=pMetrics->CalcTextWidth(pData,nLength,pDxArray);

	//�e�L�X�g�̕`��͈͂̋�`�����߂� -> rcClip
	CMyRect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + m_pEditView->GetTextMetrics().GetHankakuDy();
	if( rcClip.left < pArea->GetAreaLeft() ){
		rcClip.left = pArea->GetAreaLeft();
	}

	//�����Ԋu
	int nDx = m_pEditView->GetTextMetrics().GetHankakuDx();

	if( pArea->IsRectIntersected(rcClip) && rcClip.top >= pArea->GetAreaTop() ){

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW_AnyBuild�̐������
		if( rcClip.Width() > pArea->GetAreaWidth() ){
			rcClip.right = rcClip.left + pArea->GetAreaWidth();
		}

		// �E�B���h�E�̍��ɂ��ӂꂽ������ -> nBefore
		// 2007.09.08 kobake�� �u�E�B���h�E�̍��v�ł͂Ȃ��u�N���b�v�̍��v�����Ɍv�Z�����ق����`��̈��ߖ�ł��邪�A
		//                        �o�O���o��̂��|���̂łƂ肠�������̂܂܁B
		int nBeforeLogic = 0;
		CLayoutInt nBeforeLayout = CLayoutInt(0);
		if ( x < 0 ){
			int nLeftLayout = ( 0 - x ) / nDx - 1;
			while (nBeforeLayout < nLeftLayout){
				nBeforeLayout += CNativeW::GetKetaOfChar( pData, nLength, nBeforeLogic );
				nBeforeLogic  += CNativeW::GetSizeOfChar( pData, nLength, nBeforeLogic );
			}
		}

		/*
		// �E�B���h�E�̉E�ɂ��ӂꂽ������ -> nAfter
		int nAfterLayout = 0;
		if ( rcClip.right < x + nTextWidth ){
			//	-1���Ă��܂����i������͂�����ˁH�j
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// �`��J�n�ʒu
		int nDrawX = x + (Int)nBeforeLayout * nDx;

		// ���ۂ̕`�敶����|�C���^
		const wchar_t* pDrawData          = &pData[nBeforeLogic];
		int            nDrawDataMaxLength = nLength - nBeforeLogic;

		// ���ۂ̕����Ԋu�z��
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// �`�悷�镶���񒷂����߂� -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; //���߂�ׂ��s�N�Z����
		if(nRequiredWidth <= 0)goto end;
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while(nWorkWidth < nRequiredWidth)
		{
			if(nDrawLength >= nDrawDataMaxLength)break;
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					//X
			y,						//Y
			ExtTextOutOption(),
			&rcClip,
			pDrawData,				//������
			nDrawLength,			//������
			pDrawDxArray			//�����Ԋu�̓������z��
		);
	}

end:
	//�`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

void CTextDrawer::DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const
{
	DispPos sPos(m_pEditView->GetTextMetrics().GetHankakuDx(),m_pEditView->GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(x,y));
	DispText(hdc,&sPos,pData,nLength);
}













// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �w�茅�c��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�w�茅�c���̕`��
	@date 2005.11.08 Moca �V�K�쐬
	@date 2006.04.29 Moca �����E�_���̃T�|�[�g�B�I�𒆂̔��]�΍�ɍs���Ƃɍ�悷��悤�ɕύX
	    �c���̐F���e�L�X�g�̔w�i�F�Ɠ����ꍇ�́A�c���̔w�i�F��EXOR�ō�悷��
	@note Common::m_nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void CTextDrawer::DispVerticalLines(
	HDC			hdc,		//!< ��悷��E�B���h�E��DC
	int			nTop,		//!< ����������[�̃N���C�A���g���Wy
	int			nBottom,	//!< �����������[�̃N���C�A���g���Wy
	CLayoutInt	nLeftCol,	//!< ���������͈͂̍����̎w��
	CLayoutInt	nRightCol	//!< ���������͈͂̉E���̎w��(-1�Ŗ��w��)
) const
{
	const CEditView* pView=m_pEditView;

	const STypeConfig&	typeData = pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	CTypeSupport cVertType(pView,COLORIDX_VERTLINE);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	if(!cVertType.IsDisp())return;

	nLeftCol = t_max( pView->GetTextArea().GetViewLeftCol(), nLeftCol );

	const CLayoutInt nWrapKetas  = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharDx  = pView->GetTextMetrics().GetHankakuDx();
	if( nRightCol < 0 ){
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = GetDllShareData().m_Common.m_sWindow.m_nVertLineOffset + pView->GetTextArea().GetAreaLeft();
	const int nPosXLeft   = max( pView->GetTextArea().GetAreaLeft() + (Int)(nLeftCol  - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaLeft() );
	const int nPosXRight  = min( pView->GetTextArea().GetAreaLeft() + (Int)(nRightCol - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaRight() );
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)pView->GetTextArea().GetViewTopLine() : 0) + pView->GetTextArea().GetAreaTop() + nTop) % 2 == 1);

	// ����
	const bool bBold = cVertType.IsFatFont();
	// �h�b�g��(����������]�p/�e�X�g�p)
	const bool bDot = cVertType.HasUnderLine();
	const bool bExorPen = ( cVertType.GetTextColor() == cTextType.GetBackColor() );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetBackColor() );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetTextColor() );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXCol��1�J�n�BGetTextArea().GetViewLeftCol()��0�J�n�Ȃ̂Œ��ӁB
		CLayoutInt nXCol = typeData.m_nVertLineIdx[k];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
		// nXCol���}�C�i�X���ƌJ��Ԃ��Bk+1���I���l�Ak+2���X�e�b�v���Ƃ��ė��p����
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// ���͈͂̎n�߂܂ŃX�L�b�v
				if( nXCol < pView->GetTextArea().GetViewLeftCol() ){
					nXCol = pView->GetTextArea().GetViewLeftCol() + nXColAdd - (pView->GetTextArea().GetViewLeftCol() - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapKetas < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + (Int)( nXCol - 1 - pView->GetTextArea().GetViewLeftCol() ) * nCharDx;
			// 2006.04.30 Moca ���̈����͈́E���@��ύX
			// �����̏ꍇ�A����������悷��\��������B
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// �_���ō��B1�h�b�g�̐����쐬
					int y = nTop;
					// �X�N���[�����Ă������؂�Ȃ��悤�ɍ��W�𒲐�
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �s�ԍ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	HDC				hdc,
	CLayoutInt		nLineNum,
	int				y
) const
{
	//$$ �������FSearchLineByLayoutY�ɃL���b�V������������
	const CLayout*	pcLayout = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
	if(!pcLayout)return;

	const CEditView* pView=m_pEditView;
	const STypeConfig* pTypes=&pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	//	Sep. 23, 2002 genta ���ʎ��̂����肾��
	int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;

	CTypeSupport cTextType(pView,COLORIDX_TEXT);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndex������                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	/* �s�ԍ� */
	const CDocLine*	pCDocLine;
	if( pcLayout ){
		pCDocLine = pcLayout->GetDocLineRef();

		if( pView->m_pcEditDoc->m_cDocEditor.IsModified() && CModifyVisitor().IsLineModified(pCDocLine) ){		/* �ύX�t���O */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() )	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* �s�ԍ��i�ύX�s�j */
		}
	}

	EDiffMark type = CDiffLineGetter(pCDocLine).GetLineDiffMark();
	{
		//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//�ǉ�
				if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//�ύX
				if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//�폜
			case MARK_DIFF_DEL_EX:	//�폜
				if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	// 02/10/16 ai
	// �u�b�N�}�[�N�̕\��
	if(CBookmarkGetter(pCDocLine).IsBookmarked()){
		if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
			nColorIndex = COLORIDX_MARK;
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���肳�ꂽnColorIndex���g���ĕ`��               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* �s�ԍ��\���^��\�� */
		wchar_t szLineNum[18];

		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if( pTypes->m_bLineNumIsCRLF ){
			/* �_���s�ԍ��\�����[�h */
			if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){
				wcscpy( szLineNum, L" " );
			}else{
				_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
			}
		}else{
			/* �����s�i���C�A�E�g�s�j�ԍ��\�����[�h */
			_itow( (Int)nLineNum + 1, szLineNum, 10 );
		}

		int nLineCols = wcslen( szLineNum );

		//�F�A�t�H���g
		cTextType.SetBkColor(hdc);    //�w�i�F�F�e�L�X�g�̔w�i�F //	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ���
		cColorType.SetTextColor(hdc); //�e�L�X�g�F�s�ԍ��̐F
		cColorType.SetFont(hdc);      //�t�H���g�F�s�ԍ��̃t�H���g

		// �]���𖄂߂�
		RECT	rcClip;
		rcClip.left   = nLineNumAreaWidth;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW_AnyBuild(
			hdc,
			rcClip.left,
			y,
			ExtTextOutOption(),
			&rcClip,
			L" ",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ��邽�߁C�w�i�F�̐ݒ���ړ�
		SetBkColor( hdc, cColorType.GetBackColor() );		/* �s�ԍ��w�i�̐F */

		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 2 == pTypes->m_nLineTermType ){
			//	Sep. 22, 2002 genta
			szLineNum[ nLineCols ] = pTypes->m_cLineTermChar;
			szLineNum[ ++nLineCols ] = '\0';
		}
		rcClip.left = 0;

		//	Sep. 23, 2002 genta
		rcClip.right = nLineNumAreaWidth;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW_AnyBuild( hdc,
			drawNumTop,
			y,
			ExtTextOutOption(),
			&rcClip,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 1 == pTypes->m_nLineTermType ){
			cColorType.SetSolidPen(hdc,0);
			::MoveToEx( hdc, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( hdc, nLineNumAreaWidth - 2, y + nLineHeight );
			cColorType.RewindPen(hdc);
		}
		cColorType.RewindColors(hdc);
		cTextType.RewindColors(hdc);
		cColorType.RewindFont(hdc);
	}
	else{
		RECT	rcClip;
		// �s�ԍ��G���A�̔w�i�`��
		rcClip.left   = 0;
		rcClip.right  = pView->GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cColorType.FillBack(hdc,rcClip);
		
		// Mar. 5, 2003, Moca
		// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��
		rcClip.left   = rcClip.right;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cTextType.FillBack(hdc,rcClip);
	}

	// 2001.12.03 hor
	/* �Ƃ肠�����u�b�N�}�[�N�ɏc�� */
	if(CBookmarkGetter(pCDocLine).IsBookmarked() && !cMarkType.IsDisp() )
	{
		cColorType.SetSolidPen(hdc,2);
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		cColorType.RewindPen(hdc);
	}

	if( type )	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		cColorType.SetSolidPen(hdc,1);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//�ǉ�
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo  ( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//�ύX
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo  ( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo  ( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//�폜
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy + 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//�폜(EOF)
			cy += 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy - 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy - 4 );
			break;
		}

		cColorType.RewindPen(hdc);
	}
}

