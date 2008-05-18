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
	if(bSearchStringMode)return COLORIDX_SEARCH;						//検索ヒット色
	if(CTypeSupport(m_pEditView,nColorIdx).IsDisp())return nColorIdx;	//特殊色
	return COLORIDX_TEXT;												//通常色
}


/*
テキスト表示
@@@ 2002.09.22 YAZAKI    const unsigned char* pDataを、const char* pDataに変更
@@@ 2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const
{
	if( 0 >= nLength ){
		return;
	}
	int x=pDispPos->GetDrawPos().x;
	int y=pDispPos->GetDrawPos().y;

	//必要なインターフェースを取得
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray=pMetrics->GenerateDxArray(&vDxArray,pData,nLength,this->m_pEditView->GetTextMetrics().GetHankakuDx());

	//文字列のピクセル幅
	int nTextWidth=pMetrics->CalcTextWidth(pData,nLength,pDxArray);

	//テキストの描画範囲の矩形を求める -> rcClip
	CMyRect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + m_pEditView->GetTextMetrics().GetHankakuDy();
	if( rcClip.left < pArea->GetAreaLeft() ){
		rcClip.left = pArea->GetAreaLeft();
	}

	//文字間隔
	int nDx = m_pEditView->GetTextMetrics().GetHankakuDx();

	if( pArea->IsRectIntersected(rcClip) && rcClip.top >= pArea->GetAreaTop() ){

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW_AnyBuildの制限回避
		if( rcClip.Width() > pArea->GetAreaWidth() ){
			rcClip.right = rcClip.left + pArea->GetAreaWidth();
		}

		// ウィンドウの左にあふれた文字数 -> nBefore
		// 2007.09.08 kobake注 「ウィンドウの左」ではなく「クリップの左」を元に計算したほうが描画領域を節約できるが、
		//                        バグが出るのが怖いのでとりあえずこのまま。
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
		// ウィンドウの右にあふれた文字数 -> nAfter
		int nAfterLayout = 0;
		if ( rcClip.right < x + nTextWidth ){
			//	-1してごまかす（うしろはいいよね？）
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// 描画開始位置
		int nDrawX = x + (Int)nBeforeLayout * nDx;

		// 実際の描画文字列ポインタ
		const wchar_t* pDrawData          = &pData[nBeforeLogic];
		int            nDrawDataMaxLength = nLength - nBeforeLogic;

		// 実際の文字間隔配列
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// 描画する文字列長を求める -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; //埋めるべきピクセル幅
		if(nRequiredWidth <= 0)goto end;
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while(nWorkWidth < nRequiredWidth)
		{
			if(nDrawLength >= nDrawDataMaxLength)break;
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}

		//描画
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					//X
			y,						//Y
			ExtTextOutOption(),
			&rcClip,
			pDrawData,				//文字列
			nDrawLength,			//文字列長
			pDrawDxArray			//文字間隔の入った配列
		);
	}

end:
	//描画位置を進める
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

void CTextDrawer::DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const
{
	DispPos sPos(m_pEditView->GetTextMetrics().GetHankakuDx(),m_pEditView->GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(x,y));
	DispText(hdc,&sPos,pData,nLength);
}













// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        指定桁縦線                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	指定桁縦線の描画
	@date 2005.11.08 Moca 新規作成
	@date 2006.04.29 Moca 太線・点線のサポート。選択中の反転対策に行ごとに作画するように変更
	    縦線の色がテキストの背景色と同じ場合は、縦線の背景色をEXORで作画する
	@note Common::m_nVertLineOffsetにより、指定桁の前の文字の上に作画されることがある。
*/
void CTextDrawer::DispVerticalLines(
	HDC			hdc,		//!< 作画するウィンドウのDC
	int			nTop,		//!< 線を引く上端のクライアント座標y
	int			nBottom,	//!< 線を引く下端のクライアント座標y
	CLayoutInt	nLeftCol,	//!< 線を引く範囲の左桁の指定
	CLayoutInt	nRightCol	//!< 線を引く範囲の右桁の指定(-1で未指定)
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

	// 太線
	const bool bBold = cVertType.IsFatFont();
	// ドット線(下線属性を転用/テスト用)
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
		// nXColは1開始。GetTextArea().GetViewLeftCol()は0開始なので注意。
		CLayoutInt nXCol = typeData.m_nVertLineIdx[k];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
		// nXColがマイナスだと繰り返し。k+1を終了値、k+2をステップ幅として利用する
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// 作画範囲の始めまでスキップ
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
			// 2006.04.30 Moca 線の引く範囲・方法を変更
			// 太線の場合、半分だけ作画する可能性がある。
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// 点線で作画。1ドットの線を作成
					int y = nTop;
					// スクロールしても線が切れないように座標を調整
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
//                          行番号                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	HDC				hdc,
	CLayoutInt		nLineNum,
	int				y
) const
{
	//$$ 高速化：SearchLineByLayoutYにキャッシュを持たせる
	const CLayout*	pcLayout = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
	if(!pcLayout)return;

	const CEditView* pView=m_pEditView;
	const STypeConfig* pTypes=&pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	//	Sep. 23, 2002 genta 共通式のくくりだし
	int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;

	CTypeSupport cTextType(pView,COLORIDX_TEXT);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndexを決定                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	/* 行番号 */
	const CDocLine*	pCDocLine;
	if( pcLayout ){
		pCDocLine = pcLayout->GetDocLineRef();

		if( pView->m_pcEditDoc->m_cDocEditor.IsModified() && CModifyVisitor().IsLineModified(pCDocLine) ){		/* 変更フラグ */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() )	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* 行番号（変更行） */
		}
	}

	EDiffMark type = CDiffLineGetter(pCDocLine).GetLineDiffMark();
	{
		//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//追加
				if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//変更
				if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//削除
			case MARK_DIFF_DEL_EX:	//削除
				if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	// 02/10/16 ai
	// ブックマークの表示
	if(CBookmarkGetter(pCDocLine).IsBookmarked()){
		if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
			nColorIndex = COLORIDX_MARK;
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             決定されたnColorIndexを使って描画               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* 行番号表示／非表示 */
		wchar_t szLineNum[18];

		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( pTypes->m_bLineNumIsCRLF ){
			/* 論理行番号表示モード */
			if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){
				wcscpy( szLineNum, L" " );
			}else{
				_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* 対応する論理行番号 */
			}
		}else{
			/* 物理行（レイアウト行）番号表示モード */
			_itow( (Int)nLineNum + 1, szLineNum, 10 );
		}

		int nLineCols = wcslen( szLineNum );

		//色、フォント
		cTextType.SetBkColor(hdc);    //背景色：テキストの背景色 //	Sep. 23, 2002 余白をテキストの背景色にする
		cColorType.SetTextColor(hdc); //テキスト：行番号の色
		cColorType.SetFont(hdc);      //フォント：行番号のフォント

		// 余白を埋める
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
		
		//	Sep. 23, 2002 余白をテキストの背景色にするため，背景色の設定を移動
		SetBkColor( hdc, cColorType.GetBackColor() );		/* 行番号背景の色 */

		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
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

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
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
		// 行番号エリアの背景描画
		rcClip.left   = 0;
		rcClip.right  = pView->GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cColorType.FillBack(hdc,rcClip);
		
		// Mar. 5, 2003, Moca
		// 行番号とテキストの隙間の描画
		rcClip.left   = rcClip.right;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cTextType.FillBack(hdc,rcClip);
	}

	// 2001.12.03 hor
	/* とりあえずブックマークに縦線 */
	if(CBookmarkGetter(pCDocLine).IsBookmarked() && !cMarkType.IsDisp() )
	{
		cColorType.SetSolidPen(hdc,2);
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		cColorType.RewindPen(hdc);
	}

	if( type )	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		cColorType.SetSolidPen(hdc,1);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//追加
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo  ( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//変更
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo  ( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo  ( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//削除
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy + 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//削除(EOF)
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

