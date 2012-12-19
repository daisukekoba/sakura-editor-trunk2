/*!	@file
@brief CViewCommanderクラスのコマンド(モード切り替え系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, みつ
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, すなふき, Moca, ai
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, みちばな, Kazika
	Copyright (C) 2005, genta, novice, かろと, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, かろと, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <htmlhelp.h>
#include "CViewCommander.h"

#include "view/CEditView.h"


/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void CViewCommander::Command_CHGMOD_INS( void )
{
	/* 挿入モードか？ */
	if( m_pCommanderView->IsInsMode() ){
		m_pCommanderView->SetInsMode( false );
	}else{
		m_pCommanderView->SetInsMode( true );
	}
	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();
	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}



//	from CViewCommander_New.cpp
/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void CViewCommander::Command_CHGMOD_EOL( EEolType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		GetDocument()->m_cDocEditor.SetNewLineCode( e );
		// ステータスバーを更新するため
		// キャレットの行桁位置を表示する関数を呼び出す
		GetCaret().ShowCaretPosInfo();
	}
}



/** 各種モードの取り消し
	@param whereCursorIs 選択をキャンセルした後、キャレットをどこに置くか。0=動かさない。1=左上。2=右下。
*/
void CViewCommander::Command_CANCEL_MODE( int whereCursorIs )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
		// 選択解除後のカーソル位置を決める。
		CLayoutPoint ptTo ;
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) { // 矩形選択ではキャレットが改行の後ろに取り残されないように、左上。
			/* 2点を対角とする矩形を求める */
			CLayoutRange rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// 範囲選択開始
				GetSelect().GetTo()		// 範囲選択終了
			);
			ptTo = rcSel.GetFrom();
		} else if( 1 == whereCursorIs ) { // 左上
			ptTo = GetSelect().GetFrom();
		} else if( 2 == whereCursorIs ) { // 右下
			ptTo = GetSelect().GetTo();
		} else {
			ptTo = GetCaret().GetCaretLayoutPos();
		}

		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

		/* カーソルを移動 */
		if( ptTo.y >= GetDocument()->m_cLayoutMgr.GetLineCount() ){
			/* ファイルの最後に移動 */
			Command_GOFILEEND(FALSE);
		} else {
			GetCaret().MoveCursor( ptTo, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}else{
		// 2011.12.05 Moca 選択中の未選択状態でもLockの解除と描画が必要
		if( m_pCommanderView->GetSelectionInfo().IsTextSelecting()
				|| m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().DisableSelectArea(true);
			GetCaret().m_cUnderLine.CaretUnderLineON(true);
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
		}
	}
}
