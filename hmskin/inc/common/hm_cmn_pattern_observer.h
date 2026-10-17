//////////////////////////////////////////////////////////////////////////
// file    : hm_cmn_pattern_observer.h
// content: design-pattern classes shared across the engine
//////////////////////////////////////////////////////////////////////////
#ifndef __HM_CMN_PATTERN_OBSERVER_H__
#define __HM_CMN_PATTERN_OBSERVER_H__

// includes
#include "common\hm_cmn_envcfg.h"
#include <deque>
#include <map>
#include <string>
#include "tchar.h"
//#include "hmskin.h"
#include "win\hm_export_defs.h"
CMN_NAMESPACE_START

// observer pattern begin //////////////////////////////////////////////////////////////////////////////
// class declarations
class hm_cmn_subject;
//////////////////////////////////////////////////////////////////////////
// class: hm_cmn_observer_i
// role : observer
//////////////////////////////////////////////////////////////////////////
class HMLIB_API hm_cmn_observer_i
{	
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) = 0;
	
public:
	virtual ~hm_cmn_observer_i(){};
};

//////////////////////////////////////////////////////////////////////////
// class: hm_cmn_subject
// role : subject ( the observed side )
//////////////////////////////////////////////////////////////////////////
class HMLIB_API hm_cmn_subject
{
// type definitions
public:
	typedef hm_cmn_observer_i* obs_ptr_type;
	typedef unsigned int event_id_type;
	typedef std::pair<obs_ptr_type, event_id_type> pobs_event_pair_type;
	typedef std::deque< pobs_event_pair_type > obs_ptr_container_type;
	
public:
	void Notify(event_id_type idEvent, LPARAM pEventParam = 0, LPARAM pEventParam2 = 0 );
	void Register( obs_ptr_type pObserver, event_id_type idEvent );
	bool HasRegister( obs_ptr_type pObserver, event_id_type idEvent ) const;
	bool HasRegister( obs_ptr_type pObserver ) const;
	void UnRegister( obs_ptr_type pObserver, event_id_type idEvent);
	void EraseAllRegister();
	
public:
	hm_cmn_subject(){};
	virtual ~hm_cmn_subject(){ EraseAllRegister(); }

protected:
#pragma warning(push)
#pragma warning(disable:4251)
	obs_ptr_container_type m_pObsContainer; //don't delete point
#pragma warning(pop)
};

// observer pattern end //////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
class QParamBox
{
public:
	typedef short param_flag_type;
	typedef std::basic_string< TCHAR > string_type;
	typedef std::map< param_flag_type, int > map_flag2int_type;
	typedef std::map< param_flag_type, string_type > map_flag2str_type;
	
	// parameters
public:
	//int
	void SetParamInt( param_flag_type flag, int value );
	BOOL HasParamInt( param_flag_type flag ) const;
	BOOL Flag2ParamInt( param_flag_type flag, int& value );
	int Flag2ParamInt( param_flag_type flag );
	void ClearAllParamInt();
	void EraseParamInt( param_flag_type flag );
	//str
	void SetParamStr( param_flag_type flag, const string_type& strValue );
	BOOL HasParamStr( param_flag_type flag ) const;
	BOOL Flag2ParamStr( param_flag_type flag, string_type& strValue );
	string_type Flag2ParamStr( param_flag_type flag );
	void ClearAllParamStr();
	void EraseParamStr( param_flag_type flag );
	
protected:
	map_flag2int_type m_mapFlag2Int;
	map_flag2str_type m_mapFlag2Str;
};

CMN_NAMESPACE_END
#endif //__HM_CMN_PATTERN_OBSERVER_H__