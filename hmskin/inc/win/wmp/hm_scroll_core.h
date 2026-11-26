#ifndef __HM_SCROLL_CORE_H__
#define __HM_SCROLL_CORE_H__
#include "windows.h"
#include "win\hm_export_defs.h"

class hm_wmp_scrollbar;
class hm_keel_obj;

// vertical scroll state machine, extracted from hm_wmp_scrollview so dynamic
// content WMPs ( hm_wmp_msgout ) can embed it and self-own their scrolling -
// no event-bus round trip, no translated content keelobj. the owning WMP
// feeds viewport/extent in, calls scrollTo on every input path, and forwards
// the scrollbar's TRACKTHUMB_MOVED by value ( HETMPARAM is heap-transient,
// never cached - engine contract ).
class HMLIB_API hm_scroll_core {
public:
	hm_scroll_core();

public:
	// inputs ( scaled px ).
	void setViewport( int nViewH );
	void setExtent( int nExtentH );
	// the extent grew to nExtentH ( chat-list append ): stick to the new
	// bottom when the user WAS at the bottom ( decided against the OLD
	// extent ), otherwise keep the position ( DND ). returns true when the
	// top moved ( repaint needed ).
	bool onExtentGrown( int nExtentH );
	// clamp + apply; returns true when the top actually moved.
	bool scrollTo( int nTop );
	// reset to the unscrolled state ( content cleared / first layout ).
	void reset();

	int getTop() const { return m_nTop; }
	int getViewH() const { return m_nViewH; }
	int getExtentH() const { return m_nExtentH; }
	int getMaxTop() const;              // extentH - viewH, floored at 0
	bool isAtBottom() const;            // +2px tolerance ( legacy rule )

	// scrollbar sync: range == pixel overflow, value == top, thumb length
	// fixed or viewport/content ratio floored at the minimum. pKeelObjBar
	// supplies the track height ( proportional thumb only ).
	void syncBar( hm_wmp_scrollbar* pBar, hm_keel_obj* pKeelObjBar );

private:
	int m_nTop;
	int m_nViewH;
	int m_nExtentH;
	// thumb config ( mirrored from the owning WMP's cfg at construction ).
	int m_nFixedThumbLen;              // <0 = proportional
	unsigned int m_uMinThumbLen;

public:
	void setThumbCfg( int nFixedThumbLen, unsigned int uMinThumbLen );
};

#endif //__HM_SCROLL_CORE_H__
