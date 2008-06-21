#include "stdafx.h"
#include "CShiftJis.h"
#include "charset/charcode.h"
#include "CShareData.h"

void CShiftJis::S_GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",			0,	// EOL_NONE
		"\x0d\x0a",	2,	// EOL_CRLF
		"\x0a",		1,	// EOL_LF
		"\x0d",		1,	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].szData,aEolTable[eEolType].nLen);
}


//! 指定した位置の文字が何バイト文字かを返す
/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)
	@retval 1  1バイト文字
	@retval 2  2バイト文字
	@retval 0  エラー

	@date 2005-09-02 D.S.Koba 作成

	@note nIdxは予め文字の先頭位置とわかっていなければならない．
	2バイト文字の2バイト目をnIdxに与えると正しい結果が得られない．
*/
int CShiftJis::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen ){
		return 0;
	}else if( nIdx == (nDataLen - 1) ){
		return 1;
	}
	
	if( _IS_SJIS_1( reinterpret_cast<const unsigned char*>(pData)[nIdx] )
			&& _IS_SJIS_2( reinterpret_cast<const unsigned char*>(pData)[nIdx+1] ) ){
		return 2;
	}
	return 1;
}


/* コード変換 SJIS→Unicode */
EConvertResult CShiftJis::SJISToUnicode( CMemory* pMem )
{
	//ソース取得
	int nSrcLen;
	const char* pSrc = (const char*)pMem->GetRawPtr(&nSrcLen);

	//変換先バッファサイズ
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);

	//変換先バッファ確保
	wchar_t* pDst = new wchar_t[nDstLen+1];

	//変換
	nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen
	);
	pDst[nDstLen]=L'\0';

	//pMemを更新
	pMem->SetRawData( pDst, nDstLen * sizeof(wchar_t) );

	//後始末
	delete[] pDst;

	return RESULT_COMPLETE; //SJIS→UNICODEでデータは失わない (解釈の仕方は人によるが、ここではデータ損失無しと定める)
}


/* コード変換 Unicode→SJIS */
EConvertResult CShiftJis::UnicodeToSJIS( CMemory* pMem )
{
	//ソース取得
	int nSrcLen;
	const wchar_t* pSrc=(const wchar_t*)pMem->GetRawPtr(&nSrcLen);
	nSrcLen/=sizeof(wchar_t); //文字単位に変換

	//変換先バッファサイズ
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//変換先バッファ確保
	char* pDst = new char[nDstLen+1];

	//変換
	BOOL bLost = true;
	nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen,
		NULL,
		&bLost
	);
	pDst[nDstLen] = '\0';

	//pMemを更新
	pMem->SetRawData( pDst, nDstLen );

	//後始末
	delete[] pDst;

	//結果
	if(bLost){
		return RESULT_LOSESOME;
	}
	else{
		return RESULT_COMPLETE;
	}
}


// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CShiftJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	//変換先バッファ確保
	unsigned char sCvt[8];

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInSjis) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	//変換
	BOOL bLost = FALSE;
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		cSrc,
		1,
		(char*)sCvt,
		8,
		NULL,
		&bLost
	);

	//結果
	if (bLost) {
		return RESULT_LOSESOME;
	}

	int		i;
	TCHAR*	p; 
	for (i = 0, p = pDst; i < nDstLen; i++, p += 2) {
		auto_sprintf( p, _T("%02x"), sCvt[i]);
	}

	return RESULT_COMPLETE;
}
