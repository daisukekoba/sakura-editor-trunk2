/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cpp����؂�o��
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CWSH.h"
#include "CWSHManager.h"
#include "CMacroFactory.h"
#include "CMacro.h"
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "os.h"
#include "OleTypes.h"

static void MacroError(BSTR Description, BSTR Source, void *Data)
{
	CEditView *View = reinterpret_cast<CEditView*>(Data);

	char MessageA[1024], SourceA[1024];
	
	MessageA[WideCharToMultiByte(CP_ACP, 0, Description, SysStringLen(Description), MessageA, 1023, NULL, NULL)] = 0;
	SourceA[WideCharToMultiByte(CP_ACP, 0, Source, SysStringLen(Source), SourceA, 1023, NULL, NULL)] = 0;
	
	MessageBox(View->m_hWnd, MessageA, SourceA, MB_ICONERROR);
}

CWSHMacroManager::CWSHMacroManager(std::wstring const AEngineName) : m_EngineName(AEngineName)
{
}

CWSHMacroManager::~CWSHMacroManager()
{
}

/** WSH�}�N���̎��s

	@param EditView [in] ����Ώ�EditView
	
	@date 2007.07.20 genta : flags�ǉ�
*/
void CWSHMacroManager::ExecKeyMacro(CEditView *EditView, int flags) const
{
	CWSHClient* Engine;
	Engine = new CWSHClient(m_EngineName.c_str(), MacroError, EditView);
	if(Engine->m_Valid)
	{
/* // CSMacroMgr.h�Ŕz��̃T�C�Y�����m�ɐ錾����Ė����̂�sizeof���g���Ȃ�
		Engine->m_InterfaceObject->ReserveMethods(
						sizeof (CSMacroMgr::m_MacroFuncInfoCommandArr) / sizeof (CSMacroMgr::m_MacroFuncInfoCommandArr[0]) +
						sizeof (CSMacroMgr::m_MacroFuncInfoArr) / sizeof (CSMacroMgr::m_MacroFuncInfoArr[0]));
*/
		//	 2007.07.20 genta : �R�}���h�ɍ������ރt���O��n��
		ReadyCommands(Engine->m_InterfaceObject, CSMacroMgr::m_MacroFuncInfoCommandArr, flags | FA_FROMMACRO );
		ReadyCommands(Engine->m_InterfaceObject, CSMacroMgr::m_MacroFuncInfoArr, 0);
		
		Engine->Execute(m_Source.c_str());
		
		//EditView->Redraw();
		EditView->ShowEditCaret();
	}
	delete Engine;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�t�@�C������j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszPath   [in] �t�@�C���̃p�X
*/
BOOL CWSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	BOOL Result = FALSE;
	
	HANDLE File = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(File != INVALID_HANDLE_VALUE)
	{
		unsigned long Size = GetFileSize(File, NULL); //�M�K�P�ʂ̃}�N���͂������ɖ����ł��傤�c
		char *Buffer = new char[Size];
		wchar_t *WideBuffer = new wchar_t[Size + 1]; //Unicode�����Ē����͂Ȃ�Ȃ�

		if(ReadFile(File, Buffer, Size, &Size, NULL) != 0)
		{
			WideBuffer[MultiByteToWideChar(CP_ACP, 0, Buffer, Size, WideBuffer, Size)] = 0;
			m_Source = WideBuffer;
			Result = TRUE;
		}
		//	Nov. 10, 2003 FILE �z��̔j���Ȃ̂ŁA[����]��ǉ�
		delete [] Buffer;
		delete [] WideBuffer;
		CloseHandle(File);
	}
	return Result;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�����񂩂�j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszCode   [in] �}�N���R�[�h
*/
BOOL CWSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	unsigned long Size = strlen(pszCode); //�M�K�P�ʂ̃}�N���͂������ɖ����ł��傤�c
	wchar_t *WideBuffer = new wchar_t[Size + 1]; //Unicode�����Ē����͂Ȃ�Ȃ�

	WideBuffer[MultiByteToWideChar(CP_ACP, 0, pszCode, Size, WideBuffer, Size)] = 0;
	m_Source = WideBuffer;

	delete [] WideBuffer;

	return TRUE;
}

CMacroManagerBase* CWSHMacroManager::Creator(const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; //1024�𒴂������͒m��܂���
	
	_tcscpy( FileExtWithDot, _T(".") );
	_tcscat( FileExtWithDot, FileExt );

	if(ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024))
	{
		lstrcat(FileType, _T("\\ScriptEngine"));
		if(ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024))
		{
			wchar_t EngineNameW[1024];
			MultiByteToWideChar(CP_ACP, 0, EngineName, -1, EngineNameW, _countof(EngineNameW));
			return new CWSHMacroManager(EngineNameW);
		}
	}
	return NULL;
}

void CWSHMacroManager::declare()
{
	//�b��
	CMacroFactory::getInstance()->RegisterCreator(Creator);
}

/////////////////////////////////////////////
/*!
	�}�N���R�}���h�̎��s

	@date 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��G���[�ɂ����Ɋ֐������s����
*/
static HRESULT MacroCommand(int ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	int I;
	int ArgCount = Arguments->cArgs;
	if(ArgCount > 4) ArgCount = 4;

	CEditView *View = reinterpret_cast<CEditView*>(Data);
	//	2007.07.22 genta : �R�}���h�͉���16�r�b�g�̂�
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��֐������s����
		VariantInit(&ret);
		
		// 2011.3.18 syat �����̏����𐳂������ɂ���
		VARIANTARG rgvargBak[4];
		memcpy( rgvargBak, Arguments->rgvarg, sizeof(VARIANTARG) * ArgCount );
		for(I = 0; I < ArgCount; I++){
			Arguments->rgvarg[ArgCount-I-1] = rgvargBak[I];
		}

		bool r = CMacro::HandleFunction(View, ID, Arguments->rgvarg, Arguments->cArgs, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		return r ? S_OK : E_FAIL;
	}
	else
	{
		//	Nov. 29, 2005 FILE �����𕶎���Ŏ擾����
		char *StrArgs[4] = {NULL, NULL, NULL, NULL};	// �������K�{
		char *S = NULL;									// �������K�{
		Variant varCopy;										// VT_BYREF���ƍ���̂ŃR�s�[�p
		int Len;
		for(I = 0; I < ArgCount; ++I)
		{
			if(VariantChangeType(&varCopy.Data, &(Arguments->rgvarg[I]), 0, VT_BSTR) == S_OK)
			{
				Wrap(&varCopy.Data.bstrVal)->Get(&S, &Len);
			}
			else
			{
				S = new char[1];
				S[0] = 0;
			}
			StrArgs[ArgCount - I - 1] = S;
		}

		CMacro::HandleCommand(View, ID, const_cast<char const **>(StrArgs), ArgCount);

		//	Nov. 29, 2005 FILE �z��̔j���Ȃ̂ŁA[����]��ǉ�
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}

/** WSH�}�N���G���W���փR�}���h�o�^���s��

	@date 2007.07.20 genta flags�ǉ��Dflag�̓R�}���h�o�^�i�K�ō����Ă����D
*/
void CWSHMacroManager::ReadyCommands(CInterfaceObject *Object, MacroFuncInfo *Info, int flags)
{
	while(Info->m_nFuncID != -1)	// Aug. 29, 2002 genta �Ԑl�̒l���ύX���ꂽ�̂ł������ύX
	{
		wchar_t FuncName[256];
		MultiByteToWideChar(CP_ACP, 0, Info->m_pszFuncName, -1, FuncName, 255);

		int ArgCount = 0;
		for(int I = 0; I < 4; ++I)
			if(Info->m_varArguments[I] != VT_EMPTY) 
				++ArgCount;
		
		//	2007.07.21 genta : flag���������l��o�^����
		Object->AddMethod(FuncName, Info->m_nFuncID | flags, Info->m_varArguments, ArgCount, Info->m_varResult, MacroCommand);
		
		++Info;
	}
}

