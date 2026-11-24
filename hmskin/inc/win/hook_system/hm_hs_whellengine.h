#ifndef __HM_HS_WHELLENGINE_H__
#define __HM_HS_WHELLENGINE_H__
#include "common/hm_cmn_pattern_observer.h"
#include <list>
//
#define TRACKID_NULL ( 0 )
#define TRACKID_BEGIN ( TRACKID_NULL + 2000 )
//
#define WTDFLAG_WHEEL_DIRECT_MASK (0x00000001)
#define WTDFLAG_WHEEL_FORWORD (0x00000001)
struct WHELLTAGDATA {
	unsigned int m_uTime; //
	unsigned int m_uFlag; //
	HWND m_hWnd;
	POINT m_pt;
};
typedef std::list< WHELLTAGDATA > wtd_container_type;
//
class hm_hs_whellengine : public cmn::hm_cmn_subject {
public:
	enum ENUMFRAMEMODE { FRAME60, FRAME24, FRAME12, FRAME6 };
	
public:
	static hm_hs_whellengine* getInstance();
	~hm_hs_whellengine();

public:
	void tag( const WHELLTAGDATA& wtData );
	void setFrameMode( ENUMFRAMEMODE eFrameMode );
	unsigned int getTriggerInterval() { return m_uTriggerInterval; }
	void trigger();
	void pauseTriggerThread();
	void resumeTriggerThread();

private:
	hm_hs_whellengine();
	void _initTriggerThread();
	void _exitTriggerThread();
	void _increaseAccessOpCount();
	bool _decreaseAccessOpCount();
	void _waitAccessOpCountToZero();
	
protected:
	HANDLE m_hThreadWheelTriggerProc;
	HANDLE m_hEventExitTriggerProc;
	unsigned int m_uTriggerInterval;
	unsigned int m_uNoTriggerTagCount;
	wtd_container_type m_containerWTD; // wheel tag data container for tag.
	HANDLE m_hEventAccessWTD;
	HANDLE m_hEventCancelAllOp;
	unsigned int m_uOpCount;
	HANDLE m_hEventAccessOpCount;
	HANDLE m_hEventTriggerRun;
};
//
#define WHELLENGINE_INSTANCE (hm_hs_whellengine::getInstance())
//
class hm_hs_whellhelper {
public:
	virtual ~hm_hs_whellhelper();
	static hm_hs_whellhelper* getInstance();

public:
	// uRAccelerationPixelSec - resistance acceleration of whell action. unit: pixel / ms^2 ( normal: 1 - 10)
	int getDistancePixel( unsigned int uNoTriggerTagCount, const wtd_container_type* pWTDContainer, unsigned int uRAccelerationPixelSec = 400 );

private:
	hm_hs_whellhelper();
	int calcDistance( unsigned int uSpeedPixelSecAvailable, unsigned int uTimeEndSpeedAvailable, unsigned int uTimeCur, unsigned int uNoTriggerTagCount, const wtd_container_type* pWTDContainer, unsigned int uRAccelerationPixelSec );

private:
	unsigned int m_uLinesPerTag;
	unsigned int m_uPixelPerLine;
};
#define WHELLHELPER_INSTANCE (hm_hs_whellhelper::getInstance())

#endif //__HM_HS_WHELLENGINE_H__