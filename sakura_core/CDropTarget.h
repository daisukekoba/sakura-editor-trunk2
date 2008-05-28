/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDropTarget;
class CYbInterfaceBase;
class COleLibrary;

#ifndef _CEDITDROPTARGET_H_
#define _CEDITDROPTARGET_H_

#include <windows.h>
class CEditView;// 2002/2/3 aroka ヘッダ軽量化

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class COleLibrary
{
	friend class CYbInterfaceBase;
private:
	DWORD m_dwCount;
	COleLibrary();
public:
	~COleLibrary();
private:
	void Initialize();
	void UnInitialize();
};



class CYbInterfaceBase
{
private:
	static COleLibrary m_olelib;
protected:
	CYbInterfaceBase();
	~CYbInterfaceBase();
	static HRESULT QueryInterfaceImpl( IUnknown*, REFIID, REFIID, void** );
};


template<class BASEINTERFACE>
class CYbInterfaceImpl : public BASEINTERFACE, public CYbInterfaceBase
{
private:
	static REFIID m_owniid;
public:
	CYbInterfaceImpl(){AddRef();}
	STDMETHOD( QueryInterface )( REFIID riid, void** ppvObj )
	{return QueryInterfaceImpl( this, m_owniid, riid, ppvObj );}
	STDMETHOD_( ULONG, AddRef )( void )
	{return 1;}
	STDMETHOD_( ULONG, Release )( void )
	{return 0;}
};


class CDropTarget : public CYbInterfaceImpl<IDropTarget>
{
public:
	/*
	||  Constructors
	*/
	CDropTarget( CEditView* );
	~CDropTarget();
	/*
	||  Attributes & Operations
	*/
private: // 2002/2/10 aroka アクセス権変更
	HWND			m_hWnd_DropTarget;
	CEditView*		m_pCEditView;
//	void*			m_pCEditView;
	//	static REFIID	m_owniid;
public:
	BOOL			Register_DropTarget( HWND );
	BOOL			Revoke_DropTarget( void );
	STDMETHODIMP	DragEnter( LPDATAOBJECT, DWORD, POINTL , LPDWORD );
	STDMETHODIMP	DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP	DragLeave( void );
	STDMETHODIMP	Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
protected:
	/*
	||  実装ヘルパ関数
	*/
};


class CDropSource : public CYbInterfaceImpl<IDropSource> {
private:
	BOOL m_bLeft;
public:
	CDropSource( BOOL bLeft ):m_bLeft( bLeft ){}

	STDMETHOD( QueryContinueDrag )( BOOL bEscapePressed, DWORD dwKeyState );
	STDMETHOD( GiveFeedback )( DWORD dropEffect );
};


class CDataObject : public CYbInterfaceImpl<IDataObject> {
private:
	friend class CEnumFORMATETC;	// 2008.03.26 ryoji

	typedef struct {	// フォーマットデータの構造体	// 2008.03.26 ryoji
		CLIPFORMAT cfFormat;
		//Feb. 26, 2001, fixed by yebisuya sugoroku
		LPBYTE			data;	//データ
		unsigned int	size;	//データサイズ。バイト単位。
	} DATA, *PDATA;

	int m_nFormat;	// 保持するフォーマット数	// 2008.03.26 ryoji
	PDATA m_pData;	// フォーマットデータの配列	// 2008.03.26 ryoji

public:
	CDataObject(LPCSTR lpszText, int nTextLen, BOOL bColmnSelect ):
		m_nFormat(0),
		m_pData(NULL)
	{SetText( lpszText, nTextLen, bColmnSelect );}
	~CDataObject(){SetText( NULL, 0, FALSE );}
	void	SetText( LPCSTR lpszText, int nTextLen, BOOL bColmnSelect );
	DWORD	DragDrop( BOOL bLeft, DWORD dwEffects );

	STDMETHOD( GetData )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( GetDataHere )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( QueryGetData )( LPFORMATETC );
	STDMETHOD( GetCanonicalFormatEtc )( LPFORMATETC, LPFORMATETC );
	STDMETHOD( SetData )( LPFORMATETC, LPSTGMEDIUM, BOOL );
	STDMETHOD( EnumFormatEtc )( DWORD, IEnumFORMATETC** );
	STDMETHOD( DAdvise )( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD );
	STDMETHOD( DUnadvise )( DWORD );
	STDMETHOD( EnumDAdvise )( LPENUMSTATDATA* );
};


//! CEnumFORMATETC クラス
//	2008.03.26 ryoji 新規作成
class CEnumFORMATETC : public CYbInterfaceImpl<IEnumFORMATETC> {
private:
	LONG m_lRef;
	int m_nIndex;
	CDataObject* m_pcDataObject;
public:
	CEnumFORMATETC(CDataObject* pcDataObject) : m_lRef(1), m_nIndex(0), m_pcDataObject(pcDataObject) {}
	STDMETHOD_( ULONG, AddRef )( void )
	{return ::InterlockedIncrement(&m_lRef);}
	STDMETHOD_( ULONG, Release )( void )
	{
		if( ::InterlockedDecrement(&m_lRef) == 0 ){
			delete this;
			return 0;	// 削除後なので m_lRef は使わない
		}
		return m_lRef;
	}
	STDMETHOD( Next )(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	STDMETHOD( Skip )(ULONG celt);
	STDMETHOD( Reset )(void);
	STDMETHOD( Clone )(IEnumFORMATETC** ppenum);
};

///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDROPTARGET_H_ */


/*[EOF]*/
