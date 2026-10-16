#ifndef __HM_WMP_RADIO_H__
#define __HM_WMP_RADIO_H__
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\hm_export_defs.h"

// radio button: the checkbox state machine in radio semantics.
// - a click on the checked item KEEPS it checked ( no intermediate
//   unchecked state ever reaches the screen )
// - clicks fire HMSKIN_EVENT_RADIO_CLICK instead of CHECKBOX_CLICK, so
//   app listeners never sniff keelobj ids to tell them apart.
// group semantics ( pick one, uncheck siblings ) stay app-side: group
// membership is an application concept, setCheck covers the rest.
class HMLIB_API hm_wmp_radio : public hm_wmp_checkbox
{
public:
	hm_wmp_radio( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_radio();

protected:
	virtual BOOL _keepCheckedOnClick() { return TRUE; }
	virtual unsigned int _getClickEventId() { return HMSKIN_EVENT_RADIO_CLICK; }
};

#endif //__HM_WMP_RADIO_H__
