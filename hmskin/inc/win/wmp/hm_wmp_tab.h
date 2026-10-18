#ifndef __HM_WMP_TAB_H__
#define __HM_WMP_TAB_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include "win\hm_export_defs.h"
#include "tchar.h"
#include <deque>
#include <string>

#define INDEX_NONE (0xFFFFFFFF)

struct HMLIB_API tabbtninfo {
public:
	typedef std::basic_string< TCHAR > string_type;
	
public:
#pragma warning(push)
#pragma warning(disable:4251)
	string_type m_strTitle;
#pragma warning(pop)
};

class HMLIB_API hm_wmp_tabheader: public hm_wmp_display {
public:
	enum ENUMDISTRICTID {
		EDISTRICTID_BK = 0,
		EDISTRICTID_TABBTN_NOR,
		EDISTRICTID_TABBTN_HOV,
		EDISTRICTID_TABBTN_PRE,
		EDISTRICTID_TABBTN_SEL
	};
	typedef std::deque< tabbtninfo* > tabbtninfo_container_type;
	
public:
	hm_wmp_tabheader( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_tabheader();

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );
	
	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();
	// draws the strip for the snapshot state; _drawWithStrategy wraps this,
	// so a buffering strategy ( manifest "BufStrategy" ) caches the composed
	// frame like any display WMP ( state switches invalidate, see update() )
	virtual void _render( HDC hDC, const RECT& rcCanvas );

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

	//
public:
	bool pushbackTabBtn( tabbtninfo* pTabBtnInfo );
	bool delTabBtn( unsigned int uTabBtnIndex );
	void delAllTabBtns();
	const tabbtninfo* getTabBtn( unsigned int uTabBtnIndex );
	bool hasTabBtn( unsigned int uTabBtndIndex );
	unsigned int getTabBtnCount();
	bool setText( hm_districtobj* pDistrictObj, unsigned int uTabBtnIndex );
	void pushDown( unsigned int uTabBtnIndex );

protected:
	unsigned int _getTabBtnCount() { return m_containerTabBtns.size(); }
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas, unsigned int uIndexSel, unsigned int uIndexPre, unsigned int uIndexHover );
	void _doDrawSkinRaw( HDC hDC, const RECT& rcCanvas, unsigned int uIndexSel, unsigned int uIndexPre, unsigned int uIndexHover );
	void _doDrawBtnRaw( HDC hDC, const RECT& rcCanvas, unsigned int uTabBtnIndex, ENUMDISTRICTID eDistrictType );
	void _setTabBtnHov( unsigned int uIndexHov );
	void _setTabBtnPre( unsigned int uIndexPre );
	bool _setText( hm_districtobj* pDistrictObj, unsigned int uTabBtnIndex );

private:
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	void _Init( HDC hRealDC, unsigned int uWidthNew, unsigned int uHeightNew );
	void _UnInit();
	void _UpdateMemBitmap( HDC hRealDC, unsigned int uWidthNew, unsigned int uHeightNew );

protected:
#pragma warning(push)
#pragma warning(disable:4251)
	tabbtninfo_container_type m_containerTabBtns;
#pragma warning(pop)
	unsigned int m_uIndexSel;
	unsigned int m_uIndexHov;
	unsigned int m_uIndexPre;
	// state the pixel cache was rendered for: a sel/hov/pre switch
	// invalidates the BUF_CACHE_SIZE bitmap ( interaction handlers mark
	// it too - they paint partial states straight to the screen )
	unsigned int m_uBufIndexSel;
	unsigned int m_uBufIndexHov;
	unsigned int m_uBufIndexPre;
	//RECT m_rcCanvasMemCache;
	HDC m_hMemDC;
	HBITMAP m_hMemBitmap;
	HBITMAP m_hMemBitmapOld;
	SIZE m_sizeMemBitmap;
};

//class HMLIB_API hm_wmp_tabitem : hm_wmp_display, public cmn::hm_cmn_observer_i {
//public:
//	hm_wmp_tabitem();
//	virtual ~hm_wmp_tabitem();
//
//
//
//};



#endif //__HM_WMP_TAB_H__