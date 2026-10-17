#include "stdafx.h"
#include "win\loader\hm_loader_keelobj.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_decoders.h"
#include "win\frame\hm_uiroot.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "common\FuncPack.h"
#include <assert.h>


// DPI: scale from the session owner ( uiroot ), see hm_layout_scale.h.
static int _ScalePx( hm_uiroot* pHMUIRoot, int nValue96 ) {
	return (int)( nValue96 * pHMUIRoot->m_dblLayoutScale + 0.5 );
}


hm_loader_keelobj::hm_loader_keelobj() {

}

hm_loader_keelobj::~hm_loader_keelobj() {

}

bool hm_loader_keelobj::load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) {
	if ( !pManifestData ) return false;
	Json::Value& rootVal = pManifestData->getRootVal();
	Json::Value aryKeelObjVal;
	Json::Value keelobjVal;
	Json::Value valLayOutObj;
	Json::Value aryWMPVal;
	Json::Value valWMP;
	Json::Value aryRegEvts;
	Json::Value valRegEvt;
	Json::Value valTmp;
	unsigned int uIndex, uCount;
	unsigned int uIndexWMP, uCountWMP;
	unsigned int uIndexRegEvt, uCountRegEvt;
	unsigned int uRegEvtId;
	hm_keel_obj_factory::obj_id_type objId;
	std::string strWndMsgSourceType;
	hm_keel_obj* pNewKeelObj = NULL;
	std::string strLayOutDecoder;
	hm_layoutdecoder_i* pLayOutDecoder = NULL;
	//hm_keel_obj_factory::obj_id_type idKeelObjRef;
	//hm_keel_obj* pKeelObjRef = NULL;
	hm_keel_obj* pKeelObjParent = NULL;
	hm_layout_obj* pLayOutRef = NULL;
	hm_layout_obj* pLayOutObj = NULL;
	coordinate_system_hwnd* pCoordSys = NULL;
	hm_wmp_factory::obj_id_type idWMP;
	hm_wndmsgprocessor_i* pWMP = NULL;
	hm_wndmsg_publisher* pWndMsgPublisher = NULL;

	aryKeelObjVal = rootVal["KeelObj"];
	if ( !aryKeelObjVal.isArray() )
		return false;
	
	uCount = aryKeelObjVal.size();
	for ( uIndex = 0; uIndex < uCount; uIndex++ ) {
		keelobjVal = aryKeelObjVal[ uIndex ];
		if ( !keelobjVal.isObject() )
			return false;
		
		// id
		valTmp = keelobjVal["idKeelObj"];
		if ( !valTmp.isInt() )
			return false;
		objId = valTmp.asInt();
		pNewKeelObj = pHMUIRoot->m_pKeelObjFactory->createObj< hm_keel_obj >( objId );	
		pNewKeelObj->_setId( objId );

		// switchWndMsgSource
		valTmp = keelobjVal["WndMsgSource"];
		if ( !valTmp.isObject() )
			return false;
		pWndMsgPublisher = NULL;
		if ( !_GetWndMsgSource( pHMUIRoot, valTmp, &pWndMsgPublisher ) ) {
			assert( FALSE );
			return false;
		}
		if ( !pWndMsgPublisher ) {
			assert( FALSE );
			return false;
		}
		pNewKeelObj->switchWndMsgSource( pWndMsgPublisher );
		
		// parent.
		valTmp = keelobjVal["ParentKeelObjId"];
		if ( valTmp.isInt() ) {
			pKeelObjParent = pHMUIRoot->m_pKeelObjFactory->getObj( valTmp.asInt() );
			if ( pKeelObjParent ) {
				pNewKeelObj->setParent( pKeelObjParent );
			}
		}

		// LayOutObj Coordinate System
		valLayOutObj = keelobjVal["LayoutObj"];
		if ( valLayOutObj.isNull() )
			return false;
		if ( !_CreateLayoutObj( pHMUIRoot, valLayOutObj, &pLayOutObj ) ) {
			assert( FALSE );
			return false;
		}
		pNewKeelObj->setLayoutObj( pLayOutObj );
		
		//  window message processor
		aryWMPVal = keelobjVal["WMP"];
		if ( !aryWMPVal.isArray() )
			continue;
		uCountWMP = aryWMPVal.size();
		for ( uIndexWMP = 0; uIndexWMP<uCountWMP; ++uIndexWMP ) {
			valWMP = aryWMPVal[ uIndexWMP ];
			if ( !valWMP.isObject() )
				break;
			valTmp = valWMP["id"];
			if ( !valTmp.isInt() )
				break;
			idWMP = valTmp.asInt();
			pWMP = pHMUIRoot->m_pWMPFactory->getObj( idWMP );
			if ( !pWMP ) {
				assert( FALSE );
				break;
			}
			pNewKeelObj->pushbackWMP( pWMP );
			//
			aryRegEvts = valWMP["RegEvts"];
			if ( !aryRegEvts.isArray() )
				break;
			uCountRegEvt = aryRegEvts.size();
			for ( uIndexRegEvt = 0; uIndexRegEvt<uCountRegEvt; ++uIndexRegEvt ) {
				valRegEvt = aryRegEvts[ uIndexRegEvt ];
				if ( !valRegEvt.isInt() )
					break;
				uRegEvtId = valRegEvt.asInt();
				//
				pNewKeelObj->Register( pWMP, uRegEvtId );
			}
		}
	}

	return true;
}

bool hm_loader_keelobj::load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) {

	return true;
}


BOOL hm_loader_keelobj::_CreateLayoutObj( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutObj, hm_layout_obj** ppLayoutObj ) {
	if ( !ppLayoutObj )
		return FALSE;
	hm_layout_obj* pLayoutObjNew = NULL;
	hm_layoutdecoder_i* pLayoutDecoder = NULL;
	Json::Value valCSId;
	Json::Value valLayoutDecoder;
	int idCoordSys;
	coordinate_system_hwnd* pCoordSys = NULL;

	pLayoutObjNew = new hm_layout_obj();
	CFuncPack fpkDelLayoutObj( gfnDelAnyObj< hm_layout_obj >, pLayoutObjNew );
	valCSId = valLayoutObj[ "CSId" ];
	if ( !valCSId.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	idCoordSys = valCSId.asInt();
	pCoordSys = pHMUIRoot->m_pCoordSysFactory->getObj( idCoordSys );
	if ( !pCoordSys ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutObjNew->setCoordinateSystem( pCoordSys );
	//
	valLayoutDecoder = valLayoutObj[ "LayoutDecoder" ];
	if ( valLayoutDecoder.isNull() ) {
		pLayoutDecoder = NULL;
	} else {
		if ( !_CreateLayoutDecoder( pHMUIRoot, valLayoutDecoder, pLayoutObjNew, &pLayoutDecoder ) ) {
			assert( FALSE );
			return FALSE;
		}
	}
	if ( !pLayoutDecoder ) {
		assert( FALSE );
		return FALSE;
	}
	CFuncPack fpkDelLayoutDecoder( gfnDelAnyObj< hm_layoutdecoder_i >, pLayoutDecoder );
	pLayoutObjNew->setLayoutDecoder( pLayoutDecoder );

	//
	if ( !ppLayoutObj ) {
		assert( FALSE );
		return FALSE;
	}
	//
	fpkDelLayoutObj.Cancel();
	fpkDelLayoutDecoder.Cancel();
	*ppLayoutObj = pLayoutObjNew;
	return TRUE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoder( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( _CreateLayoutDecoderClientArea( pHMUIRoot, valLayoutDecoder, pLayoutOutObj, ppLayoutDecoder ) ) {
		return TRUE;
	} else if ( _CreateLayoutDecoderWndArea( pHMUIRoot, valLayoutDecoder, pLayoutOutObj, ppLayoutDecoder ) ) {
		return TRUE;
	} else if ( _CreateLayoutDecoderCorner( pHMUIRoot, valLayoutDecoder, pLayoutOutObj, ppLayoutDecoder ) ) {
		return TRUE;
	} else if ( _CreateLayoutDecoderDock( pHMUIRoot, valLayoutDecoder, pLayoutOutObj, ppLayoutDecoder ) ) {
		return TRUE;
	} else if ( _CreateLayoutDecoderOffset( pHMUIRoot, valLayoutDecoder, pLayoutOutObj, ppLayoutDecoder ) ) {
		return TRUE;
	}

	return FALSE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoderClientArea( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( !ppLayoutDecoder )
		return FALSE;
	std::string strDecoderClsName;
	Json::Value valDecoderType;

	valDecoderType = valLayoutDecoder[ "DecoderType" ];
	if ( !valDecoderType.isString() )
		return FALSE;
	strDecoderClsName = valDecoderType.asString();
	//
	if ( 0 != strcmp( "hm_layoutdecoder_clientarea", strDecoderClsName.c_str() ) ) 
		return FALSE;
	//
	*ppLayoutDecoder = new hm_layoutdecoder_clientarea( pLayoutOutObj );

	return TRUE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoderWndArea( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( !ppLayoutDecoder )
		return FALSE;
	std::string strDecoderClsName;
	Json::Value valDecoderType;
	hm_layoutdecoder_wndarea* pLayoutDecoderNew = NULL;

	valDecoderType = valLayoutDecoder[ "DecoderType" ];
	if ( !valDecoderType.isString() )
		return FALSE;
	strDecoderClsName = valDecoderType.asString();
	//
	if ( 0 != strcmp( "hm_layoutdecoder_wndarea", strDecoderClsName.c_str() ) ) 
		return FALSE;

	//
	pLayoutDecoderNew = new hm_layoutdecoder_wndarea( pLayoutOutObj );
	*ppLayoutDecoder = pLayoutDecoderNew;

	return TRUE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoderCorner( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( !ppLayoutDecoder )
		return FALSE;
	std::string strDecoderClsName;
	Json::Value valDecoderType;
	Json::Value valTmp;
	hm_layoutdecoder_corner* pLayoutDecoderNew = NULL;
	std::string strVal;
	hm_layoutdecoder_corner::ENUMCORNERTYPE eCornerType;
	hm_keel_obj_factory::obj_id_type objIdKeelObjRef;
	hm_keel_obj* pKeelObjRef = NULL;

	valDecoderType = valLayoutDecoder[ "DecoderType" ];
	if ( !valDecoderType.isString() )
		return FALSE;
	strDecoderClsName = valDecoderType.asString();
	//
	if ( 0 != strcmp( "hm_layoutdecoder_corner", strDecoderClsName.c_str() ) ) 
		return FALSE;
	//
	pLayoutDecoderNew = new hm_layoutdecoder_corner( pLayoutOutObj );
	CFuncPack fpkDelLayoutDecoder( gfnDelAnyObj< hm_layoutdecoder_i >, pLayoutDecoderNew );
	// get m_eCornerType.
	valTmp = valLayoutDecoder[ "m_eCornerType" ];
	if ( !valTmp.isString() ){
		assert( FALSE );
		return FALSE;
	}
	strVal = valTmp.asString();
	if ( strcmp( "CORNER_LT", strVal.c_str() ) == 0 ) {
		eCornerType = hm_layoutdecoder_corner::CORNER_LT;
	} else if ( strcmp( "CORNER_LB", strVal.c_str() ) == 0 ) {
		eCornerType = hm_layoutdecoder_corner::CORNER_LB;
	} else if ( strcmp( "CORNER_RB", strVal.c_str() ) == 0 ) {
		eCornerType = hm_layoutdecoder_corner::CORNER_RB;
	} else if ( strcmp( "CORNER_RT", strVal.c_str() ) == 0 ) {
		eCornerType = hm_layoutdecoder_corner::CORNER_RT;
	} else {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_eCornerType = eCornerType;
	// get m_space1.
	valTmp = valLayoutDecoder[ "m_space1" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_space1 = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_space2.
	valTmp = valLayoutDecoder[ "m_space2" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_space2 = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_width
	valTmp = valLayoutDecoder[ "m_width" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_width = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_height.
	valTmp = valLayoutDecoder[ "m_height" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_height = _ScalePx( pHMUIRoot, valTmp.asInt() );
	//
	valTmp = valLayoutDecoder[ "RefKeelObjId" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	//
	objIdKeelObjRef = valTmp.asInt();
	pKeelObjRef = pHMUIRoot->m_pKeelObjFactory->getObj( objIdKeelObjRef );
	if ( !pKeelObjRef ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->setLayoutRefObj( pKeelObjRef->getLayoutObj() );
	
	//
	fpkDelLayoutDecoder.Cancel();
	*ppLayoutDecoder = pLayoutDecoderNew;
	return TRUE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoderDock( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( !ppLayoutDecoder )
		return FALSE;
	std::string strDecoderClsName;
	Json::Value valDecoderType;
	hm_layoutdecoder_dock* pLayoutDecoderNew = NULL;
	Json::Value valTmp;
	std::string strDockType;
	int objIdKeelObjRef;
	hm_keel_obj* pKeelObjRef = NULL;

	valDecoderType = valLayoutDecoder[ "DecoderType" ];
	if ( !valDecoderType.isString() )
		return FALSE;
	strDecoderClsName = valDecoderType.asString();
	//
	if ( 0 != strcmp( "hm_layoutdecoder_dock", strDecoderClsName.c_str() ) ) 
		return FALSE;
	pLayoutDecoderNew = new hm_layoutdecoder_dock( pLayoutOutObj );
	CFuncPack fpkDelLayoutDecoderNew( gfnDelAnyObj< hm_layoutdecoder_dock >, pLayoutDecoderNew );
	// get strDockType.
	valTmp = valLayoutDecoder[ "m_eDockType" ];
	if ( !valTmp.isString() ) {
		assert( FALSE );
		return FALSE;
	}
	strDockType = valTmp.asString();
	//
	if ( strDockType == "DOCK_TOP" ) 
		pLayoutDecoderNew->m_eDockType = hm_layoutdecoder_dock::DOCK_TOP;
	else if ( strDockType == "DOCK_BOTTOM" )
		pLayoutDecoderNew->m_eDockType = hm_layoutdecoder_dock::DOCK_BOTTOM;
	else if ( strDockType == "DOCK_LEFT" )
		pLayoutDecoderNew->m_eDockType = hm_layoutdecoder_dock::DOCK_LEFT;
	else if ( strDockType == "DOCK_RIGHT" )
		pLayoutDecoderNew->m_eDockType = hm_layoutdecoder_dock::DOCK_RIGHT;
	else {
		assert( FALSE );
		return FALSE;
	}
	// get m_space1
	valTmp = valLayoutDecoder["m_space1"];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_space1 = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_space2
	valTmp = valLayoutDecoder["m_space2"];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_space2 = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_space3
	valTmp = valLayoutDecoder["m_space3"];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_space3 = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get m_thickness
	valTmp = valLayoutDecoder["m_thickness"];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	pLayoutDecoderNew->m_thickness = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get objIdKeelObjRef
	valTmp = valLayoutDecoder[ "RefKeelObjId" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	// get pKeelObjRef
	objIdKeelObjRef = valTmp.asInt();
	pKeelObjRef = pHMUIRoot->m_pKeelObjFactory->getObj( objIdKeelObjRef );
	if ( !pKeelObjRef ) {
		assert( FALSE );
		return FALSE;
	}
	// set layout reference object.
	pLayoutDecoderNew->setLayoutRefObj( pKeelObjRef->getLayoutObj() );

	//
	fpkDelLayoutDecoderNew.Cancel();
	*ppLayoutDecoder = pLayoutDecoderNew;
	return TRUE;
}

BOOL hm_loader_keelobj::_CreateLayoutDecoderOffset( hm_uiroot* pHMUIRoot, const Json::Value& valLayoutDecoder, hm_layout_obj* pLayoutOutObj, hm_layoutdecoder_i** ppLayoutDecoder ) {
	if ( !pLayoutOutObj || !ppLayoutDecoder ) {
		assert( FALSE );
		return FALSE;
	}
	std::string strDecoderClsName;
	Json::Value valDecoderType;
	hm_layoutdecoder_offset* pLayoutDecoderNew = NULL;
	Json::Value valTmp;
	int objIdKeelObjRef;
	hm_keel_obj* pKeelObjRef = NULL;
	int nOffsetL; 
	int nOffsetT;
	int nOffsetR;
	int nOffsetB;

	valDecoderType = valLayoutDecoder[ "DecoderType" ];
	if ( !valDecoderType.isString() )
		return FALSE;
	strDecoderClsName = valDecoderType.asString();
	//
	if ( 0 != strcmp( "hm_layoutdecoder_offset", strDecoderClsName.c_str() ) ) 
		return FALSE;
	// get objIdKeelObjRef
	valTmp = valLayoutDecoder[ "RefKeelObjId" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	objIdKeelObjRef = valTmp.asInt();
	// get pKeelObjRef
	pKeelObjRef = pHMUIRoot->m_pKeelObjFactory->getObj( objIdKeelObjRef );
	if ( !pKeelObjRef ) {
		assert( FALSE );
		return FALSE;
	}
	// get nOffsetL
	valTmp = valLayoutDecoder[ "m_nOffsetL" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	nOffsetL = _ScalePx( pHMUIRoot, valTmp.asInt() );
	// get nOffsetR
	valTmp = valLayoutDecoder[ "m_nOffsetR" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	nOffsetR = _ScalePx( pHMUIRoot, valTmp.asInt() );
	//
	valTmp = valLayoutDecoder[ "m_nOffsetT" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	nOffsetT = _ScalePx( pHMUIRoot, valTmp.asInt() );
	//
	valTmp = valLayoutDecoder[ "m_nOffsetB" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	nOffsetB = _ScalePx( pHMUIRoot, valTmp.asInt() );
	
	//
	pLayoutDecoderNew = new hm_layoutdecoder_offset( pLayoutOutObj );
	pLayoutDecoderNew->setLayoutRefObj( pKeelObjRef->getLayoutObj() );
	pLayoutDecoderNew->m_nOffsetL = nOffsetL;
	pLayoutDecoderNew->m_nOffsetR = nOffsetR;
	pLayoutDecoderNew->m_nOffsetT = nOffsetT;
	pLayoutDecoderNew->m_nOffsetB = nOffsetB;
	*ppLayoutDecoder = pLayoutDecoderNew;

	return TRUE;
}

BOOL hm_loader_keelobj::_GetWndMsgSource( hm_uiroot* pHMUIRoot, const Json::Value& valWndMsgSource, hm_wndmsg_publisher** ppWndMsgPublisher ) {
	if ( _GetWndMsgSource_MsgSink( pHMUIRoot, valWndMsgSource, ppWndMsgPublisher ) ) {
		return TRUE;
	} else if ( _GetWndMsgSource_KeelObj( pHMUIRoot, valWndMsgSource, ppWndMsgPublisher ) ) {
		return TRUE;
	} else {
		return FALSE;
	}

	assert( FALSE );
	return FALSE;
}

BOOL hm_loader_keelobj::_GetWndMsgSource_MsgSink( hm_uiroot* pHMUIRoot, const Json::Value& valWndMsgSource, hm_wndmsg_publisher** ppWndMsgPublisher ) {
	if ( !valWndMsgSource.isObject() || !ppWndMsgPublisher ) {
		assert( FALSE );
		return FALSE;
	}
	Json::Value valTmp;
	std::string strType;

	valTmp = valWndMsgSource[ "Type" ];
	if ( !valTmp.isString() ) {
		assert( FALSE );
		return FALSE;
	}
	strType = valTmp.asString();
	if ( 0 != strcmp( strType.c_str(), "MsgSink" ) )
		return FALSE;
	//
	int nMsgSinkId;
	msg_sink_i* pMsgSink = NULL;

	valTmp = valWndMsgSource[ "MsgSinkId" ];
	if ( !valTmp.isInt() ) {
		assert( FALSE );
		return FALSE;
	}
	nMsgSinkId = valTmp.asInt();
	pMsgSink = pHMUIRoot->m_pMsgSinkFactory->getObj( nMsgSinkId );
	if ( !pMsgSink ) {
		assert( FALSE );
		return FALSE;
	}
	
	//
	*ppWndMsgPublisher = pMsgSink;
	return TRUE;
}

BOOL hm_loader_keelobj::_GetWndMsgSource_KeelObj( hm_uiroot* pHMUIRoot, const Json::Value& valWndMsgSource, hm_wndmsg_publisher** ppWndMsgPublisher ) {
	if ( !valWndMsgSource.isObject() || !ppWndMsgPublisher ) {
		assert( FALSE );
		return FALSE;
	}
	Json::Value valTmp;
	std::string strType;

	valTmp = valWndMsgSource[ "Type" ];
	if ( !valTmp.isString() ) {
		assert( FALSE );
		return FALSE;
	}
	strType = valTmp.asString();
	if ( 0 != strcmp( strType.c_str(), "KeelObj" ) )
		return FALSE;
	//
	hm_keel_obj* pKeelObjMsgSource = NULL;
	hm_keel_obj_factory::obj_id_type idKeelObjMsgSource;
	//
	valTmp = valWndMsgSource["KeelObjId"];
	if ( !valTmp.isInt() )
		return false;
	idKeelObjMsgSource = valTmp.asInt();
	pKeelObjMsgSource = pHMUIRoot->m_pKeelObjFactory->getObj( idKeelObjMsgSource );
	if ( !pKeelObjMsgSource ) {
		assert( FALSE );
		return FALSE;
	}

	//
	*ppWndMsgPublisher = pKeelObjMsgSource;
	return TRUE;
}
