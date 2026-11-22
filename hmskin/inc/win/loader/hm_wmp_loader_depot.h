#ifndef __HM_WMP_LOADER_DEPOT_H__
#define __HM_WMP_LOADER_DEPOT_H__
#include <map>
#include <string>
#include "win\loader\hm_loader_base.h"   // HM_LOADER_FAIL
//
namespace Json {
	class Value;
}
class hm_wmp_loader_base;
//
class hm_wmp_loader_depot{
public:
	typedef std::map< std::string, hm_wmp_loader_base* > name2wmploader_map_type;
public:
	hm_wmp_loader_depot();
	virtual ~hm_wmp_loader_depot();

public:
	hm_wmp_loader_base* getWMPLoader( const Json::Value& valWMP );

private:
	void _init();
	void _unInit();
	bool _getWMPName( const Json::Value& valWMP, std::string& strWMPName );

private:
	name2wmploader_map_type m_mapName2WMPLoader;
};

#endif //__HM_WMP_LOADER_DEPOT_H__