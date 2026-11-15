#ifndef __HM_RS_WNDMARK_H__
#define __HM_RS_WNDMARK_H__
#include <map>
#include <deque>
//////////////////////////////////////////////////////////////////////////
struct hm_rs_wndmark
{
public:
	enum ENUMWNDMARK { WNDMARK_WNDPROPID = 0, WNDMARK_CTRLID };

public:
	ENUMWNDMARK m_eflag;
	union {
		unsigned int m_uWndPropID;
		unsigned int m_uCtrlID;
	};
};
//////////////////////////////////////////////////////////////////////////
class hm_rs_hwnddepository {
	friend class hm_uiroot;
public:
	typedef unsigned int propid_type;
	typedef unsigned int ctrlid_type;
	typedef std::map< propid_type, HWND > propid2hwnd_map_type;
	typedef std::map< ctrlid_type, HWND > ctrlid2hwnd_map_type;
	
public:
	virtual ~hm_rs_hwnddepository();
private:
	hm_rs_hwnddepository();

public:
	BOOL bindHWND2PropId( HWND hWnd, propid_type propId );
	BOOL bindHWND2CtrlId( HWND hWnd, ctrlid_type ctrlId );
	BOOL bindHWND2WndMark( HWND hWnd, const hm_rs_wndmark& wndMark );
	void unBindHWND( HWND hWnd );
	HWND propId2HWND( propid_type propId );
	HWND ctrlId2HWND( ctrlid_type ctrlId );
	HWND wndMark2HWND( const hm_rs_wndmark& wndMark );

private:
	BOOL _isPropIdBind2HWND( propid_type propId );
	BOOL _isCtrlIdBind2HWND( ctrlid_type ctrlId );
	BOOL _isHWNDBind2PropId( HWND hWnd );
	BOOL _isHWNDBind2CtrlId( HWND hWnd );
	BOOL _isBindHWND( HWND hWnd );

public:
	propid2hwnd_map_type m_mapPropId2HWND;
	ctrlid2hwnd_map_type m_mapCtrlId2HWND;
};//////////////////////////////////////////////////////////////////////////
template < typename obj_id_type >
class TWndMarkTable
{
	//type define
public:
	typedef unsigned int wnd_prop_id_type;
	typedef unsigned int ctrl_id_type;
	typedef std::deque< obj_id_type > obj_id_container_type;
	typedef std::map< wnd_prop_id_type, obj_id_container_type* > map_wpid2objid_type;
	typedef std::map< ctrl_id_type, obj_id_container_type* > map_cid2objid_type;
	typedef std::map< obj_id_type, hm_rs_wndmark* > map_objid2mark_type;
//	
public:
	bool bindObjId2WndMark( obj_id_type objId, const hm_rs_wndmark& markWnd );
	bool getObjIdContainer( const hm_rs_wndmark& markWnd, obj_id_container_type* pcontainerObjId );
	bool getWndMark( obj_id_type objId, hm_rs_wndmark* pwndMark );
//
public:
	TWndMarkTable();
	~TWndMarkTable();
//
//private:
	void clearAllData();
//
private:
	map_wpid2objid_type m_mapWPId2ObjId;
	map_cid2objid_type m_mapCId2ObjId;
	map_objid2mark_type m_mapObjId2WndMark;	
};
//
template< typename obj_id_type >
bool TWndMarkTable< obj_id_type >::bindObjId2WndMark( obj_id_type objId, const hm_rs_wndmark& markWnd )
{
	if ( m_mapObjId2WndMark.find( objId ) != m_mapObjId2WndMark.end() )
		return false; // the objId have been bind to window before.
	hm_rs_wndmark* pwndMark = NULL;
	
	//save to m_mapObjId2WndMark
	pwndMark = new hm_rs_wndmark();
	memcpy( pwndMark, &markWnd, sizeof( hm_rs_wndmark ) );
	m_mapObjId2WndMark[ objId ] = pwndMark;
	////save to m_mapWPId2ObjId or m_mapCId2ObjId
	switch ( markWnd.m_eflag )
	{
	case hm_rs_wndmark::WNDMARK_WNDPROPID:
		{
			map_wpid2objid_type::iterator it;
			obj_id_container_type* pcontainerObjId = NULL;

			it = m_mapWPId2ObjId.find( markWnd.m_uWndPropID );
			if ( it != m_mapWPId2ObjId.end() ) {
				pcontainerObjId = it->second;
			} else 	{
				pcontainerObjId = new obj_id_container_type();
				m_mapWPId2ObjId[ markWnd.m_uWndPropID ] = pcontainerObjId;
			}
			//
			pcontainerObjId->push_back( objId );
		}
		break;
	case hm_rs_wndmark::WNDMARK_CTRLID:
		{
			map_cid2objid_type::iterator it;
			obj_id_container_type* pcontainerObjId = NULL;
			
			it = m_mapCId2ObjId.find( markWnd.m_uCtrlID );
			if ( it != m_mapCId2ObjId.end() ) {
				pcontainerObjId = it->second;
			} else {
				pcontainerObjId = new obj_id_container_type();
				m_mapCId2ObjId[ markWnd.m_uCtrlID ] = pcontainerObjId;
			}
			//
			pcontainerObjId->push_back( objId );
		}
		break;
	default:
		break;
	}
	
	return true;
}
//
template< typename obj_id_type >
bool TWndMarkTable< obj_id_type >::getObjIdContainer( const hm_rs_wndmark& markWnd, obj_id_container_type* pcontainerObjId )
{
	if ( !pcontainerObjId )
		return false;
	//
	switch( markWnd.m_eflag )
	{
	case hm_rs_wndmark::WNDMARK_WNDPROPID:
		{
			map_wpid2objid_type::iterator itObjIdContainer;
			obj_id_container_type::iterator itObjId, iendObjId;
			
			itObjIdContainer = m_mapWPId2ObjId.find( markWnd.m_uWndPropID );
			if ( itObjIdContainer == m_mapWPId2ObjId.end() )
				return false;
			iendObjId = itObjIdContainer->second->end();
			for ( itObjId = itObjIdContainer->second->begin(); itObjId!=iendObjId; ++itObjId )
			{
				pcontainerObjId->push_back( *itObjId );
			}
			return true;
		}
		break;
	case hm_rs_wndmark::WNDMARK_CTRLID:
		{
			map_cid2objid_type::iterator itObjIdContainer;
			obj_id_container_type::iterator itObjId, iendObjId;
			
			itObjIdContainer = m_mapCId2ObjId.find( markWnd.m_uCtrlID );
			if ( itObjIdContainer == m_mapCId2ObjId.end() )
				return false;
			iendObjId = itObjIdContainer->second->end();
			for ( itObjId = itObjIdContainer->second->begin(); itObjId!=iendObjId; ++itObjId )
			{
				pcontainerObjId->push_back( *itObjId );
			}

			return true;
		}
		break;
	default:
		{}
		break;
	}
	
	return false;
}
//
template< typename obj_id_type >
bool TWndMarkTable< obj_id_type >::getWndMark( obj_id_type objId, hm_rs_wndmark* pwndMark )
{
	if ( !pwndMark )
		return false;
	map_objid2mark_type::iterator itWndMark;

	itWndMark = m_mapObjId2WndMark.find( objId );
	if ( itWndMark == m_mapObjId2WndMark.end() )
		return false;
	//
	*pwndMark = *(m_mapObjId2WndMark[ objId ]);
	return true;
}
//
template< typename obj_id_type >
void TWndMarkTable< obj_id_type >::clearAllData()
{
	map_wpid2objid_type::iterator itWP2Obj, iendWP2Obj;
	map_cid2objid_type::iterator itC2Obj, iendC2Obj;
	map_objid2mark_type::iterator itObj2Mark, iendObj2Mark;

	//
	iendWP2Obj = m_mapWPId2ObjId.end();
	for ( itWP2Obj = m_mapWPId2ObjId.begin(); itWP2Obj!=iendWP2Obj; ++itWP2Obj ) {
		obj_id_container_type* pcontainerObjId = itWP2Obj->second;
		if ( pcontainerObjId )
			delete pcontainerObjId;
		pcontainerObjId = NULL;
	}
	m_mapWPId2ObjId.clear();

	//
	iendC2Obj = m_mapCId2ObjId.end();
	for ( itC2Obj = m_mapCId2ObjId.begin(); itC2Obj!=iendC2Obj; ++itC2Obj ) {
		if ( itC2Obj->second )
			delete itC2Obj->second;
	}
	m_mapCId2ObjId.clear();

	//
	iendObj2Mark = m_mapObjId2WndMark.end();
	for ( itObj2Mark = m_mapObjId2WndMark.begin(); itObj2Mark!=iendObj2Mark; ++itObj2Mark ) {
		if ( itObj2Mark->second )
			delete itObj2Mark->second;
	}
	m_mapObjId2WndMark.clear();

}
//
template< typename obj_id_type >
TWndMarkTable< obj_id_type >::TWndMarkTable()
{}
//
template< typename obj_id_type >
TWndMarkTable< obj_id_type >::~TWndMarkTable()
{
	clearAllData();
}

#endif //__HM_RS_WNDMARK_H__