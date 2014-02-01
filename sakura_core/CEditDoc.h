/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, MIK, hor
	Copyright (C) 2002, genta, Moca, YAZAKI
	Copyright (C) 2003, genta, ryoji, zenryaku, naoh
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, Moca, genta, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji, maru, genta, Moca
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSave.h"
#include "CDocLineMgr.h"
#include "CEditView.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CPropertyManager.h"
#include "CShareData.h"
#include "CSplitterWnd.h"
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct EditInfo; // 20050705 aroka
class CFuncInfoArr;
struct SOneRule;

/*!
	�����֘A���̊Ǘ�

	@date 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class CEditDoc
{
public:
	/*
	||  Constructors
	*/
	CEditDoc();
	~CEditDoc();

	/*
	||  �������n�����o�֐�
	*/
	BOOL Create( HINSTANCE, CEditWnd* pcEditWnd, CImageListMgr* );
	void InitDoc();	/* �����f�[�^�̃N���A */
	void InitAllView();	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	void Clear();

	/*
	|| ���
	*/
	BOOL IsTextSelected( void ) const;		/* �e�L�X�g���I������Ă��邩 */
	bool IsEnableUndo( void ) const;		/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
	bool IsEnableRedo( void ) const;		/* Redo(��蒼��)�\�ȏ�Ԃ��H */
	BOOL IsEnablePaste( void ) const;		/* �N���b�v�{�[�h����\��t���\���H */
	void GetEditInfo( EditInfo* );	//!< �ҏW�t�@�C�������擾 //2007.10.24 kobake �֐����ύX: SetFileInfo��GetEditInfo

	BOOL OnFileClose();			/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s */
	BOOL HandleCommand( int );
	BOOL FileRead( /*const*/ TCHAR* , BOOL*, ECodeType, bool, bool );	/* �t�@�C�����J�� */
	//	Feb. 9, 2001 genta �����ǉ�
	BOOL FileWrite( const char*, EEolType cEolType );
	bool SaveFile( const char* path );	//	�t�@�C���̕ۑ��i�ɔ������낢��j
	void OpenFile( const char *filename = NULL, ECodeType nCharCode = CODE_AUTODETECT,
		bool bReadOnly = false );	//	Oct. 9, 2004 genta CEditView���ړ�
	void FileClose( void );	/* ����(����) */	// 2006.12.30 ryoji
	void FileCloseOpen( const char *filename = NULL, ECodeType nCharCode = CODE_AUTODETECT, bool bReadOnly = false );	/* ���ĊJ�� */	// 2006.12.30 ryoji
	BOOL FileSave( bool warnbeep = true, bool askname = true );	/* �㏑���ۑ� */	// 2006.12.30 ryoji
	BOOL FileSaveAs_Dialog( void );				/* ���O��t���ĕۑ��_�C�A���O */	// 2006.12.30 ryoji
	BOOL FileSaveAs( const char *filename );	/* ���O��t���ĕۑ� */	// 2006.12.30 ryoji

	int MakeBackUp( const TCHAR* target_file );	/* �o�b�N�A�b�v�̍쐬 */

	std::tstring GetDlgInitialDir();
	bool OpenFileDialog( HWND, const char*, char*, ECodeType*, bool* );	/* �u�t�@�C�����J���v�_�C�A���O */
	void OnChangeType();
	void OnChangeSetting(bool bDoRayout = true);		// �r���[�ɐݒ�ύX�𔽉f������
	//	Jul. 26, 2003 ryoji BOM�I�v�V�����ǉ�
	BOOL SaveFileDialog( char*, ECodeType*, CEol* pcEol = NULL, bool* pbBomExist = NULL );	/* �u�t�@�C������t���ĕۑ��v�_�C�A���O */

	void CheckFileTimeStamp( void );	/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
	void ReloadCurrentFile( ECodeType, bool );/* ����t�@�C���̍ăI�[�v�� */

	//	May 15, 2000 genta
	CEol  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEol& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand ) const;
	bool IsEditable() const { return ( !m_bReadOnly && !( SHAREMODE_NOT_EXCLUSIVE != m_nFileShareModeOld && INVALID_HANDLE_VALUE == m_hLockedFile ) ); }	//!< �ҏW�\���ǂ���

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	�����ۑ��Ǘ�
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	�ݒ��SharedArea����ǂݏo��

	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME��Ԃ̐ݒ�

	//	Sep. 9, 2002 genta
	const TCHAR* GetFilePath(void) const { return m_szFilePath; }
	const TCHAR* GetSaveFilePath(void) const { return (m_szSaveFilePath[0] != _T('\0')) ? m_szSaveFilePath : m_szFilePath; }
	bool IsValidPath(void) const { return m_szFilePath[0] != _T('\0'); }
	//	2002.10.13 Moca
	const char* GetFileName(void) const; //!< �t�@�C����(�p�X�Ȃ�)���擾
	void SetDocumentIcon(void);	// Sep. 10, 2002 genta

	//	Nov. 29, 2000 From Here	genta
	//	�ݒ�̈ꎞ�ύX���Ɋg���q�ɂ�鋭���I�Ȑݒ�ύX�𖳌��ɂ���
	void LockDocumentType(void){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(void){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(void){ return m_nSettingTypeLocked; }
	//	Nov. 29, 2000 To Here
	//	Nov. 23, 2000 From Here	genta
	//	������ʏ��̐ݒ�C�擾Interface
	void SetDocumentType(int type, bool force, bool bTypeOnly = false)	//!< ������ʂ̐ݒ�
	{
		if( (!m_nSettingTypeLocked) || force ){
			m_nSettingType = type;
			m_pTypes = &m_pShareData->m_Types[m_nSettingType];
			if( bTypeOnly ) return;	// bTypeOnly == true �͓���P�[�X�i�ꎞ���p�j�Ɍ���
			UnlockDocumentType();
			m_pShareData->m_Types[m_nSettingType].m_nRegexKeyMagicNumber++;	//@@@ 2001.11.17 add MIK
			SetDocumentIcon();	// Sep. 11, 2002 genta
		}
	}
	int GetDocumentType(void) const	//!<	������ʂ̓ǂݏo��
	{
		return m_nSettingType;
	}
	STypeConfig& GetDocumentAttribute(void) const	//!<	�ݒ肳�ꂽ�������ւ̎Q�Ƃ�Ԃ�
	{
		return *m_pTypes;
	}
	//	Nov. 23, 2000 To Here

	//	May 18, 2001 genta
	//! ReadOnly��Ԃ̓ǂݏo��
	bool IsReadOnly( void ){ return m_bReadOnly; }
	//! ReadOnly��Ԃ̐ݒ�
	void SetReadOnly( bool flag){ m_bReadOnly = flag; }
	
	//	Jan. 22, 2002 genta Modified Flag�̐ݒ�
	void SetModified( bool flag, bool redraw);
	/** �t�@�C�����C�������ǂ���
		@retval true �t�@�C���͏C������Ă���
		@retval false �t�@�C���͏C������Ă��Ȃ�
	*/
	bool IsModified( void ) const { return m_bIsModified; }

	/*! @brief ���̃E�B���h�E�ŐV�����t�@�C�����J���邩

		�V�����E�B���h�E���J�����Ɍ��݂̃E�B���h�E���ė��p�ł��邩�ǂ����̃e�X�g���s���D
		�ύX�ς݁C�t�@�C�����J���Ă���CGrep�E�B���h�E�C�A�E�g�v�b�g�E�B���h�E�̏ꍇ�ɂ�
		�ė��p�s�D

		@author Moca
		@date 2005.06.24 Moca
	*/
	bool IsFileOpenInThisWindow( void ){
		return !( IsModified() || IsValidPath() || m_bGrepMode || m_bDebugMode );
	}

	void AddToMRU(void); // Mar. 30, 2003 genta
	// Mar. 31, 2003 genta ExpandParameter�⏕�֐�
	const char* ExParam_SkipCond(const char* pszSource, int part);
	int ExParam_Evaluate( const char* pCond );

	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	void RunAutoMacro( int idx, LPCTSTR pszSaveFilePath = NULL );	// 2006.09.01 ryoji �}�N���������s

protected:
	int				m_nSettingType;
	STypeConfig*	m_pTypes;
	bool			m_nSettingTypeLocked;	//	������ʂ̈ꎞ�ݒ���
	//	Jan. 22, 2002 genta public -> protected�Ɉړ�
	/*! �ύX�t���O
		@date 2002.01.22 genta public��protected�Ɉړ��D�^��int��bool�ɁD
	*/
	bool			m_bIsModified;
	//	Sep. 9, 2002 genta protected�Ɉړ�
	TCHAR			m_szFilePath[_MAX_PATH];	/* ���ݕҏW���̃t�@�C���̃p�X */
	TCHAR			m_szSaveFilePath[_MAX_PATH];	/* �ۑ����̃t�@�C���̃p�X�i�}�N���p�j */	// 2006.09.04 ryoji

	void SetFilePathAndIcon(const TCHAR*);	// Sep. 9, 2002 genta
	
public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	BOOL			m_bGrepRunning;				/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	BOOL			m_bPrintPreviewMode;		/* ����v���r���[���[�h�� */
	int				m_nCommandExecNum;			/* �R�}���h���s�� */
	CFileTime		m_FileTime;					/* �t�@�C���̍ŏI�X�V���t */
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;
	ECodeType		m_nCharCode;				/* �����R�[�h��� */
	bool			m_bBomExist;	//!< �ۑ�����BOM��t���邩�ǂ��� Jul. 26, 2003 ryoji 
	bool			m_bInsMode;		//!< �}���E�㏑�����[�h Oct. 2, 2005 genta

	//	May 15, 2000 genta
protected:
	CEol 			m_cNewLineCode;				//	Enter�������ɑ}��������s�R�[�h���
public:
	CEol			m_cSaveLineCode;			//	�ۑ����̉��s�R�[�h��ʁiEOL_NONE:�ϊ��Ȃ��j


	bool			m_bReadOnly;				/* �ǂݎ���p���[�h */
	bool			m_bDebugMode;				/* �f�o�b�O���j�^���[�h */
	bool			m_bGrepMode;				/* Grep���[�h�� */
	TCHAR			m_szGrepKey[1024];			/* Grep���[�h�̏ꍇ�A���̌����L�[ */
	COpeBuf			m_cOpeBuf;					/* �A���h�D�o�b�t�@ */
public:
	void			MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );		/* C/C++�֐����X�g�쐬 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL�֐����X�g�쐬 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* �e�L�X�g�E�g�s�b�N���X�g�쐬 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java�֐����X�g�쐬 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL �A�E�g���C����� */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* �A�Z���u�� �A�E�g���C����� */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl�֐����X�g�쐬 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic�֐����X�g�쐬 */ //June 23, 2001 N.Nakatani
	void			MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr ); /* Python �A�E�g���C�����*/ // 2007.02.08 genta
	void			MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr ); /* Erlang �A�E�g���C�����*/ // 2009.08.11 genta
	void			MakeTopicList_wztxt(CFuncInfoArr*);		/* �K�w�t���e�L�X�g �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_html(CFuncInfoArr*);		/* HTML �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_tex(CFuncInfoArr*);		/* TeX �A�E�g���C����� */ // 2003.07.20 naoh
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI */
	int				ReadRuleFile( const char* pszFilename, SOneRule* pcOneRule, int nMaxCount );	/* ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* �u�b�N�}�[�N���X�g�쐬 */
// To Here 2001.12.03 hor

	CEditWnd*		m_pcEditWnd;	//	Sep. 10, 2002
	/*
	||  �����o�ϐ�
	*/
	HINSTANCE		m_hInstance;		/* �C���X�^���X�n���h�� */

	DLLSHAREDATA*	m_pShareData;

	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */

	CPropertyManager* m_pcPropertyManager;	//�v���p�e�B�Ǘ�

	EShareMode		m_nFileShareModeOld;	/* �t�@�C���̔r�����䃂�[�h */
	HANDLE			m_hLockedFile;			/* ���b�N���Ă���t�@�C���̃n���h�� */

	//	Sep. 29, 2001 genta
	CSMacroMgr*		m_pcSMacroMgr;	//!< �}�N��
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���B
//	CKeyMacroMgr	m_CKeyMacroMgr;	//	�L�[�{�[�h�}�N�� @@@ 2002.1.24 YAZAKI DLLSHAREDATA����ړ�
	//	Oct. 2, 2001 genta
	CFuncLookup		m_cFuncLookup;	//!< �@�\���C�@�\�ԍ��Ȃǂ�resolve

	//	From Here Dec. 4, 2002 genta
	//	�X�V�Ď��ƍX�V�ʒm
	/*!
		�t�@�C�����X�V���ꂽ�ꍇ�ɍēǍ����s�����ǂ����̃t���O
	*/
	enum WatchUpdate {
		WU_QUERY,	//!< �ēǍ����s�����ǂ����_�C�A���O�{�b�N�X�Ŗ₢���킹��
		WU_NOTIFY,	//!< �X�V���ꂽ���Ƃ��X�e�[�^�X�o�[�Œʒm
		WU_NONE		//!< �X�V�Ď����s��Ȃ�
	};
	WatchUpdate		m_eWatchUpdate;	//!< �X�V�Ď����@
	//	To Here Dec. 4, 2002 genta

//	CDlgTest*		m_pcDlgTest;

	int m_nTextWrapMethodCur;		// �܂�Ԃ����@					// 2008.05.30 nasukoji
	bool m_bTextWrapMethodCurTemp;	// �܂�Ԃ����@�ꎞ�ݒ�K�p��	// 2008.05.30 nasukoji

	/*
	||  �����w���p�֐�
	*/
	void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);	//	pszSource��W�J���āApszBuffer�ɃR�s�[�B
protected:
	void DoFileLock( void );	/* �t�@�C���̔r�����b�N */
	void DoFileUnlock( void );	/* �t�@�C���̔r�����b�N���� */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
	
	// 2005.11.21 aroka
	// 2008.11.23 nasukoji	�p�X����������ꍇ�ւ̑Ή�
	bool FormatBackUpPath( char* szNewPath, size_t dwSize, const char* target_file );	/* �o�b�N�A�b�v�p�X�̍쐬 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
