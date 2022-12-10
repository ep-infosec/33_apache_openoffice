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



#ifndef _CAMERA3D_HXX
#define _CAMERA3D_HXX

#include <svx/viewpt3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Kamera-Ableitung
|*
\************************************************************************/

class SVX_DLLPUBLIC Camera3D : public Viewport3D
{
 protected:
	basegfx::B3DPoint	aResetPos;
	basegfx::B3DPoint	aResetLookAt;
	double		fResetFocalLength;
	double		fResetBankAngle;

	basegfx::B3DPoint	aPosition;
	basegfx::B3DPoint	aLookAt;
	double		fFocalLength;
	double		fBankAngle;

	FASTBOOL	bAutoAdjustProjection;

 public:
	Camera3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
			 double fFocalLen = 35.0, double fBankAng = 0);
	Camera3D();

	// Anfangswerte wieder herstellen
	void Reset();

	void SetDefaults(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
					 double fFocalLen = 35.0, double fBankAng = 0);

	void SetViewWindow(double fX, double fY, double fW, double fH);

	void SetPosition(const basegfx::B3DPoint& rNewPos);
	const basegfx::B3DPoint& GetPosition() const { return aPosition; }
	void SetLookAt(const basegfx::B3DPoint& rNewLookAt);
	const basegfx::B3DPoint& GetLookAt() const { return aLookAt; }
	void SetPosAndLookAt(const basegfx::B3DPoint& rNewPos, const basegfx::B3DPoint& rNewLookAt);

	// Brennweite in mm
	void	SetFocalLength(double fLen);
	void	SetFocalLengthWithCorrect(double fLen);
	double	GetFocalLength() const { return fFocalLength; }

	// Neigung links/rechts
	void	SetBankAngle(double fAngle);
	double	GetBankAngle() const { return fBankAngle; }

	// Um die Kameraposition drehen, LookAt wird dabei veraendert
	void Rotate(double fHAngle, double fVAngle);

	// Um den Blickpunkt drehen, Position wird dabei veraendert
	void RotateAroundLookAt(double fHAngle, double fVAngle);

	void SetAutoAdjustProjection(FASTBOOL bAdjust = sal_True)
		{ bAutoAdjustProjection = bAdjust; }
	FASTBOOL IsAutoAdjustProjection() const { return bAutoAdjustProjection; }
};

#endif		// _CAMERA3D_HXX
