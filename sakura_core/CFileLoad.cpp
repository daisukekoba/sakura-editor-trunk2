/*!	@file
	@brief �t�@�C���ǂݍ��݃N���X

	@author Moca
	@date 2002/08/30 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji
	Copyright (C) 2006, rastiv

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
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "global.h"
#include "etc_uty.h"
#include "CMemory.h"
#include "CEol.h"
#include "CFileLoad.h"
#include "charcode.h"
#ifdef _DEBUG
#include "Debug.h"
#endif
/*
	@note Win32API�Ŏ���
		2GB�ȏ�̃t�@�C���͊J���Ȃ�
*/

/*! ���[�h�p�o�b�t�@�T�C�Y�̏����l */
const int CFileLoad::gm_nBufSizeDef = 32768;
//(�œK�l���}�V���ɂ���ĈႤ�̂łƂ肠����32KB�m�ۂ���)

// /*! ���[�h�p�o�b�t�@�T�C�Y�̐ݒ�\�ȍŒ�l */
// const int gm_nBufSizeMin = 1024;

/*! �R���X�g���N�^ */
CFileLoad::CFileLoad( void ) : m_cmemLine()
{
	m_hFile			= NULL;
	m_nFileSize		= 0;
	m_nFileDataLen	= 0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= FALSE;	// Jun. 08, 2003 Moca
	m_nFlag 		= 0;
	m_nReadLength	= 0;
	m_eMode			= FLMODE_CLOSE;	// Jun. 08, 2003 Moca

	m_nLineIndex	= -1;

	m_pReadBuf = NULL;
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}

/*! �f�X�g���N�^ */
CFileLoad::~CFileLoad( void )
{
	if( NULL != m_hFile ){
		FileClose();
	}
	if( NULL != m_pReadBuf ){
		free( m_pReadBuf );
	}
}

/*!
	�t�@�C�����J��
	@param pFileName [in] �t�@�C����
	@param CharCode  [in] �t�@�C���̕����R�[�h�D
	@param nFlag [in] �����R�[�h�̃I�v�V����
	@param pbBomExist [out] BOM�̗L��
	@date 2003.06.08 Moca CODE_AUTODETECT���w��ł���悤�ɕύX
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
ECodeType CFileLoad::FileOpen( LPCTSTR pFileName, ECodeType CharCode, int nFlag, BOOL* pbBomExist )
{
	HANDLE	hFile;
	DWORD	FileSize;
	DWORD	FileSizeHigh;
	ECodeType	nBomCode;

	// FileClose���Ă�ł���ɂ��Ă�������
	if( NULL != m_hFile ){
#ifdef _DEBUG
		::MessageBox( NULL, _T("CFileLoad::FileOpen\nFileClose���Ă�ł���ɂ��Ă�������") , NULL, MB_OK );
#endif
		throw CError_FileOpen();
	}
	hFile = ::CreateFile(
		pFileName,
		GENERIC_READ,
		//	Oct. 18, 2002 genta FILE_SHARE_WRITE �ǉ�
		//	���v���Z�X���������ݒ��̃t�@�C�����J����悤��
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// ���L
		NULL,						// �Z�L�����e�B�L�q�q
		OPEN_EXISTING,				// �쐬���@
		FILE_FLAG_SEQUENTIAL_SCAN,	// �t�@�C������
		NULL						// �e���v���[�g�t�@�C���̃n���h��
	);
	if( hFile == INVALID_HANDLE_VALUE ){
		throw CError_FileOpen();
	}
	m_hFile = hFile;

	FileSize = ::GetFileSize( hFile, &FileSizeHigh );
	// �t�@�C���T�C�Y���A��2GB�𒴂���ꍇ�͂Ƃ肠�����G���[
	if( 0x80000000 <= FileSize || 0 < FileSizeHigh ){
		FileClose();
		throw CError_FileOpen();
	}
	m_nFileSize = FileSize;
//	m_eMode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca �����R�[�h����
	// �f�[�^�ǂݍ���
	Buffering();

	if( CharCode == CODE_AUTODETECT ){
		CharCode = CMemory::CheckKanjiCode( (const unsigned char*)m_pReadBuf, m_nReadDataLen );
	}
	// To Here Jun. 08, 2003
	// �s���ȕ����R�[�h�̂Ƃ��̓f�t�H���g(SJIS:���ϊ�)��ݒ�
	if( 0 > CharCode || CODE_CODEMAX <= CharCode ){
		CharCode = CODE_DEFAULT;
	}
	m_CharCode = CharCode;
	m_nFlag = nFlag;

	// From Here Jun. 08, 2003 Moca BOM�̏���
	nBomCode = Charcode::DetectUnicodeBom( (const char *)m_pReadBuf, m_nReadDataLen );  // 2006.09.22  by rastiv
	m_nFileDataLen = m_nFileSize;
	if( nBomCode != 0 && nBomCode == m_CharCode ){
		//	Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		m_bBomExist = TRUE;
		if( pbBomExist != NULL ){
			*pbBomExist = TRUE;
		}
		switch( nBomCode ){
			case CODE_UNICODE:
			case CODE_UNICODEBE:
				m_nFileDataLen -= 2;
				m_nReadBufOffSet += 2;
				break;
			case CODE_UTF8:
				m_nFileDataLen -= 3;
				m_nReadBufOffSet += 3;
				break;
		}
	}else{
		//	Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		if( pbBomExist != NULL ){
			*pbBomExist = FALSE;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOM�̏���
	m_eMode = FLMODE_REDY;
	return m_CharCode;
}

/*!
	�t�@�C�������
	�ǂݍ��ݗp�o�b�t�@��m_memLine���N���A�����
*/
void CFileLoad::FileClose( void )
{
	ReadBufEmpty();
	m_cmemLine.SetString( "" );
	if( NULL != m_hFile ){
		::CloseHandle( m_hFile );
		m_hFile = NULL;
	}
	m_nFileSize		=  0;
	m_nFileDataLen	=  0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= FALSE; // From Here Jun. 08, 2003
	m_nFlag 		=  0;
	m_nReadLength	=  0;
	m_eMode			= FLMODE_CLOSE;
	m_nLineIndex	= -1;
}

/*!
	���̘_���s�𕶎��R�[�h�ϊ����ă��[�h����
	�����A�N�Z�X��p
	GetNextLine�̂悤�ȓ��������
	@return	NULL�ȊO	1�s��ێ����Ă���f�[�^�̐擪�A�h���X��Ԃ�
			NULL		�f�[�^���Ȃ�����
*/
const char* CFileLoad::ReadLine(
	int*		pnLineLen,		//!< [out]	���s�R�[�h�����܂ވ�s�̃f�[�^��
	CEol*		pcEol			//!< [i/o]
)
{
	const char	*pLine;
	int			nRetVal = 1;
	int			nBufLineLen;
	int			nEolLen;
#ifdef _DEBUG
	if( m_eMode < FLMODE_REDY ){
		MYTRACE_A( "CFileLoad::ReadLine(): m_eMode = %d\n", m_eMode );
		return NULL;
	}
#endif
	// �s�f�[�^�N���A�B�{���̓o�b�t�@�͊J���������Ȃ�
	m_cmemLine.SetString( "", 0 );

	// 1�s���o�� ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while�����𐮗�
	while( NULL != ( pLine = GetNextLineCharCode( m_pReadBuf, m_nReadDataLen,
		&nBufLineLen, &m_nReadBufOffSet, pcEol, &nEolLen ) ) ){
			// ReadBuf����1�s���擾����Ƃ��A���s�R�[�h��������\�������邽��
			if( m_nReadDataLen <= m_nReadBufOffSet && FLMODE_REDY == m_eMode ){// From Here Jun. 13, 2003 Moca
				m_cmemLine.AppendString( pLine, nBufLineLen );
				m_nReadBufOffSet -= nEolLen;
				// �o�b�t�@���[�h   File -> ReadBuf
				Buffering();
			}else{
				m_cmemLine.AppendString( pLine, nBufLineLen );
				break;
			}
	}

	m_nReadLength += ( nBufLineLen = m_cmemLine.GetStringLength() );

	// �����R�[�h�ϊ�
	switch( m_CharCode ){
	case CODE_SJIS:
		break;
	case CODE_EUC:
		m_cmemLine.EUCToSJIS();
		break;
	case CODE_JIS:
		// E-Mail(JIS��SJIS)�R�[�h�ϊ�
		m_cmemLine.JIStoSJIS( ( m_nFlag & 1 ) == 1 );
		break;
	case CODE_UNICODE:
		m_cmemLine.UnicodeToSJIS();
		break;
	case CODE_UTF8:
		m_cmemLine.UTF8ToSJIS();
		break;
	case CODE_UTF7:
		m_cmemLine.UTF7ToSJIS();
		break;
	case CODE_UNICODEBE:
		m_cmemLine.UnicodeBEToSJIS();
		break;
	}
	m_nLineIndex++;
	// �f�[�^����
	if( 0 != nBufLineLen + nEolLen ){
		// ���s�R�[�h��ǉ�
		m_cmemLine.AppendString( pcEol->GetValue(), pcEol->GetLen() );
		return m_cmemLine.GetStringPtr( pnLineLen );
	}
	// �f�[�^���Ȃ� => �I��
	m_cmemLine.SetString("");
	return NULL;
}



/*!
	�o�b�t�@�Ƀf�[�^��ǂݍ���
	@note �G���[���� throw ����
*/
void CFileLoad::Buffering( void )
{
	DWORD	ReadSize;

	// �������[�m��
	if( NULL == m_pReadBuf ){
		int nBufSize;
		nBufSize = ( m_nFileSize < gm_nBufSizeDef )?( m_nFileSize ):( gm_nBufSizeDef );
		//	Borland C++�ł�0�o�C�g��malloc���l�����s�ƌ��Ȃ�����
		//	�Œ�1�o�C�g�͎擾���邱�Ƃ�0�o�C�g�̃t�@�C�����J����悤�ɂ���
		if( 0 >= nBufSize ){
			nBufSize = 1; // Jun. 08, 2003  BCC��malloc(0)��NULL��Ԃ��d�l�ɑΏ�
		}

		m_pReadBuf = (char *)malloc( nBufSize );
		if( NULL == m_pReadBuf ){
			throw CError_FileRead(); // �������[�m�ۂɎ��s
		}
		m_nReadDataLen = 0;
		m_nReadBufSize = nBufSize;
		m_nReadBufOffSet = 0;
	}
	// ReadBuf���Ƀf�[�^���c���Ă���
	else if( m_nReadBufOffSet < m_nReadDataLen ){
		m_nReadDataLen -= m_nReadBufOffSet;
		memmove( m_pReadBuf, &m_pReadBuf[m_nReadBufOffSet], m_nReadDataLen );
		m_nReadBufOffSet = 0;
	}
	else{
		m_nReadBufOffSet = 0;
		m_nReadDataLen = 0;
	}
	// �t�@�C���̓ǂݍ���
	ReadSize = Read( &m_pReadBuf[m_nReadDataLen], m_nReadBufSize - m_nReadDataLen );
	if( 0 == ReadSize ){
		m_eMode = FLMODE_READBUFEND;	// �t�@�C���Ȃǂ̏I���ɒB�����炵��
	}
	m_nReadDataLen += ReadSize;
}

/*!
	�o�b�t�@�N���A
*/
void CFileLoad::ReadBufEmpty( void )
{
	if ( NULL != m_pReadBuf ){
		free( m_pReadBuf );
		m_pReadBuf = NULL;
	}
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}


/*!
	 ���݂̐i�s�����擾����
	 @return 0% - 100%  �኱�덷���o��
*/
int CFileLoad::GetPercent( void ){
	int nRet;
	if( 0 == m_nFileDataLen || m_nReadLength > m_nFileDataLen ){
		nRet = 100;
	}else if(  0x10000 > m_nFileDataLen ){
		nRet = m_nReadLength * 100 / m_nFileDataLen ;
	}else{
		nRet = m_nReadLength / 128 * 100 / ( m_nFileDataLen / 128 );
	}
	return nRet;
}

/*!
	GetNextLine�̔ėp�����R�[�h��
*/
const char* CFileLoad::GetNextLineCharCode(
	const char*	pData,		//!< [in]	����������
	int			nDataLen,	//!< [in]	����������̃o�C�g��
	int*		pnLineLen,	//!< [out]	1�s�̃o�C�g����Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,		//!< [i/o]	����������̃o�C�g�P�ʂ̃I�t�Z�b�g�ʒu
	CEol*		pcEol,		//!< [i/o]	EOL
	int*		pnEolLen	//!< [out]	EOL�̃o�C�g�� (Unicode�ō���Ȃ��悤��)
){
	const char *pRetStr;
	switch( m_CharCode ){
	case CODE_UNICODE:
		*pnBgn /= sizeof( wchar_t );
		pRetStr = (const char*)CFileLoad::GetNextLineW(
			(const wchar_t*)pData,
			nDataLen / sizeof( wchar_t ),
			pnLineLen,
			pnBgn,
			pcEol );
		*pnLineLen *= sizeof( wchar_t );
		*pnBgn     *= sizeof( wchar_t );
		*pnEolLen   = pcEol->GetLen() * sizeof( wchar_t );
		break;
	case CODE_UNICODEBE:
		*pnBgn /= sizeof( wchar_t );
		pRetStr = (const char*)CFileLoad::GetNextLineWB(
			(const wchar_t*)pData,
			nDataLen / sizeof( wchar_t ),
			pnLineLen,
			pnBgn,
			pcEol );
		*pnLineLen *= sizeof( wchar_t );
		*pnBgn     *= sizeof( wchar_t );
		*pnEolLen   = pcEol->GetLen() * sizeof( wchar_t );
		break;
	default:
		pRetStr = GetNextLine( pData, nDataLen, pnLineLen, pnBgn, pcEol );
		*pnEolLen = pcEol->GetLen();
		break;
	}
	return pRetStr;
}


/*!
	GetNextLine��wchar_t��
	GetNextLine���쐬
	static �����o�֐�
*/
const wchar_t* CFileLoad::GetNextLineW(
	const wchar_t*	pData,	//!< [in]	����������
	int			nDataLen,	//!< [in]	����������̕�����
	int*		pnLineLen,	//!< [out]	1�s�̕�������Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,		//!< [i/o]	����������̃I�t�Z�b�g�ʒu
	CEol*		pcEol		//!< [i/o]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// ���s�R�[�h��������
		if( pData[i] == (wchar_t)0x000a || pData[i] == (wchar_t)0x000d ){
			// �s�I�[�q�̎�ނ𒲂ׂ�
			pcEol->SetType( CEol::GetEOLTypeUni( &pData[i], nDataLen - i ) );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

/*!
	GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)
	GetNextLine���쐬
	static �����o�֐�
*/
const wchar_t* CFileLoad::GetNextLineWB(
	const wchar_t*	pData,	//!< [in]	����������
	int			nDataLen,	//!< [in]	����������̕�����
	int*		pnLineLen,	//!< [out]	1�s�̕�������Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,		//!< [i/o]	����������̃I�t�Z�b�g�ʒu
	CEol*		pcEol		//!< [i/o]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// ���s�R�[�h��������
		if( pData[i] == (wchar_t)0x0a00 || pData[i] == (wchar_t)0x0d00 ){
			// �s�I�[�q�̎�ނ𒲂ׂ�
			pcEol->SetType( CEol::GetEOLTypeUniBE( &pData[i], nDataLen - i ) );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

/*[EOF]*/
