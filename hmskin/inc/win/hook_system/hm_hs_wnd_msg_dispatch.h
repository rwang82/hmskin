#ifndef __WND_MSG_DISPATCH_H__
#define __WND_MSG_DISPATCH_H__
#include <deque>
#include <map>
#include "common\hm_cmn_patterns.h"
#include "common\hm_cmn_pattern_factory.h"
#include "common\hm_wndmsg_publisher.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include "win\hm_export_defs.h"
//////////////////////////////////////////////////////////////////////////
// msg_sink_i
class HMLIB_API msg_sink_i : public hm_wndmsg_publisher
{
	//type define
public:
	typedef unsigned int event_id_type;
	typedef UINT wnd_msg_type;
	typedef cmn::hm_cmn_observer_i* obs_ptr_type;
	typedef std::deque< cmn::hm_cmn_observer_i* > obs_ptr_container_type;
	typedef std::map< wnd_msg_type, obs_ptr_container_type* > wndmsg2obss_map_type;

	//interface
public:
	virtual void OnStartSink( HWND hWndDest ){};
	virtual void OnStopSink( HWND hWndDest ){};
	virtual void WndProc( HWND hWndDest, UINT message, WPARAM& wParam, LPARAM& lParam, WNDPROCPARAM* pParamWndProc ) = 0;

	//attribute
public:

public:
	virtual ~msg_sink_i();
protected:
	msg_sink_i();
	
	//helper
private:

private:
#pragma warning(push)
#pragma warning(disable:4251)
	wndmsg2obss_map_type m_mapWndMsg2Obsvrs;
#pragma warning(pop)
};

////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( msg_sink_i );
#pragma warning(pop)
#define msg_sink_factory_type FACTORY_TYPE( msg_sink_i )
bool ConnectHwnd2MsgSink( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark );
bool DisconnectHwnd2MsgSink( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark );
//////////////////////////////////////////////////////////////////////////
// MsgDispatcher 
// note: MsgDispatcher class don't hold the life circle of msg_sink_i.
//		 if a class obj which from msg_sink_i destoried, it need to do unregister here.
class MsgDispatcher
{
	//type define
public:
	typedef std::deque< msg_sink_i* > msg_sink_container_type;
	typedef std::map< HWND, msg_sink_container_type > map_hwnd2msc_type;
	typedef void ( *pfnEnumMsgSinkCallBackType )( HWND hWnd, msg_sink_container_type* pMsgSinkContainer, BOOL* pbContinue, void* pParam );

public:
	static MsgDispatcher* GetInstance();
	
	//operator
public:
	//
	BOOL RegMsgSink( HWND hWndDest, msg_sink_i* pMsgSink );
	void UnRegMsgSink( msg_sink_i* pMsgSink );
	void UnRegMsgSink( HWND hWndDest, msg_sink_i* pMsgSink );
	void UnRegMsgSinks( HWND hWndDest );
	void UnRegAllMsgSinks();
	BOOL HasMsgSink( HWND hWndDest, const msg_sink_i* pMsgSink ) const;
	void EnumMsgSink( pfnEnumMsgSinkCallBackType pfnEnumMsgSinkCallBack, void* pParam );
	//
	BOOL StartAllMsgSink( BOOL bStart, BOOL bForce = TRUE );
	BOOL StartMsgSink( HWND hWndDest, BOOL bStart );
	BOOL IsSinkingMsg( HWND hWndDest ) const;
	
public:
	virtual ~MsgDispatcher();
private:
	MsgDispatcher();
	
private:
	map_hwnd2msc_type m_mapHwnd2MSC;
};
#define MSGDISPATCHER_INSTANCE (MsgDispatcher::GetInstance())
//////////////////////////////////////////////////////////////////////////
//typedef TWndMarkTable< msg_sink_factory_type::obj_id_type > hm_wmark_msid_table_type;
#define hm_wmark_msid_table_type TWndMarkTable< msg_sink_factory_type::obj_id_type >
//extern HMLIB_API hm_wmark_msid_table_type g_tableWndMark2MSId;
//#define WNDMARK2MSID_TABLE (&g_tableWndMark2MSId)

EXTERN_C_START

class hm_uiroot;
HMLIB_API bool hm_msid2wndmark_bind( hm_uiroot* pHMUIRoot, unsigned int msId, const hm_rs_wndmark& wndMark );

EXTERN_C_END

// unified debug trace ( hm_hs_wnd_msg_dispatch.cpp ): OutputDebugString plus
// "hmskin_trace.log" next to the exe when activated ( hmskin.trace marker file
// or HMSKIN_TRACE env var ). shared so all engine debug points land in one file.
HMLIB_API void DisTraceOut( const TCHAR* szLine );

#endif //__WND_MSG_DISPATCH_H__