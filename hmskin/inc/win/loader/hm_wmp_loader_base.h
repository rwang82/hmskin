#ifndef __HM_WMP_LOADER_BASE_H__
#define __HM_WMP_LOADER_BASE_H__
#include <string>
//
namespace Json {
	class Value;
}
class hm_wndmsgprocessor_i;
class hm_uiroot;
//
class hm_wmp_loader_base {
public:
	hm_wmp_loader_base() : m_dblScale( 1.0 ) {};
	virtual ~hm_wmp_loader_base(){};

public:
	hm_wndmsgprocessor_i* createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
protected:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading ) { return true; };
private:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) = 0;

protected:
	// DPI scale snapshot taken from the uiroot at createWMP entry ( the
	// movewnd-loader precedent ): the fill chain below reaches posdecoder
	// parse sites that no longer see the root directly.
	double m_dblScale;
	bool _getWMPName( const Json::Value& valWMP, std::string& strWMPName );
	bool _getWMPId( const Json::Value& valWMP, unsigned int& uWMPId ) const;
};


#endif //__HM_WMP_LOADER_BASE_H__