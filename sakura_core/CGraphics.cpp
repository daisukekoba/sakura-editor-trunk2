/*
2008.05.20 kobake �쐬
*/

#include "stdafx.h"
#include "CGraphics.h"

void CGraphics::Init(HDC hdc)
{
	m_hdc = hdc;
	//�y��
	m_hpnOrg = NULL;
	//�u���V
	m_hbrOrg = NULL;
	m_hbrCurrent = NULL;
	m_bDynamicBrush = NULL;
}

CGraphics::~CGraphics()
{
	ClearMyFont();
	ClearPen();
	RestoreBrush();
	RestoreTextColors();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �`��F��ݒ�
void CGraphics::SetForegroundColor(COLORREF color)
{
	//�e�L�X�g�O�i�F
	SetTextForeColor(color);

	//�y��
//	SetPen(color);
}

//! �w�i�F��ݒ�
void CGraphics::SetBackgroundColor(COLORREF color)
{
	//�e�L�X�g�w�i�F
	SetTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�����F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextForeColor(COLORREF color)
{
	//�ݒ�
	COLORREF cOld = ::SetTextColor(m_hdc,color);
	//�L�^
	if(m_vTextForeColors.size()==0){
		m_vTextForeColors.push_back(cOld);
	}
	m_vTextForeColors.push_back(color);
}

void CGraphics::PopTextForeColor()
{
	//�߂�
	if(m_vTextForeColors.size()>=2){
		m_vTextForeColors.pop_back();
		::SetTextColor(m_hdc,m_vTextForeColors.back());
	}
}

void CGraphics::ClearTextForeColor()
{
	if(!m_vTextForeColors.empty()){
		::SetTextColor(m_hdc,m_vTextForeColors[0]);
		m_vTextForeColors.clear();
	}
}

void CGraphics::SetTextForeColor(COLORREF color)
{
	ClearTextForeColor();
	PushTextForeColor(color);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�w�i�F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextBackColor(COLORREF color)
{
	//�ݒ�
	COLORREF cOld = ::SetBkColor(m_hdc,color);
	//�L�^
	if(m_vTextBackColors.size()==0){
		m_vTextBackColors.push_back(cOld);
	}
	m_vTextBackColors.push_back(color);
}

void CGraphics::PopTextBackColor()
{
	//�߂�
	if(m_vTextBackColors.size()>=2){
		m_vTextBackColors.pop_back();
		::SetBkColor(m_hdc,m_vTextBackColors.back());
	}
}

void CGraphics::ClearTextBackColor()
{
	if(!m_vTextBackColors.empty()){
		::SetBkColor(m_hdc,m_vTextBackColors[0]);
		m_vTextBackColors.clear();
	}
}

void CGraphics::SetTextBackColor(COLORREF color)
{
	ClearTextBackColor();
	PushTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g���[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::SetTextBackTransparent(bool b)
{
	m_nTextModeOrg.AssignOnce( ::SetBkMode(m_hdc,b?TRANSPARENT:OPAQUE) );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�L�X�g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::RestoreTextColors()
{
	PopTextForeColor();
	PopTextBackColor();
	if(m_nTextModeOrg.HasData()){
		::SetBkMode(m_hdc,m_nTextModeOrg.Get());
		m_nTextModeOrg.Clear();
	}
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t�H���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushMyFont(HFONT hFont)
{
	//�ݒ�
	HFONT hfntOld = (HFONT)SelectObject(m_hdc, hFont);

	//�L�^
	if(m_vFonts.empty()){
		m_vFonts.push_back(hfntOld);
	}
	m_vFonts.push_back(hFont);
}

void CGraphics::PopMyFont()
{
	//�߂�
	if(m_vFonts.size()>=2){
		m_vFonts.pop_back();
	}
	if(!m_vFonts.empty()){
		SelectObject(m_hdc,m_vFonts.back());
	}
}

void CGraphics::ClearMyFont()
{
	if(!m_vFonts.empty()){
		SelectObject(m_hdc,m_vFonts[0]);
	}
	m_vFonts.clear();
}

//! �t�H���g�ݒ�
void CGraphics::SetMyFont(HFONT hFont)
{
	ClearMyFont();
	PushMyFont(hFont);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �y��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushPen(COLORREF color, int nPenWidth, int nStyle)
{
	HPEN hpnNew = CreatePen(nStyle,nPenWidth,color);
	HPEN hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	m_vPens.push_back(hpnNew);
	if(!m_hpnOrg){
		m_hpnOrg = hpnOld;
	}
}

void CGraphics::PopPen()
{
	//�I��������
	HPEN hpnNew = NULL;
	if(m_vPens.size()>=2){
		hpnNew = m_vPens[m_vPens.size()-2];
	}
	else{
		hpnNew = m_hpnOrg;
	}

	//�I��
	HPEN hpnOld = NULL;
	if(hpnNew){
		hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	}

	//�폜
	if(!m_vPens.empty()){
		DeleteObject(m_vPens.back());
		m_vPens.pop_back();
	}

	//�I���W�i��
	if(m_vPens.empty()){
		m_hpnOrg = NULL;
	}
}

void CGraphics::SetPen(COLORREF color)
{
	ClearPen();
	PushPen(color,1);
}

void CGraphics::ClearPen()
{
	if(m_hpnOrg){
		SelectObject(m_hdc,m_hpnOrg);
		m_hpnOrg = NULL;
	}
	for(int i=0;i<(int)m_vPens.size();i++){
		DeleteObject(m_vPens[i]);
	}
	m_vPens.clear();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �u���V                             //
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
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::DrawLine(int x1, int y1, int x2, int y2)
{
	::MoveToEx(m_hdc,x1,y1,NULL);
	::LineTo(m_hdc,x2,y2);
}

