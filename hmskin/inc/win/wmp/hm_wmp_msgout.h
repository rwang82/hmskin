#ifndef __HM_WMP_MSGOUT_H__
#define __HM_WMP_MSGOUT_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include <string>
#include <vector>
#include "tchar.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\hm_export_defs.h"

// MsgOutEditor v1a ( doc/MsgOutEditor.md ): a read-only chat message list.
// The WMP OWNS its item model ( no runtime keelobj creation in the engine ) -
// it measures and renders every message itself in content coordinates and
// reports the total height through HM_EVENT_CONTENT_EXTENT so a scrollview
// orchestrator ( AnchorBottom ) can drive the bar and the stick-to-bottom /
// do-not-disturb behavior. text measure+render both use GDI DrawText so the
// wrapping contract is one API; bubbles render as GDI+ rounded rects.
class HMLIB_API hm_wmp_msgout : public hm_wmp_display {
public:
	typedef std::basic_string< TCHAR > string_type;
	enum ENUMMSGSENDER { EMSGSENDER_PEER = 0, EMSGSENDER_SELF = 1, EMSGSENDER_SYSTEM = 2 };

	// all lengths arrive ALREADY DPI-scaled from the loader; the font and the
	// colors are owned process-wide by the rawres factories.
	struct MSGOUT_CFG {
		HFONT m_hFontText;
		COLORREF m_clrText;
		COLORREF m_clrBubbleSelf;
		COLORREF m_clrBubblePeer;
		COLORREF m_clrSystem;
		COLORREF m_clrAvatarSelf;
		COLORREF m_clrAvatarPeer;
		int m_nAvatarLen;
		int m_nMaxBubblePct;    // of the available row width, default 80
		int m_nMsgGap;          // default 10
		int m_nPadH;            // bubble inner padding, default 12
		int m_nPadV;            // default 8
		int m_nCornerLen;       // bubble corner radius, default 8
		MSGOUT_CFG();
	};

private:
	struct MsgItem {
		int m_eSender;
		string_type m_strText;
		RECT m_rcRow;      // content coords, relative to the canvas origin
		RECT m_rcBubble;
	};

public:
	hm_wmp_msgout( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_msgout();

	void setConfigure( const MSGOUT_CFG& cfgScaled );
	// appends one message; measures it at the current width, grows the extent
	// and notifies CONTENT_EXTENT. empty text is refused.
	bool appendText( int eSender, const TCHAR* szText );
	void clear();
	unsigned int getCount() const { return (unsigned int)m_containerMsgs.size(); }
	unsigned int getContentHeight() const { return (unsigned int)m_nContentH; }

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );

private:
	void _render( HDC hDC, const RECT& rcCanvas );
	void _measureAll();          // full re-measure ( width changed / clear )
	bool _measureItem( MsgItem& msg, int nRowW );
	void _notifyExtent( int nCanvasW );
	void _OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::vector<MsgItem> m_containerMsgs;
#pragma warning(pop)
	MSGOUT_CFG m_cfg;
	int m_nContentH;             // total stacked height, content px
	int m_nCanvasW;              // last known canvas width ( re-measure on change )
	bool m_bWheelStopped;        // inertia decay seen - engine parked until the next real wheel
};

#endif //__HM_WMP_MSGOUT_H__
