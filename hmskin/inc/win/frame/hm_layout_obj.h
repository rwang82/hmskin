#ifndef __HM_LAYOUT_OBJ_H__
#define __HM_LAYOUT_OBJ_H__
#include "windows.h"
#include "common\hm_cmn_patterns.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_export_defs.h"

class hm_layoutdecoder_i;
class hm_keel_obj;

class HMLIB_API hm_layout_obj: public cmn::hm_cmn_observer_i, public cmn::hm_cmn_subject
{
public:
	typedef hm_layout_obj class_type;
	typedef hm_layoutdecoder_i* decoder_ptr_type;
	typedef coordinate_system_hwnd* coordinate_system_ptr_type;
	
public:
	hm_layout_obj();
	virtual ~hm_layout_obj();
	
	//interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );
	
	//²¼¾Ö½âÊÍÆ÷
public:
	bool hasLayoutDecoder() const { return m_pLayoutDecoder != NULL; }
	void setLayoutDecoder( decoder_ptr_type pLayoutDecoder) { m_pLayoutDecoder = pLayoutDecoder; }
	decoder_ptr_type getLayoutDecoder() { return m_pLayoutDecoder; }
	const decoder_ptr_type getLayoutDecoder() const { return m_pLayoutDecoder; }
	
	//set coordinate system
public:
	bool hasCoordinateSystem() const { return m_pCoordinateSystem != NULL; }
	void setCoordinateSystem( coordinate_system_ptr_type pCoordinateSystem );
	coordinate_system_ptr_type getCoordinateSystem() const { return m_pCoordinateSystem; }
	
public:
	void calcPosition();
	bool mapPosition( HWND hWndDest, coordinate_system_hwnd::ENUMCSHTYPE eType, RECT& rcDest );
	bool screen2LayoutObj( POINT& ptS2L );

public:
	RECT m_rect;

protected:
	coordinate_system_ptr_type m_pCoordinateSystem;
	decoder_ptr_type m_pLayoutDecoder;
};

class HMLIB_API hm_layoutdecoder_i
{
public:
	typedef hm_layout_obj* ref_obj_ptr_type; //reference obj
	typedef hm_layout_obj* out_obj_ptr_type; //output result object

public:
	hm_layoutdecoder_i( out_obj_ptr_type pOutObj ) { m_pOutObj = pOutObj; m_pRefObj = NULL; }
	virtual ~hm_layoutdecoder_i(){};
	
	//interface
public:
	virtual void decoderLayout() = 0;
	
public:
	bool hasLayoutRefObj() const { return m_pRefObj != NULL; }
	void setLayoutRefObj( const ref_obj_ptr_type pRefObj );
	ref_obj_ptr_type getLayoutRefObj() { return m_pRefObj; }
	const ref_obj_ptr_type getLayoutRefObj() const { return m_pRefObj; }
	
protected:
	void setOutObjRect( const RECT& rect ) { if ( m_pOutObj ) m_pOutObj->m_rect = rect; };

protected:
	ref_obj_ptr_type m_pRefObj;
	out_obj_ptr_type m_pOutObj;
};


#endif //__HM_LAYOUT_OBJ_H__