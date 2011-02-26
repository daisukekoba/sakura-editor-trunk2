BUGS:

Original Bugs found during translation
  - Replace / Search up, if there are not more terms then the "cant find %s" message is corrupted, seems the "m_szCurSrchKey" variable isnt being initialized.
	1.6.5.0 also does this, intitial data is not set so it starts out undefined.
	
   - Problems with %s STR_ERR_SRNEXT3				"Forward searching(��) for '%s' found no further instances."
		+ Turns out that the FIRST time you search the "previous search term" is undefined so you get this - could crash the editor given
		the right condition
		


First Choose the language you wanna work in WinMain,cpp

	// LMP: Added
	// 1036 = french, 1031 = german 
    SetThreadLocale(MAKELCID(LANG_JAPANESE, SORT_DEFAULT)); 
    SetThreadLocale(MAKELCID(LANG_ENGLISH, SORT_DEFAULT)); 

Win7 has a different UI, the SetThreadLocale actually just returns true and does nothing. You will need the below.  I could switch it but should be ok?

	SetThreadUILanguage( MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT) );
	SetThreadUILanguage( MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT) );



-=-=-=-=-=-=-=-=-=-=-=-=-=-=

All started in CDlgFind.cpp

	// LMP: Added
	char _pszLabel[257];
	::LoadString( m_hInstance, STR_ERR_DLGFIND1, _pszLabel, 255 );  // LMP: Added

or ..
	::LoadString( GetModuleHandle(NULL), STR_ERR_DLGPPA1, _pszLabel, 255 );  // LMP: Added



CPPA.cpp - PPA, needs testing as I am using a NULL to lookup the resource table, correct?
				::LoadString( GetModuleHandle(NULL)...
				
CProcessFactory.cpp
	" ditto "
	
CPrintPreview.cpp
	Also trying... ::LoadString( m_pParentWnd->m_hInstance, may NOT be correct...

COpeBlk.cpp, CKeyBind.cpp, CKeyMacroMgr.cpp, CMacro.cpp, CMemory.cpp, CPropComFileName.cpp
	as CPPA.cpp
	


===================================
===   Editing the dialogs...    ===
===================================

	IDD_PROP_CUSTMENU DIALOGEX 0, 0, _WIDTH_, _HEIGHT_
	STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
	EXSTYLE WS_EX_CONTEXTHELP
	FONT 9, "�l�r �o�S�V�b�N", 0, 0, 0x1
	BEGIN
	    LTEXT           "���(&K)",IDC_LABEL_MENUFUNCKIND,27,4,23,8
	    COMBOBOX        IDC_COMBO_FUNCKIND,51,2,70,170,CBS_DROPDOWNLIST | 
                    CBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP,0,
                    HIDC_COMBO_FUNCKIND
	...
	...
	...
	    PUSHBUTTON      "��(&O)",IDC_BUTTON_DOWN,_X-POS_,_Y-POS_,_WIDTH_,_HEIGHT_,0,0,
	                    HIDC_BUTTON_DOWN
	END


=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Changing the language dynamically

Create Option in 
CreatePopUpMenu_L( ) { ... add new menu item for language ... }
CreatePopUpMenu_R( )


OnEvent:
	/* LMP: Language Changer LMP FIXME
	*/
    SetThreadLocale(MAKELCID(LANG_JAPANESE, SORT_DEFAULT)); 
//    SetThreadLocale(MAKELCID(LANG_ENGLISH, SORT_DEFAULT)); 
	
	/* LMP: FIXME LMP FIXME END */

DOESNT AFFECT Editor... ( so need to set in editor start up... )

WinMain( )
{
  if language A ...
  else ...
}

Actually when you change between TABBED and NON-TABBED viewing all the windows close down
and restart - good chance for me to throw in a language switch and piggy back off the same
thing.

Top window doesnt get hit up unfortunately, this is a problem.  May need to recode stuff?
TWNT_MODE_ENABLE/DISABLE

 
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-




PLACES THAT NEED FIXING, OR ARE A LITTLE NASTY CODING STYLES / SHOULD BE REVISITED LATER.



TODO

========================================
CEditDoc.cpp

// Supposed to be here for document format checking

Ln 2480		if( NULL != strstr( "�Z���O�l�ܘZ������\�S����Q��", szCheck ) ){
Ln 2481		strcpy( szTitle, "��" );


========================================
CEol.cpp
Ln78 - gm_pszEolNameArr   "���s��"

Probably going to need initializing in the constructor.  Reasonable code change, leaving it for now.
Used only in GetName() in the CEol.h

CFuncLookup.cpp :: Fixed but a ghastly hack to replace the inners during initialization.

const char *DynCategory[] = {
	"�O���}�N��",
	"�J�X�^�����j���["
};



Funccode.cpp

Used when listing all the different window types, or commands, etc.

Ln 33

Probably going to need initializing in the constructor.  Reasonable code change, leaving it for now.
Used only in GetName() in the CEol.h

const char* nsFuncCode::ppszFuncKind[] = {
//	"--����`--",	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX
//	Oct. 16, 2000 JEPRO �\���̏��Ԃ����j���[�o�[�̂���ɍ��킹��悤�ɏ�������ւ���(���̌ʂ̂��̂��S��)
	"�t�@�C������n",
	"�ҏW�n",
CLookupFunc.cpp <-- encoded a lookup function for this.  I do a resource lookup each time its called.
Not efficient but impacts the rest of the code least.




========================================
CMemory.cpp 

Has some setlocale( ..., "japanese" ), this MAY cause problems in Win7? (SetUI).  Either way it has no effect on the GUI code.


========================================
CPropComKeybind.cpp
Ln32

// These should be like this for the benefit of file I/O
#define STR_KEYDATA_HEAD      "�e�L�X�g�G�f�B�^ �L�[�ݒ�t�@�C��\x1a"

const char STR_KEYDATA_HEAD2[] = "// �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2";	//@@@ 2001.11.07 add MIK




========================================
CPropComFormat.cpp

Appears to not be used anywhere?

Ln 60
static const char *p_date_form[] = {
	"yyyy'�N'M'��'d'��'",
	"yyyy'�N'M'��'d'��('dddd')'",
	"yyyy'�N'MM'��'dd'��'",
	"yyyy'�N'M'��'d'��' dddd",
	"yyyy'�N'MM'��'dd'��' dddd",
	"yyyy/MM/dd",
	"yy/MM/dd",
	"yy/M/d",
	"yyyy/M/d",
	"yy/MM/dd' ('ddd')'",
	"yy/M/d' ('ddd')'",
	"yyyy/MM/dd' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	NULL
};


========================================


global.cpp
const char* const	gm_pszCodeComboNameArr[] = {
	"�����I��",


Used in OpenDialog and Grep Dialog, for now made a global that is setup in the 
WinMain.  Would be better to have a global access function but this would impact
other people who freely use this global.


