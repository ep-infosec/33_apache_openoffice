/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _SVXSWFRAMEEXAMPLE_HXX
#define _SVXSWFRAMEEXAMPLE_HXX

#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

// class SwFrmPagePreview -------------------------------------------------------

class SVX_DLLPUBLIC SvxSwFrameExample : public Window
{
	Color		m_aTransColor;		// transparency
	Color		m_aBgCol;			// background
	Color		m_aFrameColor;		// graphic frame
	Color     	m_aAlignColor;		// align anchor
	Color		m_aBorderCol;		// frame of doc
	Color		m_aPrintAreaCol;	// frame of printable area of doc
	Color		m_aTxtCol;			// symbolised text
	Color		m_aBlankCol;		// area of symbol for blank
	Color		m_aBlankFrameCol;	// frame of symbol for blank

	Rectangle 	aPage;
	Rectangle 	aPagePrtArea;
	Rectangle 	aTextLine;
	Rectangle 	aPara;
	Rectangle 	aParaPrtArea;
	Rectangle 	aFrameAtFrame;
	Rectangle 	aDrawObj;
	Rectangle 	aAutoCharFrame;
	Size      	aFrmSize;

    short       nHAlign;
    short       nHRel;

    short       nVAlign;
    short       nVRel;

    short       nWrap;
    short       nAnchor;
	sal_Bool 		bTrans;

	Point		aRelPos;

    void InitColors_Impl( void );
    void InitAllRects_Impl();
    void CalcBoundRect_Impl(Rectangle &rRect);
    Rectangle DrawInnerFrame_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rBorderColor);

    void DrawRect_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor);
    virtual void Paint(const Rectangle&);
protected:
	virtual void DataChanged( const DataChangedEvent& rDCEvt );
public:

    SvxSwFrameExample(Window* pParent, const ResId& rResID);
    ~SvxSwFrameExample();

	inline void SetWrap(sal_uInt16 nW) 			{ nWrap 	= nW; }

    inline void SetHAlign(short nH)      { nHAlign   = nH; }
    inline void SetHoriRel(short nR)         { nHRel     = nR; }

    inline void SetVAlign(short nV)      { nVAlign   = nV; }
    inline void SetVertRel(short nR)         { nVRel     = nR; }

	inline void SetTransparent(sal_Bool bT)		{ bTrans 	= bT; }
    inline void SetAnchor(short nA)          { nAnchor   = nA; }

    void SetRelPos(const Point& rP);
};


#endif // _SVXSWFRAMEEXAMPLE_HXX
