#ifndef __hm_wmp_display_H__
#define __hm_wmp_display_H__
#include "win\paint_system\hm_ps_drawtool_i.h"
#include "win\resource_system\hm_rs_uires_i.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\frame\hm_pos_decoders.h"
#include "win\hm_export_defs.h"
#include <map>
#include <stdexcept>

///////////////////////////////////////////////////////////////
struct HMLIB_API hm_showobj {
	hm_showobj( hm_rs_uires_i* pUIRes = NULL, hm_pos_decoder_i* pPosDecoder = NULL );
	~hm_showobj();

	//
	void show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool );
	bool loadRes( hm_ps_drawtool_i* pDrawTool );
	bool unloadRes( hm_ps_drawtool_i* pDrawTool );

	hm_rs_uires_i* m_pUIRes;
	hm_pos_decoder_i* m_pPosDecoder;
};

///////////////////////////////////////////////////////////////
class HMLIB_API hm_districtobj {
public:
	typedef std::deque< hm_showobj* > showobj_container_type;

public:
	hm_districtobj( hm_pos_decoder_i* pPosDecoder = NULL );
	~hm_districtobj();
	
public:
	//
	bool pushBackShowObj( hm_showobj* pShowObj );
	//
	bool pushFrontShowObj( hm_showobj* pShowObj );
	//
	unsigned int getShowObjCount() { return m_containerShowObjs.size(); };
	//
	hm_showobj* getShowObj( unsigned int uIndex );
	//
	void show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool, void* pContext = NULL );
	//
	void getDistrictRect( const RECT& rcCanvas, RECT&  );
	//
	bool loadRes( hm_ps_drawtool_i* pDrawTool );
	//
	bool unloadRes( hm_ps_drawtool_i* pDrawTool );
	//
	hm_pos_decoder_i* getPosDecoder() { return m_pPosDecoder; }
	//
	void setPosDecoder( hm_pos_decoder_i* pPosDecoder ) { m_pPosDecoder = pPosDecoder; }

private:
	void _clearAllShowObj();
	
private:
#pragma warning(push)
#pragma warning(disable:4251)
	showobj_container_type m_containerShowObjs;
#pragma warning(pop)
	hm_pos_decoder_i* m_pPosDecoder;
};

///////////////////////////////////////////////////////////////
class HMLIB_API hm_wmp_display : public hm_wndmsgprocessor_i {
public:
	typedef int district_id_type;
	typedef std::map< district_id_type, hm_districtobj* > map_id2district_type;
	typedef std::deque< district_id_type > district_id_container_type;

	// buffering policy for interaction draws ( straight to the screen DC ):
	// the framework applies it, controls only declare it. see DESIGN.md
	// "引擎演进" - the paint path ( memory DC of the root canvas ) always
	// draws direct regardless of strategy.
	typedef enum _ENUMBUFSTRATEGY {
		BUF_NONE = 0,       // draw straight onto the target DC
		BUF_ALLOC,          // compose into a fresh bitmap per draw, single blit ( click-rate controls )
		BUF_CACHE_SIZE      // compose into a bitmap cached by canvas size ( drag-rate controls )
	} ENUMBUFSTRATEGY;

public:
	hm_wmp_display( hmskin_event_listener_base* pEventListener = NULL );
	virtual ~hm_wmp_display();

public:
	void getAllDistrictId( district_id_container_type& containerIds );

	// interaction redraw entry: GetDC + buffering per m_eBufStrategy +
	// release. mouse handlers call this instead of hand-rolling the
	// GetDC/draw/ReleaseDC triplet.
	void drawSelf( hm_keel_obj* pKeelObj, HWND hWndCanvas );

	// interface
protected:
	virtual void _createDistrictObjs() = 0;
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds ) = 0;

protected:
	// funnel for both paint ( memory DC ) and interaction ( screen DC )
	// draws: detects the draw target and applies the buffer strategy.
	// protected: derived update() funnels its paint draws through here too.
	void _drawWithStrategy( HDC hDC, const RECT& rcCanvas );

	// the control's actual drawing - the ONLY render method a display
	// control implements. default empty until migrated.
	virtual void _render( HDC hDC, const RECT& rcCanvas ) {}

	// hover-leave tracking mechanics ( the MECHANISM - what to clear on
	// leave stays per-control semantics ): arm on mousemove, the flag
	// resets when the leave arrives. a control that forgets to arm gets
	// a hover state stuck until its next entry ( the tabheader bug ).
	// derived _OnMouseLeave must call onLeaveArrived() first thing.
	void armMouseLeaveTracking( HWND hWndTrack, DWORD dwFlags = TME_LEAVE ) {
		if ( m_bLeaveTracking )
			return;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = dwFlags;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = hWndTrack;
		::TrackMouseEvent( &tme );
		m_bLeaveTracking = TRUE;
	}
	void onLeaveArrived() { m_bLeaveTracking = FALSE; }

public:
	void setDrawTool( hm_ps_drawtool_i* pDrawTool );
	bool hasDrawTool() const { return m_pDrawTool!=NULL; }
	bool getDistrictObj( district_id_type idDistrict, hm_districtobj** ppDistrictObj );

protected:
	bool _setDistrictObj( district_id_type idDistrict, hm_districtobj* pDistrictObj );
	bool _hasDistrictObj( district_id_type idDistrict );
	bool _getDistrictObj( district_id_type idDistrict, hm_districtobj** ppDistrictObj );
	bool _clearAllDistrictObjs();

protected:
	hm_ps_drawtool_i* m_pDrawTool;
	BOOL m_bLeaveTracking;
	ENUMBUFSTRATEGY m_eBufStrategy;
	HBITMAP m_hBufBmp;    // BUF_CACHE_SIZE backing bitmap
	RECT m_rcBufCanvas;   // BUF_CACHE_SIZE size key
#pragma warning(push)
#pragma warning(disable:4251)
	map_id2district_type m_mapId2District;
#pragma warning(pop)
};


#endif //__hm_wmp_display_H__