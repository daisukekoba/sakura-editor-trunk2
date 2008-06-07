/*
2008.05.20 kobake 作成
*/

#pragma once

#include <windows.h>
#include <vector>

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

	//テキスト文字色
	void PushTextForeColor(COLORREF color);
	void PopTextForeColor();
	void ClearTextForeColor();
	void SetTextForeColor(COLORREF color);

	//テキスト背景色
	void PushTextBackColor(COLORREF color);
	void PopTextBackColor();
	void ClearTextBackColor();
	void SetTextBackColor(COLORREF color);

	//テキストモード
	void SetTextBackTransparent(bool b);

	//テキスト
	void RestoreTextColors();

	//フォント
	void PushMyFont(HFONT hFont);
	void PopMyFont();
	void ClearMyFont();
	void SetMyFont(HFONT hFont);				//!< フォント設定

	//ペン
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color);
	void ClearPen();

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
	HDC					m_hdc;

	//テキスト
	std::vector<COLORREF>	m_vTextForeColors;
	std::vector<COLORREF>	m_vTextBackColors;
	std::vector<HFONT>		m_vFonts;

	//テキスト
	COrgInt				m_nTextModeOrg;

	//ペン
	HPEN				m_hpnOrg;
	std::vector<HPEN>	m_vPens;

	//ブラシ
	HBRUSH				m_hbrOrg;
	HBRUSH				m_hbrCurrent;
	bool				m_bDynamicBrush;	//m_hbrCurrentを動的に作成した場合はtrue
};
