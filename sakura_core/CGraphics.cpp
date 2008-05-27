/*
2008.05.20 kobake 作成
*/

#include "stdafx.h"
#include "CGraphics.h"

void CGraphics::Init(HDC hdc)
{
	m_hdc = hdc;
	m_hfntOld = NULL;
	//ペン
	m_hpnOrg = NULL;
	m_hpnCurrent = NULL;
	//ブラシ
	m_hbrOrg = NULL;
	m_hbrCurrent = NULL;
	m_bDynamicBrush = NULL;
}

CGraphics::~CGraphics()
{
	RestoreFont();
	RestorePen();
	RestoreBrush();
	RestoreTextColors();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           総合                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 描画色を設定
void CGraphics::SetForegroundColor(COLORREF color)
{
	//テキスト前景色
	SetTextForeColor(color);

	//ペン
//	SetPenColor(color);
}

//! 背景色を設定
void CGraphics::SetBackgroundColor(COLORREF color)
{
	//テキスト背景色
	SetTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         テキスト                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::SetTextForeColor(COLORREF color)
{
	m_clrTextForeOrg.AssignOnce( ::SetTextColor(m_hdc,color) );
}

void CGraphics::SetTextBackColor(COLORREF color)
{
	m_clrTextBackOrg.AssignOnce( ::SetBkColor(m_hdc,color) );
}

void CGraphics::SetTextBackTransparent(bool b)
{
	m_nTextModeOrg.AssignOnce( ::SetBkMode(m_hdc,b?TRANSPARENT:OPAQUE) );
}

void CGraphics::RestoreTextColors()
{
	if(m_clrTextForeOrg.HasData()){
		::SetTextColor(m_hdc,m_clrTextForeOrg.Get());
		m_clrTextForeOrg.Clear();
	}
	if(m_clrTextBackOrg.HasData()){
		::SetTextColor(m_hdc,m_clrTextBackOrg.Get());
		m_clrTextBackOrg.Clear();
	}
	if(m_nTextModeOrg.HasData()){
		::SetBkMode(m_hdc,m_nTextModeOrg.Get());
		m_nTextModeOrg.Clear();
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フォント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! フォント設定
void CGraphics::SetMyFont(HFONT hFont)
{
	HFONT hfntOld = (HFONT)::SelectObject(m_hdc, hFont);
	if(!m_hfntOld){
		m_hfntOld = hfntOld;
	}
}

void CGraphics::RestoreFont()
{
	if( m_hfntOld ){
		::SelectObject( m_hdc, m_hfntOld );
		m_hfntOld = NULL;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ペン                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CGraphics::SetPenColor(COLORREF color)
{
	RestorePen();
	m_hpnCurrent = CreatePen(PS_SOLID,1,color);
	m_hpnOrg = (HPEN)SelectObject(m_hdc,m_hpnCurrent);
}

void CGraphics::RestorePen()
{
	if(m_hpnOrg){
		SelectObject(m_hdc,m_hpnOrg);
		m_hpnOrg = NULL;
	}
	if(m_hpnCurrent){
		DeleteObject(m_hpnCurrent);
		m_hpnCurrent = NULL;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ブラシ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::SetBrushColor(COLORREF color)
{
	RestoreBrush();
	m_hbrCurrent = ::CreateSolidBrush(color);
	m_bDynamicBrush = true;
	m_hbrOrg = (HBRUSH)::SelectObject(m_hdc,m_hbrCurrent);
}

void CGraphics::SetNullBrush()
{
	RestoreBrush();
	m_hbrCurrent = (HBRUSH)::GetStockObject(NULL_BRUSH);
	m_bDynamicBrush = false;
	m_hbrOrg = (HBRUSH)::SelectObject(m_hdc,m_hbrCurrent);
}

void CGraphics::RestoreBrush()
{
	if(m_hbrOrg){
		SelectObject(m_hdc,m_hbrOrg);
		m_hbrOrg = NULL;
	}
	if(m_hbrCurrent){
		if(m_bDynamicBrush){
			DeleteObject(m_hbrCurrent);
			m_bDynamicBrush = false;
		}
		m_hbrCurrent = NULL;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           直線                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::DrawLine(int x1, int y1, int x2, int y2)
{
	::MoveToEx(m_hdc,x1,y1,NULL);
	::LineTo(m_hdc,x2,y2);
}

