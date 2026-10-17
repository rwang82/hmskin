#ifndef __HM_CMN_ENVCFG_H__
#define __HM_CMN_ENVCFG_H__

#define CMN_NAMESPACE_START namespace cmn{
#define CMN_NAMESPACE_END }
#define USING_NAMESPACE_CMN using namespace cmn;

CMN_NAMESPACE_START

// event id definitions begin //////////////////////////////////////////////////////////////////////////////
const UINT ID_QEVENT_UILIB_BASE = WM_USER + 200;

// notification: a layout position has changed
const UINT ID_EVENT_POSCHANGED = ID_QEVENT_UILIB_BASE + 1;



CMN_NAMESPACE_END
#endif //__HM_CMN_ENVCFG_H__