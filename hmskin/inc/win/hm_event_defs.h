#ifndef __HM_EVENT_DEFS__
#define __HM_EVENT_DEFS__
//////////////////////////////////////////////////////////////////////////
//hmlib event define
#define HM_EVENT_BEGIN ( 1 )
#define HM_EVENT_WND_MSG ( HM_EVENT_BEGIN + 1 )
#define HM_EVENT_COORDSYS_CHANGE ( HM_EVENT_BEGIN + 2 )
/////////////////////////////////////////////////////////////////////////
// pEventParam - HDC
//#define HM_EVENT_DRAW_KEELOBJ ( HM_EVENT_BEGIN + 3 )
/////////////////////////////////////////////////////////////////////////
// pEventParam - HDC
// pEventParam2 - hWndCanvas
#define HMKEELOBJEVENT_DRAWSUBOBJS ( HM_EVENT_BEGIN + 4 )
/////////////////////////////////////////////////////////////////////////
class hm_layout_obj;
struct HEDSPARAM {
	HDC m_hDCCanvas;
	HWND m_hWndCanvas;
	hm_layout_obj* m_pLayoutObjCanvas;
};
// pEventParam - HEDSPARAM*
#define HMKEELOBJEVENT_DRAWSELF ( HM_EVENT_BEGIN + 5 )
/////////////////////////////////////////////////////////////////////////
// pEventParam - hm_layout_obj* pLayoutObjNew
// pEventParam - hm_layout_obj* pLayoutObjOld
#define HMKEELOBJEVENT_LAYOUTOBJ_CHANGED ( HM_EVENT_BEGIN + 6 )
/////////////////////////////////////////////////////////////////////////
struct content_position {
	int m_nContentPosStart;
	int m_nContentPosEnd;
};
class hm_keel_obj;
struct HEVMPARAM {
	content_position m_vertical;
	content_position m_horizontal;
	hm_keel_obj* m_pViewContentKeelObj;
	HWND m_hWndCanvas;
	HDC m_hDCMem;
	SIZE m_sizeCanvas;
};
// pEventParam - HEVMPARAM*
#define HM_EVENT_MOVE_VIEWCONTENT ( HM_EVENT_BEGIN + 7 )
/////////////////////////////////////////////////////////////////////////
class hm_wmp_scrollbar;
struct HETMPARAM {
	int m_nPosCur;
	int m_nPosMax;
	hm_wmp_scrollbar* m_pWMPScrollbar;
};
// pEventParam - HETMPARAM*
#define HM_EVENT_TRACKTHUMB_MOVED ( HM_EVENT_BEGIN + 8 )
/////////////////////////////////////////////////////////////////////////
// pEventParam - int (specify the distance that wheel moved in pixel, the value < 0 means forwards, > 0 means backwards )
// pEventParam2 - NULL.
#define HM_EVENT_MOUSEWHELL ( HM_EVENT_BEGIN + 9 )
//////////////////////////////////////////////////////////////////////////
// pEventParam - specify the pointer of hm_keel_obj
// pEventParam2 - NULL.
#define HMKEELOBJEVENT_POSCHANGED ( HM_EVENT_BEGIN + 10 )
//////////////////////////////////////////////////////////////////////////
// pEventParam - specify bVisible
// pEventParam2 - NONE.
#define HMKEELOBJEVENT_SETVISIBLE ( HM_EVENT_BEGIN + 11 )
//////////////////////////////////////////////////////////////////////////
// pEventParam - specify the current selected index of tab button.
// pEventParam2 - specify the old selected index of tab button. if no old item selected, the index is INDEX_NONE
#define HMWMPTABEVENT_SELECTCHANGED ( HM_EVENT_BEGIN + 12 )
//////////////////////////////////////////////////////////////////////////
struct HKEREPARAM{
	void* m_pEventSender;
	unsigned int m_uEvent;
	long m_lEventParam;
	long m_lEventParam2;
};
// pEventParam - specify the pointer of HKEREPARAM.
// pEventParam2 - NONE.
#define HMKEELOBJEVENT_RELAYEVENT ( HM_EVENT_BEGIN + 13 )
//////////////////////////////////////////////////////////////////////////
// a WMP's state changed from OUTSIDE the message flow ( e.g. checkbox
// setCheck called by radio-group logic ). the owning keelobj subscribes at
// pushbackWMP and invalidates its own rect - the WMP itself stays ignorant
// of keelobjs ( dependency stays owner -> member ).
// pEventParam - NONE.
// pEventParam2 - NONE.
#define HM_EVENT_WMP_STATECHANGED ( HM_EVENT_BEGIN + 14 )

#endif //__HM_EVENT_DEFS__