#pragma once

class CShareData_IO{
public:
	//�Z�[�u�E���[�h
	static bool LoadShareData();	/* ���L�f�[�^�̃��[�h */
	static void SaveShareData();	/* ���L�f�[�^�̕ۑ� */

protected:
	static bool ShareData_IO_2( bool );	/* ���L�f�[�^�̕ۑ� */

	// Feb. 12, 2006 D.S.Koba
	static void ShareData_IO_Mru( CDataProfile& );
	static void ShareData_IO_Keys( CDataProfile& );
	static void ShareData_IO_Grep( CDataProfile& );
	static void ShareData_IO_Folders( CDataProfile& );
	static void ShareData_IO_Cmd( CDataProfile& );
	static void ShareData_IO_Nickname( CDataProfile& );
	static void ShareData_IO_Common( CDataProfile& );
	static void ShareData_IO_Toolbar( CDataProfile& );
	static void ShareData_IO_CustMenu( CDataProfile& );
	static void ShareData_IO_Font( CDataProfile& );
	static void ShareData_IO_KeyBind( CDataProfile& );
	static void ShareData_IO_Print( CDataProfile& );
	static void ShareData_IO_Types( CDataProfile& );
	static void ShareData_IO_KeyWords( CDataProfile& );
	static void ShareData_IO_Macro( CDataProfile& );
	static void ShareData_IO_Other( CDataProfile& );

public:
	static void IO_ColorSet( CDataProfile* , const WCHAR* , ColorInfo* );	/* �F�ݒ� I/O */ // Feb. 12, 2006 D.S.Koba
};

