/*!	@file
	@brief 機能番号定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, MIK, Stonee, asa-o, Misaka, hor
	Copyright (C) 2002, aroka, YAZAKI, minfu, MIK, ai, genta
	Copyright (C) 2003, 鬼, genta, MIK, Moca
	Copyright (C) 2004, genta, zenryaku, kazika, Moca, isearch
	Copyright (C) 2005, genta, MIK, maru
	Copyright (C) 2006, aroka, かろと, fon, ryoji
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _FUNCCODE_H_
#define _FUNCCODE_H_

/*	コマンドコード

	エディタ内部で使われるコマンド番号．16bitの数値．

	Windows 95では32768以上のコマンドをメニューやアクセラレータに設定すると
	動作しない．

	30000-32767 : 機能番号．メニューやキーに割り当てられる．
	32768-39999 : メニューからは直接呼ばれないが、他のIDから間接的に呼ばれる機能
	40000-49511 : マクロ関数
	49512-      : 変換コマンド．状況に応じて機能を置き換える場合に用いる

	HandleCommandの引数としては32bit幅だが，上位16bitはコマンドが送られた状況を
	通知するために使用する．

	[参考]
	10000- : ウィンドウ一覧で使用する
	11000- : 最近使ったファイルで使用する
	12000- : 最近使ったフォルダで使用する

*/

/* 未定義用(ダミーとしても使う) */	//Oct. 17, 2000 jepro noted
#define F_DISABLE		0	//未使用
#define F_SEPARATOR		1	//セパレータ
#define F_TOOLBARWRAP	2	//ツールバー折返

//	2007.07.07 genta 状況通知フラグ
//	コマンドが送られた状況をコマンドと併せて通知する．
enum FunctionAttribute {
	FA_FROMKEYBOARD		 = 0x00010000,	//!< キーボードアクセラレータからのコマンド
	FA_FROMMACRO		 = 0x00020000,	//!< マクロからのコマンド実行
	FA_NONRECORD		 = 0x00040000,	//!< マクロへの記録を抑制する
};

#define F_MENU_FIRST 30000

/* ファイル操作系 */
#define F_FILENEW					30101	//新規作成
#define F_FILEOPEN					30102	//開く
#define F_FILESAVE					30103	//上書き保存
#define F_FILESAVEAS_DIALOG			30104	//名前を付けて保存
#define F_FILESAVEAS				30106	//名前を付けて保存
#define F_FILECLOSE					30105	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
#define F_FILECLOSE_OPEN			30107	//閉じて開く
#define F_FILEOPEN_DROPDOWN			30108	//開く(ドロップダウン)
#define F_FILESAVECLOSE				30109	//保存して閉じる Feb. 28, 2004 genta
#define F_FILESAVEALL				30120	//上書き保存 //	Jan. 24, 2005 genta
#define F_FILESAVE_QUIET			30121	//上書き保存(限定動作) //	Jan. 24, 2005 genta
//	Dec. 4, 2002 genta
#define F_FILE_REOPEN				30119	//現在と同じ文字コードでで開き直す
#define F_FILE_REOPEN_SJIS			30111	//SJISで開き直す
#define F_FILE_REOPEN_JIS			30112	//JISで開き直す
#define F_FILE_REOPEN_EUC			30113	//EUCで開き直す
#define F_FILE_REOPEN_UNICODE		30114	//Unicodeで開き直す
#define F_FILE_REOPEN_UNICODEBE		30117	//UnicodeBEで開き直す
#define F_FILE_REOPEN_UTF8			30115	//UTF-8で開き直す
#define F_FILE_REOPEN_UTF7			30116	//UTF-7で開き直す
#define F_PRINT						30150	//印刷
#define F_PRINT_PREVIEW				30151	//印刷プレビュー
#define F_PRINT_PAGESETUP			30152	//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から「印刷ページ設定」に変更
//#define F_PRINT_DIALOG				30151	//印刷ダイアログ
#define F_OPEN_HHPP					30160	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
#define F_OPEN_CCPP					30161	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
#define	F_OPEN_HfromtoC				30162	//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
#define F_ACTIVATE_SQLPLUS			30170	/* Oracle SQL*Plusをアクティブ表示 */
#define F_PLSQL_COMPILE_ON_SQLPLUS	30171	/* Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
#define F_BROWSE					30180	//ブラウズ
#define F_READONLY					30185	/* ファイルのプロパティ */
#define F_PROPERTY_FILE				30190	/* ファイルのプロパティ */
#define F_EXITALLEDITORS			30194	/* 編集の全終了 */	// 2007.02.13 ryoji 追加
#define F_EXITALL					30195	/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加
#define F_NOWOPENFILE				30197	//現在開いているファイル
#define F_RECENTFILE				30198	//最近使ったファイル
#define F_RECENTFOLDER				30199	//最近使ったフォルダ

/* 編集系 */
#define F_CHAR				30200	//文字入力
#define F_IME_CHAR			30201	//全角文字入力
#define F_UNDO				30210	//元に戻す(Undo)
#define F_REDO				30211	//やり直し(Redo)
#define F_DELETE			30221	//削除
#define F_DELETE_BACK		30222	//カーソル前を削除
#define F_WordDeleteToStart	30230	//単語の左端まで削除
#define F_WordDeleteToEnd	30231	//単語の右端まで削除
#define F_WordCut			30232	//単語切り取り
#define F_WordDelete		30233	//単語削除
#define F_LineCutToStart	30240	//行頭まで切り取り(改行単位)
#define F_LineCutToEnd		30241	//行末まで切り取り(改行単位)
#define F_LineDeleteToStart	30242	//行頭まで削除(改行単位)
#define F_LineDeleteToEnd	30243	//行末まで削除(改行単位)
#define F_CUT_LINE			30244	//行切り取り(折り返し単位)
#define F_DELETE_LINE		30245	//行削除(折り返し単位)
#define F_DUPLICATELINE		30250	//行の二重化(折り返し単位)
#define F_INDENT_TAB		30260	//TABインデント
#define F_UNINDENT_TAB		30261	//逆TABインデント
#define F_INDENT_SPACE		30262	//SPACEインデント
#define F_UNINDENT_SPACE	30263	//逆SPACEインデント
//#define F_WORDSREFERENCE	30270	//単語リファレンス
#define F_LTRIM				30280	//左(先頭)の空白を削除	2001.12.03 hor
#define F_RTRIM				30281	//右(末尾)の空白を削除	2001.12.03 hor
#define F_SORT_ASC			30282	//選択行の昇順ソート	2001.12.06 hor
#define F_SORT_DESC			30283	//選択行の降順ソート	2001.12.06 hor
#define F_MERGE				30284	//選択行のマージ		2001.12.06 hor
#define F_RECONVERT			30285	//メニューからの再変換対応 2002.04.09  minfu 


/* カーソル移動系 */
#define F_UP				30311	//カーソル上移動
#define F_DOWN				30312	//カーソル下移動
#define F_LEFT				30313	//カーソル左移動
#define F_RIGHT				30314	//カーソル右移動
#define F_UP2				30315	//カーソル上移動(２行ごと)
#define F_DOWN2				30316	//カーソル下移動(２行ごと)
#define F_WORDLEFT			30320	//単語の左端に移動
#define F_WORDRIGHT			30321	//単語の右端に移動
//#define F_GOLINETOP		30330	//行頭に移動(改行単位)
//#define F_GOLINEEND		30331	//行末に移動(改行単位)
#define F_GOLINETOP			30332	//行頭に移動(折り返し単位)
#define F_GOLINEEND			30333	//行末に移動(折り返し単位)
//#define F_ROLLDOWN			30340	//スクロールダウン//Oct. 10, 2000 JEPRO 下の半ページアップに名称変更
//#define F_ROLLUP			30341	//スクロールアップ//Oct. 10, 2000 JEPRO 下の半ページダウンに名称変更
#define F_HalfPageUp		30340	//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_HalfPageDown		30341	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_1PageUp			30342	//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
#define F_1PageDown			30343	//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//#define F_DISPLAYTOP		30344	//画面の先頭に移動(未実装)
//#define F_DISPLAYEND		30345	//画面の最後に移動(未実装)
#define F_GOFILETOP			30350	//ファイルの先頭に移動
#define F_GOFILEEND			30351	//ファイルの最後に移動
#define F_CURLINECENTER		30360	//カーソル行をウィンドウ中央へ
#define F_JUMPHIST_PREV		30370	//移動履歴: 前へ
#define F_JUMPHIST_NEXT		30371	//移動履歴: 次へ
#define F_JUMPHIST_SET		30372	//現在位置を移動履歴に登録
#define F_WndScrollDown		30380	//テキストを１行下へスクロール	// 2001/06/20 asa-o
#define F_WndScrollUp		30381	//テキストを１行上へスクロール	// 2001/06/20 asa-o
#define F_GONEXTPARAGRAPH	30382	//次の段落へ
#define F_GOPREVPARAGRAPH	30383	//前の段落へ

/* 選択系 */
#define F_SELECTWORD		30400	//現在位置の単語選択
#define F_SELECTALL			30401	//すべて選択
#define F_SELECTLINE		30403	//1行選択	// 2007.10.06 nasukoji
#define F_BEGIN_SEL			30410	//範囲選択開始
#define F_UP_SEL			30411	//(範囲選択)カーソル上移動
#define F_DOWN_SEL			30412	//(範囲選択)カーソル下移動
#define F_LEFT_SEL			30413	//(範囲選択)カーソル左移動
#define F_RIGHT_SEL			30414	//(範囲選択)カーソル右移動
#define F_UP2_SEL			30415	//(範囲選択)カーソル上移動(２行ごと)
#define F_DOWN2_SEL			30416	//(範囲選択)カーソル下移動(２行ごと)
#define F_WORDLEFT_SEL		30420	//(範囲選択)単語の左端に移動
#define F_WORDRIGHT_SEL		30421	//(範囲選択)単語の右端に移動
//#define F_GOLINETOP_SEL		30430	//(範囲選択)行頭に移動(改行単位)
//#define F_GOLINEEND_SEL		30431	//(範囲選択)行末に移動(改行単位)
#define F_GOLINETOP_SEL		30432	//(範囲選択)行頭に移動(折り返し単位)
#define F_GOLINEEND_SEL		30433	//(範囲選択)行末に移動(折り返し単位)
//#define F_ROLLDOWN_SEL		30440	//(範囲選択)スクロールダウン//Oct. 10, 2000 JEPRO 下の半ページアップに名称変更
//#define F_ROLLUP_SEL		30441	//(範囲選択)スクロールアップ//Oct. 10, 2000 JEPRO 下の半ページダウンに名称変更
#define F_HalfPageUp_Sel	30440	//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_HalfPageDown_Sel	30441	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_1PageUp_Sel		30442	//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
#define F_1PageDown_Sel		30443	//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//#define F_DISPLAYTOP_SEL	30444	//(範囲選択)画面の先頭に移動(未実装)
//#define F_DISPLAYEND_SEL	30445	//(範囲選択)画面の最後に移動(未実装)
#define F_GOFILETOP_SEL		30450	//(範囲選択)ファイルの先頭に移動
#define F_GOFILEEND_SEL		30451	//(範囲選択)ファイルの最後に移動
#define F_GONEXTPARAGRAPH_SEL	30482	//(範囲選択)次の段落へ
#define F_GOPREVPARAGRAPH_SEL	30483	//(範囲選択)前の段落へ


/* 矩形選択系 */
//#define F_BOXSELALL		30500	//矩形ですべて選択
#define	F_BEGIN_BOX			30510	//矩形範囲選択開始
//Oct. 17, 2000 JEPRO 以下に矩形選択のコマンド名のみ準備しておいた
//#define F_UP_BOX			30511	//(矩形選択)カーソル上移動
//#define F_DOWN_BOX			30512	//(矩形選択)カーソル下移動
//#define F_LEFT_BOX			30513	//(矩形選択)カーソル左移動
//#define F_RIGHT_BOX			30514	//(矩形選択)カーソル右移動
//#define F_UP2_BOX			30515	//(矩形選択)カーソル上移動(２行ごと)
//#define F_DOWN2_BOX			30516	//(矩形選択)カーソル下移動(２行ごと)
//#define F_WORDLEFT_BOX		30520	//(矩形選択)単語の左端に移動
//#define F_WORDRIGHT_BOX		30521	//(矩形選択)単語の右端に移動
////#define F_GOLINETOP_BOX		30530	//(矩形選択)行頭に移動(改行単位)
////#define F_GOLINEEND_BOX		30531	//(矩形選択)行末に移動(改行単位)
//#define F_GOLINETOP_BOX		30532	//(矩形選択)行頭に移動(折り返し単位)
//#define F_GOLINEEND_BOX		30533	//(矩形選択)行末に移動(折り返し単位)
//#define F_HalfPageUp_Box	30540	//(矩形選択)半ページアップ
//#define F_HalfPageDown_Box	30541	//(矩形選択)半ページダウン
//#define F_1PageUp_Box		30542	//(矩形選択)１ページアップ
//#define F_1PageDown_Box		30543	//(矩形選択)１ページダウン
////#define F_DISPLAYTOP_BOX	30444	//(矩形選択)画面の先頭に移動(未実装)
////#define F_DISPLAYEND_BOX	30445	//(矩形選択)画面の最後に移動(未実装)
//#define F_GOFILETOP_BOX		30550	//(矩形選択)ファイルの先頭に移動
//#define F_GOFILEEND_BOX		30551	//(矩形選択)ファイルの最後に移動


/* クリップボード系 */
#define F_CUT						30601	//切り取り(選択範囲をクリップボードにコピーして削除)
#define F_COPY						30602	//コピー(選択範囲をクリップボードにコピー)
#define F_COPY_ADDCRLF				30608	//折り返し位置に改行をつけてコピー
#define F_COPY_CRLF					30603	//CRLF改行でコピー
#define F_PASTE						30604	//貼り付け(クリップボードから貼り付け)
#define F_PASTEBOX					30605	//矩形貼り付け(クリップボードから矩形貼り付け)
#define F_INSTEXT					30606	//テキストを貼り付け
#define F_ADDTAIL					30607	//最後にテキストを追加
#define F_COPYLINES					30610	//選択範囲内全行コピー
#define F_COPYLINESASPASSAGE		30611	//選択範囲内全行引用符付きコピー
#define F_COPYLINESWITHLINENUMBER	30612	//選択範囲内全行行番号付きコピー
#define F_COPYPATH					30620	//このファイルのパス名をクリップボードにコピー
#define F_COPYTAG					30621	//このファイルのパス名とカーソル位置をコピー
#define F_COPYFNAME					30622	//このファイル名をクリップボードにコピー // 2002/2/3 aroka
//Sept. 16, 2000 JEPRO ショートカットキーがうまく働かないので次行は殺してある	//Dec. 25, 2000 復活
#define F_CREATEKEYBINDLIST			30630	//キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO 上の行はIDM_TESTのままではうまくいかないのでFに変えて登録


/* 挿入系 */
#define F_INS_DATE				30790	//日付挿入
#define F_INS_TIME				30791	//時刻挿入
#define F_CTRL_CODE_DIALOG		30792	//コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK


/* 変換系 */
#define F_TOLOWER				30800	//小文字
#define F_TOUPPER				30801	//大文字
#define F_TOHANKAKU				30810	/* 全角→半角 */
#define F_TOHANKATA				30817	/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
#define F_TOZENKAKUKATA			30811	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
#define F_TOZENKAKUHIRA			30812	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
#define F_HANKATATOZENKAKUKATA	30813	/* 半角カタカナ→全角カタカナ */
#define F_HANKATATOZENKAKUHIRA	30814	/* 半角カタカナ→全角ひらがな */
#define F_TOZENEI				30815	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
#define F_TOHANEI				30816	/* 全角英数→半角英数 */			//@@@ 2002.2.11 YAZAKI
#define F_TABTOSPACE			30830	/* TAB→空白 */
#define F_SPACETOTAB			30831	/* 空白→TAB *///#### Stonee, 2001/05/27
#define F_CODECNV_AUTO2SJIS		30850	/* 自動判別→SJISコード変換 */
#define F_CODECNV_EMAIL			30851	//E-Mail(JIS→SJIS)コード変換
#define F_CODECNV_EUC2SJIS		30852	//EUC→SJISコード変換
#define F_CODECNV_UNICODE2SJIS	30853	//Unicode→SJISコード変換
#define F_CODECNV_UNICODEBE2SJIS 30856	/*Unicode→SJISコード変換*/
#define F_CODECNV_UTF82SJIS		30854	/* UTF-8→SJISコード変換 */
#define F_CODECNV_UTF72SJIS		30855	/* UTF-7→SJISコード変換 */
#define F_CODECNV_SJIS2JIS		30860	/* SJIS→JISコード変換 */
#define F_CODECNV_SJIS2EUC		30861	/* SJIS→EUCコード変換 */
#define F_CODECNV_SJIS2UTF8		30862	/* SJIS→UTF-8コード変換 */
#define F_CODECNV_SJIS2UTF7		30863	/* SJIS→UTF-7コード変換 */
#define F_BASE64DECODE			30870	//Base64デコードして保存
#define F_UUDECODE				30880	//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


/* 検索系 */
#define F_SEARCH_DIALOG		30901	//検索(単語検索ダイアログ)
#define F_SEARCH_NEXT		30902	//次を検索
#define F_SEARCH_PREV		30903	//前を検索
#define F_REPLACE_DIALOG	30904	//置換(置換ダイアログ)
#define F_REPLACE			30906	//置換(実行)
#define F_REPLACE_ALL		30907	//すべて置換(実行)
#define F_SEARCH_CLEARMARK	30905	//検索マークのクリア
#define F_JUMP_SRCHSTARTPOS	30909	//検索開始位置へ戻る	// 02/06/26 ai
#define F_SEARCH_BOX		30908	//検索(ボックス)	//@@@ 2002.06.15 MIK
#define F_GREP_DIALOG		30910	//Grep
#define F_GREP				30911	//Grep
#define F_JUMP_DIALOG		30920	//指定行へジャンプ
#define F_JUMP				30921	//指定行へジャンプ
#define F_OUTLINE			30930	//アウトライン解析
#define F_OUTLINE_TOGGLE	30931	//アウトライン解析(toggle) // 20060201 aroka
#define F_TAGJUMP			30940	//タグジャンプ機能
#define F_TAGJUMPBACK		30941	//タグジャンプバック機能
#define	F_TAGJUMP_LIST		30942	//タグジャンプリスト	//@@@ 2003.04.13 MIK
#define	F_TAGS_MAKE			30943	//タグファイルの作成	//@@@ 2003.04.13 MIK
#define	F_DIRECT_TAGJUMP	30944	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
#define F_TAGJUMP_CLOSE		30945	//閉じてタグジャンプ(元ウィンドウclose) // Apr. 03. 2003 genta
#define F_TAGJUMP_KEYWORD	30946	//キーワードを指定してダイレクトタグジャンプ	//@@@ 2005.03.31 MIK
#define F_COMPARE			30950	//ファイル内容比較
#define F_BRACKETPAIR		30960	//対括弧の検索
#define F_BOOKMARK_SET		30970	//ブックマーク設定・解除
#define F_BOOKMARK_NEXT		30971	//次のブックマークへ
#define F_BOOKMARK_PREV		30972	//前のブックマークへ
#define F_BOOKMARK_RESET	30973	//ブックマークの全解除
#define F_BOOKMARK_VIEW		30974	//ブックマークの一覧
#define F_BOOKMARK_PATTERN	30975	//パターンに一致する行をマーク
#define F_DIFF_DIALOG		30976	//DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
#define F_DIFF				30977	//DIFF差分表示	//@@@ 2002.05.25 MIK
#define F_DIFF_NEXT			30978	//次の差分へ	//@@@ 2002.05.25 MIK
#define F_DIFF_PREV			30979	//前の差分へ	//@@@ 2002.05.25 MIK
#define F_DIFF_RESET		30980	//差分の全解除	//@@@ 2002.05.25 MIK
#define F_ISEARCH_NEXT	    30981	//前方インクリメンタルサーチ //2004.10.13 isearch
#define F_ISEARCH_PREV		30982	//後方インクリメンタルサーチ //2004.10.13 isearch
#define F_ISEARCH_REGEXP_NEXT   30983  //前方正規表現インクリメンタルサーチ  //2004.10.13 isearch
#define F_ISEARCH_REGEXP_PREV   30984  //後方正規表現インクリメンタルサーチ  //2004.10.13 isearch
#define F_ISEARCH_MIGEMO_NEXT   30985  //前方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
#define F_ISEARCH_MIGEMO_PREV   30986  //後方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch


/* モード切り替え系 */
#define F_CHGMOD_INS		31001	//挿入／上書きモード切り替え
#define F_CHGMOD_EOL_CRLF	31081	//入力改行コード指定(CRLF)	// 2003.06.08 Moca
#define F_CHGMOD_EOL_LF		31082	//入力改行コード指定(LF)	// 2003.06.08 Moca
#define F_CHGMOD_EOL_CR		31083	//入力改行コード指定(CR)	// 2003.06.08 Moca
#define F_CANCEL_MODE		31099	//各種モードの取り消し

/* 設定系 */
#define F_SHOWTOOLBAR		31100	/* ツールバーの表示 */
#define F_SHOWFUNCKEY		31101	/* ファンクションキーの表示 */
#define F_SHOWSTATUSBAR		31102	/* ステータスバーの表示 */
#define F_SHOWTAB			31103	/* タブの表示 */	//@@@ 2003.06.10 MIK
#define F_TYPE_LIST			31110	/* タイプ別設定一覧 */
#define F_OPTION_TYPE		31111	/* タイプ別設定 */
#define F_OPTION			31112	/* 共通設定 */
#define	F_FAVORITE			31113	//履歴の管理	//@@@ 2003.04.08 MIK

//From here 設定ダイアログ用の機能番号を用意  Stonee, 2001/05/18
#define F_TYPE_SCREEN		31115	/* タイプ別設定『スクリーン』 */
#define F_TYPE_COLOR		31116	/* タイプ別設定『カラー』 */
#define F_TYPE_HELPER		31117	/* タイプ別設定『支援』 */	//Jul. 03, 2001 JEPRO 追加
#define F_TYPE_REGEX_KEYWORD	31118	/* タイプ別設定『正規表現キーワード』 */	//@@@ 2001.11.17 add MIK
#define F_TYPE_KEYHELP		31119	/* タイプ別設定『キーワードヘルプ』 */	// 2006.10.06 ryoji 追加
#define F_OPTION_GENERAL	32000	/* 共通設定『全般』 */
#define F_OPTION_WINDOW		32001	/* 共通設定『ウィンドウ』 */
#define F_OPTION_EDIT		32002	/* 共通設定『編集』 */
#define F_OPTION_FILE		32003	/* 共通設定『ファイル』 */
#define F_OPTION_BACKUP		32004	/* 共通設定『バックアップ』 */
#define F_OPTION_FORMAT		32005	/* 共通設定『書式』 */
//#define F_OPTION_URL		32006	/* 共通設定『クリッカブルURL』 */
#define F_OPTION_GREP		32007	/* 共通設定『Grep』 */
#define F_OPTION_KEYBIND	32008	/* 共通設定『キー割り当て』 */
#define F_OPTION_CUSTMENU	32009	/* 共通設定『カスタムメニュー』 */
#define F_OPTION_TOOLBAR	32010	/* 共通設定『ツールバー』 */
#define F_OPTION_KEYWORD	32011	/* 共通設定『強調キーワード』 */
#define F_OPTION_HELPER		32012	/* 共通設定『支援』 */
//To here  Stonee, 2001/05/18
#define F_OPTION_MACRO		32013	/* 共通設定『マクロ』 */
#define F_OPTION_FNAME		32014	/* 共通設定『ファイル名表示』 */
#define F_OPTION_TAB		32015	/* 共通設定『タブ』*/ // Feb. 11, 2007 genta 追加

#define F_FONT				31120	/* フォント設定 */
#define F_WRAPWINDOWWIDTH	31140	//現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
#define F_TMPWRAPNOWRAP		31141	// 折り返さない（一時設定）			// 2008.05.30 nasukoji
#define F_TMPWRAPSETTING	31142	// 指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
#define F_TMPWRAPWINDOW		31143	// 右端で折り返す（一時設定）		// 2008.05.30 nasukoji


/* マクロ系 */
#define F_RECKEYMACRO		31250	/* キーマクロの記録開始／終了 */
#define F_SAVEKEYMACRO		31251	/* キーマクロの保存 */
#define F_LOADKEYMACRO		31252	/* キーマクロの読み込み */
#define F_EXECKEYMACRO		31253	/* キーマクロの実行 */
#define F_EXECEXTMACRO		31254	/* 名前を指定してマクロ実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	#define F_EXECCMMAND		31270	/* 外部コマンド実行 */
#define F_EXECCOMMAND_DIALOG	31270	/* 外部コマンド実行 */
#define F_EXECCOMMAND		31271	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

//	Jul. 4, 2000 genta
#define F_USERMACRO_0		31600	/* 登録マクロ開始 */


/* カスタムメニュー */
#define F_MENU_RBUTTON		31580	/* 右クリックメニュー */
#define F_CUSTMENU_1		31501	/* カスタムメニュー1 */
#define F_CUSTMENU_2		31502	/* カスタムメニュー2 */
#define F_CUSTMENU_3		31503	/* カスタムメニュー3 */
#define F_CUSTMENU_4		31504	/* カスタムメニュー4 */
#define F_CUSTMENU_5		31505	/* カスタムメニュー5 */
#define F_CUSTMENU_6		31506	/* カスタムメニュー6 */
#define F_CUSTMENU_7		31507	/* カスタムメニュー7 */
#define F_CUSTMENU_8		31508	/* カスタムメニュー8 */
#define F_CUSTMENU_9		31509	/* カスタムメニュー9 */
#define F_CUSTMENU_10		31510	/* カスタムメニュー10 */
#define F_CUSTMENU_11		31511	/* カスタムメニュー11 */
#define F_CUSTMENU_12		31512	/* カスタムメニュー12 */
#define F_CUSTMENU_13		31513	/* カスタムメニュー13 */
#define F_CUSTMENU_14		31514	/* カスタムメニュー14 */
#define F_CUSTMENU_15		31515	/* カスタムメニュー15 */
#define F_CUSTMENU_16		31516	/* カスタムメニュー16 */
#define F_CUSTMENU_17		31517	/* カスタムメニュー17 */
#define F_CUSTMENU_18		31518	/* カスタムメニュー18 */
#define F_CUSTMENU_19		31519	/* カスタムメニュー19 */
#define F_CUSTMENU_20		31520	/* カスタムメニュー20 */
#define F_CUSTMENU_21		31521	/* カスタムメニュー21 */
#define F_CUSTMENU_22		31522	/* カスタムメニュー22 */
#define F_CUSTMENU_23		31523	/* カスタムメニュー23 */
#define F_CUSTMENU_24		31524	/* カスタムメニュー24 */

#define F_CUSTMENU_BASE		31500	/* カスタムメニュー基準番号 */

/* ウィンドウ系 */
#define F_SPLIT_V			31310	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
#define F_SPLIT_H			31311	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
#define F_SPLIT_VH			31312	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
#define F_WINCLOSE			31320	//ウィンドウを閉じる
#define F_WIN_CLOSEALL		31321	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
#define F_CASCADE			31330	//重ねて表示
#define F_TILE_V			31331	//上下に並べて表示
#define F_TILE_H			31332	//左右に並べて表示
#define F_BIND_WINDOW		31333	//結合して表示	2004.07.14 kazika
#define F_TOPMOST			31334	//常に手前に表示 // 2004-09-21 Moca
#define F_NEXTWINDOW		31340	//次のウィンドウ
#define F_PREVWINDOW		31341	//前のウィンドウ
#define F_WINLIST			31336	//開いているウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
#define F_MAXIMIZE_V		31350	//縦方向に最大化
#define F_MINIMIZE_ALL		31351	//すべて最小化		//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
#define F_MAXIMIZE_H		31352	//横方向に最大化	//2001.02.10 by MIK
#define F_REDRAW			31360	//再描画
#define F_WIN_OUTPUT		31370	//アウトプットウィンドウ表示
#define F_GROUPCLOSE		31380	/* グループを閉じる */	// 2007.06.20 ryoji
#define F_NEXTGROUP			31381	/* 次のグループ */	// 2007.06.20 ryoji
#define F_PREVGROUP			31382	/* 前のグループ */	// 2007.06.20 ryoji
#define F_TAB_MOVERIGHT		31383	/* タブを右に移動 */	// 2007.06.20 ryoji
#define F_TAB_MOVELEFT		31384	/* タブを左に移動 */	// 2007.06.20 ryoji
#define F_TAB_SEPARATE		31385	/* 新規グループ */	// 2007.06.20 ryoji
#define F_TAB_JOINTNEXT		31386	/* 次のグループに移動 */	// 2007.06.20 ryoji
#define F_TAB_JOINTPREV		31387	/* 前のグループに移動 */	// 2007.06.20 ryoji
#define F_TAB_CLOSEOTHER	31388	/* このタブ以外を閉じる */	// 2009.07.20 syat
#define F_TAB_CLOSELEFT		31389	/* 左をすべて閉じる */		// 2009.07.20 syat
#define F_TAB_CLOSERIGHT	31390	/* 右をすべて閉じる */		// 2009.07.20 syat


/* 支援 */
#define F_HOKAN				31430		/* 入力補完 */
#define F_TOGGLE_KEY_SEARCH	31431		/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon
//Sept. 16, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
#define F_HELP_CONTENTS		31440		/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
#define F_HELP_SEARCH		31441		/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
#define F_MENU_ALLFUNC		31445		/* コマンド一覧 */
#define F_EXTHELP1			31450		/* 外部ヘルプ１ */
#define F_EXTHTMLHELP		31451		/* 外部HTMLヘルプ */
#define F_ABOUT				31455		/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加


/* その他 */
//#define F_SENDMAIL		31570		/* メール送信 */	//Oct. 17, 2000 JEPRO メール機能は死んでいるのでコメントアウトにした

//	Windows 95の制約により，WM_COMMANDで使う機能番号として32768以上の値を用いることができません．
//	メニュー等で使われないマクロ専用のコマンドにはそれ以上の値を割り当てましょう．

//	メニューからは直接呼ばれないが、他のIDから間接的に呼ばれる機能
#define F_MENU_NOT_USED_FIRST 32768

#define F_CHGMOD_EOL          32800		// 入力改行コード指定 2003.06.23 Moca
#define F_SET_QUOTESTRING     32801		// 共通設定: 引用符の設定	2005.01.29 genta
#define F_TRACEOUT            32802		// マクロ用アウトプットウインドウに出力	2006.04.26 maru
#define F_PUTFILE             32803 	// 作業中ファイルの一時出力 2006.12.10 maru
#define F_INSFILE             32804 	// キャレット位置にファイル挿入 2006.12.10 maru
#define F_TEXTWRAPMETHOD      32805		// テキストの折り返し方法			// 2008.05.30 nasukoji



// LMP: Start -----------------------------------------------------
// LMP: Start -----------------------------------------------------
// LMP: Start -----------------------------------------------------

// 33,000 - 39,999 Resource strings
#define F_TAB_MANIP_SUBMENU				34000
#define F_TAB_GROUPIZE					34001
#define F_TAB_GROUPDEL					34002

#define F_TOPMOST_REL					34003
#define F_TOPMOST_SET					34004

#define F_FILE_REOPEN_SUBMENU			34005
#define F_FILE_RCNTFILE_SUBMENU			34006
#define F_FILE_RCNTFLDR_SUBMENU			34007
#define F_EDIT_INS_SUBMENU   			34008
#define F_EDIT_HLV_SUBMENU   			34009
#define F_EDIT_MOV_SUBMENU   			34010
#define F_EDIT_SEL_SUBMENU   			34011
#define F_EDIT_COS_SUBMENU   			34012
#define F_CONV_ENCODE_SUBMENU			34013

#define F_RECKEYMACRO_REC    			34014
#define F_SAVEKEYMACRO_REC   			34015
#define F_LOADKEYMACRO_REC   			34016
#define F_EXECKEYMACRO_REC   			34017

#define F_RECKEYMACRO_APPE    			34018
#define F_SAVEKEYMACRO_APPE   			34019
#define F_LOADKEYMACRO_APPE   			34020
#define F_EXECKEYMACRO_APPE   			34021

#define F_EXECKEYMACRO_REGD   			34022
#define F_TOOL_CUSTOM_SUBMENU 			34023

#define F_SHOWTOOLBAR_ON      			34024
#define F_SHOWFUNCKEY_ON      			34025
#define F_SHOWTAB_ON          			34026
#define F_SHOWSTATUSBAR_ON    			34027

#define F_SHOWTOOLBAR_OFF     			34028
#define F_SHOWFUNCKEY_OFF     			34029
#define F_SHOWTAB_OFF         			34030
#define F_SHOWSTATUSBAR_OFF   			34031

#define F_TEXTWRAPMETHOD_OFF  			34032
#define F_TEXTWRAPMETHOD_ON   			34033

#define F_WRAPWINDOWWIDTH_NONE			34034
#define F_WRAPWINDOWWIDTH_MAX 			34035
#define F_WRAPWINDOWWIDTH_LEFT			34036
#define F_WRAPWINDOWWIDTH_SET 			34037

#define F_TOGGLE_KEY_SEARCH_OFF 		34038
#define F_TOGGLE_KEY_SEARCH_ON  		34039

#define F_SPLIT_V_ON            		34040
#define F_SPLIT_V_OFF           		34041

#define F_SPLIT_H_ON            		34042
#define F_SPLIT_H_OFF           		34043

#define F_CHGMOD_EOL_SUBMENU    		34044


#define F_SPLIT_VH_ON           		34045
#define F_SPLIT_VH_OFF          		34046


#define F_BOOKMARK_SUBMENU      		34047
#define F_ISEARCH_SUBMENU       		34048

#define STR_SQLERR_NOTRUN       		34049
#define STR_ERR_CMPERR          		34050

#define STR_ERR_MACRO1					34051
#define STR_ERR_INSTXT1					34052
#define STR_ERR_SRPREV1					34053
#define STR_ERR_SRPREV2					34054
#define STR_ERR_SRPREV3					34055
#define STR_ERR_SRNEXT1					34056
#define STR_ERR_SRNEXT2					34057
#define STR_ERR_SRNEXT3					34058
#define STR_ERR_UNINDENT1				34059
#define STR_ERR_TAGJMP1					34060
#define STR_ERR_TAGJMPBK1				34061


// CCommandLine.cpp - cant map properly	
#define STR_ERR_MACROERR1				34062
#define STR_ERR_OVERFLOW1				34063
#define STR_ERR_PARSECMD1				34064


// CControlProcess.cpp
#define STR_ERR_CTRLMTX1				34065
#define STR_ERR_CTRLMTX2				34066
#define STR_ERR_CTRLMTX3				34067
#define STR_ERR_CTRLMTX4				34068


// CDlgCompare.cpp
#define STR_ERR_DLGCMP1					34069


// CDlgCtrlCode.cpp
#define STR_ERR_DLGCTL1					34070
#define STR_ERR_DLGCTL2					34071
#define STR_ERR_DLGCTL3					34072
#define STR_ERR_DLGCTL4					34073

#define STR_ERR_DLGCTL5					34074
#define STR_ERR_DLGCTL6					34075
#define STR_ERR_DLGCTL7					34076
#define STR_ERR_DLGCTL8					34077
#define STR_ERR_DLGCTL9					34078
#define STR_ERR_DLGCTL10				34079
#define STR_ERR_DLGCTL11				34080
#define STR_ERR_DLGCTL12				34081
#define STR_ERR_DLGCTL13				34082
#define STR_ERR_DLGCTL14				34083
#define STR_ERR_DLGCTL15				34084
#define STR_ERR_DLGCTL16				34085
#define STR_ERR_DLGCTL17				34086
#define STR_ERR_DLGCTL18				34087
#define STR_ERR_DLGCTL19				34088
#define STR_ERR_DLGCTL20				34089
#define STR_ERR_DLGCTL21				34090
#define STR_ERR_DLGCTL22				34091
#define STR_ERR_DLGCTL23				34092
#define STR_ERR_DLGCTL24				34093
#define STR_ERR_DLGCTL25				34094
#define STR_ERR_DLGCTL26				34095
#define STR_ERR_DLGCTL27				34096
#define STR_ERR_DLGCTL28				34097
#define STR_ERR_DLGCTL29				34098
#define STR_ERR_DLGCTL30				34099
#define STR_ERR_DLGCTL31				34100
#define STR_ERR_DLGCTL32				34101
#define STR_ERR_DLGCTL33				34102
#define STR_ERR_DLGCTL34				34103
#define STR_ERR_DLGCTL35				34104
#define STR_ERR_DLGCTL36				34105
#define STR_ERR_DLGCTL37				34106

//
#define STR_ERR_DLGDIFF1				34107

//
#define STR_ERR_DLGFAV1					34108
#define STR_ERR_DLGFAV2					34109
#define STR_ERR_DLGFAV3					34110
#define STR_ERR_DLGFAV4					34111
#define STR_ERR_DLGFAV5					34112
#define STR_ERR_DLGFAV6					34113
#define STR_ERR_DLGFAV7					34114

#define STR_ERR_DLGFAV8					34115	
#define STR_ERR_DLGFAV9					34116
		
				
#define STR_ERR_DLGFAV10				34117
#define STR_ERR_DLGFAV11				34118
#define STR_ERR_DLGFAV12				34119
#define STR_ERR_DLGFAV13				34120
#define STR_ERR_DLGFAV14				35120

#define STR_ERR_DLGUPQRY1				34121
#define STR_ERR_DLGUPQRY2				34122

#define STR_ERR_DLGFIND1				34123

#define STR_ERR_DLGFNCLST1				34124	
#define STR_ERR_DLGFNCLST2				34125
#define STR_ERR_DLGFNCLST3				34126
#define STR_ERR_DLGFNCLST4_1			34127
#define STR_ERR_DLGFNCLST5				34128
#define STR_ERR_DLGFNCLST6				34129
#define STR_ERR_DLGFNCLST7				34130
#define STR_ERR_DLGFNCLST8				34131
#define STR_ERR_DLGFNCLST9				34132
#define STR_ERR_DLGFNCLST10				34133
#define STR_ERR_DLGFNCLST11				34134
#define STR_ERR_DLGFNCLST12				34135
#define STR_ERR_DLGFNCLST13				34136


#define STR_ERR_DLGFNCLST14				34137
#define STR_ERR_DLGFNCLST15				34138
#define STR_ERR_DLGFNCLST16				34139
#define STR_ERR_DLGFNCLST17				34140
#define STR_ERR_DLGFNCLST18				34141
#define STR_ERR_DLGFNCLST19				34142
#define STR_ERR_DLGFNCLST20				34143
#define STR_ERR_DLGFNCLST21				34144
#define STR_ERR_DLGFNCLST22				34145
#define STR_ERR_DLGFNCLST23				34146

#define STR_ERR_DLGFNCLST24				34147	
#define STR_ERR_DLGFNCLST25				34148
#define STR_ERR_DLGFNCLST4				34149


#define STR_ERR_DLGFNCLST26				34150	
#define STR_ERR_DLGFNCLST27				34151	
#define STR_ERR_DLGFNCLST28				34152	

#define STR_ERR_DLGFNCLST29				34153	
#define STR_ERR_DLGFNCLST30				34154	
#define STR_ERR_DLGFNCLST31				34155	
#define STR_ERR_DLGFNCLST32				34156	
#define STR_ERR_DLGFNCLST33				34157	
#define STR_ERR_DLGFNCLST34				34158	

#define STR_ERR_DLGFNCLST35				34159	

#define STR_ERR_DLGFNCLST36				34160	
#define STR_ERR_DLGFNCLST37				34161	
#define STR_ERR_DLGFNCLST38				34162	
#define STR_ERR_DLGFNCLST39				34163	
#define STR_ERR_DLGFNCLST40				34164	
#define STR_ERR_DLGFNCLST41				34165	
#define STR_ERR_DLGFNCLST42				34166	
#define STR_ERR_DLGFNCLST43				34167	
#define STR_ERR_DLGFNCLST44				34168	
#define STR_ERR_DLGFNCLST45				34169	
#define STR_ERR_DLGFNCLST46				34170	
#define STR_ERR_DLGFNCLST47				34171	

#define STR_ERR_DLGFNCLST48				34172	
#define STR_ERR_DLGFNCLST49				34173	
#define STR_ERR_DLGFNCLST50				34174	

#define STR_ERR_DLGFNCLST51				34175	
#define STR_ERR_DLGFNCLST52				34176	
#define STR_ERR_DLGFNCLST53				34177	
#define STR_ERR_DLGFNCLST54				34178	
#define STR_ERR_DLGFNCLST55				34179	
#define STR_ERR_DLGFNCLST56				34180	

#define STR_ERR_DLGFNCLST57				34181	


#define STR_ERR_DLGGREP1				34182
#define STR_ERR_DLGGREP2				34183
#define STR_ERR_DLGGREP3				34184

#define STR_ERR_DLGJUMP1				34185
#define STR_ERR_DLGJUMP2				34186

#define STR_ERR_DLGOPNFL1				34187
#define STR_ERR_DLGOPNFL2				34188
#define STR_ERR_DLGOPNFL3				34189
#define STR_ERR_DLGOPNFL4				34190

#define STR_ERR_DLGOPNFL5				34191
#define STR_ERR_DLGOPNFL6				34192

#define STR_ERR_DLGOPNFL7				34193

#define STR_ERR_DLGOPNFL8				34194

#define STR_ERR_DLGOPNFL9				34195


// CDlgPrintPage.cpp
#define STR_ERR_DLGPRNPG1				34196	
#define STR_ERR_DLGPRNPG2				34197	
#define STR_ERR_DLGPRNPG3				34198	


// CDlgPrintSetting.cpp
#define STR_ERR_DLGPRNST1				34199	
#define STR_ERR_DLGPRNST2				34200	


// CDlgProperty.cpp
#define STR_ERR_DLGFLPROP1				34201
#define STR_ERR_DLGFLPROP2				34202	
#define STR_ERR_DLGFLPROP3				34203	

#define STR_ERR_DLGFLPROP4				34204	
#define STR_ERR_DLGFLPROP5				34205	
#define STR_ERR_DLGFLPROP6				34206	
#define STR_ERR_DLGFLPROP7				34207	
#define STR_ERR_DLGFLPROP8				34208	
#define STR_ERR_DLGFLPROP9				34209	
#define STR_ERR_DLGFLPROP10				34210	
#define STR_ERR_DLGFLPROP11				34211	
#define STR_ERR_DLGFLPROP12				34212	
#define STR_ERR_DLGFLPROP13				34213	
#define STR_ERR_DLGFLPROP14				34214	
#define STR_ERR_DLGFLPROP15				34215	
#define STR_ERR_DLGFLPROP16				34216	
#define STR_ERR_DLGFLPROP17				34217	
#define STR_ERR_DLGFLPROP18				34218	
#define STR_ERR_DLGFLPROP19				34219	
#define STR_ERR_DLGFLPROP20				34220	
#define STR_ERR_DLGFLPROP21				34221	
#define STR_ERR_DLGFLPROP22				34222	
#define STR_ERR_DLGFLPROP23				34223	
#define STR_ERR_DLGFLPROP24				34224	
#define STR_ERR_DLGFLPROP25				34225	
#define STR_ERR_DLGFLPROP26				34226	
#define STR_ERR_DLGFLPROP27				34227	
// #define STR_ERR_DLGFLPROP28				
#define STR_ERR_DLGFLPROP29				34228	
#define STR_ERR_DLGFLPROP30				34229	
#define STR_ERR_DLGFLPROP31				34230	
#define STR_ERR_DLGFLPROP32				34231	


// CDlgReplace.cpp
#define STR_ERR_DLGREPLC1				34232	
#define STR_ERR_DLGREPLC2				34233	
#define STR_ERR_DLGREPLC3				34234	
#define STR_ERR_DLGREPLC4				34235	


// CDlgSameColor.cpp
#define STR_ERR_DLGSMCLR1				34236	
#define STR_ERR_DLGSMCLR2				34237	


// CDlgTagJumpList.cpp
#define STR_ERR_DLGTAGJMP1				34238	
#define STR_ERR_DLGTAGJMP2				34239	
#define STR_ERR_DLGTAGJMP3				34240	
#define STR_ERR_DLGTAGJMP4				34241	
#define STR_ERR_DLGTAGJMP5				34242	
#define STR_ERR_DLGTAGJMP6				34243	
#define STR_ERR_DLGTAGJMP7				34244	
#define STR_ERR_DLGTAGJMP8				34245	


// CDlgTagJumpList.cpp
#define STR_ERR_DLGTAGMAK1				34246


// CDlgWinSize.cpp
#define STR_ERR_DLGWINSZ1				34247	
#define STR_ERR_DLGWINSZ2				34248		
#define STR_ERR_DLGWINSZ3				34249


// CDocLineMgr.cpp
#define STR_ERR_DLGDOCLM1				34250	
#define STR_ERR_DLGDOCLM2				34251
#define STR_ERR_DLGDOCLM3				34252
#define STR_ERR_DLGDOCLM4				34253
#define STR_ERR_DLGDOCLM5				34254
#define STR_ERR_DLGDOCLM6				34255


// CDocLineMgr_New.cpp
#define STR_ERR_DLGDOCLMN1				34256


// CDropTarget.cpp
#define STR_ERR_DLGDRPTGT1				34257


// CEditApp.cpp
#define STR_ERR_DLGEDITAPP1				34258
#define STR_ERR_DLGEDITAPP2				34259
#define STR_ERR_DLGEDITAPP3				34260
#define STR_ERR_DLGEDITAPP4				34261
#define STR_ERR_DLGEDITAPP5				34262
#define STR_ERR_DLGEDITAPP6				34263
#define STR_ERR_DLGEDITAPP7				34264
#define STR_ERR_DLGEDITAPP8				34265
#define STR_ERR_DLGEDITAPP9				34266
#define STR_ERR_DLGEDITAPP10			34267
#define STR_ERR_DLGEDITAPP11			34268
#define STR_ERR_DLGEDITAPP12			34269
#define STR_ERR_DLGEDITAPP13			34270


// CEditDoc.cpp
#define STR_ERR_DLGEDITDOC1				34271
#define STR_ERR_DLGEDITDOC2				34272
#define STR_ERR_DLGEDITDOC3				34273
#define STR_ERR_DLGEDITDOC4				34274
#define STR_ERR_DLGEDITDOC5				34275
#define STR_ERR_DLGEDITDOC6				34276
#define STR_ERR_DLGEDITDOC7				34277
#define STR_ERR_DLGEDITDOC8				34278
#define STR_ERR_DLGEDITDOC9				34279
#define STR_ERR_DLGEDITDOC10			34280
#define STR_ERR_DLGEDITDOC11			34281
#define STR_ERR_DLGEDITDOC12			34282
#define STR_ERR_DLGEDITDOC13			34283
#define STR_ERR_DLGEDITDOC14			34284
#define STR_ERR_DLGEDITDOC15			34285
#define STR_ERR_DLGEDITDOC16			34286
#define STR_ERR_DLGEDITDOC17			34287
#define STR_ERR_DLGEDITDOC18			34288
#define STR_ERR_DLGEDITDOC19			34289
#define STR_ERR_DLGEDITDOC20			34290
#define STR_ERR_DLGEDITDOC21			34291
#define STR_ERR_DLGEDITDOC22			34292

#define STR_ERR_DLGEDITDOC23			34293
#define STR_ERR_DLGEDITDOC24			34294
#define STR_ERR_DLGEDITDOC25			34295
#define STR_ERR_DLGEDITDOC26			34296
#define STR_ERR_DLGEDITDOC27			34297
#define STR_ERR_DLGEDITDOC28			34298

#define STR_ERR_DLGEDITDOC29			34299
#define STR_ERR_DLGEDITDOC30			34300
#define STR_ERR_DLGEDITDOC31			34301
#define STR_ERR_DLGEDITDOC32			34302


// 64 bytes max, automat
#define STR_ERR_DLGEDITDOC33			34303
#define STR_ERR_DLGEDITDOC34			34304
#define STR_ERR_DLGEDITDOC35			34305


// CEditDoc_FuncList1.cpp
#define STR_ERR_DLGEDITDOCFL11			34306
#define STR_ERR_DLGEDITDOCFL12			34307


// CEditDoc_FuncList2.cpp
#define STR_ERR_DLGEDITDOCFL21			34308
#define STR_ERR_DLGEDITDOCFL22			34309


// CEditDoc_new.cpp
#define STR_ERR_DLGEDITDOCNEW1			34310
#define STR_ERR_DLGEDITDOCNEW2			34311
#define STR_ERR_DLGEDITDOCNEW3			34312
#define STR_ERR_DLGEDITDOCNEW4			34313
#define STR_ERR_DLGEDITDOCNEW5			34314


// CEditView.cpp
#define STR_ERR_DLGEDITVW1 				34315
#define STR_ERR_DLGEDITVW2 				34316
#define STR_ERR_DLGEDITVW3 				34317
#define STR_ERR_DLGEDITVW4 				34318
#define STR_ERR_DLGEDITVW5 				34319
#define STR_ERR_DLGEDITVW6 				34320
#define STR_ERR_DLGEDITVW7 				34321
#define STR_ERR_DLGEDITVW8 				34322
#define STR_ERR_DLGEDITVW9 				34323
#define STR_ERR_DLGEDITVW10 			34324
#define STR_ERR_DLGEDITVW11 			34325
#define STR_ERR_DLGEDITVW12 			34326
#define STR_ERR_DLGEDITVW13 			34327
#define STR_ERR_DLGEDITVW14 			34328
#define STR_ERR_DLGEDITVW15 			34329
#define STR_ERR_DLGEDITVW16 			34330
#define STR_ERR_DLGEDITVW17 			34331
#define STR_ERR_DLGEDITVW18 			34332
#define STR_ERR_DLGEDITVW19 			34333
#define STR_ERR_DLGEDITVW20 			34334
#define STR_ERR_DLGEDITVW21 			34335
#define STR_ERR_DLGEDITVW22 			34336
#define STR_ERR_DLGEDITVW23 			34337
#define STR_ERR_DLGEDITVW24 			34338
#define STR_ERR_DLGEDITVW25 			34339
#define STR_ERR_DLGEDITVW26 			34340
#define STR_ERR_DLGEDITVW27 			34341
#define STR_ERR_DLGEDITVW28 			34342
#define STR_ERR_DLGEDITVW29 			34343
#define STR_ERR_DLGEDITVW30 			34344
#define STR_ERR_DLGEDITVW31 			34345
#define STR_ERR_DLGEDITVW32 			34346
#define STR_ERR_DLGEDITVW33 			34347
#define STR_ERR_DLGEDITVW34 			34348
#define STR_ERR_DLGEDITVW35 			34349
#define STR_ERR_DLGEDITVW36 			37349


// CEditView_Cmdgrep.cpp 33217
#define STR_ERR_DLGEDITVWGREP1			34350


// CEditView_CmdHokan.cpp 33220
#define STR_ERR_DLGEDITVWHOKAN1			34351


// CEditView_Cmdisrch.cpp 33223
#define STR_ERR_DLGEDITVWISRCH1			34352
#define STR_ERR_DLGEDITVWISRCH2			34353
#define STR_ERR_DLGEDITVWISRCH3			34354


// CEditView_Command.cpp 33228
//#define STR_ERR_MACRO1 33229
//#define STR_ERR_INSTXT1 33230
//#define STR_ERR_SRPREV1 33231
//#define STR_ERR_SRPREV2 33232
//#define STR_ERR_SRPREV3 33233
//#define STR_ERR_SRNEXT1 33234
//#define STR_ERR_SRNEXT2 33235
//#define STR_ERR_SRNEXT3 33236
//#define STR_ERR_UNINDENT1 33237
//#define STR_ERR_TAGJMP1 33238
//#define STR_ERR_TAGJMPBK1 33239
//#define STR_ERR_MACROERR1 33240
#define STR_ERR_CEDITVIEW_CMD01			34355
#define STR_ERR_CEDITVIEW_CMD02			34356
#define STR_ERR_CEDITVIEW_CMD03			34357
#define STR_ERR_CEDITVIEW_CMD04			34358
#define STR_ERR_CEDITVIEW_CMD05			34359
#define STR_ERR_CEDITVIEW_CMD06			34360
#define STR_ERR_CEDITVIEW_CMD07			34361
#define STR_ERR_CEDITVIEW_CMD08			34362
#define STR_ERR_CEDITVIEW_CMD09			34363
#define	STR_ERR_CEDITVIEW_CMD10			34364
#define	STR_ERR_CEDITVIEW_CMD11			34365
#define	STR_ERR_CEDITVIEW_CMD12			34366
#define	STR_ERR_CEDITVIEW_CMD13			34367
#define	STR_ERR_CEDITVIEW_CMD14			34368
#define	STR_ERR_CEDITVIEW_CMD15			34369
#define	STR_ERR_CEDITVIEW_CMD16			34370
#define	STR_ERR_CEDITVIEW_CMD17			34371
#define	STR_ERR_CEDITVIEW_CMD18			34372
#define	STR_ERR_CEDITVIEW_CMD19			34373
#define	STR_ERR_CEDITVIEW_CMD20			34374
#define	STR_ERR_CEDITVIEW_CMD21			34375
#define	STR_ERR_CEDITVIEW_CMD22			34376
#define	STR_ERR_CEDITVIEW_CMD23			34377
#define	STR_ERR_CEDITVIEW_CMD24			34378
#define	STR_ERR_CEDITVIEW_CMD25			34379
#define	STR_ERR_CEDITVIEW_CMD26			34380
#define	STR_ERR_CEDITVIEW_CMD27			34381
#define	STR_ERR_CEDITVIEW_CMD28			34382
#define	STR_ERR_CEDITVIEW_CMD29			34383


// CEditView_Command_New.cpp 33242
#define STR_ERR_DLGEDITVWCMDNW1 		34384
#define STR_ERR_DLGEDITVWCMDNW2 		34385
#define STR_ERR_DLGEDITVWCMDNW3 		34386
#define STR_ERR_DLGEDITVWCMDNW4 		34387
#define STR_ERR_DLGEDITVWCMDNW5 		34388
#define STR_ERR_DLGEDITVWCMDNW6 		34389
#define STR_ERR_DLGEDITVWCMDNW7 		34390
#define STR_ERR_DLGEDITVWCMDNW8 		34391
#define STR_ERR_DLGEDITVWCMDNW9 		34392
#define STR_ERR_DLGEDITVWCMDNW10		34393
#define STR_ERR_DLGEDITVWCMDNW11		34394
#define STR_ERR_DLGEDITVWCMDNW12		34395


// CEditView_Diff.cpp 33256
#define STR_ERR_DLGEDITVWDIFF1 			34396
#define STR_ERR_DLGEDITVWDIFF2 			34397
#define STR_ERR_DLGEDITVWDIFF3 			34398
#define STR_ERR_DLGEDITVWDIFF4 			34399
#define STR_ERR_DLGEDITVWDIFF5 			34400
#define STR_ERR_DLGEDITVWDIFF6 			34401
#define STR_ERR_DLGEDITVWDIFF7 			34402
#define STR_ERR_DLGEDITVWDIFF8 			34403
#define STR_ERR_DLGEDITVWDIFF9 			34404
#define STR_ERR_DLGEDITVWDIFF10			34405
#define STR_ERR_DLGEDITVWDIFF11			34406
#define STR_ERR_DLGEDITVWDIFF12			34407
#define STR_ERR_DLGEDITVWDIFF13			34408
#define STR_ERR_DLGEDITVWDIFF14			34409


// CEditView_New.cpp -- no w 33272


// CEditView_New2.cpp -- no 33274


// CEditView_New3.cpp -- no 33276


// CEditWnd.cpp 33278
#define STR_ERR_DLGEDITWND1 			34410
#define STR_ERR_DLGEDITWND2 			34411
#define STR_ERR_DLGEDITWND3 			34412
#define STR_ERR_DLGEDITWND4 			34413
#define STR_ERR_DLGEDITWND5 			34414
#define STR_ERR_DLGEDITWND6 			34415
#define STR_ERR_DLGEDITWND7 			34416
#define STR_ERR_DLGEDITWND8 			34417
#define STR_ERR_DLGEDITWND9				34418
#define STR_ERR_DLGEDITWND10			34419
#define STR_ERR_DLGEDITWND11			34420
#define STR_ERR_DLGEDITWND12			34421
#define STR_ERR_DLGEDITWND13			34422
#define STR_ERR_DLGEDITWND14			34423
#define STR_ERR_DLGEDITWND15			34424
#define STR_ERR_DLGEDITWND16			34425
#define STR_ERR_DLGEDITWND17			34426
#define STR_ERR_DLGEDITWND18			34427

#define STR_ERR_DLGEDITWND19			34428
#define STR_ERR_DLGEDITWND20			34429
#define STR_ERR_DLGEDITWND21			34430
#define STR_ERR_DLGEDITWND22			34431
#define STR_ERR_DLGEDITWND23			34432
#define STR_ERR_DLGEDITWND24			34433
#define STR_ERR_DLGEDITWND25			34434
#define STR_ERR_DLGEDITWND26			34435
#define STR_ERR_DLGEDITWND27			34436


// CFuncInfoArr.cpp
#define STR_ERR_DLGFUNCKEYWN1			34437	


// CFuncLookup.cpp
#define STR_ERR_DLGFUNCLKUP1			34438
#define STR_ERR_DLGFUNCLKUP2			34439
#define STR_ERR_DLGFUNCLKUP3			34440

#define STR_ERR_DLGFUNCLKUP04			34787
#define STR_ERR_DLGFUNCLKUP05			34788
#define STR_ERR_DLGFUNCLKUP06			34789
#define STR_ERR_DLGFUNCLKUP07			34790
#define STR_ERR_DLGFUNCLKUP08			34791
#define STR_ERR_DLGFUNCLKUP09			34792
#define STR_ERR_DLGFUNCLKUP10			34793
#define STR_ERR_DLGFUNCLKUP11			34794
#define STR_ERR_DLGFUNCLKUP12			34795
#define STR_ERR_DLGFUNCLKUP13			34796
#define STR_ERR_DLGFUNCLKUP14			34797
#define STR_ERR_DLGFUNCLKUP15			34798
#define STR_ERR_DLGFUNCLKUP16			34799
#define STR_ERR_DLGFUNCLKUP17			34800
#define STR_ERR_DLGFUNCLKUP18			34801



// charchode.cpp


// CHokanMgr.cpp


// CHtmlHelp.cpp


// CImageListMgr.cpp


// CKeyBind.cpp
#define STR_ERR_DLGKEYBIND1				34441
#define STR_ERR_DLGKEYBIND2				34442


// CKeyMacroMgr.cpp
#define STR_ERR_DLGKEYMACMGR1			34443
#define STR_ERR_DLGKEYMACMGR2			34444
#define STR_ERR_DLGKEYMACMGR3			34445
#define STR_ERR_DLGKEYMACMGR4			34446
#define STR_ERR_DLGKEYMACMGR5			34447
#define STR_ERR_DLGKEYMACMGR6			34448
#define STR_ERR_DLGKEYMACMGR7			34449
#define STR_ERR_DLGKEYMACMGR8			34450


// CKeyWordSetMgr.cpp


// CLayout.cpp


// CLayoutMgr.cpp


// CLayoutMgr.cpp


// CLayoutMgr_New.cpp


// CLayoutMgr_New2.cpp


// CLineComment.cpp


// CMacro.cpp
#define STR_ERR_DLGMACRO1				34451
#define STR_ERR_DLGMACRO2				34452
#define STR_ERR_DLGMACRO3				34453
#define STR_ERR_DLGMACRO4				34454
#define STR_ERR_DLGMACRO5				34455
#define STR_ERR_DLGMACRO6				34456
#define STR_ERR_DLGMACRO7				34457
#define STR_ERR_DLGMACRO8				34458
#define STR_ERR_DLGMACRO9				34459
#define STR_ERR_DLGMACRO10				34460
#define STR_ERR_DLGMACRO11				34461
#define STR_ERR_DLGMACRO12				34462
#define STR_ERR_DLGMACRO13				34463
#define STR_ERR_DLGMACRO14				34464
#define STR_ERR_DLGMACRO15				34465

// CMacroFactory.cpp


// CMacroManagerBase.cpp


// CMarkMgr.cpp


// CMemory.cpp
#define STR_ERR_DLGMEM1					34466


// CMenuDrawer.cpp
#define STR_ERR_DLGMNUDRAW1				34467


// CMigemo.cpp


// CMRU.cpp


// CMRUFolder.cpp


// CNormalProcess.cpp
#define STR_ERR_DLGNRMPROC1				34468
#define STR_ERR_DLGNRMPROC2				34469


// COpe.cpp


// COpeBlk.cpp
#define STR_ERR_DLGOPEBLK1				34470
#define STR_ERR_DLGOPEBLK2				34471


// CPPA.cpp
#define STR_ERR_DLGPPA1					34472
#define STR_ERR_DLGPPA2					34473
#define STR_ERR_DLGPPA3					34474
#define STR_ERR_DLGPPA4					34475
#define STR_ERR_DLGPPA5					34476
#define STR_ERR_DLGPPA6					34477
#define STR_ERR_DLGPPA7					34478


// CPPAMacroMgr.cpp


// CPrint.cpp
#define STR_ERR_CPRINT01				34479
#define STR_ERR_CPRINT02				34480
#define STR_ERR_CPRINT03				34481


// CPrintPreview.cpp
#define STR_ERR_DLGPRNPRVW1				34482
#define STR_ERR_DLGPRNPRVW2				34483
#define STR_ERR_DLGPRNPRVW3				34484
#define STR_ERR_DLGPRNPRVW4				34485
#define STR_ERR_DLGPRNPRVW5				34486
#define STR_ERR_DLGPRNPRVW6				34487
#define STR_ERR_DLGPRNPRVW7				34488
#define STR_ERR_DLGPRNPRVW8				34489


// CProcess.cpp
#define STR_ERR_DLGPROCESS1				34490


// CProcessFactory.cpp
#define STR_ERR_DLGPROCFACT1			34491
#define STR_ERR_DLGPROCFACT2			34492
#define STR_ERR_DLGPROCFACT3			34493
#define STR_ERR_DLGPROCFACT4			34494
#define STR_ERR_DLGPROCFACT5			34495


// CProfile.cpp


// CPropComBackup.cpp
#define STR_ERR_DLGPROPCOMBK1			34496
#define STR_ERR_DLGPROPCOMBK2			34497	


// CPropComCustmenu.cpp
#define STR_ERR_DLGPROPCOMCUST1			34498
#define STR_ERR_DLGPROPCOMCUST2			34499
#define STR_ERR_DLGPROPCOMCUST3			34500
#define STR_ERR_DLGPROPCOMCUST4			34501
#define STR_ERR_DLGPROPCOMCUST5			34502
#define STR_ERR_DLGPROPCOMCUST6			34503
// Has a HEADER #define, sho


// CPropComEdit.cpp


// CPropComFile.cpp
#define STR_ERR_DLGPROPCOMFNM1			34504
#define STR_ERR_DLGPROPCOMFNM2			34505
#define STR_ERR_DLGPROPCOMFNM3			34506


// CPropComFormat.cpp
// Has static date-time stri


// CPropComGrep.cpp 
#define STR_ERR_DLGPROPCOMGREP1			34507


// CPropComHelper.cpp
#define STR_ERR_DLGPROPCOMHELP1			34508


// CPropComKeybind.cpp
// STR_KEYDATA_HEAD... 
#define STR_ERR_DLGPROPCOMKEYBIND1		34509
#define STR_ERR_DLGPROPCOMKEYBIND2		34510
#define STR_ERR_DLGPROPCOMKEYBIND3		34511


// CPropComKeyword.cpp
#define STR_ERR_DLGPROPCOMKEYWORD1		34512
#define STR_ERR_DLGPROPCOMKEYWORD2		34513
#define STR_ERR_DLGPROPCOMKEYWORD3		34514
#define STR_ERR_DLGPROPCOMKEYWORD4		34515
#define STR_ERR_DLGPROPCOMKEYWORD5		34516
#define STR_ERR_DLGPROPCOMKEYWORD6		34517
#define STR_ERR_DLGPROPCOMKEYWORD7		34518
#define STR_ERR_DLGPROPCOMKEYWORD8		34519
#define STR_ERR_DLGPROPCOMKEYWORD9		34520
#define STR_ERR_DLGPROPCOMKEYWORD10		34521
#define STR_ERR_DLGPROPCOMKEYWORD11		34522
#define STR_ERR_DLGPROPCOMKEYWORD12		34523
#define STR_ERR_DLGPROPCOMKEYWORD13		34524
#define STR_ERR_DLGPROPCOMKEYWORD14		34525
#define STR_ERR_DLGPROPCOMKEYWORD15		34526
#define STR_ERR_DLGPROPCOMKEYWORD16		34527
#define STR_ERR_DLGPROPCOMKEYWORD17		34528
#define STR_ERR_DLGPROPCOMKEYWORD18		34529
#define STR_ERR_DLGPROPCOMKEYWORD19		34530


// CPropComMacro.cpp
#define STR_ERR_DLGPROPCOMMACRO01		34531
#define STR_ERR_DLGPROPCOMMACRO02		34532
#define STR_ERR_DLGPROPCOMMACRO03		34533
#define STR_ERR_DLGPROPCOMMACRO04		34534
#define STR_ERR_DLGPROPCOMMACRO05		34535
#define STR_ERR_DLGPROPCOMMACRO06		34536
#define STR_ERR_DLGPROPCOMMACRO07		34537
#define STR_ERR_DLGPROPCOMMACRO08		34538


// CPropCommon.cpp ( Must use consecutive ordering here ... )
#define STR_ERR_DLGPROPCOMMON01			34539
#define STR_ERR_DLGPROPCOMMON02			34540
#define STR_ERR_DLGPROPCOMMON03			34541
#define STR_ERR_DLGPROPCOMMON04			34542
#define STR_ERR_DLGPROPCOMMON05			34543
#define STR_ERR_DLGPROPCOMMON06			34544
#define STR_ERR_DLGPROPCOMMON07			34545
#define STR_ERR_DLGPROPCOMMON08			34546
#define STR_ERR_DLGPROPCOMMON09			34547
#define STR_ERR_DLGPROPCOMMON10			34548
#define STR_ERR_DLGPROPCOMMON11			34549
#define STR_ERR_DLGPROPCOMMON12			34550
#define STR_ERR_DLGPROPCOMMON13			34551
#define STR_ERR_DLGPROPCOMMON14			34552
#define STR_ERR_DLGPROPCOMMON15			34553

#define STR_ERR_DLGPROPCOMMON16			34554
#define STR_ERR_DLGPROPCOMMON17			34555
#define STR_ERR_DLGPROPCOMMON18			34556
#define STR_ERR_DLGPROPCOMMON19			34557
#define STR_ERR_DLGPROPCOMMON20			34558
#define STR_ERR_DLGPROPCOMMON21			34559

#define STR_ERR_DLGPROPCOMMON22			34560
#define STR_ERR_DLGPROPCOMMON23			34561
#define STR_ERR_DLGPROPCOMMON24			34562
#define STR_ERR_DLGPROPCOMMON25			34563
#define STR_ERR_DLGPROPCOMMON26			34564
#define STR_ERR_DLGPROPCOMMON27			34565
#define STR_ERR_DLGPROPCOMMON28			34566


// CPropComToolbar.cpp
#define STR_ERR_DLGPROPCOMTOOL01		34567
#define STR_ERR_DLGPROPCOMTOOL02		34568
#define STR_ERR_DLGPROPCOMTOOL03		34569
#define STR_ERR_DLGPROPCOMTOOL04		34570
#define STR_ERR_DLGPROPCOMTOOL05		34571
#define STR_ERR_DLGPROPCOMTOOL06		34572
#define STR_ERR_DLGPROPCOMTOOL07		34573
#define STR_ERR_DLGPROPCOMTOOL08		34574


// CPropComWin.cpp


// CPropTypes.cpp
#define STR_ERR_DLGCPROPTYPES01			34575
#define STR_ERR_DLGCPROPTYPES02			34576
#define STR_ERR_DLGCPROPTYPES03			34577
#define STR_ERR_DLGCPROPTYPES04			34578
#define STR_ERR_DLGCPROPTYPES05			34579
#define STR_ERR_DLGCPROPTYPES06			34580
#define STR_ERR_DLGCPROPTYPES07			34581
#define STR_ERR_DLGCPROPTYPES08			34582
#define STR_ERR_DLGCPROPTYPES09			34583
#define STR_ERR_DLGCPROPTYPES10			34584
#define STR_ERR_DLGCPROPTYPES11			34585
#define STR_ERR_DLGCPROPTYPES12			34586
#define STR_ERR_DLGCPROPTYPES13			34587
#define STR_ERR_DLGCPROPTYPES14			34588
#define STR_ERR_DLGCPROPTYPES15			34589
#define STR_ERR_DLGCPROPTYPES16			34590
#define STR_ERR_DLGCPROPTYPES17			34591
#define STR_ERR_DLGCPROPTYPES18			34592
#define STR_ERR_DLGCPROPTYPES19			34593
#define STR_ERR_DLGCPROPTYPES20			34594
#define STR_ERR_DLGCPROPTYPES21			34595
#define STR_ERR_DLGCPROPTYPES22			34596
#define STR_ERR_DLGCPROPTYPES23			34597
#define STR_ERR_DLGCPROPTYPES24			34598
#define STR_ERR_DLGCPROPTYPES25			34599
#define STR_ERR_DLGCPROPTYPES26			34600
#define STR_ERR_DLGCPROPTYPES27			34601
#define STR_ERR_DLGCPROPTYPES28			34602
#define STR_ERR_DLGCPROPTYPES29			34603
#define STR_ERR_DLGCPROPTYPES30			34604
#define STR_ERR_DLGCPROPTYPES31			34605
#define STR_ERR_DLGCPROPTYPES32			34606
#define STR_ERR_DLGCPROPTYPES33			34607
#define STR_ERR_DLGCPROPTYPES34			34608
#define STR_ERR_DLGCPROPTYPES35			34609
#define STR_ERR_DLGCPROPTYPES36			34610
#define STR_ERR_DLGCPROPTYPES37			34611
#define STR_ERR_DLGCPROPTYPES38			34612
#define STR_ERR_DLGCPROPTYPES39			34613
#define STR_ERR_DLGCPROPTYPES40			34614


// CPropTypesKeyHelp.cpp
#define STR_ERR_DLGPROPTYPKEYHELP01		34615
#define STR_ERR_DLGPROPTYPKEYHELP02		34616
#define STR_ERR_DLGPROPTYPKEYHELP03		34617
#define STR_ERR_DLGPROPTYPKEYHELP04		34618
#define STR_ERR_DLGPROPTYPKEYHELP05		34619
#define STR_ERR_DLGPROPTYPKEYHELP06		34620
#define STR_ERR_DLGPROPTYPKEYHELP07		34621
#define STR_ERR_DLGPROPTYPKEYHELP08		34622
#define STR_ERR_DLGPROPTYPKEYHELP09		34623
#define STR_ERR_DLGPROPTYPKEYHELP10		34624
#define STR_ERR_DLGPROPTYPKEYHELP11		34625
#define STR_ERR_DLGPROPTYPKEYHELP12		34626
#define STR_ERR_DLGPROPTYPKEYHELP13		34627
#define STR_ERR_DLGPROPTYPKEYHELP14		34628
#define STR_ERR_DLGPROPTYPKEYHELP15		34629
#define STR_ERR_DLGPROPTYPKEYHELP16		34630
#define STR_ERR_DLGPROPTYPKEYHELP17		34631
#define STR_ERR_DLGPROPTYPKEYHELP18		34632



// CPropTypesRegex.cpp
#define	STR_ERR_DLGPROPTYPESREGEX01		34633
#define	STR_ERR_DLGPROPTYPESREGEX02		34634
#define	STR_ERR_DLGPROPTYPESREGEX03		34635
#define	STR_ERR_DLGPROPTYPESREGEX04		34636
#define	STR_ERR_DLGPROPTYPESREGEX05		34637
#define	STR_ERR_DLGPROPTYPESREGEX06		34638
#define	STR_ERR_DLGPROPTYPESREGEX07		34639
#define	STR_ERR_DLGPROPTYPESREGEX08		34640
#define	STR_ERR_DLGPROPTYPESREGEX09		34641
#define	STR_ERR_DLGPROPTYPESREGEX10		34642
#define	STR_ERR_DLGPROPTYPESREGEX11		34643
#define	STR_ERR_DLGPROPTYPESREGEX12		34644
#define	STR_ERR_DLGPROPTYPESREGEX13		34645
#define	STR_ERR_DLGPROPTYPESREGEX14		34646
#define	STR_ERR_DLGPROPTYPESREGEX15		34647
#define	STR_ERR_DLGPROPTYPESREGEX16		34648
#define	STR_ERR_DLGPROPTYPESREGEX17		34649
#define	STR_ERR_DLGPROPTYPESREGEX18		34650
#define	STR_ERR_DLGPROPTYPESREGEX19		34651
#define	STR_ERR_DLGPROPTYPESREGEX20		34652
#define	STR_ERR_DLGPROPTYPESREGEX21		34653
#define	STR_ERR_DLGPROPTYPESREGEX22		34654
#define	STR_ERR_DLGPROPTYPESREGEX23		34655
#define	STR_ERR_DLGPROPTYPESREGEX24		34656
#define	STR_ERR_DLGPROPTYPESREGEX25		34657
#define	STR_ERR_DLGPROPTYPESREGEX26		34658
#define	STR_ERR_DLGPROPTYPESREGEX27		34659


// CRecent.cpp


// CRegexKeyword.cpp


// CRunningTimer.cpp


// CShareData.cpp
#define	STR_ERR_CSHAREDATA01			34660
#define	STR_ERR_CSHAREDATA02			34661
#define	STR_ERR_CSHAREDATA03			34662
#define	STR_ERR_CSHAREDATA04			34663
#define	STR_ERR_CSHAREDATA05			34664
#define	STR_ERR_CSHAREDATA06			34665
#define	STR_ERR_CSHAREDATA07			34666
#define	STR_ERR_CSHAREDATA08			34667
#define	STR_ERR_CSHAREDATA09			34668

					
#define	STR_ERR_CSHAREDATA10			34669
#define	STR_ERR_CSHAREDATA11			34670
#define	STR_ERR_CSHAREDATA12			34671
#define	STR_ERR_CSHAREDATA13			34672
#define	STR_ERR_CSHAREDATA14			34673
#define	STR_ERR_CSHAREDATA15			34674
#define	STR_ERR_CSHAREDATA16			34675
#define	STR_ERR_CSHAREDATA17			34676
#define	STR_ERR_CSHAREDATA18			34677
#define	STR_ERR_CSHAREDATA19			34678
#define	STR_ERR_CSHAREDATA20			34679
#define	STR_ERR_CSHAREDATA21			34680
#define	STR_ERR_CSHAREDATA22			34681
#define	STR_ERR_CSHAREDATA23			34682

// ...
#define	STR_ERR_CSHAREDATA24			34683
#define	STR_ERR_CSHAREDATA25			34684
#define	STR_ERR_CSHAREDATA26			34685
#define	STR_ERR_CSHAREDATA27			34686
#define	STR_ERR_CSHAREDATA28			34687
#define	STR_ERR_CSHAREDATA29			34688
#define	STR_ERR_CSHAREDATA30			34689
#define	STR_ERR_CSHAREDATA31			34690
#define	STR_ERR_CSHAREDATA32			34691
#define	STR_ERR_CSHAREDATA33			34692
#define	STR_ERR_CSHAREDATA34			34693
#define	STR_ERR_CSHAREDATA35			34694

#define	STR_ERR_CSHAREDATA36			34695
#define	STR_ERR_CSHAREDATA37			34696
#define	STR_ERR_CSHAREDATA38			34697
#define	STR_ERR_CSHAREDATA39			34698
#define	STR_ERR_CSHAREDATA40			34699
#define	STR_ERR_CSHAREDATA41			34700
#define	STR_ERR_CSHAREDATA42			34701
#define	STR_ERR_CSHAREDATA43			34702
#define	STR_ERR_CSHAREDATA44			34703
#define	STR_ERR_CSHAREDATA45			34704
#define	STR_ERR_CSHAREDATA46			34705
#define	STR_ERR_CSHAREDATA47			34706
#define	STR_ERR_CSHAREDATA48			34707
#define	STR_ERR_CSHAREDATA49			34708
#define	STR_ERR_CSHAREDATA50			34709
#define	STR_ERR_CSHAREDATA51			34710

#define	STR_ERR_CSHAREDATA52			34711

#define	STR_ERR_CSHAREDATA53			34712
#define	STR_ERR_CSHAREDATA54			34713
#define	STR_ERR_CSHAREDATA55			34714
#define	STR_ERR_CSHAREDATA56			34715
#define	STR_ERR_CSHAREDATA57			34716
#define	STR_ERR_CSHAREDATA58			34717
#define	STR_ERR_CSHAREDATA59			34718
#define	STR_ERR_CSHAREDATA60			34719
#define	STR_ERR_CSHAREDATA61			34720
#define	STR_ERR_CSHAREDATA62			34721
#define	STR_ERR_CSHAREDATA63			34722
#define	STR_ERR_CSHAREDATA64			34723

#define	STR_ERR_CSHAREDATA65			34724
#define	STR_ERR_CSHAREDATA66			34725
#define	STR_ERR_CSHAREDATA67			34726
#define	STR_ERR_CSHAREDATA68			34727
#define	STR_ERR_CSHAREDATA69			34728
#define	STR_ERR_CSHAREDATA70			34729
#define	STR_ERR_CSHAREDATA71			34730
#define	STR_ERR_CSHAREDATA72			34731
#define	STR_ERR_CSHAREDATA73			34732
#define	STR_ERR_CSHAREDATA74			34733
#define	STR_ERR_CSHAREDATA75			34734
#define	STR_ERR_CSHAREDATA76			34735
#define	STR_ERR_CSHAREDATA77			34736
#define	STR_ERR_CSHAREDATA78			34737
#define	STR_ERR_CSHAREDATA79			34738
#define	STR_ERR_CSHAREDATA80			34739
#define	STR_ERR_CSHAREDATA81			34740
#define	STR_ERR_CSHAREDATA82			34741

#define	STR_ERR_CSHAREDATA83			34742
#define	STR_ERR_CSHAREDATA84			34743
#define	STR_ERR_CSHAREDATA85			34744
#define	STR_ERR_CSHAREDATA86			34745
#define	STR_ERR_CSHAREDATA87			34746
#define	STR_ERR_CSHAREDATA88			34747
#define	STR_ERR_CSHAREDATA89			34748
#define	STR_ERR_CSHAREDATA90			34749
#define	STR_ERR_CSHAREDATA91			34750
#define	STR_ERR_CSHAREDATA92			34751
#define	STR_ERR_CSHAREDATA93			34752
#define	STR_ERR_CSHAREDATA94			34753
#define	STR_ERR_CSHAREDATA95			34754
#define	STR_ERR_CSHAREDATA96			34755
#define	STR_ERR_CSHAREDATA97			34756
#define	STR_ERR_CSHAREDATA98			34757

#define	STR_ERR_CSHAREDATA99			34758		


// CShareData_new.cpp


// CSMacroMgr.cpp


// CSortedTagJumpList.cpp


// CSplitBoxWnd.cpp


// CSplitterWnd.cpp
#define	STR_ERR_CSPLITTER01				34759


// CTabWnd.cpp
#define	STR_ERR_CTABWND01				34760
#define	STR_ERR_CTABWND02				34761
#define	STR_ERR_CTABWND03				34762
#define	STR_ERR_CTABWND04				34763
#define	STR_ERR_CTABWND05				34764

#define	STR_ERR_CTABWND06				34765
#define	STR_ERR_CTABWND07				34766
#define	STR_ERR_CTABWND08				34767


// CTipWnd.cpp


// CUxTheme.cpp


// CWaitCursor.cpp


// CWnd.cpp


// CWSH.cpp
#define	STR_ERR_CWSH01					34768
#define	STR_ERR_CWSH02					34769
#define	STR_ERR_CWSH03					34770
#define	STR_ERR_CWSH04					34771
#define	STR_ERR_CWSH05					34772
#define	STR_ERR_CWSH06					34773
#define	STR_ERR_CWSH07					34774
#define	STR_ERR_CWSH08					34775


// etc_uty.cpp
#define	STR_ERR_ETCUTY01				34776
#define	STR_ERR_ETCUTY02				34777
#define	STR_ERR_ETCUTY03				34778
#define	STR_ERR_ETCUTY04				34779
#define	STR_ERR_ETCUTY05				34780
#define	STR_ERR_ETCUTY06				34781
#define	STR_ERR_ETCUTY07				34782
#define	STR_ERR_ETCUTY08				34783
#define	STR_ERR_ETCUTY09				34784
#define	STR_ERR_ETCUTY10				34785
#define	STR_ERR_ETCUTY11				34786

// global.cpp
#define	STR_ERR_GLOBAL01				34802


// LMP: End -----------------------------------------------------
// LMP: End -----------------------------------------------------
// LMP: End -----------------------------------------------------





#define F_FUNCTION_FIRST  40000 // 2003-02-21 鬼 これ以上だと関数

#define F_GETFILENAME     40001 /* 編集中のファイル名を取得する */
#define F_GETSAVEFILENAME 40018 /* 保存時のファイル名を取得する */	// 2006.09.04 ryoji
#define F_GETSELECTED     40002 // Oct. 19, 2002 genta 選択範囲の取得
#define F_EXPANDPARAMETER 40003 // 2003-02-21 鬼 コマンドラインパラメータ展開
#define F_GETLINESTR      40004 // 指定行論理データを取得する 2003.06.25 Moca
#define F_GETLINECOUNT    40005 // 論理行数を取得する 2003.06.25 Moca
#define F_CHGTABWIDTH     40006 // タブサイズを取得、設定する 2004.03.16 zenryaku
#define F_ISTEXTSELECTED  40007 // テキストが選択されているか 2005.7.30 maru
#define F_GETSELLINEFROM  40008 // 選択開始行の取得 2005.7.30 maru
#define F_GETSELCOLMFROM  40009 // 選択開始桁の取得 2005.7.30 maru
#define F_GETSELLINETO    40010 // 選択終了行の取得 2005.7.30 maru
#define F_GETSELCOLMTO    40011 // 選択終了桁の取得 2005.7.30 maru
#define F_ISINSMODE       40012 // 挿入／上書きモードの取得 2005.7.30 maru
#define F_GETCHARCODE     40013 // 文字コード取得 2005.07.31 maru
#define F_GETLINECODE     40014 // 改行コード取得 2005.08.05 maru
#define F_ISPOSSIBLEUNDO  40015 // Undo可能か調べる 2005.08.05 maru
#define F_ISPOSSIBLEREDO  40016 // Redo可能か調べる 2005.08.05 maru
#define F_CHGWRAPCOLM     40017 // 折り返し桁を取得、設定する 2008.06.19 ryoji
#define F_ISCURTYPEEXT    40019 // 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
#define F_ISSAMETYPEEXT   40020 // ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji


//	2005.01.10 genta ISearch用補助コード
//	2007.07.07 genta 16bit以内に収めないと状況コードと衝突するのでコードを変更
#define F_ISEARCH_ADD_CHAR	0xC001	//	Incremental Searchへ1文字へ追加
#define F_ISEARCH_ADD_STR	0xC002	//	Incremental Searchへ文字列へ追加
#define F_ISEARCH_DEL_BACK	0xC003	//	Incremental Searchの末尾から1文字削除

/* 機能一覧に関するデータ宣言 */
namespace nsFuncCode{
	extern const char*	ppszFuncKind[];
	extern const int	nFuncKindNum;
	extern const int	pnFuncListNumArr[];
	extern const int*	ppnFuncListArr[];
	extern const int	nFincListNumArrNum;
};
///////////////////////////////////////////////////////////////////////
#endif /* _FUNCCODE_H_ */


/*[EOF]*/
