/*!	@file
	@brief �f�o�b�O�p�֐�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2007, kobake

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <windows.h>
#include "global.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
SAKURA_CORE_API void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError );
SAKURA_CORE_API void DebugOutA( LPCSTR lpFmt, ...);
#ifdef _UNICODE
#define DebugOut DebugOutW
#else
#define DebugOut DebugOutA
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake ���b�Z�[�W�{�b�N�X�̎g�p�̓f�o�b�O���Ɍ���Ȃ��̂ŁA�uDebug�`�v�Ƃ������O��p�~

//�e�L�X�g���`�@�\�t��MessageBox
SAKURA_CORE_API int MessageBoxF( HWND, UINT, LPCTSTR, LPCTSTR, ... );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 �f�o�b�O�p���b�Z�[�W�o��                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACE�̓����[�X���[�h�ł̓R���p�C���G���[�ƂȂ�悤�ɂ��Ă���̂ŁC
	MYTRACE���g���ꍇ�ɂ͕K��#ifdef _DEBUG �` #endif �ň͂ޕK�v������D
*/
#ifdef _DEBUG
	#define MYTRACE DebugOut
	#define MYTRACE_A DebugOutA
#endif
#ifndef _DEBUG
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG�`#endif�ň͂܂Ȃ��Ă��ǂ���
#ifdef _DEBUG
#define DBPRINT DebugOut
#else
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT(...)
#else
// Not support C99 variable macro
inline void DBPRINT( ... ){}
#endif
#endif // _DEBUG

#define DEBUG_TRACE DBPRINT

#define MYASSERT AssertError


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                ���[�U�p���b�Z�[�W�{�b�N�X                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//$$�����FDebug.h�ȊO�̒u���ꏊ���l����

//�f�o�b�O�p���b�Z�[�W�{�b�N�X
#define MYMESSAGEBOX MessageBoxF


//�G���[�F�ԊۂɁu�~�v[OK]
#define ErrorBeep()     MessageBeep(MB_ICONSTOP)

//�x���F�O�p�Ɂu�I�v[OK]
#define WarningBeep()   MessageBeep(MB_ICONEXCLAMATION)

//���F�ۂɁui�v[OK]
#define InfoBeep()      MessageBeep(MB_ICONINFORMATION)

//�m�F�F�����o���́u�H�v [�͂�][������] �߂�l:IDYES,IDNO
#define ConfirmBeep()   MessageBeep(MB_ICONQUESTION)




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t���O��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#ifdef _DEBUG
	//!	�ݒ肵�Ă���ꏊ�͂��邪�C�Q�Ƃ��Ă���ꏊ���Ȃ��ϐ�
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */


/*[EOF]*/
