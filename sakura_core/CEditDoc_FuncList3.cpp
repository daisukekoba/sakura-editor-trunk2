/*!	@file
	@brief Erlang �A�E�g���C�����
	
	@author genta
	@date 2009.08.11 created
	
*/
/*
	Copyright (C) 2009, genta

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
#include <string.h>
#include "global.h"
#include <assert.h>

#include "CEditDoc.h"
#include "CFuncInfoArr.h"
#include "CDocLine.h"
#include "Debug.h"

// helper functions
inline bool IS_ATOM_HEAD( char wc )
{
	return ( 'a' <= wc && wc <= 'z' )
		|| ( wc == '_' ) || ( wc == '@' );
}

inline bool IS_ALNUM( char wc )
{
	return IS_ATOM_HEAD(wc) || ( 'A' <= wc && wc <= 'Z' ) || ( '0' <= wc && wc <= '9' );
}

inline bool IS_COMMENT( char wc )
{
	return ( wc == '%' );
}

inline bool IS_SPACE( char wc )
{
	return ( strchr( " \t\r\n", wc ) != 0 );
}

/** Erlang �A�E�g���C����� �Ǘ������
*/
struct COutlineErlang {
	enum {
		STATE_NORMAL,	//!< ��͒��łȂ�
		STATE_FUNC_CANDIDATE_FIN,	//!< �֐��炵������(�s����atom)����͍ς�
		STATE_FUNC_ARGS1,	//!< �ŏ��̈����m�F��
		STATE_FUNC_ARGS,	//!< 2�߈ȍ~�̈����m�F��
		STATE_FUNC_ARGS_FIN,//!< �֐��̉�͂�����
		STATE_FUNC_FOUND,	//!< �֐��𔭌��D�f�[�^�̎擾���\
	} m_state;

	char m_func[64];	//!< �֐���(Arity�܂�) = �\����
	int m_lnum;	//!< �֐��̍s�ԍ�
	int m_argcount;		//!< �������������̐�
	char m_parenthesis[32];	//!< ���ʂ̃l�X�g���Ǘ��������
	int m_parenthesis_ptr;	//!< ���ʂ̃l�X�g���x��
	
	COutlineErlang();
	bool parse( const char* buf, int linelen, int linenum );
	
	const char* ScanFuncName( const char* buf, const char* end, const char* p );
	const char* EnterArgs( const char* end, const char* p );
	const char* ScanArgs1( const char* end, const char* p );
	const char* ScanArgs( const char* end, const char* p );
	const char* EnterCond( const char* end, const char* p );
	const char* GetFuncName() const { return m_func; }
	int GetFuncLine() const { return m_lnum; }

private:
	void build_arity(int);
};

COutlineErlang::COutlineErlang() :
	m_state( STATE_NORMAL ), m_lnum( 0 ), m_argcount( 0 )
{
}

/** �֐����̎擾

	@param[in] buf �s(�擪����)
	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�֐�����atom�Datom�� �������A���t�@�x�b�g�C_, @ �̂����ꂩ����n�܂�
	�p�������񂩁C���邢�̓V���O���N�H�[�e�[�V�����ň͂܂ꂽ������D
*/
const char* COutlineErlang::ScanFuncName( const char* buf, const char* end, const char* p )
{
	assert( m_state == STATE_NORMAL );

	if( p > buf || ! ( IS_ATOM_HEAD( *p ) || *p == '\'' )) {
		return end;
	}
	
	if( *p == '\'' ){
		do {
			++p;
		} while( *p != '\'' && p < end );
		if( p >= end ){
			// invalid atom
			return p;
		}
		++p;
	}
	else {
		do {
			++p;
		} while( IS_ALNUM( *p ) && p < end );
	}
	
	int buf_len = sizeof( m_func ) / sizeof( m_func[0]);
	const char *head = buf;
	int len = p - buf;
	if( buf[0] == '\'' ){
		++buf;
		len -= 2;
		--buf_len;
	}
	len = len < buf_len - 1 ? len : buf_len - 1;
	strncpy( m_func, buf, len );
	m_func[len] = '\0';
	m_state = STATE_FUNC_CANDIDATE_FIN;
	return p;
}

/** �p�����[�^�̔���

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�֐����̎擾���������C�p�����[�^�擪�̊��ʂ�T���D
*/
const char* COutlineErlang::EnterArgs( const char* end, const char* p )
{
	assert( m_state == STATE_FUNC_CANDIDATE_FIN );

	while( IS_SPACE( *p ) && p < end )
		p++;
	
	if( p >= end )
		return end;

	if( IS_COMMENT( *p )){
		return end;
	}
	else if( *p == '(' ){ // )
		m_state = STATE_FUNC_ARGS1;
		m_argcount = 0;
		m_parenthesis_ptr = 1;
		m_parenthesis[0] = *p;
		++p;

		return p;
	}

	// not a function
	m_state = STATE_NORMAL;
	return end;
}

/** �擪�p�����[�^�̔���

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^��0��1�ȏ�̔��ʂ̂��߂ɏ�Ԃ�݂��Ă���D
*/
const char* COutlineErlang::ScanArgs1( const char* end, const char* p )
{
	assert( m_state == STATE_FUNC_ARGS1 );
	
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( *p == /* ( */ ')' ){
		// no argument
		m_state = STATE_FUNC_ARGS_FIN;
		p++;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		// argument found
		m_state = STATE_FUNC_ARGS;
		++m_argcount;
	}
	return p;
}

/** �p�����[�^�̉�͂ƃJ�E���g

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^����͂���D�p�����[�^�̐��Ɩ����̕����ʂ𐳂������ʂ��邽�߂ɁC
	���p���C���ʁC�p�����[�^�̋�؂�̃J���}�ɒ��ڂ���D
	���p���͉��s���܂ނ��Ƃ��ł��Ȃ��D
*/
const char* COutlineErlang::ScanArgs( const char* end, const char* p )
{
	assert( m_state == STATE_FUNC_ARGS );

	const int parptr_max = sizeof( m_parenthesis ) / sizeof( m_parenthesis[0] );
	char quote = '\0'; // �擪�ʒu��ۑ�
	for(const char* head = p ; p < end ; p++ ){
		if( quote ){
			if( *p == quote )
				quote = '\0';
		}
		else {
			if( strchr( "([{", *p )){	//)
				// level up
				if( m_parenthesis_ptr < parptr_max ){
					m_parenthesis[ m_parenthesis_ptr ] = *p;
				}
				m_parenthesis_ptr++;
			}
			else if( strchr( ")]}", *p )){	//)
				char op;
				switch( *p ){
					case ')': op = '('; break;
					case ']': op = '['; break;
					case '}': op = '{'; break;
					default:
						::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "��҂ɋ����ė~�����G���[",
							_T("COutlineErlang::ScanArgs ���m�̊���"));
						break;
				}
				// level down
				--m_parenthesis_ptr;
				while( 1 <= m_parenthesis_ptr && m_parenthesis_ptr < parptr_max ){
					if( m_parenthesis[ m_parenthesis_ptr ] != op ){
						// if unmatch then skip
						--m_parenthesis_ptr;
					}
					else{
						break;
					}
				}
				
				// check level
				if( m_parenthesis_ptr == 0 ){
					m_state = STATE_FUNC_ARGS_FIN;
					++p;
					return p;
				}
			}
			else if( *p == ',' && m_parenthesis_ptr == 1 ){
				++m_argcount;
			}
			else if( *p == ';' ){
				//	�Z�~�R�����͕����̕��̋�؂�D
				//	�p�����[�^���ɂ͌���Ȃ��̂ŁC��͂����s���Ă���
				//	���ʂ̕��Y�ꂪ�l������̂ŁC�d�؂蒼��
				m_state = STATE_NORMAL;
				return end;
			}
			else if( *p == '.' ){
				//	�s���I�h�͎��̖������C�����_�Ƃ��Ďg����D
				if( p > head && ( '0' <= p[-1] && p[-1] <= '9' )){
					//	�����_��������Ȃ��̂ŁC���̂܂܂ɂ���
				}
				else {
					//	�����̓r���ŕ��������ꂽ�͉̂�͂����s���Ă���
					//	���ʂ̕��Y�ꂪ�l������̂ŁC�d�؂蒼��
					m_state = STATE_NORMAL;
					return end;
				}
			}
			else if( *p == '"' ){
				quote = '"';
			}
			else if( *p == '\'' ){
				quote = '\'';
			}
			else if( IS_COMMENT( *p )){
				return end;
			}
		}
	}
	return p;
}

/** �֐��{�̂̋�؂�C�܂��͏������̌��o

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^�{�̂�\���L��(->)���������̊J�n�L�[���[�h(when)��
	��������C�֐������Ƃ���D
	����ȊO�̏ꍇ�͊֐��ł͂Ȃ������ƍl����D
*/
const char* COutlineErlang::EnterCond( const char* end, const char* p )
{
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( p + 1 < end && strncmp( p, "->", 2 ) == 0){
		p += 2;
		m_state = STATE_FUNC_FOUND;
	}
	else if( p + 3 < end && strncmp( p, "when", 4 ) == 0 ){
		m_state = STATE_FUNC_FOUND;
		p += 4;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		m_state = STATE_NORMAL;
	}
	return end;
}

/** �s�̉��

	@param[in] buf �s(�擪����)
	@param[in] linelen �s�̒���
	@param[in] linenum �s�ԍ�
*/
bool COutlineErlang::parse( const char* buf, int linelen, int linenum )
{
	const char* pos = buf;
	const char* const end = buf + linelen;
	if( m_state == STATE_FUNC_FOUND ){
		m_state = STATE_NORMAL;
	}
	if( m_state == STATE_NORMAL ){
		pos = ScanFuncName( buf, end, pos );
		if( m_state != STATE_NORMAL ){
			m_lnum = linenum;
		}
	}
	while( pos < end ){
		switch( m_state ){
			case STATE_FUNC_CANDIDATE_FIN:
				pos = EnterArgs( end, pos ); break;
			case STATE_FUNC_ARGS1:
				pos = ScanArgs1( end, pos ); break;
			case STATE_FUNC_ARGS:
				pos = ScanArgs( end, pos ); break;
			case STATE_FUNC_ARGS_FIN:
				pos = EnterCond( end, pos ); break;
			default:
				::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "��҂ɋ����ė~�����G���[",
					_T("COutlineErlang::parse Unknown State: %d"), m_state );
				break;
		}
		if( m_state == STATE_FUNC_FOUND ){
			build_arity( m_argcount );
			break;
		}
	}
	return m_state == STATE_FUNC_FOUND;
}

/** �֐����̌��� Arity (�����̐�)��t������

	@param[in] arity �����̐�
	
	�֐����̌��� /�p�����[�^�� �̌`�ŕ������ǉ�����D
	�o�b�t�@���s������ꍇ�͂ł���Ƃ���܂ŏ������ށD
	���̂��߁C10�ȏ�̈���������ꍇ�ɁC�����̐��̉��ʌ��������邱�Ƃ�����D
*/ 
void COutlineErlang::build_arity( int arity )
{
	int len = strlen( m_func );
	const int buf_size = sizeof( m_func ) / sizeof( m_func[0]);
	char *p = &m_func[len];
	char numstr[12];
	
	if( len + 1 >= buf_size )
		return; // no room

	numstr[0] = '/';
	itoa( arity, numstr + 1, 10 );
	strncpy( p, numstr, buf_size - len - 1 );
	m_func[ buf_size - 1 ] = '\0';
}

/** Erlang �A�E�g���C�����

	@par ��ȉ���ƕ��j
	�֐��錾��1�J�����ڂ���L�ڂ���Ă���D
	
	
	@par ��̓A���S���Y��
	1�J�����ڂ��A���t�@�x�b�g�̏ꍇ: �֐��炵���Ƃ��ĉ�͊J�n / �֐�����ۑ�
	�X�y�[�X�͓ǂݔ�΂�
	( �𔭌������� ) �܂ň����𐔂���D���̏ꍇ����q�̊��ʂƕ�������l��
	-> �܂��� when ������Ί֐���`�ƌ��Ȃ�(���̍s�ɂ܂������Ă��ǂ�)
	�r�� % (�R�����g) �����ꂽ��s���܂œǂݔ�΂�
*/
void CEditDoc::MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr )
{

	COutlineErlang erl_state_machine;
	int	nLineCount;

	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		int nLineLen;

		const char* pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( erl_state_machine.parse( pLine, nLineLen, nLineCount )){
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			int		nPosX;
			int		nPosY;
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				erl_state_machine.GetFuncLine()/*nFuncLine - 1*/,
				&nPosX,
				&nPosY
			);
			pcFuncInfoArr->AppendData( erl_state_machine.GetFuncLine() + 1, nPosY + 1, erl_state_machine.GetFuncName(), 0, 0 );
		}
	}
}
