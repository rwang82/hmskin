#ifndef __HM_WMP_DIALOGVIEW_H__
#define __HM_WMP_DIALOGVIEW_H__
#include "win\wmp\hm_wmp_display.h"
#include "common\hm_cmn_pattern_observer.h"
#include <string>
#include "tchar.h"
#include "win\frame\hm_debug_utils.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\paint_system\hm_ps_drawtool_i.h"
#include "win\keelctrl_repository\hm_keelctrl_scrollview.h"
#include "win\hm_export_defs.h"
//
struct HMLIB_API hm_dlgmsg_item_param
{
	typedef std::basic_string< TCHAR > string_type;	
#pragma warning(push)
#pragma warning(disable:4251)
	string_type m_strMsgFrom;
	string_type m_strMsgContent;
#pragma warning(pop)
	hm_rs_rawres_font_factory::obj_id_type m_idFontMsgFrom;
	hm_rs_rawres_color_factory::obj_id_type m_idColorMsgFrom;
	hm_rs_rawres_font_factory::obj_id_type m_idFontMsgContent;
	hm_rs_rawres_color_factory::obj_id_type m_idColorMsgContent;
};
//
class HMLIB_API hm_dlgmsg_item_default : public hm_districtobj {
public:
	hm_dlgmsg_item_default( hm_dlgmsg_item_param* pParam );
	virtual ~hm_dlgmsg_item_default();

	//
public:
	bool calcHeight( HDC hDC, hm_ps_drawtool_i* pDrawTool, unsigned int uWidth, unsigned int& uHeight );

private:
	unsigned int m_uHeight;
	hm_dlgmsg_item_param* m_pParam;
	unsigned int m_uWidthOld;
};
//
#define DLGVIEWFLAG_CONTENT_DIRTY ( 0x00000001 )
class hm_keelctrl_scrollview;
//
template< class item_type, typename item_param_type >
class hm_wmp_dialogview_t : public hm_wmp_display {  
// type define
public:
	typedef std::basic_string< TCHAR > string_type;
	typedef std::deque< item_type* > dlgmsg_container_type;

public:
	hm_wmp_dialogview_t( hm_keelctrl_scrollview* pKeelCtrlScrollViewObj );
	virtual ~hm_wmp_dialogview_t();

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds ){};

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

public:
	int* getStackPostionPtr() { return &m_nStackPosition; }; 

	//attribute
public:
	void pushbackItem( item_param_type* pParam );
	void pushfrontItem( item_param_type* pParam );
	void clearAllItem();
	//void push_back( hm_dlgmsg_item_default* pDlgMsgItem );
	//void clear();

private:
	void _doDrawSkinRaw( HDC hDC, RECT rcCanvas );

private:
	dlgmsg_container_type m_dlgmsgContainer;
	int m_nStackPosition;
	unsigned int m_uFlag;
	HBITMAP m_hContentMemBitmap;
	SIZE m_sizeContentMemBitmap;
	hm_keelctrl_scrollview* m_pKeelCtrlScrollViewObj;
	COLORREF m_clrBK;
};
typedef hm_wmp_dialogview_t< hm_dlgmsg_item_default, hm_dlgmsg_item_param > hm_wmp_dialogview;
//////////////////////////////////////////////////////////////////////////////
template< class item_type, typename item_param_type >
hm_wmp_dialogview_t< item_type, item_param_type >::hm_wmp_dialogview_t( hm_keelctrl_scrollview* pKeelCtrlScrollViewObj )
: m_nStackPosition( 0 )
, m_uFlag( DLGVIEWFLAG_CONTENT_DIRTY )
, m_hContentMemBitmap( NULL )
, m_pKeelCtrlScrollViewObj( pKeelCtrlScrollViewObj )
{
	_createDistrictObjs();
	memset( &m_sizeContentMemBitmap, 0, sizeof( m_sizeContentMemBitmap ) );
	m_clrBK = RGB( 255, 255, 255 );
}
//
template< class item_type, typename item_param_type >
hm_wmp_dialogview_t< item_type, item_param_type >::~hm_wmp_dialogview_t() {
	clearAllItem();
	//
	if ( m_hContentMemBitmap )
		::DeleteObject( m_hContentMemBitmap );
	m_hContentMemBitmap = NULL;
}
//
MSGBIND_BEGIN_T2( hm_wmp_dialogview_t )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN_T2( hm_wmp_dialogview_t )
ORDERWNDMSG_END()
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::_createDistrictObjs() {
	
}
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
	case HMKEELOBJEVENT_DRAWSELF:
		{
			HEDSPARAM* pHEDSParam = NULL;
			RECT rcCanvas;
			
			pHEDSParam = (HEDSPARAM*)pEventParam;
			if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
				return;
			_doDrawSkinRaw( pHEDSParam->m_hDCCanvas, rcCanvas );	
		}
		break;
	case HM_EVENT_MOVE_VIEWCONTENT:
		{
			HEVMPARAM* pParam = NULL;
			RECT rcCanvas;
			
			pParam = (HEVMPARAM*)pEventParam;
			rcCanvas.left = 0;
			rcCanvas.top = 0;
			rcCanvas.right = pParam->m_sizeCanvas.cx;
			rcCanvas.bottom = pParam->m_sizeCanvas.cy;
			_doDrawSkinRaw( pParam->m_hDCMem, rcCanvas );
		}
		break;
	default:
		break;
	}
}
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::_doDrawSkinRaw( HDC hDC, RECT rcCanvas ) {
	hm_posdecoder_stack* pPosDecoderStack = NULL;
	unsigned int uItemHeight;
	HDC hMemDC;
	HBITMAP hBitmapOld;
	item_type* pDistrictObj = NULL;
	dlgmsg_container_type::iterator itDlgMsg, iendDlgMsg;
	RECT rcMemCanvas;
	RECT rcContentVisiblt;
	int nMemBitmapHeight = 0;
	SIZE sizeCanvas;

	if ( ( m_uFlag & DLGVIEWFLAG_CONTENT_DIRTY ) 
		|| ( m_sizeContentMemBitmap.cx != ( rcCanvas.right - rcCanvas.left ) ) ) {

		// calc nMemBitmapHeight
		nMemBitmapHeight = 0;
		iendDlgMsg = m_dlgmsgContainer.end();
		for ( itDlgMsg = m_dlgmsgContainer.begin(); itDlgMsg!=iendDlgMsg; ++itDlgMsg ) {
			pDistrictObj = ( *itDlgMsg );
			//
			if ( !pDistrictObj->calcHeight( hDC, m_pDrawTool, ( rcCanvas.right - rcCanvas.left ), uItemHeight ) )
				continue;
			nMemBitmapHeight += uItemHeight;
		}
		
		// prepare m_hContentMemBitmap and m_sizeContentMemBitmap.
		if ( m_hContentMemBitmap )
			::DeleteObject( m_hContentMemBitmap );
		m_hContentMemBitmap = NULL;
		//
		m_sizeContentMemBitmap.cx = ( rcCanvas.right - rcCanvas.left );
		m_sizeContentMemBitmap.cy = nMemBitmapHeight;
		hMemDC = ::CreateCompatibleDC( hDC );
		m_hContentMemBitmap = ::CreateCompatibleBitmap( hDC, m_sizeContentMemBitmap.cx, m_sizeContentMemBitmap.cy );
		hBitmapOld = (HBITMAP)::SelectObject( hMemDC, m_hContentMemBitmap );
		RECT rcBKCanvas;
		rcBKCanvas.left = 0;
		rcBKCanvas.right = m_sizeContentMemBitmap.cx;
		rcBKCanvas.top = 0;
		rcBKCanvas.bottom = m_sizeContentMemBitmap.cy;
		::hm_util_draw_rect( hMemDC, rcBKCanvas, m_clrBK );

		rcMemCanvas.left = 0;
		rcMemCanvas.top = 0;
		rcMemCanvas.right = ( rcCanvas.right - rcCanvas.left );
		rcMemCanvas.bottom = ( rcCanvas.bottom - rcCanvas.top );
		m_nStackPosition = 0;
		iendDlgMsg = m_dlgmsgContainer.end();
		for ( itDlgMsg = m_dlgmsgContainer.begin(); itDlgMsg!=iendDlgMsg; ++itDlgMsg ) {
			pDistrictObj = ( *itDlgMsg );
			//
			if ( !pDistrictObj->calcHeight( hMemDC, m_pDrawTool, ( rcCanvas.right - rcCanvas.left ), uItemHeight ) )
				continue;
			//
			pPosDecoderStack = ( hm_posdecoder_stack* )pDistrictObj->getPosDecoder();
			pPosDecoderStack->m_nThickness = uItemHeight;
			//
			pDistrictObj->show( hMemDC, rcMemCanvas, m_pDrawTool );
			//
			hm_util_draw_line( hMemDC, rcMemCanvas.left, m_nStackPosition, rcMemCanvas.right, m_nStackPosition, RGB( 196, 228, 241 ) );
		}

//		::BitBlt( hMemDC, 0, 0, m_sizeContentMemBitmap.cx, m_sizeContentMemBitmap.cy, hDC, rcCanvas.left, rcCanvas.top, SRCCOPY );

		::SelectObject( hMemDC, hBitmapOld );
		::DeleteDC( hMemDC );
		//
		m_uFlag &= ~DLGVIEWFLAG_CONTENT_DIRTY;
		m_pKeelCtrlScrollViewObj->setContentWidth( m_sizeContentMemBitmap.cx );
		m_pKeelCtrlScrollViewObj->setContentHeight( m_sizeContentMemBitmap.cy );
		sizeCanvas.cx = ( rcCanvas.right - rcCanvas.left );
		sizeCanvas.cy = ( rcCanvas.bottom - rcCanvas.top );
		m_pKeelCtrlScrollViewObj->calcContentVisibleRect( sizeCanvas );
		//
		m_pKeelCtrlScrollViewObj->updateScrollBarPos();
	} 

	hMemDC = ::CreateCompatibleDC( hDC );
	m_pKeelCtrlScrollViewObj->getContentVisibleRect( rcContentVisiblt );
	hBitmapOld = (HBITMAP)::SelectObject( hMemDC, m_hContentMemBitmap );
	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, (rcContentVisiblt.right - rcContentVisiblt.left), (rcContentVisiblt.bottom - rcContentVisiblt.top), hMemDC, rcContentVisiblt.left, rcContentVisiblt.top, SRCCOPY );
	::SelectObject( hMemDC, hBitmapOld );
	::DeleteDC( hMemDC );

}
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::pushbackItem( item_param_type* pParam ) {
	item_type* pItemNew = new item_type( pParam );
	hm_posdecoder_stack* pPosDecoder = new hm_posdecoder_stack( hm_posdecoder_stack::STACK_T2B, &m_nStackPosition, 1, 1, 100 );

	//
	pItemNew->setPosDecoder( pPosDecoder );
	m_dlgmsgContainer.push_back( pItemNew );
	//
	m_uFlag &= DLGVIEWFLAG_CONTENT_DIRTY;
}
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::pushfrontItem( item_param_type* pParam ) {
	item_type* pItemNew = new item_type( pParam );
	hm_posdecoder_stack* pPosDecoder = new hm_posdecoder_stack( hm_posdecoder_stack::STACK_T2B, &m_nStackPosition );

	//
	pItemNew->setPosDecoder( pPosDecoder );
	m_dlgmsgContainer.push_front( pItemNew );
	//
	m_uFlag &= DLGVIEWFLAG_CONTENT_DIRTY;
}
//
template< class item_type, typename item_param_type >
void hm_wmp_dialogview_t< item_type, item_param_type >::clearAllItem() {
	if ( m_dlgmsgContainer.size() == 0 ) return;
	dlgmsg_container_type::iterator itItem, iendItem;

	iendItem = m_dlgmsgContainer.end();
	for ( itItem = m_dlgmsgContainer.begin(); itItem!=iendItem; ++itItem ) {
		delete (*itItem);
	}
	m_dlgmsgContainer.clear();
	//
	m_uFlag &= DLGVIEWFLAG_CONTENT_DIRTY;
}

#endif //__HM_WMP_DIALOGVIEW_H__