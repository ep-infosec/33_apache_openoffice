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

#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <tools/stream.hxx>
#include <tools/debug.hxx>

#include <vcl/settings.hxx>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/cdeint.hxx>

CDEIntegrator::CDEIntegrator()
{
	meType = DtCDE;
}

CDEIntegrator::~CDEIntegrator()
{
}

static int getHexDigit( const char c )
{
	if( c >= '0' && c <= '9' )
		return (int)(c-'0');
	else if( c >= 'a' && c <= 'f' )
		return (int)(c-'a'+10);
	else if( c >= 'A' && c <= 'F' )
		return (int)(c-'A'+10);
	return -1;
}


void CDEIntegrator::GetSystemLook( AllSettings& rSettings )
{
	static Color aColors[ 8 ];
	static sal_Bool bRead  = sal_False;
    static sal_Bool bValid = sal_False;

	if( ! bRead )
	{
		// get used palette from xrdb
		char **ppStringList = 0;
		int nStringCount;
		XTextProperty aTextProperty;
		aTextProperty.value = 0;
		int i;
		
		static Atom nResMgrAtom = XInternAtom( mpDisplay, "RESOURCE_MANAGER", False );
		
		if( XGetTextProperty( mpDisplay,
							  RootWindow( mpDisplay, 0 ),
							  &aTextProperty,
							  nResMgrAtom )
			&& aTextProperty.value
			&& XTextPropertyToStringList( &aTextProperty, &ppStringList, &nStringCount )
			)
		{
			// format of ColorPalette resource:
			// *n*ColorPalette: palettefile

			ByteString aLines;
			for( i=0; i < nStringCount; i++ )
				aLines += ppStringList[i];
			for( i = aLines.GetTokenCount( '\n' )-1; i >= 0; i-- )
			{
				ByteString aLine = aLines.GetToken( i, '\n' );
				int nIndex = aLine.Search( "ColorPalette" );
				if( nIndex != STRING_NOTFOUND )
				{
					int nPos = nIndex;

					nIndex+=12;
					const char* pStr = aLine.GetBuffer() +nIndex;
					while( *pStr && isspace( *pStr ) && *pStr != ':' )
					{
						pStr++;
						nIndex++;
					}
					if( *pStr != ':' )
						continue;
					pStr++, nIndex++;
					for( ; *pStr && isspace( *pStr ); pStr++, nIndex++ )
						;
					if( ! *pStr )
						continue;
					int nIndex2 = nIndex;
					for( ; *pStr && ! isspace( *pStr ); pStr++, nIndex2++ )
						;
					ByteString aPaletteFile( aLine.Copy( nIndex, nIndex2 - nIndex ) );
					// extract number before ColorPalette;
					for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
						;
					nPos--;
					for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
						;
					int nNumber = aLine.Copy( ++nPos ).ToInt32();
					
					DBG_TRACE2( "found palette %d in resource \"%s\"", nNumber, aLine.GetBuffer() );

					// found no documentation what this number actually means;
					// might be the screen number. 0 seems to be the right one
					// in most cases.
					if( nNumber )
						continue;

					DBG_TRACE1( "Palette file is \"%s\".\n", aPaletteFile.GetBuffer() );
					
					String aPath( aHomeDir );
					aPath.AppendAscii( "/.dt/palettes/" );
					aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );

					SvFileStream aStream( aPath, STREAM_READ );
					if( ! aStream.IsOpen() )
					{
						aPath = String::CreateFromAscii( "/usr/dt/palettes/" );
						aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );
						aStream.Open( aPath, STREAM_READ );
						if( ! aStream.IsOpen() )
							continue;
					}

					ByteString aBuffer;
					for( nIndex = 0; nIndex < 8; nIndex++ )
					{
						aStream.ReadLine( aBuffer );
						// format is "#RRRRGGGGBBBB"

						DBG_TRACE1( "\t\"%s\".\n", aBuffer.GetBuffer() );

						if( aBuffer.Len() )
						{
							const char* pArr = (const char*)aBuffer.GetBuffer()+1;
							aColors[nIndex] = Color(
								getHexDigit( pArr[1] )
								| ( getHexDigit( pArr[0] ) << 4 ),
								getHexDigit( pArr[5] ) 
								| ( getHexDigit( pArr[4] ) << 4 ),
								getHexDigit( pArr[9] )
								| ( getHexDigit( pArr[8] ) << 4 )
								);
							
							DBG_TRACE1( "\t\t%lx\n", aColors[nIndex].GetColor() );
						}
					}

                    bValid = sal_True;
					break;
				}
			}
		}
		
		if( ppStringList )
			XFreeStringList( ppStringList );
		if( aTextProperty.value )
			XFree( aTextProperty.value );
	}

    
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    // #i48001# set a default blink rate
    aStyleSettings.SetCursorBlinkTime( 500 );
    if (bValid)
	{
        aStyleSettings.SetActiveColor( aColors[0] );
        aStyleSettings.SetActiveColor2( aColors[0] );
        aStyleSettings.SetActiveBorderColor( aColors[0] );

        aStyleSettings.SetDeactiveColor( aColors[0] );
        aStyleSettings.SetDeactiveColor2( aColors[0] );
        aStyleSettings.SetDeactiveBorderColor( aColors[0] );

        Color aActive =
            aColors[ 0 ].GetBlue() < 128		||
            aColors[ 0 ].GetGreen() < 128		||
            aColors[ 0 ].GetRed() < 128
            ? Color( COL_WHITE ) : Color( COL_BLACK );
        Color aDeactive =
            aColors[ 1 ].GetBlue() < 128		||
            aColors[ 1 ].GetGreen() < 128		||
            aColors[ 1 ].GetRed() < 128
            ? Color( COL_WHITE ) : Color( COL_BLACK );
        aStyleSettings.SetActiveTextColor( aActive );
        aStyleSettings.SetDeactiveTextColor( aDeactive );

        aStyleSettings.SetDialogTextColor( aDeactive );
        aStyleSettings.SetMenuTextColor( aDeactive );
        aStyleSettings.SetMenuBarTextColor( aDeactive );
        aStyleSettings.SetButtonTextColor( aDeactive );
        aStyleSettings.SetRadioCheckTextColor( aDeactive );
        aStyleSettings.SetGroupTextColor( aDeactive );
        aStyleSettings.SetLabelTextColor( aDeactive );
        aStyleSettings.SetInfoTextColor( aDeactive );

        aStyleSettings.Set3DColors( aColors[1] );
        aStyleSettings.SetFaceColor( aColors[1] );
        aStyleSettings.SetDialogColor( aColors[1] );
        aStyleSettings.SetMenuColor( aColors[1] );
        aStyleSettings.SetMenuBarColor( aColors[1] );
        if ( aStyleSettings.GetFaceColor() == COL_LIGHTGRAY )
            aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
        else
        {
            // calculate Checked color
            Color   aColor2 = aStyleSettings.GetLightColor();
            sal_uInt8    nRed    = (sal_uInt8)(((sal_uInt16)aColors[1].GetRed()   + (sal_uInt16)aColor2.GetRed())/2);
            sal_uInt8    nGreen  = (sal_uInt8)(((sal_uInt16)aColors[1].GetGreen() + (sal_uInt16)aColor2.GetGreen())/2);
            sal_uInt8    nBlue   = (sal_uInt8)(((sal_uInt16)aColors[1].GetBlue()  + (sal_uInt16)aColor2.GetBlue())/2);
            aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );
        }
    }
    rSettings.SetStyleSettings( aStyleSettings );
}
