#ifndef __HM_WMP_SCROLLVIEW_H__
#define __HM_WMP_SCROLLVIEW_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\hm_export_defs.h"

class hm_uiroot;
class hm_keel_obj;
class hm_layout_obj;
class hm_wmp_scrollbar;

// scroll ORCHESTRATOR - a plain WMP ( no districts, no rendering ). attach it
// to the VIEWPORT keelobj of a manifest-declared subtree:
//
//   viewport keelobj   ( ClipChildren: true, hosts this WMP, RegEvts:[10] )
//     |- content keelobj   ( LayoutScroll: translated via layout scroll offset )
//     |- scrollbar keelobj ( hm_wmp_scrollbar WMP, typically docked right )
//
// it carries the scroll state machine of the retired hm_keelctrl_scrollview:
// content extent <-> visible-rect clamping <-> scrollbar value-range/thumb
// binding; thumb drag and wheel-over-bar arrive through the bar's
// TRACKTHUMB_MOVED; wheel over the viewport is bound here directly ( self
// hit-tested, bar strip excluded ). every scroll lands in _scrollTo: clamp ->
// update the visible window -> optionally shift the content layout ->
// broadcast HM_EVENT_MOVE_VIEWCONTENT ( pure notification, content WMPs
// repaint through the normal paint cycle ) -> sync the bar -> invalidate the
// viewport minus the bar strip.
//
// SHUTDOWN CONTRACT: every WMP id referenced by the config ( bar AND content
// WMP ) must be SMALLER than this WMP's own idWMP - the WMP factory destroys
// by ascending id, so referenced WMPs die first and their
// ~hm_cmn_subject::EraseAllRegister removes our subscriptions for free; this
// destructor must NOT touch any referenced pointer. the loader asserts it.
class HMLIB_API hm_wmp_scrollview : public hm_wndmsgprocessor_i {
public:
	// all lengths arrive ALREADY DPI-scaled from the loader.
	struct SCROLLVIEW_CFG {
		unsigned int m_idWMPSearchBar;    // required; must be < own idWMP
		unsigned int m_idKeelObjContent;  // required; content root keelobj
		unsigned int m_idKeelObjBar;      // 0 = no bar-strip wheel exclusion / no track measure
		unsigned int m_idWMPContent;      // 0 = no dynamic extent source ( static extent below )
		unsigned int m_uContentExtentW;   // 0 = measure the content keelobj rect at runtime
		unsigned int m_uContentExtentH;   // 0 = measure the content keelobj rect at runtime
		unsigned int m_uArrowLen;         // default 20 ( = sbbtn bitmap size )
		unsigned int m_uMinThumbLen;      // default 20
		int          m_nFixedThumbLen;    // <0 = proportional thumb; >=0 = fixed length
		BOOL         m_bLayoutScroll;     // TRUE = translate the content layout ( static subtree scrolling )
		int          m_nWheelStep;        // px per wheel notch, default 60
		BOOL         m_bAnchorBottom;     // chat-list mode: extent growth pins to the new bottom when the
		                                  // user was at the bottom; scrolled-up users keep their position
		SCROLLVIEW_CFG();
	};
	enum { FLAG_SVIEW_CONTENT_DIRTY = 0x00000001 };

public:
	hm_wmp_scrollview( const SCROLLVIEW_CFG& cfgScaled, hm_uiroot* pUIRoot,
		hmskin_event_listener_base* pEventListener = NULL );
	virtual ~hm_wmp_scrollview();   // shutdown contract: see class comment

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()   // WM_MOUSEWHEEL

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

private:
	bool _ensureWiring();           // lazy id resolution + subscriptions ( idempotent )
	void _recalcFromViewport( hm_keel_obj* pKeelObjViewport );
	void _updateScrollBarPos( hm_keel_obj* pKeelObjViewport );
	void _scrollTo( hm_keel_obj* pKeelObjViewport, int nTop, bool bFromBar );
	void _applyContentOffset( int nTop );
	void _invalidateContentViewport( hm_keel_obj* pKeelObjViewport );
	bool _getContentExtent( unsigned int& uW, unsigned int& uH, hm_keel_obj* pKeelObjViewport ) const;
	void _calcContentVisibleRect( const SIZE& sizeCanvas );
	hm_keel_obj* _getKeelObj( unsigned int uId ) const;
	bool _isCursorOverViewport( hm_keel_obj* pKeelObjViewport, HWND hWndMsg, POINT ptScreen ) const;

private:
	void _OnMouseWheel( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	SCROLLVIEW_CFG m_cfg;              // scaled px
	hm_uiroot* m_pHMUIRoot;
	hm_wmp_scrollbar* m_pWMPBar;       // lazy-resolved, non-owning
	hm_keel_obj* m_pKeelObjViewport;   // cached host keelobj ( first POSCHANGED relay / wheel )
	RECT m_rcContentVisible;
	unsigned int m_uContentExtentW;    // seeded from cfg, replaced by CONTENT_EXTENT
	unsigned int m_uContentExtentH;
	unsigned int m_uFlag;
	bool m_bWired;
	bool m_bRangeBound;                // scrollbar range synced at least once
};

#endif //__HM_WMP_SCROLLVIEW_H__
