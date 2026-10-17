#include "stdafx.h"
#include "win\loader\hm_wmp_loaders.h"
#include "win\hm_win_misc.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_bkgnd.h"
#include "win\wmp\hm_wmp_bkgnd_nca.h"
#include "win\wmp\hm_wmp_button.h"
#include "win\wmp\hm_wmp_button_nca.h"
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\wmp\hm_wmp_radio.h"
#include "win\wmp\hm_wmp_dialogview.h"
#include "win\wmp\hm_wmp_drawtrigger.h"
#include "win\wmp\hm_wmp_drawtrigger_nca.h"
#include "win\wmp\hm_wmp_layouttrigger.h"
#include "win\wmp\hm_wmp_popupmenu.h"
#include "win\wmp\hm_wmp_rootmenu.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\wmp\hm_wmp_tab.h"
#include "win\wmp\hm_wmp_viewbkcontent.h"
#include "win\wmp\hm_wmp_movewnd.h"
#include "win\resource_system\hm_rs_uires_pic.h"
#include "win\resource_system\hm_rs_uires_color.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
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

const char STR_DRAWTOOL[] = "DrawTool";
const char STR_DISTRICTOBJS[] = "DistrictObjs";
const char STR_ROOTMENUCFG[] = "RootMenuCfg";
const char STR_TABBTNS[] = "TabBtns";
const char STR_THICK[] = "Thick";
const char STR_WNDMARK[] = "WndMark";
const char STR_CTRLID[] = "CtrlId";
const char STR_PROPID[] = "PropId";
//
#define HM_WMP_X_LOADER_IMPL( hm_wmp_cls_name ) \
	if ( !pHMUIRoot ) { assert( false ); return NULL; } \
	Json::Value valTmp; \
	hm_wmp_factory::obj_id_type idWMP = 0; \
	std::string strWMPClsName; \
	hm_wndmsgprocessor_i* pWMPNew = NULL; \
	assert( _getWMPName( valWMP, strWMPClsName ) ); \
	assert( strWMPClsName == #hm_wmp_cls_name ); \
	valTmp = valWMP[ "idWMP" ]; \
	if ( !valTmp.isInt() ) { assert( false ); return NULL; }\
	idWMP = valTmp.asInt(); \
	pWMPNew = pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_cls_name, hmskin_event_listener_base* >( idWMP, pHMUIRoot->m_pEventListener ); \
	return pWMPNew;
//////////////////////////////////////////////////////////////////////////
bool hm_wmp_display_loader::_fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) {
	hm_wmp_display* pWMPDisplay = NULL;

	// get pWMPDisplay
	pWMPDisplay = dynamic_cast< hm_wmp_display* >( pWMPLoading );
	if ( !pWMPDisplay ) {
		assert( false );
		return false;
	}
	// fill drawtool to pWMPDisplay.
	if ( !_fillDrawTool( valWMP, pWMPDisplay ) ) {
		assert( false );
		return false;
	}
	// fill district objects to pWMPDisplay.
	if ( !_fillDistrictObjs( valWMP, pWMPDisplay ) ) {
		assert( false );
		return false;
	}

	return true;
}

bool hm_wmp_display_loader::_fillDrawTool( const Json::Value& valWMP, hm_wmp_display* pWMPDest ) {
	Json::Value valDrawTool;
	hm_ps_drawtool_i* pDrawTool = NULL;
	std::string strDrawTool;

	valDrawTool = valWMP[ STR_DRAWTOOL ];
	if ( !valDrawTool.isString() ) {
		assert( false );
		return false;
	}
	strDrawTool = valDrawTool.asString();
	if ( 0 == strcmp( strDrawTool.c_str(), "Gdiplus" ) ) {
		pDrawTool = hm_ps_drawtool_gdiplus::getInstance();
	} else if ( 0 == strcmp( strDrawTool.c_str(), "Gdi" ) ) {
		pDrawTool = hm_ps_drawtool_gdi::getInstance();
	} else {
		assert( false );
		return false;
	}

	pWMPDest->setDrawTool( pDrawTool );
	return true;
}

bool hm_wmp_display_loader::_fillDistrictObjs( const Json::Value& valWMP, hm_wmp_display* pWMPDest ) {
	unsigned int uIndexDistrictObj, uCountDistrictObj;
	Json::Value aryDistrictObjs;
	Json::Value valDistrict;
	Json::Value valTmp;
	hm_wmp_display::district_id_type idDistrict;
	hm_districtobj* pDistrictObj = NULL;

	aryDistrictObjs = valWMP[ STR_DISTRICTOBJS ];
	if ( !aryDistrictObjs.isArray() || !pWMPDest ) {
		assert( false );
		return false;
	}
	uCountDistrictObj = aryDistrictObjs.size();
	for ( uIndexDistrictObj = 0; uIndexDistrictObj < uCountDistrictObj; ++uIndexDistrictObj ) {
		valDistrict = aryDistrictObjs[ uIndexDistrictObj ];
		if ( !valDistrict.isObject() ) {
			assert( false );
			return false;
		}
		//
		valTmp = valDistrict[ "id" ];
		if ( !valTmp.isInt() ) {
			assert( false );
			return false;
		}
		idDistrict = valTmp.asInt();
		//
		if ( !pWMPDest->getDistrictObj( idDistrict, &pDistrictObj ) ) {
			assert( false );
			return false;
		}
		// fill showobj to district object.
		valTmp = valDistrict[ "ShowObjs" ];
		if ( !_fillShowObjs( valTmp, pDistrictObj ) ) {
			assert( false );
			return false;
		}
	}

	return true;
}

bool hm_wmp_display_loader::_fillShowObjs( const Json::Value& aryShowObjs, hm_districtobj* pDistrict ) {
	if ( !aryShowObjs.isArray() || !pDistrict ) {
		assert( false );
		return false;
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
			assert( false );
			return false;
		}
		valTmp = valShowObj[ "UIRes" ];
		if ( !_createUIRes( valTmp, &pUIRes ) ) {
			assert( false );
			return false;
		}
		// may be need more code here, to create hm_pos_decoder_i.
		valTmp = valShowObj[ "PosDecoder" ];
		if ( !valTmp.isNull() ) {
			if ( !_createPosDecoder( valTmp, &pPDC ) ) {
				assert( false );
				return false;
			}
		}
		//
		pShowObj = new hm_showobj( pUIRes, pPDC );
		pDistrict->pushBackShowObj( pShowObj );
	}
	//

	return true;
}

bool hm_wmp_display_loader::_createPosDecoder( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( _createPosDecoder_Dock( valPDC, ppPDC ) ) {
		return true;
	} else if ( _createPosDecoder_Slider( valPDC, ppPDC ) ) {
		return true;
	} else if ( _createPosDecoder_Stack( valPDC, ppPDC ) ) {
		return true;
	} else if ( _createPosDecoder_Offset( valPDC, ppPDC ) ) {
		return true;
	}
	assert( false );
	return false;
}

bool hm_wmp_display_loader::_createPosDecoder_Dock( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		assert( false );
		return false;
	}
	Json::Value valTmp;
	hm_posdecoder_dock::ENUMDOCKTYPE eDockType;
	std::string strTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_dock" ) )
		return false;
	valTmp = valPDC[ "eDockType" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
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
		assert( false );
		return false;
	}
	//
	hm_posdecoder_dock* pPDCDock = new hm_posdecoder_dock( eDockType );
	//
	valTmp = valPDC[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCDock->m_space1 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCDock->m_space2 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space3" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCDock->m_space3 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nThickness" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCDock->m_nThickness = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCDock;
	return true;
}

bool hm_wmp_display_loader::_createPosDecoder_Slider( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		assert( false );
		return false;
	}
	Json::Value valTmp;
	hm_posdecoder_slider::ENUMSLIDERTYPE eSliderType;
	std::string strTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_slider" ) )
		return false;
	valTmp = valPDC[ "eSliderType" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	strTmp = valTmp.asString().c_str();
	if ( 0 == strcmp( strTmp.c_str(), "SLIDER_T2B" ) ) {
		eSliderType = hm_posdecoder_slider::SLIDER_T2B;
	} else if ( 0 == strcmp( strTmp.c_str(), "SLIDER_L2R" ) ) {
		eSliderType = hm_posdecoder_slider::SLIDER_L2R;
	} else {
		assert( false );
		return false;
	}
	//
	hm_posdecoder_slider* pPDCSlider = new hm_posdecoder_slider( eSliderType );
	//
	valTmp = valPDC[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCSlider->m_space1 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCSlider->m_space2 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_distance" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCSlider->m_distance = valTmp.asInt();
	//
	valTmp = valPDC[ "m_nThickness" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCSlider->m_nThickness = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCSlider;

	return true;
}

bool hm_wmp_display_loader::_createPosDecoder_Stack( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		assert( false );
		return false;
	}
	Json::Value valTmp;
	hm_posdecoder_stack::ENUMDIRECTIONTYPE eStackType;
	std::string strTmp;
	int* pctxStartPos;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_stack" ) )
		return false;
	valTmp = valPDC[ "eStackType" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	strTmp = valTmp.asString().c_str();
	if ( 0 == strcmp( strTmp.c_str(), "STACK_T2B" ) ) {
		eStackType = hm_posdecoder_stack::STACK_T2B;
	} else if ( 0 == strcmp( strTmp.c_str(), "STACK_B2T" ) ) {
		eStackType = hm_posdecoder_stack::STACK_B2T;
	} else if ( 0 == strcmp( strTmp.c_str(), "STACK_L2R" ) ) {
		eStackType = hm_posdecoder_stack::STACK_L2R;
	} else if ( 0 == strcmp( strTmp.c_str(), "STACK_R2L" ) ) {
		eStackType = hm_posdecoder_stack::STACK_R2L;
	} else {
		assert( false );
		return false;
	}
	//
	valTmp = valPDC[ "m_pctxStartPos" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pctxStartPos = (int*)valTmp.asInt();
	//
	hm_posdecoder_stack* pPDCStack = new hm_posdecoder_stack( eStackType, pctxStartPos );
	//
	valTmp = valPDC[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCStack->m_space1 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCStack->m_space2 = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nThickness" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCStack->m_nThickness = _ScalePx( m_dblScale, valTmp.asInt() );

	*ppPDC = pPDCStack;

	return true;
}

bool hm_wmp_display_loader::_createPosDecoder_Offset( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC ) {
	if ( !ppPDC ) {
		assert( false );
		return false;
	}
	Json::Value valTmp;

	valTmp = valPDC[ "PDCName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_posdecoder_offset" ) )
		return false;
	
	hm_posdecoder_offset* pPDCOffset = new hm_posdecoder_offset();
	valTmp = valPDC[ "m_nOffsetL" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCOffset->m_nOffsetL = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetT" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCOffset->m_nOffsetT = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetR" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	pPDCOffset->m_nOffsetR = _ScalePx( m_dblScale, valTmp.asInt() );
	//
	valTmp = valPDC[ "m_nOffsetB" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
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

	assert( false );
	return true;
}

bool hm_wmp_display_loader::_createUIRes_Pic( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( !ppUIRes ) {
		assert( false );
		return false;
	}
	Json::Value valTmp;
	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_pic" ) )
		return false;
	//
	hm_rs_uires_pic* pUIResPic = NULL;
	hm_rawres_pic_factory::obj_id_type idRawPic;

	valTmp = valUIRes[ "RawPicId" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	idRawPic = valTmp.asInt();
	pUIResPic = new hm_rs_uires_pic( idRawPic );
	CFuncPack fpkDelUIRes( gfnDelAnyObj< hm_rs_uires_pic >, pUIResPic );

	//
	valTmp = valUIRes[ "m_rcSelect" ];
	if ( !valTmp.isNull() ) {
		if ( !_fillRect( valTmp, pUIResPic->m_rcSelect ) ) {
			assert( false );
			return false;
		}
	}

	//
	valTmp = valUIRes[ "m_ufixL" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			assert( false );
			return false;
		}
		pUIResPic->m_ufixL = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixR" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			assert( false );
			return false;
		}
		pUIResPic->m_ufixR = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixT" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			assert( false );
			return false;
		}
		pUIResPic->m_ufixT = valTmp.asInt();
	}
	//
	valTmp = valUIRes[ "m_ufixB" ];
	if ( !valTmp.isNull() ) {
		if ( !valTmp.isInt() ) {
			assert( false );
			return false;
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
		assert( false );
		return false;
	}
	Json::Value valTmp;
	hm_rs_uires_color* pUIResColor = NULL;
	hm_rawres_color_factory::obj_id_type idRawColor;

	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_color" ) )
		return false;
	//
	valTmp = valUIRes[ "RawColorId" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	idRawColor = valTmp.asInt();
	//
	pUIResColor = new hm_rs_uires_color( idRawColor );
	*ppUIRes = pUIResColor;
	return true;
}

bool hm_wmp_display_loader::_createUIRes_Text( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes ) {
	if ( !ppUIRes ) {
		assert( false );
		return false;
	}

	Json::Value valTmp;
	hm_rs_uires_text* pUIResText = NULL;
	hm_rawres_font_factory::obj_id_type idRawFont;
	hm_rawres_color_factory::obj_id_type idRawColor;
	hm_uires_text_data textData;

	valTmp = valUIRes[ "UIResName" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	if ( 0 != strcmp( valTmp.asString().c_str(), "hm_rs_uires_text" ) )
		return false;
	//
	valTmp = valUIRes[ "Font" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	idRawFont = valTmp.asInt();
	//
	valTmp = valUIRes[ "Color" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	idRawColor = valTmp.asInt();
	//
	valTmp = valUIRes[ "text" ];
	if ( !valTmp.isString() ) {
		assert( false );
		return false;
	}
	//
	MultiByteToTChar( valTmp.asString(), textData.m_tstrData );
	//
	valTmp = valUIRes[ "Alignment" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
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
		assert( false );
		return false;
	}
	Json::Value valTmp;

	//
	valTmp = valRect[ "L" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rect.left = valTmp.asInt();
	//
	valTmp = valRect[ "R" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rect.right = valTmp.asInt();
	//
	valTmp = valRect[ "T" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rect.top = valTmp.asInt();
	//
	valTmp = valRect[ "B" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
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
		assert( false );
		return false;
	}
	valThick = valWMP[ STR_THICK ];
	if ( !valThick.isObject() ) {
		assert( false );
		return false;
	}
	//
	valTmp = valThick[ "L" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickL( valTmp.asInt() );
	}
	//
	valTmp = valThick[ "R" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickR( valTmp.asInt() );
	}
	//
	valTmp = valThick[ "T" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickT( valTmp.asInt() );
	}
	//
	valTmp = valThick[ "B" ];
	if ( valTmp.isInt() ) {
		pWMPBkgndNCA->setThickB( valTmp.asInt() );
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
hm_wmp_dialogview_loader::hm_wmp_dialogview_loader() {

}

hm_wmp_dialogview_loader::~hm_wmp_dialogview_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_dialogview_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	return NULL;
	// HM_WMP_X_LOADER_IMPL( hm_wmp_dialogview );
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
		assert( false );
		return false;
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
		assert( false );
		return false;
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

	assert( false );
	return false;
}

//////////////////////////////////////////////////////////////////////////
hm_wmp_popupmenu_loader::hm_wmp_popupmenu_loader() {

}

hm_wmp_popupmenu_loader::~hm_wmp_popupmenu_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_popupmenu_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { assert( false ); return NULL; }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_popupmenu" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { assert( false ); return NULL; }
	idWMP = valTmp.asInt();
	// MenuCfg: item size + spacing ( defaults 25 x 160, space 6 ).
	unsigned int uMIHeight = 25;
	unsigned int uMIWidth = 160;
	unsigned int uSpace = 6;
	valTmp = valWMP[ "MenuCfg" ];
	if ( valTmp.isObject() ) {
		Json::Value valCfgItem;
		valCfgItem = valTmp[ "m_uMIHeight" ]; if ( valCfgItem.isInt() ) uMIHeight = valCfgItem.asInt();
		valCfgItem = valTmp[ "m_uMIWidth" ]; if ( valCfgItem.isInt() ) uMIWidth = valCfgItem.asInt();
		valCfgItem = valTmp[ "m_uSpace" ]; if ( valCfgItem.isInt() ) uSpace = valCfgItem.asInt();
	}
	dimension_menuwnd_default* pDimensionMenuWnd = new dimension_menuwnd_default( uMIHeight, uMIWidth, uSpace );
	return pHMUIRoot->m_pWMPFactory->createObj< hm_wmp_popupmenu, dimension_menuwnd_i*, hmskin_event_listener_base* >(
		idWMP, ( dimension_menuwnd_i* )pDimensionMenuWnd, pHMUIRoot->m_pEventListener );
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
		assert( false );
		return false;
	}
	valRMCfg = valWMP[ STR_ROOTMENUCFG ];
	//
	valTmp = valRMCfg[ "m_uLenMenuItem" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rmCfg.m_uLenMenuItem = valTmp.asInt();
	//
	valTmp = valRMCfg[ "m_uSpaceH" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rmCfg.m_uSpaceH = valTmp.asInt();
	//
	valTmp = valRMCfg[ "m_uSpaceV" ];
	if ( !valTmp.isInt() ) {
		assert( false );
		return false;
	}
	rmCfg.m_uSpaceV = valTmp.asInt();
	//
	pWMPRootMenu->setConfigure( rmCfg );
	return true;
}
//////////////////////////////////////////////////////////////////////////
hm_wmp_scrollbar_loader::hm_wmp_scrollbar_loader() {

}

hm_wmp_scrollbar_loader::~hm_wmp_scrollbar_loader() {

}

hm_wndmsgprocessor_i* hm_wmp_scrollbar_loader::_createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot ) { assert( false ); return NULL; }
	Json::Value valTmp;
	hm_wmp_factory::obj_id_type idWMP = 0;
	std::string strWMPClsName;

	assert( _getWMPName( valWMP, strWMPClsName ) );
	assert( strWMPClsName == "hm_wmp_scrollbar" );
	valTmp = valWMP[ "idWMP" ];
	if ( !valTmp.isInt() ) { assert( false ); return NULL; }
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
		assert( false );
		return false;
	}
	//
	nCountTabBtn = aryTabBtns.size();
	for ( nIndexTabBtn = 0; nIndexTabBtn<nCountTabBtn; ++nIndexTabBtn ) {
		valTabBtn = aryTabBtns[ nIndexTabBtn ];
		if ( !valTabBtn.isObject() ) {
			assert( false );
			return false;
		}
		valTitle = valTabBtn["Title"];
		if ( !valTitle.isString() ) {
			assert( false );
			return false;
		}
		//
		pTabBtnInfo = new tabbtninfo();
		MultiByteToTChar( valTitle.asString(), pTabBtnInfo->m_strTitle );
		pWMPLoading->pushbackTabBtn( pTabBtnInfo );
	}

	valDefaultBtn = valWMP[ "DefaultBtn" ];
	if ( !valDefaultBtn.isInt() ) {
		assert( false );
		return false;
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