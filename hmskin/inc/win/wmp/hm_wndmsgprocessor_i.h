#ifndef __HM_WNDMSGPROCESSOR_H__
#define __HM_WNDMSGPROCESSOR_H__
#include "common\hm_cmn_pattern_factory.h"
#include "common\hm_cmn_pattern_observer.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\frame\hm_keel_obj.h"
#include "win\hm_export_defs.h"
#include "hmskin_event_listener_base.h"
#include <map>
#include <stdexcept>
#include <deque>
//
typedef unsigned int hm_wmp_id_type;
#define HMWMP_ID_INVALID (0xFFFFFFFF)
//
class HMLIB_API hm_wndmsgprocessor_i : public cmn::hm_cmn_subject, public cmn::hm_cmn_observer_i
{
	friend class hm_wmp_loader_base;
public:
	typedef std::deque< unsigned int > msg_container_type;
public:
	hm_wndmsgprocessor_i( hmskin_event_listener_base* pEventListener = NULL ) : m_pEventListener( pEventListener ), m_uId( HMWMP_ID_INVALID ) {};
	virtual ~hm_wndmsgprocessor_i(){};
	
	// interface
public:
	virtual void doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;
	virtual void getWndMsgs( msg_container_type& containerWndMsg ) = 0;
	
	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {};

public:
	unsigned int getId() const { return m_uId; }

protected:
	void _setId( unsigned int uWMPId ) { m_uId = uWMPId; }

protected:
	hmskin_event_listener_base* m_pEventListener;
	unsigned int m_uId;
};
//
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( hm_wndmsgprocessor_i );
#pragma warning(pop)
//
#define hm_wmp_factory FACTORY_TYPE( hm_wndmsgprocessor_i )
//extern HMLIB_API hm_wmp_factory g_factoryWndMsgProcessor;
//#define WMPFACTORY_INSTANCE (&g_factoryWndMsgProcessor)
//
#define MSGBIND_BEGIN( class_name ) void class_name::doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) { \
	if ( !pKeelObj || !pWP || !pParamWndProc ) \
		return; \
	switch ( pWP->message ) \
	{ \
	case WM_NULL: {} break;
//
#define MSGBIND_BEGIN_T( class_name ) template< typename T > \
	void class_name< T >::doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) { \
	if ( !pKeelObj || !pWP || !pParamWndProc ) \
		return; \
	switch ( pWP->message ) \
	{ \
	case WM_NULL: {} break;
//
#define MSGBIND_BEGIN_T2( class_name ) template< typename T1, typename T2 > \
	void class_name< T1, T2 >::doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) { \
	if ( !pKeelObj || !pWP || !pParamWndProc ) \
		return; \
	switch ( pWP->message ) \
	{ \
	case WM_NULL: {} break;
//
#define MSGBIND( msg, function ) case msg: { function( pKeelObj, pWP, pParamWndProc ); } break;
//
#define MSGBIND_END() default: break; } }
//
#define ORDERWNDMSG_BEGIN( class_name ) void class_name::getWndMsgs( msg_container_type& containerWndMsg ) { containerWndMsg.clear();
//
#define ORDERWNDMSG_BEGIN_T( class_name ) template< typename T > \
	void class_name< T >::getWndMsgs( msg_container_type& containerWndMsg ) { containerWndMsg.clear();
//
#define ORDERWNDMSG_BEGIN_T2( class_name ) template< typename T1, typename T2 > \
	void class_name< T1, T2 >::getWndMsgs( msg_container_type& containerWndMsg ) { containerWndMsg.clear();
//
#define ORDERWNDMSG( msg ) containerWndMsg.push_back( msg );
//
#define ORDERWNDMSG_END() }
//
#define DECLARE_MSGBIND() \
	public: \
	virtual void doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ); \
	virtual void getWndMsgs( msg_container_type& containerWndMsg );



#endif //__HM_WNDMSGPROCESSOR_H__