#ifndef __HM_WMP_LOADERS_H__
#define __HM_WMP_LOADERS_H__
#include "hm_wmp_loader_base.h"
//
class hm_wmp_display;
class hm_districtobj;
class hm_rs_uires_i;
class hm_wmp_tabheader;
class hm_wmp_movewnd;
class hm_pos_decoder_i;
//
class hm_wmp_display_loader : public hm_wmp_loader_base {
public:
	hm_wmp_display_loader(){};
	virtual ~hm_wmp_display_loader(){};

public:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading );

private:
	bool _fillDrawTool( const Json::Value& valWMP, hm_wmp_display* pWMPDest );
	bool _fillBufStrategy( const Json::Value& valWMP, hm_wmp_display* pWMPDest );
	bool _fillDistrictObjs( const Json::Value& valWMP, hm_wmp_display* pWMPDest );
	bool _fillShowObjs( const Json::Value& aryShowObjs, hm_districtobj* pDistrict );
	bool _fillRect( const Json::Value& valRect, RECT& rect );
	bool _createUIRes( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes );
	bool _createUIRes_Pic( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes );
	bool _createUIRes_Color( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes );
	bool _createUIRes_Text( const Json::Value& valUIRes, hm_rs_uires_i** ppUIRes );
	//
	bool _createPosDecoder( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC );
	bool _createPosDecoder_Dock( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC );
	bool _createPosDecoder_Slider( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC );
	bool _createPosDecoder_Stack( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC );
	bool _createPosDecoder_Offset( const Json::Value& valPDC, hm_pos_decoder_i** ppPDC );
};
//
class hm_wmp_bkgnd_loader : public hm_wmp_display_loader {
public:
	hm_wmp_bkgnd_loader();
	virtual ~hm_wmp_bkgnd_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );

};
//
class hm_wmp_bkgnd_nca_loader : public hm_wmp_display_loader {
public:
	hm_wmp_bkgnd_nca_loader();
	virtual ~hm_wmp_bkgnd_nca_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
protected:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading );
};
//
class hm_wmp_button_loader : public hm_wmp_display_loader {
public:
	hm_wmp_button_loader();
	virtual ~hm_wmp_button_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );

};
//
class hm_wmp_button_nca_loader : public hm_wmp_display_loader {
public:
	hm_wmp_button_nca_loader();
	virtual ~hm_wmp_button_nca_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_dialogview_loader : public hm_wmp_display_loader {
public:
	hm_wmp_dialogview_loader();
	virtual ~hm_wmp_dialogview_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_drawtrigger_loader : public hm_wmp_loader_base {
public:
	hm_wmp_drawtrigger_loader();
	virtual ~hm_wmp_drawtrigger_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_drawtrigger_nca_loader : public hm_wmp_loader_base {
public:
	hm_wmp_drawtrigger_nca_loader();
	virtual ~hm_wmp_drawtrigger_nca_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_layouttrigger_loader : public hm_wmp_loader_base {
public:
	hm_wmp_layouttrigger_loader();
	virtual ~hm_wmp_layouttrigger_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_movewnd_loader : public hm_wmp_loader_base {
public:
	hm_wmp_movewnd_loader();
	virtual ~hm_wmp_movewnd_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );

protected:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading );

private:
	bool _fillWndMark( const Json::Value& valWMP, hm_wmp_movewnd* pWMPMoveWnd );

private:
	hm_uiroot* m_pUIRoot;
};
//
class hm_wmp_popupmenu_loader : public hm_wmp_display_loader {
public:
	hm_wmp_popupmenu_loader();
	virtual ~hm_wmp_popupmenu_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_rootmenu_loader : public hm_wmp_display_loader {
public:
	hm_wmp_rootmenu_loader();
	virtual ~hm_wmp_rootmenu_loader();

private:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
protected:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading );
};
//
class hm_wmp_scrollbar_loader : public hm_wmp_display_loader {
public:
	hm_wmp_scrollbar_loader();
	virtual ~hm_wmp_scrollbar_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_tabheader_loader : public hm_wmp_display_loader {
public:
	hm_wmp_tabheader_loader();
	virtual ~hm_wmp_tabheader_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );

protected:
	virtual bool _fillWMP( const Json::Value& valWMP, hm_wndmsgprocessor_i* pWMPLoading );

private:
	bool _fillBtns( const Json::Value& valWMP, hm_wmp_tabheader* pWMPLoading );
};
//
class hm_wmp_viewbkcontent_loader : public hm_wmp_display_loader {
public:
	hm_wmp_viewbkcontent_loader();
	virtual ~hm_wmp_viewbkcontent_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_checkbox_loader : public hm_wmp_display_loader {
public:
	hm_wmp_checkbox_loader();
	virtual ~hm_wmp_checkbox_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};
//
class hm_wmp_radio_loader : public hm_wmp_display_loader {
public:
	hm_wmp_radio_loader();
	virtual ~hm_wmp_radio_loader();

public:
	virtual hm_wndmsgprocessor_i* _createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot );
};


#endif //__HM_WMP_LOADERS_H__