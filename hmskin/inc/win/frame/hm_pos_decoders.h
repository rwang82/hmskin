#ifndef __HM_POS_DECODERS_H__
#define __HM_POS_DECODERS_H__
#include "win\hm_export_defs.h"

/////////////////////////////////////////////////////////////////////////
class HMLIB_API hm_pos_decoder_i{
public:
	virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj ) = 0;
};

/////////////////////////////////////////////////////////////////////////
// position decoder of docktype
// 
// DOCK_LEFT:
//		---------------------------------------------
//		|rcRefObj			|						|
//		|				  m_space2					|
//		|					|						|
//		|			-----------------				|
//		|		 	|rcDestObj		|				|
//		|- m_space1-|				|				|
//		|			|				|				|
//		|			-----------------  ---			|
//		|			|- m_nThickness-|	|			|
//		|							m_space3		|
//		|								|			|
//		---------------------------------------------
// 
// DOCK_TOP:
//		---------------------------------------------
//		|rcRefObj			|						|
//		|				  m_space1					|
//		|					|						|
//		|			-----------------		---		|
//		|		 	|rcDestObj		|	     |		|
//		|- m_space2-|				|  m_nThickness	|
//		|			|				|		 |		|
//		|			-----------------		---		|
//		|											|
//		|							|--- m_space3 --|
//		|											|
//		---------------------------------------------
// 
// DOCK_RIGHT:
//		---------------------------------------------
//		|rcRefObj			|						|
//		|				  m_space2					|
//		|					|						|
//		|			-----------------				|
//		|		 	|rcDestObj		|				|
//		|			|				|--- m_space1 --|
//		|			|				|				|
//		|			-----------------  ---			|
//		|			|- m_nThickness-|	|			|
//		|							m_space3		|
//		|								|			|
//		---------------------------------------------
// 
// DOCK_BOTTOM:
//		---------------------------------------------
//		|rcRefObj									|
//		|											|
//		|											|
//		|			-----------------		---		|
//		|		 	|rcDestObj		|	     |		|
//		|- m_space2-|				|  m_nThickness	|
//		|			|				|		 |		|
//		|			-----------------		---		|
//		|					|						|
//		|				m_space1	|--- m_space3 --|
//		|					|						|
//		---------------------------------------------
class HMLIB_API hm_posdecoder_dock : public hm_pos_decoder_i {
public:
	typedef enum _ENUMDOCKTYPE {
		DOCK_TOP = 0,
		DOCK_LEFT,
		DOCK_RIGHT,
		DOCK_BOTTOM
	} ENUMDOCKTYPE;
public:
	hm_posdecoder_dock( ENUMDOCKTYPE eDockType, int nSpace1 = 0, int nSpace2 = 0, int nSpace3 = 0, int nThickness = 20 );
	virtual ~hm_posdecoder_dock();
	virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj );
public:
	int m_space1;
	int m_space2;
	int m_space3;
	int m_nThickness;
	ENUMDOCKTYPE m_eDockType;
};

/////////////////////////////////////////////////////////////////////////
//
//
// SLIDER_T2B:
//		-------------------------------------------------
//		|rcRefObj								|		|
//		|										|		|
//		|									m_distance	|
//		|	----		-----------------		|		|
//		|	  |	 		|rcDestObj		|	    |		|
//		|m_nThickness	|				|	  -----		|
//		|	  |			|				|		 		|
//		|	----		-----------------				|
//		|												|
//		|-- m_space1 ---|				|--- m_space2 --|
//		|												|
//		-------------------------------------------------
//
// SLIDER_L2R:
//		-------------------------------------------------
//		|rcRefObj								|		|
//		|		  		|- m_nThickness |	m_space1	|
//		|										|		|
//		|				-----------------	   ---		|
//		|		 		|rcDestObj	  	|	    		|
//		|				|				|				|
//		|	  			|				|		 		|
//		|				-----------------	   ---		|
//		|										|		|
//		|----- m_distance -----|			m_space2	|
//		|										|		|
//		-------------------------------------------------
class HMLIB_API hm_posdecoder_slider : public hm_pos_decoder_i {
public:
	typedef enum _ENUMSLIDERTYPE {
		SLIDER_T2B = 0,
		SLIDER_L2R
	} ENUMSLIDERTYPE;
public:
	hm_posdecoder_slider( ENUMSLIDERTYPE eSliderType, int space1 = 0, int space2 = 0, int distance = 0, int nThickness = 20 );
	virtual ~hm_posdecoder_slider();
	virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj );
public:
	int m_space1;
	int m_space2;
	int m_distance;
	int m_nThickness;
	ENUMSLIDERTYPE m_eSliderType;
};

/////////////////////////////////////////////////////////////////////////
// 
// STACK_T2B :
//		-------------------------------------------------
//		|rcRefObj								|		|
//		|								*pctxStartPos   |
//		|										|		|
//		|	----		-----------------	  -----     |
//		|	  |	 		|rcDestObj		|	    		|
//		|m_nThickness	|				|	     		|
//		|	  |			|				|		 		|
//		|	----		-----------------				|
//		|												|
//		|-- m_space1 ---|				|--- m_space2 --|
//		|												|
//		-------------------------------------------------
//
// STACK_B2T :
//		-------------------------------------------------
//		|rcRefObj										|
//		|-- m_space1 ---|				|--- m_space2 --|
//		|												|
//		|	----		-----------------	            |
//		|	  |	 		|rcDestObj		|	    		|
//		|m_nThickness	|				|	     		|
//		|	  |			|				|		 		|
//		|	----		-----------------	  -----     |
//		|										|		|
//		|				                *pctxStartPos   |
//		|										|		|
//		-------------------------------------------------
//
// STACK_L2R :
//		-------------------------------------------------
//		|rcRefObj								|		|
//		|		  		|- m_nThickness |	m_space1	|
//		|										|		|
//		|				-----------------	   ---		|
//		|		 		|rcDestObj	  	|	    		|
//		|				|				|				|
//		|	  			|				|		 		|
//		|				-----------------	   ---		|
//		|										|		|
//		|-*pctxStartPos-|			        m_space2	|
//		|										|		|
//		-------------------------------------------------
//
// STACK_R2L :
//		-------------------------------------------------
//		|rcRefObj	|									|
//		|      m_space1 |- m_nThickness |		        |
//		|			|									|
//		|		   ---  -----------------	     		|
//		|		 		|rcDestObj	  	|	    		|
//		|				|				|				|
//		|	  			|				|		 		|
//		|		   ---  -----------------	     		|
//		|			|									|
//		|      m_space2	                |-*pctxStartPos-|
//		|			|									|
//		-------------------------------------------------
//
/////////////////////////////////////////////////////////////////////////
//		---------------------------------------------
//		|rcRefObj			|						|
//		|				 m_nOffsetT					|
//		|					|						|
//		|			-----------------				|
//		|		 	|rcDestObj		|				|
//		|			|				|-  m_nOffsetR -|
//		|		 	|				|				|
//		|			-----------------				|
//		|-m_nOffsetL-|		|						|
//		|				m_nOffsetB					|
//		|					|						|
//		---------------------------------------------
class HMLIB_API hm_posdecoder_offset : public hm_pos_decoder_i {
public:
	hm_posdecoder_offset( int nOffsetL = 0, int nOffsetT = 0, int nOffsetR = 0, int nOffsetB = 0 );
	virtual ~hm_posdecoder_offset();
public:
	virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj );
public:
	int m_nOffsetL; 
	int m_nOffsetT;
	int m_nOffsetR;
	int m_nOffsetB;
};


//
///////////////////////////////////////////////////////////////////////////
//// 
//// CORNER_LT :
////		-------------------------------------------------
////		|rcRefObj								|		|
////		|									m_space1    |
////		|										|		|
////		|	----		-----------------	  -----     |
////		|	  |	 		|rcDestObj		|	    		|
////		|m_nThickness	|				|	     		|
////		|	  |			|				|		 		|
////		|	----		-----------------				|
////		|												|
////		|-- m_space2 ---|				|				|
////		|												|
////		-------------------------------------------------
////
//class hm_posdecoder_corner : public hm_pos_decoder_i {
//public:
//	typedef enum _ENUMCORNERTYPE {
//		CORNER_LT = 0,
//		CORNER_LB,
//		CORNER_RB,
//		CORNER_RT
//	} ENUMCORNERTYPE;
//
//public:
//	hm_posdecoder_corner();
//	virtual ~hm_posdecoder_corner();
//	virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj );
//
//public:
//	ENUMCORNERTYPE m_eType;
//
//};

#endif //__HM_POS_DECODERS_H__