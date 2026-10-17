#ifndef __HMSKIN_HELPER_H__
#define __HMSKIN_HELPER_H__

//
unsigned int getWMPDisplayText( void* pWMPDisplayRaw, LPTSTR lpszStringBuf, int nMaxTextCount, int nDistrictId = 0 );
//
#define DISTRICT_ID_ALL (0xFFFFFFFF)
bool setWMPDisplayText( void* pWMPDisplayRaw, LPTSTR lpString, int nDistrictId = DISTRICT_ID_ALL );

// AI-agent / debugging aid: dump the live keelobj tree as JSON.
// Every keelobj in the factory is emitted as { id, parentId, visible, rect }
// where rect is the layout rect mapped into the CLIENT area of hWndMain
// ( mapPosition ). when the mapping is unavailable - e.g. a popup-slot
// keelobj whose window does not exist yet - the node carries "rect": null
// plus "rectRaw": the layout-rect in its own coordinate system instead
// ( known wrinkle: cross-window CLIENT->CLIENT mapping via window-rect
// deltas ignores non-client offsets, so popup-slot rects may be off by the
// title-bar height - consumers that need precision should re-check ). the
// parent chain comes from hm_keel_obj::getParent(), so no tree-traversal
// API is needed inside the engine.
// returns the node count written, or 0 on failure.
unsigned int hmsh_dump_tree( void* pHMUIRootRaw, HWND hWndMain, const TCHAR* szJsonPath );

#endif //__HMSKIN_HELPER_H__
