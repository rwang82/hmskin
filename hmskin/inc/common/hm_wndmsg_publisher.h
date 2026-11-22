#ifndef __hm_wndmsg_publisher_H__
#define __hm_wndmsg_publisher_H__
#include "WinDef.h"
#include "common\hm_cmn_pattern_observer.h"
#include "win\hm_export_defs.h"
///////////////////////////////////////////////////////////////////////
// dispatch-truncation flags - each layer stops AFTER the current handler:
//   DISABLE_OLD_WNDPROC (0x1)          : skip the host's original WndProc
//   DISABLE_OTHER_MSGSINK (0x2)        : skip subsequent msg sinks
//   DISABLE_OTHER_MSGOBSRV (0x4)       : skip subsequent keelobj observers
//   DISABLE_OTHER_WNDMSGPROCESSOR (0x8): skip the remaining WMPs of THIS keelobj
#define EATMSGFLAG_DISABLE_NONE (0x00000000)
#define EATMSGFLAG_DISABLE_OLD_WNDPROC (0x0000001)
#define EATMSGFLAG_DISABLE_OTHER_MSGSINK (0x00000002)
#define EATMSGFLAG_DISABLE_OTHER_MSGOBSRV (0x00000004)
#define EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR (0x00000008)
// the usual "message fully handled" combination.
#define EATMSGFLAG_CUT_ALL ( EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR )
//
typedef UINT wnd_msg_type;
typedef std::deque< wnd_msg_type > wndmsg_container_type;
//
class hm_wndmsg_publisher;
//
class HMLIB_API hm_wndmsg_observer : public cmn::hm_cmn_observer_i
{
public:
	typedef std::deque< UINT > wndmsg_container_type;

public:
	hm_wndmsg_observer();
	virtual ~hm_wndmsg_observer();

public:
	virtual void switchWndMsgSource( hm_wndmsg_publisher* pWndMsgSource );
	void clearWndMsgSource() { m_pWndMsgSource = NULL; }

protected:
	virtual void pushbackWndMsgNeed( wndmsg_container_type& containerWndMsg ) = 0;

protected:
#pragma warning(push)
#pragma warning(disable:4251)
	hm_wndmsg_publisher* m_pWndMsgSource;
#pragma warning(pop)
};
//
class HMLIB_API hm_wndmsg_publisher
{
public:
	struct WNDPROCPARAM 
	{
		int m_flagEatMsg;
		BOOL m_bValidRes;
		LRESULT m_lResult;
	};
	typedef hm_wndmsg_observer* obs_ptr_type;
	typedef std::deque< hm_wndmsg_observer* > obs_ptr_container_type;
	typedef std::map< wnd_msg_type, obs_ptr_container_type* > wndmsg2obss_map_type;

public:
	hm_wndmsg_publisher();
	virtual ~hm_wndmsg_publisher();
	
public:
	void notifyWndMsg( HWND hWnd, UINT message, WPARAM& wParam, LPARAM& lParam, WNDPROCPARAM* pParamWndProc );
	void orderWndMsg2Front( UINT message, obs_ptr_type pObserver );
	void orderWndMsg2Back( UINT message, obs_ptr_type pObserver );
	bool hasOrderWndMsg( UINT message, obs_ptr_type pObserver ) const;
	void cancelWndMsg( UINT message, obs_ptr_type pObserver );
	void cancelAllWndMsg();
	void RegObserver( hm_wndmsg_observer* pObserver );
	void UnRegObserver( hm_wndmsg_observer* pObserver );

public:
	void pushbackAllWndMsgOrdered( wndmsg_container_type& containerWndMsg ) const;
	void getWndMsgOrdered( obs_ptr_type pObserver, wndmsg_container_type& containerWndMsg ) const;

protected:
	virtual void _doOrderWndMsgFromSource( wnd_msg_type message ){};
	virtual void _doCancelWndMsgFromSource( wnd_msg_type message ){};

protected:
	void _checkCancelWndMsg();
	BOOL _hasObserver( hm_wndmsg_observer* pObserver );
	
private:
#pragma warning(push)
#pragma warning(disable:4251)
	wndmsg2obss_map_type m_mapWndMsg2Obsvrs;
	obs_ptr_container_type m_containerObsvr;
#pragma warning(pop)
};




#endif //__hm_wndmsg_publisher_H__