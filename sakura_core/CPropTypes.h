/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/08  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, MIK, aroka, genta
	Copyright (C) 2006, fon

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#ifndef SAKURA_TYPES_CPROPTYPES_H_
#define SAKURA_TYPES_CPROPTYPES_H_

class CPropTypes;


#include "CShareData.h"
//2007.11.29 kobake �ϐ��̈Ӗ��𖾊m�ɂ��邽�߁AnMethos �� �e���v���[�g���B
template <class TYPE>
struct TYPE_NAME {
	TYPE		nMethod;
	const TCHAR*	pszName;
};

//!< �v���p�e�B�V�[�g�ԍ�
enum PropTypeSheetOrder {
	ID_PROPTYPE_PAGENUM_SCREEN = 0,		//!< �X�N���[��
	ID_PROPTYPE_PAGENUM_COLOR,			//!< �J���[
	ID_PROPTYPE_PAGENUM_SUPPORT,		//!< �x��
	ID_PROPTYPE_PAGENUM_REGEX,			//!< ���K�\���L�[���[�h
	ID_PROPTYPE_PAGENUM_KEYHELP,		//!< �X�e�[�^�X�o�[
	ID_PROPTYPE_PAGENUM_MAX,
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class CPropTypes{

public:
	//�����Ɣj��
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	//!< ������
	INT_PTR DoPropertySheet( int );		//!< �v���p�e�B�V�[�g�̍쐬

	//�C���^�[�t�F�[�X	
	void SetTypeData( const STypeConfig& t ){ m_Types = t; }	//!< �^�C�v�ʐݒ�f�[�^�̐ݒ�  Jan. 23, 2005 genta
	void GetTypeData( STypeConfig& t ) const { t = m_Types; }	//!< �^�C�v�ʐݒ�f�[�^�̎擾  Jan. 23, 2005 genta
	int GetPageNum(){ return m_nPageNum; }

protected:
	//�C�x���g
	void OnHelp( HWND , int );	//!< �w���v

protected:
	//�e��Q��
	HINSTANCE	m_hInstance;	//!< �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND		m_hwndParent;	//!< �I�[�i�[�E�B���h�E�̃n���h��
	HWND		m_hwndThis;		//!< ���̃_�C�A���O�̃n���h��

	//�_�C�A���O�f�[�^
	PropTypeSheetOrder	m_nPageNum;
	DLLSHAREDATA*		m_pShareData;
	STypeConfig			m_Types;

	// �J���[�p�f�[�^
	DWORD			m_dwCustColors[16];						//!< �t�H���gDialog�J�X�^���p���b�g
	int				m_nSet[ MAX_KEYWORDSET_PER_TYPE ];		//!< keyword set index  2005.01.13 MIK
	int				m_nCurrentColorType;					//!< ���ݑI������Ă���F�^�C�v
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;						//!< �������팸�̂��߃|�C���^��  Mar. 31, 2003 genta

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �e�v���p�e�B�y�[�W                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
	bool Import( HWND );											//!< �C���|�[�g
	bool Export( HWND );											//!< �G�N�X�|�[�g
};


/*!
	@brief �^�C�v�ʐݒ�v���p�e�B�y�[�W�N���X

	�v���p�e�B�y�[�W���ɒ�`
	�ϐ��̒�`��CPropTypes�ōs��
*/
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �X�N���[��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesScreen : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesColor : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
	bool Import( HWND );											//!< �C���|�[�g
	bool Export( HWND );											//!< �G�N�X�|�[�g

protected:
	void DrawColorListItem( DRAWITEMSTRUCT* );				//!< �F��ʃ��X�g �I�[�i�[�`��
	void EnableTypesPropInput( HWND hwndDlg );				//!< �^�C�v�ʐݒ�̃J���[�ݒ��ON/OFF
	void RearrangeKeywordSet( HWND );						//!< �L�[���[�h�Z�b�g�Ĕz�u  Jan. 23, 2005 genta
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );		//!< �F�{�^���̕`��
public:
	static BOOL SelectColor( HWND , COLORREF*, DWORD* );	//!< �F�I���_�C�A���O
private:
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �x��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesSupport : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesRegex : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	void SetDataKeywordList( HWND );								//!< �_�C�A���O�f�[�^�̐ݒ胊�X�g����
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
	bool Import( HWND );											//!< �C���|�[�g
	bool Export( HWND );											//!< �G�N�X�|�[�g
private:
	BOOL RegexKakomiCheck(const char *s);	//@@@ 2001.11.17 add MIK

};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesKeyHelp : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< ���b�Z�[�W����
protected:
	void SetData( HWND );											//!< �_�C�A���O�f�[�^�̐ݒ�
	int  GetData( HWND );											//!< �_�C�A���O�f�[�^�̎擾
	bool Import( HWND );											//!< �C���|�[�g
	bool Export( HWND );											//!< �G�N�X�|�[�g
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_TYPES_CPROPTYPES_H_ */


/*[EOF]*/
