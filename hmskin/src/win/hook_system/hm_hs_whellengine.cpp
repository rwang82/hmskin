#include "stdafx.h"
#include "win\hook_system\hm_hs_whellengine.h"
#include "common\MemFuncPack.h"
#include "win\hm_wndmsg_defs.h"
#include <assert.h>
//
#define PIXELPERTAG ( m_uLinesPerTag * m_uPixelPerLine )
#define TAGDATAMAX ( 50 )
const unsigned int TRIGGERTAILMAX (5*1000); //ms
const unsigned int FRAME24_MS = 1000/24;
const unsigned int FRAME12_MS = 1000/12;
const unsigned int FRAME6_MS = 1000/6;
struct TWTPPARAM {
	hm_hs_whellengine* m_pWheelEngine;
	HANDLE m_hEventExitTriggerProc;
	HANDLE m_hEventTriggerRun;
};
DWORD WINAPI ThreadWheelTriggerProc( LPVOID lpParameter );
#define ENTER_TAGS_ACCESS() \
	HANDLE aryEvent[ 2 ]; \
	DWORD dwWait; \
	_increaseAccessOpCount(); \
	CMemFuncPack mfpkDCAOC( this, &hm_hs_whellengine::_decreaseAccessOpCount ); \
	aryEvent[ 0 ] = m_hEventCancelAllOp; \
	aryEvent[ 1 ] = m_hEventAccessWTD; \
	dwWait = ::WaitForMultipleObjects( 2, aryEvent, false, INFINITE ); \
	if ( dwWait == WAIT_OBJECT_0 ) { \
		return; \
	} else if ( dwWait == ( WAIT_OBJECT_0 + 1 ) ) { \
	}
#define EXIT_TAGS_ACCESS() \
	if ( dwWait == ( WAIT_OBJECT_0 + 1 ) ) { \
	::SetEvent( m_hEventAccessWTD ); \
	} \
	mfpkDCAOC.Do();
//
hm_hs_whellengine::hm_hs_whellengine() 
: m_hThreadWheelTriggerProc ( NULL )
, m_uTriggerInterval( FRAME24_MS )
, m_uOpCount( 0 )
, m_uNoTriggerTagCount( 0 ) {
	m_hEventExitTriggerProc = ::CreateEvent( NULL, false, false, NULL );
	m_hEventAccessWTD = ::CreateEvent( NULL, false, true, NULL );
	m_hEventCancelAllOp = ::CreateEvent( NULL, true, false, NULL );
	m_hEventAccessOpCount = ::CreateEvent( NULL, false, true, NULL );
	m_hEventTriggerRun = ::CreateEvent( NULL, true, false, NULL );

	//
	_initTriggerThread();
}

hm_hs_whellengine::~hm_hs_whellengine() {
	//
	_exitTriggerThread();

	// cancel all operation.
	::SetEvent( m_hEventCancelAllOp );
	_waitAccessOpCountToZero();
	::CloseHandle( m_hEventAccessWTD );
	::CloseHandle( m_hEventCancelAllOp );
	::CloseHandle( m_hEventAccessOpCount );
	::CloseHandle( m_hEventTriggerRun );


}

void hm_hs_whellengine::_increaseAccessOpCount() {
	::WaitForSingleObject( m_hEventAccessOpCount, INFINITE );
	++m_uOpCount;
	::SetEvent( m_hEventAccessOpCount );
}

bool hm_hs_whellengine::_decreaseAccessOpCount() {
	if ( m_uOpCount == 0 ) return false;
	::WaitForSingleObject( m_hEventAccessOpCount, INFINITE );
	--m_uOpCount;
	::SetEvent( m_hEventAccessOpCount );
	return true;
}

void hm_hs_whellengine::_waitAccessOpCountToZero() {
	while ( 1 ) {
		::WaitForSingleObject( m_hEventAccessOpCount, INFINITE );	
		if ( m_uOpCount == 0 ) {
			::SetEvent( m_hEventAccessOpCount );
			break;
		}
		::SetEvent( m_hEventAccessOpCount );
	}
}

void hm_hs_whellengine::setFrameMode( ENUMFRAMEMODE eFrameMode ) {
	switch ( eFrameMode ) {
	case FRAME24:
		{
			m_uTriggerInterval = FRAME24_MS;
		}
		break;
	case FRAME12:
		{
			m_uTriggerInterval = FRAME12_MS;
		}
		break;
	case FRAME6:
		{
			m_uTriggerInterval = FRAME6_MS;
		}
		break;
	default:
		break;
	}
}

void hm_hs_whellengine::tag( const WHELLTAGDATA& wtData ) {
	//
	ENTER_TAGS_ACCESS()
	//::OutputDebugString( _T("enter hm_hs_whellengine::tag\n") );
	//
	if ( m_containerWTD.size() != 0 ) { // check if direct of wheel change.
		wtd_container_type::iterator itWTData;

		itWTData = m_containerWTD.begin();
		if ( ( itWTData->m_uFlag & WTDFLAG_WHEEL_DIRECT_MASK ) 
			!= ( wtData.m_uFlag & WTDFLAG_WHEEL_DIRECT_MASK ) ) {
				m_containerWTD.clear();
				m_uNoTriggerTagCount = 0;
		}
	}
	if ( m_containerWTD.size() > TAGDATAMAX )
		m_containerWTD.pop_back();

	//
	m_containerWTD.push_front( wtData );
	++m_uNoTriggerTagCount;
	//
	//::OutputDebugString( _T("exit hm_hs_whellengine::tag\n") );
	EXIT_TAGS_ACCESS()

	//
	resumeTriggerThread();
	
}

void hm_hs_whellengine::_initTriggerThread() {
	if ( m_hThreadWheelTriggerProc ) return;
	DWORD dwTId;
	TWTPPARAM* pTWTPParam = new TWTPPARAM();
	//
	pTWTPParam->m_pWheelEngine = this;
	pTWTPParam->m_hEventExitTriggerProc = this->m_hEventExitTriggerProc;
	pTWTPParam->m_hEventTriggerRun = this->m_hEventTriggerRun;
	//
	m_hThreadWheelTriggerProc = ::CreateThread( NULL, 0, ThreadWheelTriggerProc, pTWTPParam, 0, &dwTId );
}

void hm_hs_whellengine::_exitTriggerThread() {
	if ( m_hThreadWheelTriggerProc ) {
		::SetEvent( m_hEventExitTriggerProc );
		::WaitForSingleObject( m_hThreadWheelTriggerProc, INFINITE );
		::CloseHandle( m_hEventExitTriggerProc );
		m_hThreadWheelTriggerProc = NULL;
	}
}

hm_hs_whellengine* hm_hs_whellengine::getInstance() {
	static hm_hs_whellengine s_instance;
	return &s_instance;
}

void hm_hs_whellengine::trigger() {
	//
	ENTER_TAGS_ACCESS()
	// post HMWM_MOUSEWHEEL message - ONLY while new wheel tags keep coming.
	// the original design relied on a consumer ( viewbkcontent ) calling
	// pauseTriggerThread() when the inertia decayed to zero; with no
	// consumer the thread posted forever once woken ( 5668 msgs per
	// session ). no new tags = self-park.
	if ( m_containerWTD.size() > 0 && m_uNoTriggerTagCount > 0 ) {
		wtd_container_type::iterator itWTD;

		itWTD = m_containerWTD.begin();
		::PostMessage( itWTD->m_hWnd, HMWM_MOUSEWHEEL, m_uNoTriggerTagCount, (LPARAM)(&m_containerWTD) );
	} else {
		::ResetEvent( m_hEventTriggerRun );
	}
	// reset m_uNoTriggerTagCount.
	m_uNoTriggerTagCount = 0;
	
	//
	EXIT_TAGS_ACCESS()
}

void hm_hs_whellengine::pauseTriggerThread() {
	::ResetEvent( m_hEventTriggerRun );
}

void hm_hs_whellengine::resumeTriggerThread() {
	::SetEvent( m_hEventTriggerRun );
}

DWORD WINAPI ThreadWheelTriggerProc( LPVOID lpParameter ) {
	if ( !lpParameter ) return 0;
	TWTPPARAM* pTWTPParam = ( TWTPPARAM* )lpParameter;
	hm_hs_whellengine* whellengine = NULL;
	HANDLE aryEvent[ 2 ];
	int nWait;

	whellengine = pTWTPParam->m_pWheelEngine;
	aryEvent[ 0 ] = pTWTPParam->m_hEventExitTriggerProc;
	aryEvent[ 1 ] = pTWTPParam->m_hEventTriggerRun;
	//
	delete pTWTPParam;
	pTWTPParam = NULL;
	//
	while ( 1 ) {
		nWait = ::WaitForMultipleObjects( 2, aryEvent, false, INFINITE );
		//
		if ( nWait == WAIT_OBJECT_0 ) {
			break; //m_hEventExitTriggerProc
		} else if ( nWait == ( WAIT_OBJECT_0 + 1 ) ) {
			// continue send trigger.
			int a = 0;
			//::OutputDebugString( _T("vvvvvvvvvvvvvv whellengine->trigger()\n") );
			whellengine->trigger();
			//::OutputDebugString( _T("^^^^^^^^^^^^^^ whellengine->trigger()\n") );
			::Sleep( whellengine->getTriggerInterval() );
		}

	}
	
	return 0;
}
//
hm_hs_whellhelper::~hm_hs_whellhelper() {}
//
hm_hs_whellhelper* hm_hs_whellhelper::getInstance() {
	static hm_hs_whellhelper s_instance;
	return &s_instance;
}
//
int hm_hs_whellhelper::getDistancePixel( unsigned int uNoTriggerTagCount, const wtd_container_type* pWTDContainer, unsigned int uRAccelerationPixelSec ) {
	if ( !pWTDContainer || ( pWTDContainer->size() == 0 ) ) return 0;
	int nPixelTotal = 0;
	unsigned int uTriggerInterval;
	unsigned int uTimeBegin;
	unsigned int uTimeEnd;
	unsigned int uTimeCur;
	unsigned int uTimeEndSpeedAvailable = 0;
	unsigned int uSpeedPixelSecAvailable = 0;
	unsigned int uSpeedPixelSec;
	unsigned int uSpeedPixelCalc;
	unsigned int uTagCount = 0;
	unsigned int uPixelPerTag = 0;
	wtd_container_type::const_reverse_iterator critWTD, criendWTD; 
	wtd_container_type::const_iterator citWTD;
	
	uTriggerInterval = WHELLENGINE_INSTANCE->getTriggerInterval();
	uTimeCur = ::GetTickCount();
	{
		static unsigned int s_uTimeLast = 0;
		TCHAR szMsg[ 100 ];
		_stprintf_s( szMsg, _T("uTimeCur:%d time_space:%d uNoTriggerTagCount:%d uTriggerInterval:%d\n"), uTimeCur, ( s_uTimeLast == 0 ) ? 0 : ( uTimeCur - s_uTimeLast ), uNoTriggerTagCount, uTriggerInterval );
//		::OutputDebugString( szMsg );
		s_uTimeLast = uTimeCur;
	}
	uPixelPerTag = PIXELPERTAG;
	// get available whell speed.
	uSpeedPixelSecAvailable = 0;
	uTagCount = 0;
	uTimeBegin = pWTDContainer->rbegin()->m_uTime;
	uTimeEnd = uTimeBegin + uTriggerInterval;
	uTimeEndSpeedAvailable = uTimeEnd;
	criendWTD = pWTDContainer->rend();
	for ( critWTD = pWTDContainer->rbegin(); critWTD!=criendWTD; ++critWTD ) {
		//
		if ( critWTD->m_uTime > uTimeEnd ) {
			uSpeedPixelSec = uTagCount * uPixelPerTag * 1000 / uTriggerInterval;
			if ( uSpeedPixelSec + uRAccelerationPixelSec*( uTimeEnd - uTimeEndSpeedAvailable )/1000 > uSpeedPixelSecAvailable ) {
				uSpeedPixelSecAvailable = uSpeedPixelSec;
				uTimeEndSpeedAvailable = uTimeEnd;
				{
					TCHAR szMsg[200];
					_stprintf_s( szMsg, _T("speed_availd:%d, uTagCount:%d, timeend_avaid:%d\n"), uSpeedPixelSecAvailable, uTagCount, uTimeEndSpeedAvailable  );
//					::OutputDebugString( szMsg );
				}
			}
			uTimeBegin = critWTD->m_uTime;
			uTimeEnd = uTimeBegin + uTriggerInterval;
			uTagCount = 1;
			continue;
		}
		//
		++uTagCount;
	}
	assert( uTagCount != 0 );
	if ( uTimeCur < uTimeEnd ) {
		uTimeEnd = uTimeCur;
	}
	{
		TCHAR szMsg[100];
		_stprintf_s( szMsg, _T("uTagCount:%d, time:%d, time2available:%d\n"), uTagCount, ( uTimeEnd - uTimeBegin ), ( uTimeEnd - uTimeEndSpeedAvailable ) );
//		::OutputDebugString( szMsg );
	}
	assert( uTimeEnd >= uTimeBegin );
	if ( uTimeEnd == uTimeBegin ) {
		uSpeedPixelSec = 0;
	} else {
		uSpeedPixelSec = uTagCount * uPixelPerTag * 1000 / ( uTimeEnd - uTimeBegin );
	}
	uSpeedPixelCalc = uSpeedPixelSec + uRAccelerationPixelSec*( uTimeEnd - uTimeEndSpeedAvailable )/1000;
	if ( uSpeedPixelCalc > uSpeedPixelSecAvailable ) {
		uSpeedPixelSecAvailable = uSpeedPixelSec;
		uTimeEndSpeedAvailable = uTimeEnd;
		{
	//		TCHAR szMsg[100];
	//		_stprintf_s( szMsg, _T("uSpeedPixelSec:%d, uSpeedPixelCalc:%d, uSpeedPixelSecAvailable:%d\n"), uSpeedPixelSec, uSpeedPixelCalc, uSpeedPixelSecAvailable );
	//		::OutputDebugString( szMsg );
		}
		
		nPixelTotal = uTagCount * PIXELPERTAG;
		citWTD = pWTDContainer->begin();
		//nPixelTotal *= ( ( citWTD->m_uFlag & WTDFLAG_WHEEL_FORWORD ) ? -1 : 1 );
		nPixelTotal = calcDistance( uSpeedPixelSecAvailable, uTimeEndSpeedAvailable, uTimeCur, uNoTriggerTagCount, pWTDContainer, uRAccelerationPixelSec );		
		::OutputDebugString(_T("uSpeedPixelCalc > uSpeedPixelSec\n"));
	} else {
		nPixelTotal = calcDistance( uSpeedPixelSecAvailable, uTimeEndSpeedAvailable, uTimeCur, uNoTriggerTagCount, pWTDContainer, uRAccelerationPixelSec );		
		::OutputDebugString(_T("uSpeedPixelCalc <= uSpeedPixelSec\n"));
	}

	
	TCHAR szMsg[100];
	_stprintf_s( szMsg, _T("nPixelTotal:%d\n"), nPixelTotal );
	::OutputDebugString( szMsg );
	return nPixelTotal;
}

int hm_hs_whellhelper::calcDistance( unsigned int uSpeedPixelSecAvailable, unsigned int uTimeEndSpeedAvailable, unsigned int uTimeCur, unsigned int uNoTriggerTagCount, const wtd_container_type* pWTDContainer, unsigned int uRAccelerationPixelSec ) {
	if ( !pWTDContainer 
	|| ( pWTDContainer->size() == 0 ) 
	|| ( uRAccelerationPixelSec == 0 ) ) 
		return 0;
	int nPixelTotal = 0;
	unsigned int uTimeWheelStop;
	unsigned int uTriggerInterval;
	unsigned int uPixelPerTag;
	unsigned int uSpeedCur;
	unsigned int uSpeedLastTrigger;
	unsigned int uTimeLastTrigger;
	wtd_container_type::const_iterator citWTD;
	
	uPixelPerTag = PIXELPERTAG;
	uTriggerInterval = WHELLENGINE_INSTANCE->getTriggerInterval();
	uTimeLastTrigger = uTimeCur - uTriggerInterval;
	// calc wheel stoped time
	uTimeWheelStop = ( uSpeedPixelSecAvailable * 1000 / uRAccelerationPixelSec ) + uTimeEndSpeedAvailable;
	
	// calc nPixelTotal
	{
		TCHAR szMsg[100];
		_stprintf_s( szMsg, _T("uTimeEndSpeedAvailable:%d, uTimeLastTrigger:%d\n"), uTimeEndSpeedAvailable, uTimeLastTrigger );
		::OutputDebugString( szMsg );
		_stprintf_s( szMsg, _T("uTimeCur:%d, uTimeWheelStop:%d\n"), uTimeCur, uTimeWheelStop );
		::OutputDebugString( szMsg );
	}
	if ( ( pWTDContainer->size() == 1 && ( uNoTriggerTagCount == 1 ) ) 
		|| ( uTimeCur == uTimeEndSpeedAvailable ) ) {
		nPixelTotal = PIXELPERTAG;
		::OutputDebugString(_T("--------L------T1-----C-------\n"));
	} else {
		if ( uTimeEndSpeedAvailable < uTimeLastTrigger ) {
			if ( uTimeLastTrigger > uTimeWheelStop ) {
				nPixelTotal = 0;
				::OutputDebugString(_T("--E--S--L------------C-------\n"));
			} else {
				if ( uTimeCur > uTimeWheelStop ) {
					uSpeedLastTrigger = uSpeedPixelSecAvailable - ( uTimeLastTrigger - uTimeEndSpeedAvailable ) * uRAccelerationPixelSec / 1000;
					nPixelTotal = uSpeedLastTrigger * ( uTimeWheelStop - uTimeLastTrigger ) / 1000 / 2;
					::OutputDebugString(_T("--E-----L-----S------C-------\n"));
				} else {
					uSpeedLastTrigger = uSpeedPixelSecAvailable - ( uTimeLastTrigger - uTimeEndSpeedAvailable ) * uRAccelerationPixelSec / 1000;
					uSpeedCur = uSpeedPixelSecAvailable - ( uTimeCur - uTimeEndSpeedAvailable ) * uRAccelerationPixelSec / 1000;
					nPixelTotal = ( uSpeedLastTrigger + uSpeedCur ) * uTriggerInterval / 1000 / 2;
					::OutputDebugString(_T("--E-----L------------C--S----\n"));
				}
			}
		} else {
			if ( uTimeCur > uTimeWheelStop ) {
				nPixelTotal = uSpeedPixelSecAvailable * ( uTimeWheelStop - uTimeEndSpeedAvailable ) / 1000 / 2;
				::OutputDebugString(_T("--------L---E---S----C-------\n"));
			} else {
				uSpeedCur = uSpeedPixelSecAvailable - ( uTimeCur - uTimeEndSpeedAvailable ) * uRAccelerationPixelSec / 1000;
				nPixelTotal = ( uSpeedPixelSecAvailable + uSpeedCur ) * ( uTimeCur - uTimeEndSpeedAvailable ) / 1000 / 2;
				::OutputDebugString(_T("--------L---E--------C---S---\n"));
			}
		}
	}

	//
	citWTD = pWTDContainer->begin();
	nPixelTotal *= ( ( citWTD->m_uFlag & WTDFLAG_WHEEL_FORWORD ) ? -1 : 1 );
	
	return nPixelTotal;
}

//
hm_hs_whellhelper::hm_hs_whellhelper() 
: m_uLinesPerTag( 3 )
, m_uPixelPerLine( 12 ) {
	unsigned int uLines;

	uLines = 1;
	if ( SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 ) ) {
		m_uLinesPerTag = uLines;
	}
}