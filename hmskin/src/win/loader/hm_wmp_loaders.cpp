#include "stdafx.h"
#include "win\loader\hm_wmp_loaders.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include "win\hm_win_misc.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_bkgnd.h"
#include "win\wmp\hm_wmp_bkgnd_nca.h"
#include "win\wmp\hm_wmp_button.h"
#include "win\wmp\hm_wmp_button_nca.h"
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\wmp\hm_wmp_radio.h"
#include "win\wmp\hm_wmp_drawtrigger.h"
#include "win\wmp\hm_wmp_drawtrigger_nca.h"
#include "win\wmp\hm_wmp_layouttrigger.h"
#include "win\wmp\hm_wmp_popupmenu.h"
#include "win\wmp\hm_wmp_rootmenu.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\wmp\hm_wmp_scrollview.h"
#include "win\wmp\hm_wmp_msgout.h"
#include "win\wmp\hm_wmp_tab.h"
#include "win\wmp\hm_wmp_viewbkcontent.h"
#include "win\wmp\hm_wmp_movewnd.h"
#include "win\resource_system\hm_rs_uires_pic.h"
#include "win\resource_system\hm_rs_uires_color.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\paint_system\hm_ps_drawtool_d2d.h"
#include "win\frame\hm_uiroot.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "common\FuncPack.h"
#include "common\MemFuncPack.h"
#include "json.h"
#include <assert.h>
#include <atlconv.h>
//

// DPI: posdecoder parse sites sit below the createWMP choke point and read
// the loader-base scale snapshot ( m_dblScale, taken from the uiroot ).
static int _ScalePx( double dblScale, int nValue96 ) {
	return (int)( nValue96 * dblScale + 0.5 );
}

// menu Cfg color lookup: returns false when the key is absent; sets *pbBad
// when it names an id the manifest Color section never declared.
static bool _MenuCfgColor( const Json::Value& valCfg, const char* szKey, bool& bBad, COLORREF* pClr ) {
	Json::Value valKey = valCfg[ szKey ];
	if ( !valKey.isInt() )
		return false;
	hm_rs_rawres_color* pRawColor = RAWCOLORFACTORY_INSTANCE->getObj(
		(hm_rawres_color_factory::obj_id_type)valKey.asInt() );
	if ( !pRawColor ) {
		bBad = true;   // the id names a color the manifest never declared
		HM_LOADER_FAIL( false );
	}
	*pClr = pRawColor->getCOLORREF();
	return true;
}

const char STR_DRAWTOOL[] = "DrawTool";
const char STR_BUFSTRATEGY[] = "BufStrategy";
const char STR_DISTRICTOBJS[] = "DistrictObjs";
const char STR_ROOTMENUCFG[] = "RootMenuCfg";
const char STR_TABBTNS[] = "TabBtns";
const char STR_THICK[] = "Thick";
const char STR_WNDMARK[] = "WndMark";
const char STR_CTRLID[] = "CtrlId";
const char STR_PROPID[] = "PropId";
//
#define HM_WMP_X_LOADER_IMPL( hm_wmp_cls_name ) \
	if ( !pHMUIRoot ) { HM_LOADER_FAIL( NULL ); } \
	Json::Value valTmp; \
	hm_wmp_factory::obj_id_type idWMP = 0; \
	std::string strWMPClsName; \
	hm_wndmsgprocessor_i* pWMPNew = NULL; \
	assert( _getWMPName( valWMP, strWMPClsName ) ); \
	assert( strWMPClsName == #hm_wmp_cls_name ); \
	valTmp = valWMP[ "idWMP" ]; \
	if ( !valTmp.isInt() ) { HM_LOADER_FAIL( NULL ); }\
	idWMP = valTmp.asInt(); \
	pWMPNew = pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_cls_name, hmskin_event_listener_base* >( idWMP, pHMUIRoot->m_pEventListener ); \
	return pWMPNew;
//////////////////////////////////////////////////////////////////////////
bool hm_wmp_display_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	hm_wmp_display* pWMPDisplay = NULL;

	// get pWMPDisplay
	pWMPDisplay = dynamic_cast< hm_wmp_display* >( pWMPLoading );
	if ( !pWMPDisplay ) {
		HM_LOADER_FAIL( false );
	}
	// fill district objects to pWMPDisplay FIRST: _fillDrawTool's setDrawTool
	// loads every district's resources - with the old order ( tool first )
	// districts were still empty at that moment, so the load pass was a
	// no-op and nothing was ever preloaded ( resources lived on draw-time
	// lazy get/free instead ). that made a later setDrawTool unload
	// resources nobody loaded -> refcount underflow. districts must be
	// populated before the tool loads them.
	if ( !_fillDistrictObjs( valWMP, pWMPDisplay ) ) {
		HM_LOADER_FAIL( false );
	}
	// fill drawtool to pWMPDisplay.
	if ( !_fillDrawTool( valWMP, pWMPDisplay ) ) {
		HM_LOADER_FAIL( false );
	}
	// optional buffering strategy ( pixel cache ) for this WMP
	if ( !_fillBufStrategy( valWMP, pWMPDisplay ) ) {
		HM_LOADER_FAIL( false );
	}

	return true;
}

bool hm_wmp_display_loader::_fillDrawTool( const Json::Value& valWMP, hm_wmp_display* pWMPDest ) {
	Json::Value valDrawTool;
	hm_ps_drawtool_i* pDrawTool = NULL;
	std::string strDrawTool;

	valDrawTool = valWMP[ STR_DRAWTOOL ];
	if ( !valDrawTool.isString() ) {
		HM_LOADER_FAIL( false );
	}
	strDrawTool = valDrawTool.asString();
	if ( 0 == strcmp( strDrawTool.c_str(), "Gdiplus" ) ) {
		pDrawTool = hm_ps_drawtool_gdiplus::getInstance();
	} else if ( 0 == strcmp( strDrawTool.c_str(), "Gdi" ) ) {
		pDrawTool = hm_ps_drawtool_gdi::getInstance();
	} else if ( 0 == strcmp( strDrawTool.c_str(), "D2D" ) ) {
		// hybrid M1 backend: text via D2D/DirectWrite, rest forwarded to gdiplus
		pDrawTool = hm_ps_drawtool_d2d::getInstance();
	} else {
		HM_LOADER_FAIL( false );
	}

	pWMPDest->setDrawTool( pDrawTool );
	return true;
}

bool hm_wmp_display_loader::_fillBufStrategy( const Json::Value& valWMP, hm_wmp_display* pWMPDest ) {
	// optional per-WMP buffering strategy; absent field keeps the class
	// default ( BUF_NONE for everything except code-set controls like the
	// checkbox ). "CACHE_SIZE" opts the WMP into the pixel cache - the
	// WMP's render must be self-contained ( opaque background inside its
	// district ) because the blit is SRCCOPY.
	Json::Value valBuf = valWMP[ STR_BUFSTRATEGY ];
	if ( valBuf.isNull() )
		return true;   // not declared: keep default
	if ( !valBuf.isString() ) {
		HM_LOADER_FAIL( false );
	}
	std::string strBuf = valBuf.asString();
	if ( 0 == strcmp( strBuf.c_str(), "NONE" ) ) {
		pWMPDest->setBufStrategy( hm_wmp_display::BUF_NONE );
	} else if ( 0 == strcmp( strBuf.c_str(), "ALLOC" ) ) {
		pWMPDest->setBufStrategy( hm_wmp_display::BUF_ALLOC );
	} else if ( 0 == strcmp( strBuf.c_str(), "CACHE_SIZE" ) ) {
		pWMPDest->setBufStrategy( hm_wmp_display::BUF_CACHE_SIZE );
	} else {
		HM_LOADER_FAIL( false );
	}
	return true;
}

bool hm_wmp_display_loader::_fillDistrictObjs( const Json::Value& valWMP, hm_wmp_display* pWMPDest ) {
	unsigned int uIndexDistrictObj, uCountDistrictObj;
	Json::Value aryDistrictObjs;
	Json::Value valDistrict;
	Json::Value valTmp;
	hm_wmp_display::district_id_type idDistrict;

	aryDistrictObjs = valWMP[ STR_DISTRICTOBJS ];
	if ( !aryDistrictObjs.isArray() || !pWMPDest ) {
		HM_LOADER_FAIL( false );
	}
	uCountDistrictObj = aryDistrictObjs.size();
	for ( uIndexDistrictObj = 0; uIndexDistrictObj < uCountDistrictObj; ++uIndexDistrictObj ) {
		valDistrict = aryDistrictObjs[ uIndexDistrictObj ];
		if ( !valDistrict.isObject() ) {
			HM_LOADER_FAIL( false );
		}
		//
		valTmp = valDistrict[ "id" ];
		if ( !valTmp.isInt() ) {
			HM_LOADER_FAIL( false );
		}
		idDistrict = valTmp.asInt();
		//
		// fill showobj through the WMP proxy: store + uires subscriptions.
		valTmp = valDistrict[ "ShowObjs" ];
		if ( !_fillShowObjs( valTmp, pWMPDest, idDistrict ) ) {
			HM_LOADER_FAIL( false );
		}
	}

	return true;
}

bool hm_wmp_display_loader::_fillShowObjs( const Json::Value& aryShowObjs, hm_wmp_display* pWMPDest, hm_wmp_display::district_id_type idDistrict ) {
	if ( !aryShowObjs.isArray() || !pWMPDest ) {
		HM_LOADER_FAIL( false );
	}
	unsigned int uIndexShowObj, uCountShowObj;
	Json::Value valShowObj;
	Json::Value valTmp;
	hm_showobj* pShowObj = NULL;
	hm_rs_uires_i* pUIRes = NULL;

	uCountShowObj = aryShowObjs.size();
	for ( uIndexShowObj = 0; uIndexShowObj < uCountShowObj; ++uIndexShowObj ) {
		// fresh per showobj: a decoder-less showobj used to INHERIT the previous
		// one's decoder ( pPDC was initialized once outside the loop ) - a
		// silent wrong-position landmine.
		hm_pos_decoder_i* pPDC = NULL;
		valShowObj = aryShowObjs[ uIndexShowObj ];
		if ( !valShowObj.isObject() ) {
			HM_LOADER_FAIL( false );
		}
		valTmp = valShowObj[ "UIRes" ];
		if ( !_createUIRes( valTmp, &pUIRes ) ) {
			HM_LOADER_FAIL( false );
		}
		// may be need more code here, to create hm_pos_decoder_i.
		valTmp = valShowObj[ "PosDecoder" ];
		if ( !valTmp.isNull() ) {
			if ( !_createPosDecoder( valTmp, &pPDC ) ) {
				HM_LOADER_FAIL( false );
			}
		}
		//
		pShowObj = new hm_showobj( pUIRes, pPDC );
		if ( !pWMPDest->pushShowObj( idDistrict, pShowObj ) ) {
			HM_LOADER_FAIL( false );
		}
	}
	//

	return true;
}

bool hm_wmp_display_loader::_createPosDecoder( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( _createPosDecoder_Dock( valPDC, ppPDC ) ) {
		return true;
	} else if ( _createPosDecoder_Slider( valPDC, ppPDC ) ) {
		return true;
	} else if ( _createPosDecoder_Offset( valPDC, ppPDC ) ) {
		return true;
	}
	HM_LOADER_FAIL( false );
}

bool hm_wmp_display_loader::_createPosDecoder_Dock( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;
	hm_posdecoder_dock::ENUMDOCKTYPE eDockType;
	std::string strTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_dock" ) )
		return false;
	valTmp = valPDC[ "eDockType" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	strTmp = valTmp.asString().c_str();
	if ( 0 == strcmp( strTmp.c_str(), "DOCK_TOP" ) ) {
		eDockType = hm_posdecoder_dock::DOCK_TOP;
	} else if ( 0 == strcmp( strTmp.c_str(), "DOCK_LEFT" ) ) {
		eDockType = hm_posdecoder_dock::DOCK_LEFT;
	} else if ( 0 == strcmp( strTmp.c_str(), "DOCK_RIGHT" ) ) {
		eDockType = hm_posdecoder_dock::DOCK_RIGHT;
	} else if ( 0 == strcmp( strTmp.c_str(), "DOCK_BOTTOM" ) ) {
		eDockType = hm_posdecoder_dock::DOCK_BOTTOM;
	} else {
		HM_LOADER_FAIL( false );
	}
	//
	hm_posdecoder_dock* pPDCDock = new hm_posdecoder_dock( eDockType );
	//
	valTmp = valPDC[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCDock->m_space1 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCDock->m_space2 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space3" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCDock->m_space3 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nThickness" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCDock->m_nThickness = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCDock;
	return true;
}

bool hm_wmp_display_loader::_createPosDecoder_Slider( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;
	hm_posdecoder_slider::ENUMSLIDERTYPE eSliderType;
	std::string strTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_slider" ) )
		return false;
	valTmp = valPDC[ "eSliderType" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	strTmp = valTmp.asString().c_str();
	if ( 0 == strcmp( strTmp.c_str(), "SLIDER_T2B" ) ) {
		eSliderType = hm_posdecoder_slider::SLIDER_T2B;
	} else if ( 0 == strcmp( strTmp.c_str(), "SLIDER_L2R" ) ) {
		eSliderType = hm_posdecoder_slider::SLIDER_L2R;
	} else {
		HM_LOADER_FAIL( false );
	}
	//
	hm_posdecoder_slider* pPDCSlider = new hm_posdecoder_slider( eSliderType );
	//
	valTmp = valPDC[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCSlider->m_space1 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCSlider->m_space2 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_distance" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCSlider->m_distance = valTmp.asInt();
	//
	valTmp = valPDC[ "m_nThickness" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCSlider->m_nThickness = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCSlider;

	return true;
}

bool hm_wmp_display_loader::_createPosDecoder_Offset( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_offset" ) )
		return false;
	
	hm_posdecoder_offset* pPDCOffset = new hm_posdecoder_offset();
	valTmp = valPDC[ "m_nOffsetL" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCOffset->m_nOffsetL = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetT" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCOffset->m_nOffsetT = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetR" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCOffset->m_nOffsetR = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetB" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	pPDCOffset->m_nOffsetB = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCOffset;

	return true;
}

bool hm_wmp_display_loader::_createUIRes( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( _createUIRes_Pic( valUIRes, ppUIRes ) ) {
		return true;
	} else if ( _createUIRes_Color( valUIRes, ppUIRes ) ) {
		return true;
	} else if ( _createUIRes_Text( valUIRes, ppUIRes ) ) {
		return true;
	}

	// unknown uires type: returning TRUE here passed an unwritten out-param
	// ( a NULL uires ) downstream as a success - a latent null deref.
	HM_LOADER_FAIL( false );
}

bool hm_wmp_display_loader::_createUIRes_Pic( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( !ppUIRes ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;
	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_pic" ) )
		return false;
	//
	hm_rs_uires_pic* pUIResPic = NULL;
	hm_rawres_pic_factory::obj_id_type idRawPic;

	valTmp = valUIRes[ "RawPicId" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	idRawPic = valTmp.asInt();
	pUIResPic = new hm_rs_uires_pic( idRawPic );
	CFuncPack fpkDelUIRes( gfnDelAnyObj< hm_rs_uires_pic >, pUIResPic );

	//
	valTmp = valUIRes[ "m_rcSelect" ];
	if ( !valTmp.isNull() ) {
		if ( !_fillRect( valTmp, pUIResPic->m_rcSelect ) ) {
			HM_LOADER_FAIL( false );
		}
	}

	//
	valTmp = valUIRes[ "m_ufixL" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			HM_LOADER_FAIL( false );
		}
		pUIResPic->m_ufixL = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixR" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			HM_LOADER_FAIL( false );
		}
		pUIResPic->m_ufixR = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixT" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			HM_LOADER_FAIL( false );
		}
		pUIResPic->m_ufixT = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixB" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			HM_LOADER_FAIL( false );
		}
		pUIResPic->m_ufixB = valTmp.asInt();
	}

	//
	fpkDelUIRes.Cancel();
	*ppUIRes = pUIResPic;
	return true;
}

bool hm_wmp_display_loader::_createUIRes_Color( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( !ppUIRes ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;
	hm_rs_uires_color* pUIResColor = NULL;
	hm_rawres_color_factory::obj_id_type idRawColor;

	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_color" ) )
		return false;
	//
	valTmp = valUIRes[ "RawColorId" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	idRawColor = valTmp.asInt();
	//
	pUIResColor = new hm_rs_uires_color( idRawColor );
	*ppUIRes = pUIResColor;
	return true;
}

bool hm_wmp_display_loader::_createUIRes_Text( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( !ppUIRes ) {
		HM_LOADER_FAIL( false );
	}

	Json::Value valTmp;
	hm_rs_uires_text* pUIResText = NULL;
	hm_rawres_font_factory::obj_id_type idRawFont;
	hm_rawres_color_factory::obj_id_type idRawColor;
	hm_uires_text_data textData;

	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_text" ) )
		return false;
	//
	valTmp = valUIRes[ "Font" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	idRawFont = valTmp.asInt();
	//
	valTmp = valUIRes[ "Color" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	idRawColor = valTmp.asInt();
	//
	valTmp = valUIRes[ "text" ];
	if ( !valTmp.isString() ) {
		HM_LOADER_FAIL( false );
	}
	//
	MultiByteToTChar( valTmp.asString(), textData.m_tstrData );
	//
	valTmp = valUIRes[ "Alignment" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	textData.setAlignment( valTmp.asInt() );
	//
	textData.m_idRawResColor = idRawColor;
	textData.m_idRawResFont = idRawFont;
	pUIResText = new hm_rs_uires_text( textData );
	*ppUIRes = pUIResText;
	return true;
}

bool hm_wmp_display_loader::_fillRect( const Json::Value& valRect, RECT& rect ) {
	if ( !valRect.isObject() ) {
		HM_LOADER_FAIL( false );
	}
	Json::Value valTmp;

	//
	valTmp = valRect[ "L" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rect.left = valTmp.asInt();
	//
	valTmp = valRect[ "R" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rect.right = valTmp.asInt();
	//
	valTmp = valRect[ "T" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rect.top = valTmp.asInt();
	//
	valTmp = valRect[ "B" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rect.bottom = valTmp.asInt();

	return true;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_bkgnd_loader::hm_wmp_bkgnd_loader() {
}

hm_wmp_bkgnd_loader::~hm_wmp_bkgnd_loader() {
}

hm_wndmsgprocessor_i* hm_wmp_bkgnd_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_bkgnd );
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_bkgnd_nca_loader::hm_wmp_bkgnd_nca_loader() {

}

hm_wmp_bkgnd_nca_loader::~hm_wmp_bkgnd_nca_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_bkgnd_nca_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_bkgnd_nca );
}

bool hm_wmp_bkgnd_nca_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	if ( !__super::_fillWMP( valWMP, pWMPLoading ) ) {
		return false;
	}
	Json::Value valThick;
	Json::Value valTmp;
	hm_wmp_bkgnd_nca* pWMPBkgndNCA = NULL;

	pWMPBkgndNCA = dynamic_cast< hm_wmp_bkgnd_nca* >( pWMPLoading );
	if ( !pWMPBkgndNCA ) {
		HM_LOADER_FAIL( false );
	}
	valThick = valWMP[ STR_THICK ];
	if ( !valThick.isObject() ) {
		HM_LOADER_FAIL( false );
	}
	//
	valTmp = valThick[ "L" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickL( _ScalePx( m_dblScale, valTmp.asInt() ) );
	}
	//
	valTmp = valThick[ "R" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickR( _ScalePx( m_dblScale, valTmp.asInt() ) );
	}
	//
	valTmp = valThick[ "T" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickT( _ScalePx( m_dblScale, valTmp.asInt() ) );
	}
	//
	valTmp = valThick[ "B" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickB( _ScalePx( m_dblScale, valTmp.asInt() ) );
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_button_loader::hm_wmp_button_loader() {

}

hm_wmp_button_loader::~hm_wmp_button_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_button_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_button );
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_button_nca_loader::hm_wmp_button_nca_loader() {

}
	
hm_wmp_button_nca_loader::~hm_wmp_button_nca_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_button_nca_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_button_nca );
};
//////////////////////////////////////////////////////////////////////////
hm_wmp_drawtrigger_loader::hm_wmp_drawtrigger_loader() {

}

hm_wmp_drawtrigger_loader::~hm_wmp_drawtrigger_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_drawtrigger_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_drawtrigger );
};
//////////////////////////////////////////////////////////////////////////
hm_wmp_drawtrigger_nca_loader::hm_wmp_drawtrigger_nca_loader() {

}

hm_wmp_drawtrigger_nca_loader::~hm_wmp_drawtrigger_nca_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_drawtrigger_nca_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_drawtrigger_nca );
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_layouttrigger_loader::hm_wmp_layouttrigger_loader() {

}

hm_wmp_layouttrigger_loader::~hm_wmp_layouttrigger_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_layouttrigger_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_layouttrigger );
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_movewnd_loader::hm_wmp_movewnd_loader() {

}

hm_wmp_movewnd_loader::~hm_wmp_movewnd_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_movewnd_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	m_pUIRoot = pHMUIRoot;
	HM_WMP_X_LOADER_IMPL( hm_wmp_movewnd );
}

bool hm_wmp_movewnd_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	hm_wmp_movewnd* pWMPMoveWnd = NULL;

	pWMPMoveWnd = dynamic_cast< hm_wmp_movewnd* >( pWMPLoading );
	if ( !pWMPMoveWnd ) {
		HM_LOADER_FAIL( false );
	}
	//
	if ( !_fillWndMark( valWMP, pWMPMoveWnd ) ) {
		return false;
	}
	//
	pWMPMoveWnd->setUIRoot( m_pUIRoot );

	return true;
}

bool hm_wmp_movewnd_loader::_fillWndMark( const Json::Value& valWMP, hm_wmp_movewnd* pWMPMoveWnd ) {
	Json::Value valWndMark;
	Json::Value valTmp;
	hm_rs_wndmark wndMark;

	valWndMark = valWMP[ STR_WNDMARK ];
	if ( !valWndMark.isObject() ) {
		HM_LOADER_FAIL( false );
	}
	// CtrlId
	valTmp = valWndMark[ STR_CTRLID ];
	if ( valTmp.isInt() ) {
		wndMark.m_eflag = hm_rs_wndmark::WNDMARK_CTRLID;
		wndMark.m_uCtrlID = valTmp.asInt();
		pWMPMoveWnd->setWndMark( wndMark );
		return true;
	} 
	// PropId
	valTmp = valWndMark[ STR_PROPID ];
	if ( valTmp.isInt() ) {
		wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
		wndMark.m_uWndPropID = valTmp.asInt();
		pWMPMoveWnd->setWndMark( wndMark );
		return true;
	} 

	HM_LOADER_FAIL( false );
}

//////////////////////////////////////////////////////////////////////////
hm_wmp_popupmenu_loader::hm_wmp_popupmenu_loader() {

}

hm_wmp_popupmenu_loader::~hm_wmp_popupmenu_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_popupmenu_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { HM_LOADER_FAIL( NULL ); }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_popupmenu" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { HM_LOADER_FAIL( NULL ); }
	idWMP = valTmp.asInt();
	// MenuCfg: item size + spacing ( defaults 25 x 160, space 6 ).
	unsigned int uMIHeight = 25;
	unsigned int uMIWidth = 160;
	unsigned int uSpace = 6;
	// ( defaults scaled too - they are geometry )
	valTmp = valWMP[ "MenuCfg" ];
	hm_rs_rawres_font* pRawFontMenu = NULL;
	bool bBadColor = false;
	bool bHasText = false, bHasTextDis = false, bHasSep = false;
	COLORREF clrText = 0, clrTextDis = 0, clrSep = 0;
	if ( valTmp.isObject() ) {
		Json::Value valCfgItem;
		valCfgItem = valTmp[ "m_uMIHeight" ]; if ( valCfgItem.isInt() ) uMIHeight = valCfgItem.asInt();
		valCfgItem = valTmp[ "m_uMIWidth" ]; if ( valCfgItem.isInt() ) uMIWidth = valCfgItem.asInt();
		valCfgItem = valTmp[ "m_uSpace" ]; if ( valCfgItem.isInt() ) uSpace = valCfgItem.asInt();
		// optional Font section reference for the menu labels ( default:
		// the built-in shared menu font ).
		valCfgItem = valTmp[ "FontId" ];
		if ( valCfgItem.isInt() ) {
			pRawFontMenu = RAWFONTFACTORY_INSTANCE->getObj(
				(hm_rawres_font_factory::obj_id_type)valCfgItem.asInt() );
			if ( !pRawFontMenu ) {   // FontId names a font the manifest never declared
				HM_LOADER_FAIL( NULL );
			}
		}
		// optional Color section references ( defaults: hm_ui_defaults.h ).
		bHasText = _MenuCfgColor( valTmp, "TextColor", bBadColor, &clrText );
		bHasTextDis = _MenuCfgColor( valTmp, "TextColorDisabled", bBadColor, &clrTextDis );
		bHasSep = _MenuCfgColor( valTmp, "SeparatorColor", bBadColor, &clrSep );
		if ( bBadColor )
			return NULL;
	}
	dimension_menuwnd_default* pDimensionMenuWnd = new dimension_menuwnd_default(
		_ScalePx( m_dblScale, (int)uMIHeight ), _ScalePx( m_dblScale, (int)uMIWidth ), _ScalePx( m_dblScale, (int)uSpace ) );
	hm_wmp_popupmenu* pWMPPopup = pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_popupmenu, dimension_menuwnd_i*, hmskin_event_listener_base* >(
		idWMP, ( dimension_menuwnd_i* )pDimensionMenuWnd, pHMUIRoot->m_pEventListener );
	if ( pRawFontMenu )
		pWMPPopup->setLabelFont( pRawFontMenu );
	if ( bHasText && bHasTextDis )
		pWMPPopup->setLabelColors( clrText, clrTextDis );
	if ( bHasSep )
		pWMPPopup->setSeparatorColor( clrSep );
	return pWMPPopup;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_rootmenu_loader::hm_wmp_rootmenu_loader() {

}

hm_wmp_rootmenu_loader::~hm_wmp_rootmenu_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_rootmenu_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_rootmenu );
}

bool hm_wmp_rootmenu_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	if ( !__super::_fillWMP( valWMP, pWMPLoading ) ) {
		return false;
	}
	hm_wmp_rootmenu* pWMPRootMenu = NULL;
	Json::Value valRMCfg;
	Json::Value valTmp;
	RootMenuCfg rmCfg;

	pWMPRootMenu = dynamic_cast< hm_wmp_rootmenu* >(pWMPLoading);
	if ( !pWMPRootMenu ) {
		HM_LOADER_FAIL( false );
	}
	valRMCfg = valWMP[ STR_ROOTMENUCFG ];
	//
	valTmp = valRMCfg[ "m_uLenMenuItem" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rmCfg.m_uLenMenuItem = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valRMCfg[ "m_uSpaceH" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rmCfg.m_uSpaceH = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valRMCfg[ "m_uSpaceV" ];
	if ( !valTmp.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	rmCfg.m_uSpaceV = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	pWMPRootMenu->setConfigure( rmCfg );
	// optional Font section reference for the menu labels ( default: the
	// built-in shared menu font ).
	valTmp = valRMCfg[ "FontId" ];
	if ( valTmp.isInt() ) {
		hm_rs_rawres_font* pRawFontMenu = RAWFONTFACTORY_INSTANCE->getObj(
			(hm_rawres_font_factory::obj_id_type)valTmp.asInt() );
		if ( !pRawFontMenu ) {   // FontId names a font the manifest never declared
			HM_LOADER_FAIL( false );
		}
		pWMPRootMenu->setLabelFont( pRawFontMenu );
	}
	// optional Color section references ( defaults: hm_ui_defaults.h ).
	{
		bool bBadColor = false;
		COLORREF clrText = 0, clrTextDis = 0;
		bool bHasText = _MenuCfgColor( valRMCfg, "TextColor", bBadColor, &clrText );
		bool bHasTextDis = _MenuCfgColor( valRMCfg, "TextColorDisabled", bBadColor, &clrTextDis );
		if ( bBadColor ) {
			HM_LOADER_FAIL( false );
		}
		if ( bHasText && bHasTextDis )
			pWMPRootMenu->setLabelColors( clrText, clrTextDis );
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_scrollbar_loader::hm_wmp_scrollbar_loader() {

}

hm_wmp_scrollbar_loader::~hm_wmp_scrollbar_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_scrollbar_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { HM_LOADER_FAIL( NULL ); }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_scrollbar" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { HM_LOADER_FAIL( NULL ); }
	idWMP = valTmp.asInt();
	// ScrollBarCfg: direction ( "T2B" default | "L2R" ).
	hm_wmp_scrollbar::ENUMDIRECTIONTYPE eDirection = hm_wmp_scrollbar::T2B_TYPE;
	valTmp = valWMP[ "ScrollBarCfg" ];
	if ( valTmp.isObject() ) {
		Json::Value valDir = valTmp[ "Direction" ];
		if ( valDir.isString() && valDir.asString() == "L2R" )
			eDirection = hm_wmp_scrollbar::L2R_TYPE;
	}
	return pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_scrollbar, hm_wmp_scrollbar::ENUMDIRECTIONTYPE, hmskin_event_listener_base* >(
		idWMP, eDirection, pHMUIRoot->m_pEventListener );
};
//////////////////////////////////////////////////////////////////////////
hm_wmp_scrollview_loader::hm_wmp_scrollview_loader() {

}

hm_wmp_scrollview_loader::~hm_wmp_scrollview_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_scrollview_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { HM_LOADER_FAIL( NULL ); }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_scrollview" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { HM_LOADER_FAIL( NULL ); }
	idWMP = valTmp.asInt();

	// ScrollViewCfg: binds this orchestrator to a manifest-declared subtree.
	hm_wmp_scrollview::SCROLLVIEW_CFG cfg;
	valTmp = valWMP[ "ScrollViewCfg" ];
	if ( !valTmp.isObject() ) { HM_LOADER_FAIL( NULL ); }
	Json::Value valCfg = valTmp;
	Json::Value v;
	// ScrollBarWMPId ( required ) - shutdown contract: referenced WMP ids
	// must be SMALLER than ours ( factory destroys by ascending id ).
	v = valCfg[ "ScrollBarWMPId" ];
	if ( !v.isInt() ) { HM_LOADER_FAIL( NULL ); }
	cfg.m_idWMPSearchBar = v.asInt();
	assert( cfg.m_idWMPSearchBar < idWMP );
	// ContentKeelObjId ( required ).
	v = valCfg[ "ContentKeelObjId" ];
	if ( !v.isInt() ) { HM_LOADER_FAIL( NULL ); }
	cfg.m_idKeelObjContent = v.asInt();
	// BarKeelObjId ( optional: wheel hit-test exclusion + track measure ).
	v = valCfg[ "BarKeelObjId" ];
	if ( v.isInt() ) cfg.m_idKeelObjBar = v.asInt();
	// ContentWMPId ( optional: dynamic CONTENT_EXTENT source ).
	v = valCfg[ "ContentWMPId" ];
	if ( v.isInt() ) {
		cfg.m_idWMPContent = v.asInt();
		assert( cfg.m_idWMPContent < idWMP );   // shutdown contract
	}
	// ContentExtent96 ( optional: static extent; 0 = measure at runtime ).
	v = valCfg[ "ContentExtent96" ];
	if ( v.isObject() ) {
		Json::Value ve = v[ "W" ];
		if ( ve.isInt() && ve.asInt() > 0 )
			cfg.m_uContentExtentW = (unsigned int)_ScalePx( m_dblScale, ve.asInt() );
		ve = v[ "H" ];
		if ( ve.isInt() && ve.asInt() > 0 )
			cfg.m_uContentExtentH = (unsigned int)_ScalePx( m_dblScale, ve.asInt() );
	}
	// ArrowLen ( optional, default 20 = sbbtn bitmap size ).
	v = valCfg[ "ArrowLen" ];
	if ( v.isInt() && v.asInt() > 0 )
		cfg.m_uArrowLen = (unsigned int)_ScalePx( m_dblScale, v.asInt() );
	// MinThumbLen ( optional, default 20 ).
	v = valCfg[ "MinThumbLen" ];
	if ( v.isInt() && v.asInt() > 0 )
		cfg.m_uMinThumbLen = (unsigned int)_ScalePx( m_dblScale, v.asInt() );
	// FixedThumbLen ( optional; absent = proportional thumb ).
	v = valCfg[ "FixedThumbLen" ];
	if ( v.isInt() && v.asInt() > 0 )
		cfg.m_nFixedThumbLen = _ScalePx( m_dblScale, v.asInt() );
	// LayoutScroll ( optional: translate the content layout on scroll ).
	v = valCfg[ "LayoutScroll" ];
	if ( v.isBool() )
		cfg.m_bLayoutScroll = v.asBool() ? TRUE : FALSE;
	// WheelStep ( optional: px per wheel notch, default 60 ).
	v = valCfg[ "WheelStep" ];
	if ( v.isInt() && v.asInt() > 0 )
		cfg.m_nWheelStep = _ScalePx( m_dblScale, v.asInt() );
	// AnchorBottom ( optional: chat-list extent-growth behavior ).
	v = valCfg[ "AnchorBottom" ];
	if ( v.isBool() )
		cfg.m_bAnchorBottom = v.asBool() ? TRUE : FALSE;

	return pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_scrollview, const hm_wmp_scrollview::SCROLLVIEW_CFG&, hm_uiroot* >(
		idWMP, cfg, pHMUIRoot );
};
//////////////////////////////////////////////////////////////////////////
hm_wmp_msgout_loader::hm_wmp_msgout_loader() {

}

hm_wmp_msgout_loader::~hm_wmp_msgout_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_msgout_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { HM_LOADER_FAIL( NULL ); }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_msgout" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { HM_LOADER_FAIL( NULL ); }
	idWMP = valTmp.asInt();

	hm_wmp_msgout* pWMPMsgOut = pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_msgout, hm_uiroot*, hmskin_event_listener_base* >(
		idWMP, pHMUIRoot, pHMUIRoot->m_pEventListener );

	// MsgOutCfg: BarWMPId required ( self-owned scrolling - must be < idWMP,
	// shutdown contract ); the rest optional, WeChat-style defaults.
	hm_wmp_msgout::MSGOUT_CFG cfg;
	valTmp = valWMP[ "MsgOutCfg" ];
	if ( valTmp.isObject() ) {
		Json::Value valCfg = valTmp;
		Json::Value v;
		v = valCfg[ "BarWMPId" ];
		if ( !v.isInt() ) { HM_LOADER_FAIL( NULL ); }
		cfg.m_idWMPSearchBar = v.asInt();
		assert( cfg.m_idWMPSearchBar < idWMP );
		v = valCfg[ "BarKeelObjId" ];
		if ( v.isInt() ) cfg.m_idKeelObjBar = v.asInt();
		// FontId -> the raw-font factory ( borrowed HFONT, menu contract ).
		v = valCfg[ "FontId" ];
		if ( v.isInt() ) {
			hm_rs_rawres_font* pRawFont = RAWFONTFACTORY_INSTANCE->getObj(
				(hm_rawres_font_factory::obj_id_type)v.asInt() );
			if ( !pRawFont ) {   // FontId names a font the manifest never declared
				HM_LOADER_FAIL( NULL );
			}
			cfg.m_hFontText = pRawFont->getHFont();
			cfg.m_pRawFontText = pRawFont;   // drawtool text path
		}
		// colors ( Color section ids ).
		{
			bool bBadColor = false;
			struct { const char* m_szKey; COLORREF* m_pClr; } aryclr[] = {
				{ "TextColor", &cfg.m_clrText },
				{ "SelfBubbleColor", &cfg.m_clrBubbleSelf },
				{ "PeerBubbleColor", &cfg.m_clrBubblePeer },
				{ "SystemColor", &cfg.m_clrSystem },
				{ "SelfAvatarColor", &cfg.m_clrAvatarSelf },
				{ "PeerAvatarColor", &cfg.m_clrAvatarPeer },
			};
			for ( int i = 0; i < 6; ++i ) {
				COLORREF clrDump;
				Json::Value vKey = valCfg[ aryclr[ i ].m_szKey ];
				hm_rs_rawres_color* pRawClr = vKey.isInt() ? RAWCOLORFACTORY_INSTANCE->getObj(
					(hm_rawres_color_factory::obj_id_type)vKey.asInt() ) : NULL;
				if ( !_MenuCfgColor( valCfg, aryclr[ i ].m_szKey, bBadColor, aryclr[ i ].m_pClr ) && bBadColor ) {
					assert( false );   // the id names a color the manifest never declared
					HM_LOADER_FAIL( NULL );
				}
				if ( pRawClr ) {
					if ( 0 == strcmp( aryclr[ i ].m_szKey, "TextColor" ) )
						cfg.m_pRawClrText = pRawClr;
					else if ( 0 == strcmp( aryclr[ i ].m_szKey, "SystemColor" ) )
						cfg.m_pRawClrSystem = pRawClr;
				}
				UNREFERENCED_PARAMETER( clrDump );
			}
		}
		// geometry ( 96-baseline, scaled here ). the ctor defaults ARE the 96
		// values: override them with the manifest, then scale everything - an
		// absent key must not leave unscaled px in the cfg.
		int nAvatarLen = cfg.m_nAvatarLen, nMsgGap = cfg.m_nMsgGap, nPadH = cfg.m_nPadH;
		int nPadV = cfg.m_nPadV, nCornerLen = cfg.m_nCornerLen, nWheelStep = cfg.m_nWheelStep;
		int nFixedThumbLen = cfg.m_nFixedThumbLen;
		unsigned int uMinThumbLen = cfg.m_uMinThumbLen, uArrowLen = cfg.m_uArrowLen;
		v = valCfg[ "AvatarLen" ];
		if ( v.isInt() && v.asInt() > 0 ) nAvatarLen = v.asInt();
		v = valCfg[ "MaxBubblePct" ];
		if ( v.isInt() && v.asInt() >= 10 && v.asInt() <= 100 ) cfg.m_nMaxBubblePct = v.asInt();
		v = valCfg[ "MsgGap" ];
		if ( v.isInt() && v.asInt() >= 0 ) nMsgGap = v.asInt();
		v = valCfg[ "PadH" ];
		if ( v.isInt() && v.asInt() > 0 ) nPadH = v.asInt();
		v = valCfg[ "PadV" ];
		if ( v.isInt() && v.asInt() > 0 ) nPadV = v.asInt();
		v = valCfg[ "CornerLen" ];
		if ( v.isInt() && v.asInt() > 0 ) nCornerLen = v.asInt();
		v = valCfg[ "WheelStep" ];
		if ( v.isInt() && v.asInt() > 0 ) nWheelStep = v.asInt();
		v = valCfg[ "FixedThumbLen" ];
		if ( v.isInt() && v.asInt() > 0 ) nFixedThumbLen = v.asInt();
		v = valCfg[ "MinThumbLen" ];
		if ( v.isInt() && v.asInt() > 0 ) uMinThumbLen = (unsigned int)v.asInt();
		v = valCfg[ "ArrowLen" ];
		if ( v.isInt() && v.asInt() > 0 ) uArrowLen = (unsigned int)v.asInt();
		cfg.m_nAvatarLen = _ScalePx( m_dblScale, nAvatarLen );
		cfg.m_nMsgGap = _ScalePx( m_dblScale, nMsgGap );
		cfg.m_nPadH = _ScalePx( m_dblScale, nPadH );
		cfg.m_nPadV = _ScalePx( m_dblScale, nPadV );
		cfg.m_nCornerLen = _ScalePx( m_dblScale, nCornerLen );
		cfg.m_nWheelStep = _ScalePx( m_dblScale, nWheelStep );
		cfg.m_nFixedThumbLen = nFixedThumbLen > 0 ? _ScalePx( m_dblScale, nFixedThumbLen ) : nFixedThumbLen;   // <0 = proportional
		cfg.m_uMinThumbLen = (unsigned int)_ScalePx( m_dblScale, (int)uMinThumbLen );
		cfg.m_uArrowLen = (unsigned int)_ScalePx( m_dblScale, (int)uArrowLen );
	}

	pWMPMsgOut->setConfigure( cfg );
	return pWMPMsgOut;
};
//////////////////////////////////////////////////////////////////////////
hm_wmp_tabheader_loader::hm_wmp_tabheader_loader() {

}

hm_wmp_tabheader_loader::~hm_wmp_tabheader_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_tabheader_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_tabheader );
}

bool hm_wmp_tabheader_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	if ( !__super::_fillWMP( valWMP, pWMPLoading ) ) {
		return false;
	}
	hm_wmp_tabheader* pWMPTabHeader = NULL;

	pWMPTabHeader = dynamic_cast< hm_wmp_tabheader* >( pWMPLoading );
	_fillBtns( valWMP, pWMPTabHeader );
	return true;
}

bool hm_wmp_tabheader_loader::_fillBtns( const Json::Value& valWMP, hm_wmp_tabheader* pWMPLoading ) {
	Json::Value aryTabBtns;
	int nCountTabBtn;
	int nIndexTabBtn;
	int nIndexTabBtnDown;
	Json::Value valTabBtn;
	Json::Value valTitle;
	Json::Value valDefaultBtn;
	tabbtninfo* pTabBtnInfo = NULL;

	aryTabBtns = valWMP[ STR_TABBTNS ];
	if ( !aryTabBtns.isArray() || !pWMPLoading ) {
		HM_LOADER_FAIL( false );
	}
	//
	nCountTabBtn = aryTabBtns.size();
	for ( nIndexTabBtn = 0; nIndexTabBtn<nCountTabBtn; ++nIndexTabBtn ) {
		valTabBtn = aryTabBtns[ nIndexTabBtn ];
		if ( !valTabBtn.isObject() ) {
			HM_LOADER_FAIL( false );
		}
		valTitle = valTabBtn["Title"];
		if ( !valTitle.isString() ) {
			HM_LOADER_FAIL( false );
		}
		//
		pTabBtnInfo = new tabbtninfo();
		MultiByteToTChar( valTitle.asString(), pTabBtnInfo->m_strTitle );
		pWMPLoading->pushbackTabBtn( pTabBtnInfo );
	}

	valDefaultBtn = valWMP[ "DefaultBtn" ];
	if ( !valDefaultBtn.isInt() ) {
		HM_LOADER_FAIL( false );
	}
	nIndexTabBtnDown = valDefaultBtn.asInt();
	pWMPLoading->pushDown( nIndexTabBtnDown );

	return true;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_viewbkcontent_loader::hm_wmp_viewbkcontent_loader() {

}

hm_wmp_viewbkcontent_loader::~hm_wmp_viewbkcontent_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_viewbkcontent_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_viewbkcontent );
}

//////////////////////////////////////////////////////////////////////////
hm_wmp_checkbox_loader::hm_wmp_checkbox_loader() {

}

hm_wmp_checkbox_loader::~hm_wmp_checkbox_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_checkbox_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_checkbox );
}

//////////////////////////////////////////////////////////////////////////
hm_wmp_radio_loader::hm_wmp_radio_loader() {

}

hm_wmp_radio_loader::~hm_wmp_radio_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_radio_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	HM_WMP_X_LOADER_IMPL( hm_wmp_radio );
}