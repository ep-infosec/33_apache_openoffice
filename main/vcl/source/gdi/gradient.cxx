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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>
#include <vcl/gradient.hxx>

// =======================================================================

DBG_NAME( Gradient )

// -----------------------------------------------------------------------

Impl_Gradient::Impl_Gradient() :
    maStartColor( COL_BLACK ),
    maEndColor( COL_WHITE )
{
    mnRefCount          = 1;
    meStyle             = GRADIENT_LINEAR;
    mnAngle             = 0;
    mnBorder            = 0;
    mnOfsX              = 50;
    mnOfsY              = 50;
    mnIntensityStart    = 100;
    mnIntensityEnd      = 100;
    mnStepCount         = 0;
}

// -----------------------------------------------------------------------

Impl_Gradient::Impl_Gradient( const Impl_Gradient& rImplGradient ) :
    maStartColor( rImplGradient.maStartColor ),
    maEndColor( rImplGradient.maEndColor )
{
    mnRefCount          = 1;
    meStyle             = rImplGradient.meStyle;
    mnAngle             = rImplGradient.mnAngle;
    mnBorder            = rImplGradient.mnBorder;
    mnOfsX              = rImplGradient.mnOfsX;
    mnOfsY              = rImplGradient.mnOfsY;
    mnIntensityStart    = rImplGradient.mnIntensityStart;
    mnIntensityEnd      = rImplGradient.mnIntensityEnd;
    mnStepCount         = rImplGradient.mnStepCount;
}

// -----------------------------------------------------------------------

void Gradient::MakeUnique()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpImplGradient->mnRefCount != 1 )
	{
		if( mpImplGradient->mnRefCount )
			mpImplGradient->mnRefCount--;

        mpImplGradient = new Impl_Gradient( *mpImplGradient );
	}
}

// -----------------------------------------------------------------------

Gradient::Gradient()
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient = new Impl_Gradient;
}

// -----------------------------------------------------------------------

Gradient::Gradient( const Gradient& rGradient )
{
    DBG_CTOR( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    // Instance Daten uebernehmen und Referenzcounter erhoehen
    mpImplGradient = rGradient.mpImplGradient;
    mpImplGradient->mnRefCount++;
}

// -----------------------------------------------------------------------

Gradient::Gradient( GradientStyle eStyle )
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient          = new Impl_Gradient;
    mpImplGradient->meStyle = eStyle;
}

// -----------------------------------------------------------------------

Gradient::Gradient( GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor )
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient                  = new Impl_Gradient;
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

// -----------------------------------------------------------------------

Gradient::~Gradient()
{
    DBG_DTOR( Gradient, NULL );

    // Wenn es die letzte Referenz ist, loeschen,
    // sonst Referenzcounter decrementieren
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
}

// -----------------------------------------------------------------------

void Gradient::SetStyle( GradientStyle eStyle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->meStyle = eStyle;
}

// -----------------------------------------------------------------------

void Gradient::SetStartColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maStartColor = rColor;
}

// -----------------------------------------------------------------------

void Gradient::SetEndColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maEndColor = rColor;
}

// -----------------------------------------------------------------------

void Gradient::SetAngle( sal_uInt16 nAngle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnAngle = nAngle;
}

// -----------------------------------------------------------------------

void Gradient::SetBorder( sal_uInt16 nBorder )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnBorder = nBorder;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsX = nOfsX;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsY = nOfsY;
}

// -----------------------------------------------------------------------

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityStart = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityEnd = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetSteps( sal_uInt16 nSteps )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnStepCount = nSteps;
}

// -----------------------------------------------------------------------

Gradient& Gradient::operator=( const Gradient& rGradient )
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rGradient.mpImplGradient->mnRefCount++;

    // Wenn es die letzte Referenz ist, loeschen,
    // sonst Referenzcounter decrementieren
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
    mpImplGradient = rGradient.mpImplGradient;

    return *this;
}

// -----------------------------------------------------------------------

sal_Bool Gradient::operator==( const Gradient& rGradient ) const
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if ( mpImplGradient == rGradient.mpImplGradient )
        return sal_True;

    if ( (mpImplGradient->meStyle           == rGradient.mpImplGradient->meStyle)           ||
         (mpImplGradient->mnAngle           == rGradient.mpImplGradient->mnAngle)           ||
         (mpImplGradient->mnBorder          == rGradient.mpImplGradient->mnBorder)          ||
         (mpImplGradient->mnOfsX            == rGradient.mpImplGradient->mnOfsX)            ||
         (mpImplGradient->mnOfsY            == rGradient.mpImplGradient->mnOfsY)            ||
         (mpImplGradient->mnStepCount       == rGradient.mpImplGradient->mnStepCount)       ||
         (mpImplGradient->mnIntensityStart  == rGradient.mpImplGradient->mnIntensityStart)  ||
         (mpImplGradient->mnIntensityEnd    == rGradient.mpImplGradient->mnIntensityEnd)    ||
         (mpImplGradient->maStartColor      == rGradient.mpImplGradient->maStartColor)      ||
         (mpImplGradient->maEndColor        == rGradient.mpImplGradient->maEndColor) )
         return sal_True;
    else
        return sal_False;
}

SvStream& operator>>( SvStream& rIStm, Impl_Gradient& rImpl_Gradient )
{
    VersionCompat	aCompat( rIStm, STREAM_READ );
    sal_uInt16			nTmp16;

    rIStm >> nTmp16; rImpl_Gradient.meStyle = (GradientStyle) nTmp16;

    rIStm >> rImpl_Gradient.maStartColor >>
             rImpl_Gradient.maEndColor >>
             rImpl_Gradient.mnAngle >>
             rImpl_Gradient.mnBorder >>
             rImpl_Gradient.mnOfsX >>
             rImpl_Gradient.mnOfsY >>
             rImpl_Gradient.mnIntensityStart >>
             rImpl_Gradient.mnIntensityEnd >>
             rImpl_Gradient.mnStepCount;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Impl_Gradient& rImpl_Gradient )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (sal_uInt16) rImpl_Gradient.meStyle <<
             rImpl_Gradient.maStartColor <<
             rImpl_Gradient.maEndColor <<
             rImpl_Gradient.mnAngle <<
             rImpl_Gradient.mnBorder <<
             rImpl_Gradient.mnOfsX <<
             rImpl_Gradient.mnOfsY <<
             rImpl_Gradient.mnIntensityStart <<
             rImpl_Gradient.mnIntensityEnd <<
             rImpl_Gradient.mnStepCount;

    return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Gradient& rGradient )
{
    rGradient.MakeUnique();
    return( rIStm >> *rGradient.mpImplGradient );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Gradient& rGradient )
{
    return( rOStm << *rGradient.mpImplGradient );
}
