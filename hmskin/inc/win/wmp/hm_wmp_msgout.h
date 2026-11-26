#ifndef __HM_WMP_MSGOUT_H__
#define __HM_WMP_MSGOUT_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include <string>
#include <vector>
#include "tchar.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\wmp\hm_scroll_core.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\hm_export_defs.h"

class hm_wmp_scrollbar;
class hm_uiroot;
class hm_ps_drawtool_i;
class hm_keel_obj;

// MsgOutEditor v1b ( doc/MsgOutEditor.md ): a read-only chat message list
// that SELF-OWNS its scrolling - one WMP on the viewport keelobj
// ( ClipChildren ), embedding hm_scroll_core, driving the scrollbar
// directly and rendering every item translated by its own scroll offset
// ( no content keelobj, no event-bus round trip ). the content model is
// the extensible item interface below: text is built in, image and host
// item classes plug in through hm_msgout_append_item.
class HMLIB_API hm_msgout_item_i {
public:
	virtual ~hm_msgout_item_i() {}
	// row height at the given content width ( scaled px ).
	virtual int measure( int nRowW ) = 0;
	// paint into the translated row band ( bubble/avatar/text by sender ).
	virtual void render( HDC hDC, const RECT& rcRow, hm_ps_drawtool_i* pDrawTool ) = 0;
	// bubble box at the last measure, padding included ( 0,0 = fill the row ).
	virtual void getBubbleSize( SIZE& size ) { size.cx = 0; size.cy = 0; }
};

class hm_wmp_msgout;

// optional text-selection capability ( implemented by text items ).
class HMLIB_API hm_msgout_textsel_i {
public:
	virtual ~hm_msgout_textsel_i() {}
	virtual bool hitTestChar( POINT ptContent, int& nCharIdx ) = 0;
	virtual void getSelRects( int nCharS, int nCharE, std::vector<RECT>& containerRects ) = 0;
	virtual void getPlainText( std::basic_string<TCHAR>& strOut ) = 0;
};

class HMLIB_API hm_wmp_msgout : public hm_wmp_display {
	friend class hm_msgout_item_text;
	friend class hm_msgout_item_image;
public:
	typedef std::basic_string< TCHAR > string_type;
	enum ENUMMSGSENDER { EMSGSENDER_PEER = 0, EMSGSENDER_SELF = 1, EMSGSENDER_SYSTEM = 2 };

	// all lengths arrive ALREADY DPI-scaled from the loader; the rawres
	// pointers drive the drawtool text path ( three-backend parity ); the
	// HFONT is the fallback when no DrawTool is set ( headless ).
	struct MSGOUT_CFG {
		unsigned int m_idWMPSearchBar;    // required; must be < own idWMP
		unsigned int m_idKeelObjBar;      // bar keelobj ( wheel exclusion + track )
		hm_rs_rawres_font* m_pRawFontText;
		hm_rs_rawres_color* m_pRawClrText;
		hm_rs_rawres_color* m_pRawClrSystem;
		HFONT m_hFontText;
		COLORREF m_clrText;
		COLORREF m_clrBubbleSelf;
		COLORREF m_clrBubblePeer;
		COLORREF m_clrSystem;
		COLORREF m_clrAvatarSelf;
		COLORREF m_clrAvatarPeer;
		COLORREF m_clrSelHilite;
		int m_nAvatarLen;
		int m_nMaxBubblePct;    // of the available row width, default 80
		int m_nMsgGap;          // default 10
		int m_nPadH;            // bubble inner padding, default 12
		int m_nPadV;            // default 8
		int m_nCornerLen;       // bubble corner radius, default 8
		int m_nWheelStep;       // px per wheel notch, default 60
		int m_nFixedThumbLen;   // <0 = proportional thumb
		unsigned int m_uMinThumbLen;
		unsigned int m_uArrowLen;
		MSGOUT_CFG();
	};

public:
	hm_wmp_msgout( hm_uiroot* pUIRoot, hmskin_event_listener_base* pEventListener = NULL );
	virtual ~hm_wmp_msgout();   // owns every item; bar referenced by id ( dies last )

	void setConfigure( const MSGOUT_CFG& cfgScaled );
	// appends one item ( the list OWNS it ). measures at the current width,
	// grows the extent, applies stick/DND. NULL items are refused.
	bool appendItem( hm_msgout_item_i* pItem, int eSender );
	bool appendText( int eSender, const TCHAR* szText );
	bool appendImage( int eSender, const TCHAR* szImagePath, HBITMAP hBmp = NULL );
	void clear();
	unsigned int getCount() const { return (unsigned int)m_containerItems.size(); }
	int getScrollTop() const { return m_core.getTop(); }

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );

private:
	struct ItemSlot;
	void _render( HDC hDC, const RECT& rcCanvas );
	void _rowRects( const ItemSlot& slot, const RECT& rcCanvas, int nTop,
	                RECT& rcRow, RECT& rcBubble, RECT& rcContent );
	int _lineHeight();
	void _measureAll();
	void _relayoutAndNotify();
	void _notifyExtent();
	void _ensureWiring();
	void _scrollApplied( bool bRepaintBar );
	void _invalidateViewport();
	bool _isCursorOverContent( hm_keel_obj* pKeelObj, HWND hWndMsg, POINT ptScreen ) const;
	bool _measureTextExtent( const TCHAR* szText, int nW, int& nWOut, int& nHOut );
	bool _measureTextHeight( const TCHAR* szText, int nW, bool bMultiLine, int& nHOut );
	void _drawText( HDC hDC, const TCHAR* szText, const RECT& rc,
	                hm_rs_rawres_color* pRawClr, bool bMultiLine, bool bCenter, int nCharS = -1, int nCharE = -1 );
	void _copySelection();
	void _selectAll();
	void _OnMouseWheel( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	struct ItemSlot {
		hm_msgout_item_i* m_pItem;
		hm_msgout_textsel_i* m_pTextSel;   // NULL for non-text items
		int m_eSender;
		RECT m_rcRow;      // content coords ( unscrolled )
	};
#pragma warning(push)
#pragma warning(disable:4251)
	std::vector<ItemSlot> m_containerItems;
#pragma warning(pop)
	MSGOUT_CFG m_cfg;
	hm_uiroot* m_pHMUIRoot;
	hm_wmp_scrollbar* m_pWMPBar;      // lazy-resolved, non-owning
	hm_keel_obj* m_pKeelObjViewport;  // cached host keelobj
	hm_scroll_core m_core;
	int m_nContentH;                  // total stacked height, content px
	int m_nCanvasW;                   // last known content width ( re-measure on change )
	int m_nLineH;                     // text line pitch ( lazily measured, selection model )
	bool m_bWired;
	bool m_bWheelStopped;             // inertia decay seen - engine parked
	bool m_bSelDragging;              // mouse selection in progress
	int m_nSelItemAnchor, m_nSelCharAnchor;
	int m_nSelItemActive, m_nSelCharActive;
};

#endif //__HM_WMP_MSGOUT_H__
