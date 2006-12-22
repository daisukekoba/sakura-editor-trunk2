/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス

	@author genta
	@date 2002.12.04
*/
/*
	Copyright (C) 2002, genta

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
#ifndef _DLG_FILE_UPDATE_QUERY_H_
#define _DLG_FILE_UPDATE_QUERY_H_

#include <windows.h>
#include "global.h"
#include "CDialog.h"

class SAKURA_CORE_API CDlgFileUpdateQuery : public CDialog {
private:
	const char* m_pFilename;
	bool m_bModified;

public:
	CDlgFileUpdateQuery(const char* filename, bool IsModified)
		: m_pFilename( filename ), m_bModified( IsModified ) {}
	virtual BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	virtual BOOL OnBnClicked( int );
};
#endif
/*[EOF]*/
