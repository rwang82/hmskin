#ifndef __HM_KEEL_OBJ_H__
#define __HM_KEEL_OBJ_H__
#include "common\hm_cmn_patterns.h"
#include "common\hm_cmn_pattern_factory.h"
#include "common\hm_wndmsg_publisher.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\hm_export_defs.h"
#include <map>
//
class hm_layout_obj;
class hm_wndmsgprocessor_i;
//
class HMLIB_API hm_keel_obj : public cmn::hm_cmn_subject, public hm_wndmsg_observer, public hm_wndmsg_publisher
{
	friend class hm_keel_obj_factory;
	friend class hm_loader_keelobj;
	// type define
public:
	typedef unsigned int layoutobj_sort_type;
	//typedef unsigned int hm_wmp_id_type;
	//typedef std::deque< hm_wmp_id_type > wmpid_container_type;
	typedef std::deque< hm_wndmsgprocessor_i* > wmp_container_type;
	typedef std::deque< hm_keel_obj* > keelobj_container_type;
	
protected:
	hm_keel_obj();
public:
	virtual ~hm_keel_obj();
	
	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

	// interface of hm_wndmsg_observer
protected:
	virtual void pushbackWndMsgNeed( wndmsg_container_type& containerWndMsgNeed );
	
	// interface of hm_wndmsg_publisher
protected:
	virtual void _doOrderWndMsgFromSource( wnd_msg_type message );
	virtual void _doCancelWndMsgFromSource( wnd_msg_type message );

public:
	unsigned int getId() const { return m_uId; }

	// layoutobj
public:
	bool hasLayoutObj() const;
	hm_layout_obj* getLayoutObj();
	const hm_layout_obj* getLayoutObj() const;
	void setLayoutObj( hm_layout_obj* pLayoutObj );
	
	// wmdmsg processor
public: 
	//void pushbackWMP( hm_wmp_id_type wmpId );
	//bool hasWMP( hm_wmp_id_type wmpId );
	//void delWMP( hm_wmp_id_type wmpId );
	void pushbackWMP( hm_wndmsgprocessor_i* pWndMsgProcessor );
	bool hasWMP( const hm_wndmsgprocessor_i* pWndMsgProcessor ) const;
	void delWMP( hm_wndmsgprocessor_i* pWndMsgProcessor );
	hm_wndmsgprocessor_i* getWMP( unsigned int uWMPId );

public:
	void setParent( hm_keel_obj* pParentObj );
	hm_keel_obj* getParent();

	// draw
public:
	void doDrawAll( HDC hDC, HWND hWndCanvas );
	void setVisible( BOOL bVisible );
	BOOL isVisible() const;

	// helper
private:
	void _delAllLayoutObj();
	void _doDrawSelf( HDC hDC, HWND hWndCanvas );
	void _doDrawChildKeelObjs( HDC hDC, HWND hWndCanvas );
	void _doWndMsg( CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	bool _IsNeedWndMsg( unsigned int message );
	void _pushbackSubObj( hm_keel_obj* pSubKeelObj );
	void _eraseSubObj( hm_keel_obj* pSubKeelObj );
	bool _hasSubObj( hm_keel_obj* pSubKeelObj );
	void _setId( unsigned int uId ) { m_uId = uId; }

protected:
	hm_layout_obj* m_pLayoutObj;
	hm_keel_obj* m_pParentObj;
	//wmpid_container_type m_containerWMPId;
#pragma warning(push)
#pragma warning(disable:4251)
	wmp_container_type m_containerWMP;
	keelobj_container_type m_containerKeelObjs;
#pragma warning(pop)
	unsigned int m_uFlag;
	unsigned int m_uId;
};

//declare keel obj factory
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( hm_keel_obj );
#pragma warning(pop)
//
typedef FACTORY_TYPE( hm_keel_obj ) hm_keel_obj_factory;
//extern HMLIB_API hm_keel_obj_factory g_factoryKeelObj;
//#define KEELOBJ_FACTORY (&g_factoryKeelObj)

//EXTERN_C_START
//HMLIB_API hm_keel_obj* hm_keelobj_create( hm_keel_obj_factory::obj_id_type objId );
//EXTERN_C_END
#endif //__HM_KEEL_OBJ_H__