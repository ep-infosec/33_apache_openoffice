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



#ifndef _SV_MAPMOD_HXX
#define _SV_MAPMOD_HXX

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <tools/mapunit.hxx>

class SvStream;

// --------------
// - ImplMapMode -
// --------------

class	OutputDevice;

class ImplMapMode
{
	friend class	MapMode;
	friend class	OutputDevice;

private:
	sal_uLong			mnRefCount;
	MapUnit 		meUnit;
	Point			maOrigin;
	Fraction		maScaleX;
	Fraction		maScaleY;
	sal_Bool			mbSimple;

	friend SvStream& operator>>( SvStream& rIStm, ImplMapMode& rMapMode );
	friend SvStream& operator<<( SvStream& rOStm, const ImplMapMode& rMapMode );

    static ImplMapMode* ImplGetStaticMapMode( MapUnit eUnit );
public:
					ImplMapMode();
					ImplMapMode( const ImplMapMode& rImpMapMode );
};

// -----------
// - MapMode -
// -----------

class VCL_DLLPUBLIC MapMode
{
	friend class	    OutputDevice;

private:
	ImplMapMode*	    mpImplMapMode;

	SAL_DLLPRIVATE void ImplMakeUnique();

public:
					MapMode();
					MapMode( const MapMode& rMapMode );
					MapMode( MapUnit eUnit );
					MapMode( MapUnit eUnit, const Point& rLogicOrg,
							 const Fraction& rScaleX, const Fraction& rScaleY );
					~MapMode();

	void			SetMapUnit( MapUnit eUnit );
	MapUnit 		GetMapUnit() const
						{ return mpImplMapMode->meUnit; }

	void			SetOrigin( const Point& rOrigin );
	const Point&	GetOrigin() const
						{ return mpImplMapMode->maOrigin; }

	void			SetScaleX( const Fraction& rScaleX );
	const Fraction& GetScaleX() const
						{ return mpImplMapMode->maScaleX; }
	void			SetScaleY( const Fraction& rScaleY );
	const Fraction& GetScaleY() const
						{ return mpImplMapMode->maScaleY; }

	MapMode&		operator=( const MapMode& rMapMode );
	sal_Bool			operator==( const MapMode& rMapMode ) const;
	sal_Bool			operator!=( const MapMode& rMapMode ) const
						{ return !(MapMode::operator==( rMapMode )); }
	sal_Bool			IsDefault() const;
	sal_Bool			IsSameInstance( const MapMode& rMapMode ) const
						{ return (mpImplMapMode == rMapMode.mpImplMapMode); }

	friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, MapMode& rMapMode );
	friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const MapMode& rMapMode );
};

#endif	// _SV_MAPMOD_HXX
