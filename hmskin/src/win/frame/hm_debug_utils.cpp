#include "stdafx.h"
#include "win\frame\hm_debug_utils.h"
#include <stdio.h>
#include <tchar.h>

// ---- invalidate / paint-rect logging ( see header for the contract ) ----
void hm_util_invlog( const TCHAR* szTag, const RECT* prc ) {
	static int s_nChecked = 0;
	static FILE* s_fp = NULL;
	static BOOL s_bOff = FALSE;
	if ( s_bOff )
		return;
	if ( !s_nChecked ) {
		s_nChecked = 1;
		TCHAR szPath[ MAX_PATH ];
		if ( ::GetModuleFileName( NULL, szPath, MAX_PATH ) == 0 ) {
			s_bOff = TRUE;
			return;
		}
		TCHAR* posSlash = _tcsrchr( szPath, _T('\\') );
		if ( !posSlash ) {
			s_bOff = TRUE;
			return;
		}
		*( posSlash + 1 ) = _T('\0');
		TCHAR szMarker[ MAX_PATH ];
		_tcsncpy_s( szMarker, MAX_PATH, szPath, _TRUNCATE );
		_tcscat_s( szMarker, MAX_PATH, _T("hmskin.trace") );
		FILE* fpMarker = _tfopen( szMarker, _T("r") );
		if ( !fpMarker ) {
			s_bOff = TRUE;   // no marker: stay silent for the whole run
			return;
		}
		fclose( fpMarker );
		_tcscat_s( szPath, MAX_PATH, _T("hmskin_invalidate.log") );
		s_fp = _tfopen( szPath, _T("w") );   // fresh file per process
		if ( !s_fp ) {
			s_bOff = TRUE;
			return;
		}
	}
	if ( !s_fp )
		return;
	if ( prc )
		_ftprintf( s_fp, _T("%08u %-12s (%ld,%ld,%ld,%ld)\n"),
		           ::GetTickCount(), szTag, prc->left, prc->top, prc->right, prc->bottom );
	else
		_ftprintf( s_fp, _T("%08u %-12s FULL\n"), ::GetTickCount(), szTag );
	fflush( s_fp );   // survive hard kills - diagnosis runs end that way
}

void hm_util_draw_rect( HDC hDC, const RECT& rcDraw )
{
	//
	::MoveToEx( hDC, rcDraw.left, rcDraw.top, NULL );
	::LineTo( hDC, rcDraw.left, rcDraw.bottom );
	::LineTo( hDC, rcDraw.right, rcDraw.bottom );
	::LineTo( hDC, rcDraw.right, rcDraw.top );
	::LineTo( hDC, rcDraw.left, rcDraw.top );

	//X
	::MoveToEx( hDC, rcDraw.left, rcDraw.top, NULL );
	::LineTo( hDC, rcDraw.right, rcDraw.bottom );
	::MoveToEx( hDC, rcDraw.right, rcDraw.top, NULL );
	::LineTo( hDC, rcDraw.left, rcDraw.bottom );
}

void hm_util_draw_rect( HDC hDC, const RECT& rcDraw, const COLORREF& clr )
{
	HBRUSH hBrush;

	hBrush = CreateSolidBrush( clr );
	
	FillRect( hDC, &rcDraw, hBrush );
	
	DeleteObject( hBrush );
}

void hm_util_draw_line( HDC hDC, int x1, int y1, int x2, int y2, const COLORREF& clr, int nWidth, int fnPenStyle ) {
	HPEN hPen, hPenOld;

	hPen = ::CreatePen( fnPenStyle, nWidth, clr );
	hPenOld = (HPEN)::SelectObject( hDC, hPen );

	::MoveToEx( hDC, x1, y1, NULL );
	::LineTo( hDC, x2, y2 );

	::SelectObject( hDC, hPenOld );
	::DeleteObject( hPen );
}