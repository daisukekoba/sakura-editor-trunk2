/*
2008.05.20 kobake �쐬
*/

#pragma once

#include <windows.h>

//! �I���W�i���l�ۑ��N���X
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


//! �`��Ǘ�
class CGraphics{
public:
	CGraphics(const CGraphics& rhs){ Init(rhs.m_hdc); }
	CGraphics(HDC hdc = NULL){ Init(hdc); }
	~CGraphics();
	void Init(HDC hdc);

	operator HDC() const{ return m_hdc; }

	//�F�ݒ�
	void SetForegroundColor(COLORREF color);	//!< �`��F��ݒ�
	void SetBackgroundColor(COLORREF color);	//!< �w�i�F��ݒ�

	//�e�L�X�g
	void SetTextForeColor(COLORREF color);
	void SetTextBackColor(COLORREF color);
	void SetTextBackTransparent(bool b);
	void RestoreTextColors();

	//�t�H���g
	void SetMyFont(HFONT hFont);				//!< �t�H���g�ݒ�
	void RestoreFont();

	//�y��
	void SetPenColor(COLORREF color);
	void RestorePen();

	//�u���V
	void SetBrushColor(COLORREF color);
	void SetNullBrush();
	void RestoreBrush();
	HBRUSH GetCurrentBrush() const{ return m_hbrCurrent; }

	//����
	void DrawLine(int x1, int y1, int x2, int y2);
private:
	//�^
	typedef TOriginalHolder<COLORREF>	COrgColor;
	typedef TOriginalHolder<int>		COrgInt;
private:
	HDC			m_hdc;

	//�e�L�X�g
	COrgColor	m_clrTextForeOrg;
	COrgColor	m_clrTextBackOrg;
	COrgInt		m_nTextModeOrg;
	HFONT		m_hfntOld;

	//�y��
	HPEN		m_hpnOrg;
	HPEN		m_hpnCurrent;

	//�u���V
	HBRUSH		m_hbrOrg;
	HBRUSH		m_hbrCurrent;
	bool		m_bDynamicBrush;	//m_hbrCurrent�𓮓I�ɍ쐬�����ꍇ��true
};
