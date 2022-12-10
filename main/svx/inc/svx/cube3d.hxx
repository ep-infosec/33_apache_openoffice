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



#ifndef _E3D_CUBE3D_HXX
#define _E3D_CUBE3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|*                                                              |
|* 3D-Quader erzeugen; aPos: Zentrum oder links, unten, hinten  |__
|*                           (abhaengig von bPosIsCenter)      /
|* Mit nSideFlags kann angegeben werden, ob nur ein Teil der
|* Quaderflaechen erzeugt werden kann; die entsprechenden Bits
|* sind in dem enum definiert. Das Flag bDblSided legt fest,
|* ob die erzeugten Flaechen doppelseitig sind (nur sinnvoll,
|* wenn nicht alle Flaechen erzeugt wurden).
|*
\************************************************************************/

enum { CUBE_BOTTOM = 0x0001, CUBE_BACK = 0x0002, CUBE_LEFT = 0x0004,
	   CUBE_TOP = 0x0008, CUBE_RIGHT = 0x0010, CUBE_FRONT = 0x0020,
	   CUBE_FULL = 0x003F, CUBE_OPEN_TB = 0x0036, CUBE_OPEN_LR = 0x002B,
	   CUBE_OPEN_FB = 0x001D };

class SVX_DLLPUBLIC E3dCubeObj : public E3dCompoundObject
{
private:
	// Parameter
	basegfx::B3DPoint					aCubePos;
	basegfx::B3DVector					aCubeSize;
	sal_uInt16								nSideFlags;

	// BOOLeans
	unsigned							bPosIsCenter : 1;

protected:
	void SetDefaultAttributes(E3dDefaultAttributes& rDefault);
	virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
	TYPEINFO();
	E3dCubeObj(E3dDefaultAttributes& rDefault, basegfx::B3DPoint aPos, const basegfx::B3DVector& r3DSize);
	E3dCubeObj();

	virtual sal_uInt16 GetObjIdentifier() const;
	virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

	virtual void operator=(const SdrObject&);

	// Lokale Parameter setzen mit Geometrieneuerzeugung
	void SetCubePos(const basegfx::B3DPoint& rNew);
	const basegfx::B3DPoint& GetCubePos() { return aCubePos; }

	void SetCubeSize(const basegfx::B3DVector& rNew);
	const basegfx::B3DVector& GetCubeSize() { return aCubeSize; }

	void SetPosIsCenter(sal_Bool bNew);
	sal_Bool GetPosIsCenter() { return (sal_Bool)bPosIsCenter; }

	void SetSideFlags(sal_uInt16 nNew);
	sal_uInt16 GetSideFlags() { return nSideFlags; }

	// TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
	virtual void TakeObjNameSingul(String& rName) const;
	virtual void TakeObjNamePlural(String& rName) const;
};

#endif			// _E3D_CUBE3D_HXX
