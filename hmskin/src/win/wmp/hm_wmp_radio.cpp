#include "stdafx.h"
#include "win\wmp\hm_wmp_radio.h"

// all behavior lives in the two protected overrides declared in the header
// ( keep-checked + the RADIO_CLICK event id ). everything else - the 8-state
// machine, double-buffered drawSelf, setCheck + STATECHANGED notification -
// is inherited from hm_wmp_checkbox untouched.

hm_wmp_radio::hm_wmp_radio( hmskin_event_listener_base* pEventListener )
: hm_wmp_checkbox( pEventListener ) {
}

hm_wmp_radio::~hm_wmp_radio() {
}
