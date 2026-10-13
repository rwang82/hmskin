#ifndef __HM_WMP_SCROLLBAR_H__
#define __HM_WMP_SCROLLBAR_H__
#include "win\wmp\hm_wmp_display.h"
#include "win\frame\hm_pos_decoders.h"
#include "win\hm_export_defs.h"

//
#define SCRBAR_FLAG_STARTTHUMB_LBTNDOWN (0x00000001)
#define SCRBAR_FLAG_ENDTHUMB_LBTNDOWN (0x00000002)
#define SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN (0x00000004)
#define SCRBAR_FLAG_STARTSPACE_LBTNDOWN (0x00000008)
#define SCRBAR_FLAG_ENDSPACE_LBTNDOWN (0x00000010)
#define SCRBAR_FLAG_BKGND_LBTNDOWN (0x00000020)
#define SCRBAR_FLAG_TRACEMOUSELEAVE (0x40000000)
#define SCRBAR_FLAG_DISABLE (0x80000000)
//
#define SCRBAR_FLAG_LBTNDOWN ( SCRBAR_FLAG_STARTTHUMB_LBTNDOWN \
	| SCRBAR_FLAG_ENDTHUMB_LBTNDOWN \
	| SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN \
	| SCRBAR_FLAG_STARTSPACE_LBTNDOWN \
	| SCRBAR_FLAG_ENDSPACE_LBTNDOWN \
	| SCRBAR_FLAG_BKGND_LBTNDOWN )
//
class scrollbar_state_base;
class scrollbar_state_nondragtrackthumb;
class scrollbar_state_dragtrackthumb;
struct HEDSPARAM;
//
class HMLIB_API hm_wmp_scrollbar : public hm_wmp_display {
	friend class scrollbar_state_nondragtrackthumb;
	friend class scrollbar_state_dragtrackthumb;
	friend class scrollbar_state_base;

public:
	typedef enum _ENUMDIRECTIONTYPE { L2R_TYPE = 0, T2B_TYPE } ENUMDIRECTIONTYPE;
	typedef enum _ENUMDISTRICTID {
		EDISTRICTID_BKGND = 0,
		EDISTRICTID_THUMB_START_NOR,
		EDISTRICTID_THUMB_START_HOV,
		EDISTRICTID_THUMB_START_PRE,
		EDISTRICTID_THUMB_END_NOR,
		EDISTRICTID_THUMB_END_HOV,
		EDISTRICTID_THUMB_END_PRE,
		EDISTRICTID_THUMB_TRACK_NOR,
		EDISTRICTID_THUMB_TRACK_HOV,
		EDISTRICTID_THUMB_TRACK_PRE,
	} ENUMDISTRICTID;
	typedef enum _ENUMCURSORPOS { ECP_OUTSIDE = 0, ECP_STARTTHUMB, ECP_ENDTHUMB, ECP_TRACKTHUMB, ECP_STARTSPACE, ECP_ENDSPACE, ECP_BKGND } ENUMCURSORPOS;
	typedef enum _ENUMSTATETYPE { ESTATE_UNKNOWN = 0, ESTATE_DRAG, ESTATE_NONDRAG } ENUMSTATETYPE;

public:
	hm_wmp_scrollbar( ENUMDIRECTIONTYPE eDirectType, hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_scrollbar();

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

public:
	bool setValRange( int nValMin, int nValMax );
	void getValRange( int& nValMin, int& nValMax ) { nValMin = this->m_nValMin; nValMax = m_nValMax; }
	bool setCurValue( int nValCur );
	void getCurValue( int& nValCur ) { nValCur = m_nValCur; };
	bool setTrackThumbLen( unsigned int uLen );
	void drawSelf();

public:
	hm_posdecoder_dock* getStartThumbPosDecoder() { return m_pPosDecoderStartThumb; }
	hm_posdecoder_dock* getEndThumbPosDecoder() { return m_pPosDecoderEndThumb; }
	hm_posdecoder_slider* getTrackThumbPosDecoder() { return m_pPosDecoderTrackThumb; }
	
private:
	void _uninitDistricts();
	void _initStateObj();
	void _uninitStateObj();
	ENUMCURSORPOS _getCursorPos( const RECT& rcKeelObj, const POINT& curPos );
	bool _getTrackThumbRefRect( const RECT& rcKeelObj, RECT& rcTrackThumbRefRect );
	bool _updateCurVal2TrackThumbPos( const RECT& rcKeelObj );
	bool _moveTrackThumb( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, const POINT& ptCursorClient );
	bool _isSameRect( const RECT& rcL, const RECT& rcR ) const;
	void _switchState( ENUMSTATETYPE eStateType );
	ENUMSTATETYPE _getCurState();
	
private:
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnRButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	ENUMDIRECTIONTYPE m_eDirectType;
	hm_posdecoder_dock* m_pPosDecoderStartThumb;
	hm_posdecoder_dock* m_pPosDecoderEndThumb;
	hm_posdecoder_slider* m_pPosDecoderTrackThumb;
	//
	scrollbar_state_base* m_pCurStateObj;
	scrollbar_state_dragtrackthumb* m_pStateObjDrag;
	scrollbar_state_nondragtrackthumb* m_pStateObjNonDrag;
	//
	int m_nValMin;
	int m_nValMax;
	int m_nValCur;
	//
	POINT m_ptTrackThumbLBtnDown;
	int m_nDistanceLBtnDown;
	unsigned int m_uMouseStatusFlags;
	ENUMCURSORPOS m_eCursorPos;
	//
	HEDSPARAM* m_pHEDSParamCache;
};

// state machine mode.
class scrollbar_state_base{
public:
	scrollbar_state_base(){};
	virtual ~scrollbar_state_base(){};
 
public:
	virtual void _OnLButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnLButtonDblClk( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnLButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnRButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnRButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnMouseMove( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void _OnMouseLeave( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
    //
	virtual void _OnDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) = 0;
	virtual hm_wmp_scrollbar::ENUMSTATETYPE getEState() = 0;

public:
	virtual void _doDrawSkin( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) = 0;
};

class scrollbar_state_nondragtrackthumb : public scrollbar_state_base {
public:
	scrollbar_state_nondragtrackthumb();
	virtual ~scrollbar_state_nondragtrackthumb();

public:
	virtual void _OnLButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnLButtonDblClk( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnLButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnRButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnRButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnMouseMove( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnMouseLeave( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	//
	virtual void _OnDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );
	virtual hm_wmp_scrollbar::ENUMSTATETYPE getEState() { return hm_wmp_scrollbar::ESTATE_NONDRAG; }
private:
	void _doDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );
	void _doDrawSkin( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );

private:
	HBITMAP m_hBitmapMemCanvasCache;
	RECT m_rcCanvasMemCache;
};

class scrollbar_state_dragtrackthumb : public scrollbar_state_base {
public:
	scrollbar_state_dragtrackthumb();
	virtual ~scrollbar_state_dragtrackthumb();

public:
	virtual void _OnLButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnLButtonDblClk( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnLButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnRButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnRButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnMouseMove( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void _OnMouseLeave( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	//
	virtual void _OnDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );
	virtual hm_wmp_scrollbar::ENUMSTATETYPE getEState() { return hm_wmp_scrollbar::ESTATE_DRAG; }
private:
	void _doDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );
	void _doDrawSkin( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos );

private:
	HBITMAP m_hBitmapMemCanvasCache;
	RECT m_rcCanvasMemCache;
	//
	HBITMAP m_hBitmapBK;
	SIZE m_sizeBK;
};


#endif //__HM_WMP_SCROLLBAR_H__