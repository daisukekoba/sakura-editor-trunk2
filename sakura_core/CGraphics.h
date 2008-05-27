/*
2008.05.20 kobake 作成
*/

#pragma once

#include <windows.h>

//! オリジナル値保存クラス
template <class T>
class TOriginalHolder{
public:
	TOriginalHolder<T>()
	{
		m_data = 0;
		m_hold = false;
	}
	void Clear()
	{
		m_data = 0;
		m_hold = false;
	}
	void AssignOnce(const T& t)
	{
		if(!m_hold){
			m_data = t;
			m_hold = true;
		}
	}
	const T& Get() const
	{
		return m_data;
	}
	bool HasData() const
	{
		return m_hold;
	}
private:
	T		m_data;
	bool	m_hold;
};


//! 描画管理
class CGraphics{
public:
	CGraphics(const CGraphics& rhs){ Init(rhs.m_hdc); }
	CGraphics(HDC hdc = NULL){ Init(hdc); }
	~CGraphics();
	void Init(HDC hdc);

	operator HDC() const{ return m_hdc; }

	//色設定
	void SetForegroundColor(COLORREF color);	//!< 描画色を設定
	void SetBackgroundColor(COLORREF color);	//!< 背景色を設定

	//テキスト
	void SetTextForeColor(COLORREF color);
	void SetTextBackColor(COLORREF color);
	void SetTextBackTransparent(bool b);
	void RestoreTextColors();

	//フォント
	void SetMyFont(HFONT hFont);				//!< フォント設定
	void RestoreFont();

	//ペン
	void SetPenColor(COLORREF color);
	void RestorePen();

	//ブラシ
	void SetBrushColor(COLORREF color);
	void SetNullBrush();
	void RestoreBrush();
	HBRUSH GetCurrentBrush() const{ return m_hbrCurrent; }

	//直線
	void DrawLine(int x1, int y1, int x2, int y2);
private:
	//型
	typedef TOriginalHolder<COLORREF>	COrgColor;
	typedef TOriginalHolder<int>		COrgInt;
private:
	HDC			m_hdc;

	//テキスト
	COrgColor	m_clrTextForeOrg;
	COrgColor	m_clrTextBackOrg;
	COrgInt		m_nTextModeOrg;
	HFONT		m_hfntOld;

	//ペン
	HPEN		m_hpnOrg;
	HPEN		m_hpnCurrent;

	//ブラシ
	HBRUSH		m_hbrOrg;
	HBRUSH		m_hbrCurrent;
	bool		m_bDynamicBrush;	//m_hbrCurrentを動的に作成した場合はtrue
};
