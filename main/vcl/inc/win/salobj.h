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



#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <salobj.hxx>

// -----------------
// - SalObjectData -
// -----------------

class WinSalObject : public SalObject
{
public:
	HWND					mhWnd;					// Window handle
	HWND					mhWndChild; 			// Child Window handle
	HWND					mhLastFocusWnd; 		// Child-Window, welches als letztes den Focus hatte
	SystemChildData 		maSysData;				// SystemEnvData
	RGNDATA*				mpClipRgnData;			// ClipRegion-Data
	RGNDATA*				mpStdClipRgnData;		// Cache Standard-ClipRegion-Data
	RECT*					mpNextClipRect; 		// Naechstes ClipRegion-Rect
	sal_Bool				mbFirstClipRect;		// Flag for first cliprect to insert
    sal_Bool                mbInterceptChildWindowKeyDown; // Intercept the KeyDown event sent to system child window
	WinSalObject*				mpNextObject;			// pointer to next object


    WinSalObject();
    virtual ~WinSalObject();

	virtual void					ResetClipRegion();
	virtual sal_uInt16					GetClipRegionType();
	virtual void					BeginSetClipRegion( sal_uIntPtr nRects );
	virtual void					UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual void					EndSetClipRegion();
	virtual void					SetPosSize( long nX, long nY, long nWidth, long nHeight );
	virtual void					Show( sal_Bool bVisible );
	virtual void					Enable( sal_Bool nEnable );
	virtual void					GrabFocus();
	virtual void					SetBackground();
	virtual void					SetBackground( SalColor nSalColor );
	virtual const SystemEnvData*	GetSystemData() const;
    virtual void InterceptChildWindowKeyDown( sal_Bool bIntercept );
};

#endif // _SV_SALOBJ_H
