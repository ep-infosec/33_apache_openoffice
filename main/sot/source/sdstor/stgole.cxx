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
#include "precompiled_sot.hxx"

#include "rtl/string.h"
#include "rtl/string.h"
#include "stgole.hxx"
#include "sot/storinfo.hxx"		// Read/WriteClipboardFormat()

#include <tools/debug.hxx>
#if defined(_MSC_VER) && (_MSC_VER>=1400)
#pragma warning(disable: 4342)
#endif
///////////////////////// class StgInternalStream ////////////////////////

StgInternalStream::StgInternalStream
    ( BaseStorage& rStg, const String& rName, sal_Bool bWr )
{
	bIsWritable = sal_True;
	sal_uInt16 nMode = bWr
				 ? STREAM_WRITE | STREAM_SHARE_DENYALL
				 : STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE;
    pStrm = rStg.OpenStream( rName, nMode );

	// set the error code right here in the stream
	SetError( rStg.GetError() );
	SetBufferSize( 1024 );
}

StgInternalStream::~StgInternalStream()
{
	delete pStrm;
}

sal_uLong StgInternalStream::GetData( void* pData, sal_uLong nSize )
{
	if( pStrm )
	{
		nSize = pStrm->Read( pData, nSize );
		SetError( pStrm->GetError() );
		return nSize;
	}
	else
		return 0;
}

sal_uLong StgInternalStream::PutData( const void* pData, sal_uLong nSize )
{
	if( pStrm )
	{
		nSize = pStrm->Write( pData, nSize );
		SetError( pStrm->GetError() );
		return nSize;
	}
	else
		return 0;
}

sal_uLong StgInternalStream::SeekPos( sal_uLong nPos )
{
	return pStrm ? pStrm->Seek( nPos ) : 0;
}

void StgInternalStream::FlushData()
{
	if( pStrm )
	{
		pStrm->Flush();
		SetError( pStrm->GetError() );
	}
}

void StgInternalStream::Commit()
{
	Flush();
	pStrm->Commit();
}

///////////////////////// class StgCompObjStream /////////////////////////

StgCompObjStream::StgCompObjStream( BaseStorage& rStg, sal_Bool bWr )
			: StgInternalStream( rStg, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1CompObj" ) ), bWr )
{
	memset( &aClsId, 0, sizeof( ClsId ) );
	nCbFormat = 0;
}

sal_Bool StgCompObjStream::Load()
{
	memset( &aClsId, 0, sizeof( ClsId ) );
	nCbFormat = 0;
	aUserName.Erase();
	if( GetError() != SVSTREAM_OK )
		return sal_False;
	Seek( 8L );		// skip the first part
	sal_Int32 nMarker = 0;
	*this >> nMarker;
	if( nMarker == -1L )
	{
		*this >> aClsId;
		sal_Int32 nLen1 = 0;
		*this >> nLen1;
        if ( nLen1 > 0 )
        {
            // higher bits are ignored
            sal_uLong nStrLen = ::std::min( nLen1, (sal_Int32)0xFFFE );

            sal_Char* p = new sal_Char[ nStrLen+1 ];
            p[nStrLen] = 0;
            if( Read( p, nStrLen ) == nStrLen )
            {
                aUserName = nStrLen ? String( p, gsl_getSystemTextEncoding() ) : String();
                nCbFormat = ReadClipboardFormat( *this );
            }
            else
                SetError( SVSTREAM_GENERALERROR );
            delete [] p;
        }
	}
	return sal_Bool( GetError() == SVSTREAM_OK );
}

sal_Bool StgCompObjStream::Store()
{
	if( GetError() != SVSTREAM_OK )
		return sal_False;
	Seek( 0L );
	ByteString aAsciiUserName( aUserName, RTL_TEXTENCODING_ASCII_US );
	*this << (sal_Int16) 1			// Version?
              << (sal_Int16) -2                     // 0xFFFE = Byte Order Indicator
              << (sal_Int32) 0x0A03			// Windows 3.10
              << (sal_Int32) -1L
              << aClsId				// Class ID
              << (sal_Int32) (aAsciiUserName.Len() + 1)
              << (const char *)aAsciiUserName.GetBuffer()
              << (sal_uInt8) 0;				// string terminator
/*	// determine the clipboard format string
	String aCbFmt;
	if( nCbFormat > FORMAT_GDIMETAFILE )
	aCbFmt = Exchange::GetFormatName( nCbFormat );
	if( aCbFmt.Len() )
		*this << (sal_Int32) aCbFmt.Len() + 1
			   << (const char*) aCbFmt
			   << (sal_uInt8) 0;
	else if( nCbFormat )
 		*this << (sal_Int32) -1	   		// for Windows
 			   << (sal_Int32) nCbFormat;
	else
		*this << (sal_Int32) 0;			// no clipboard format
*/
	WriteClipboardFormat( *this, nCbFormat );
	*this << (sal_Int32) 0;				// terminator
	Commit();
	return sal_Bool( GetError() == SVSTREAM_OK );
}

/////////////////////////// class StgOleStream ///////////////////////////

StgOleStream::StgOleStream( BaseStorage& rStg, sal_Bool bWr )
			: StgInternalStream( rStg, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole" ) ), bWr )
{
	nFlags = 0;
}

sal_Bool StgOleStream::Load()
{
	nFlags = 0;
	if( GetError() != SVSTREAM_OK )
		return sal_False;
	sal_Int32 version = 0;
	Seek( 0L );
	*this >> version >> nFlags;
	return sal_Bool( GetError() == SVSTREAM_OK );
}

sal_Bool StgOleStream::Store()
{
	if( GetError() != SVSTREAM_OK )
		return sal_False;
	Seek( 0L );
	*this << (sal_Int32) 0x02000001			// OLE version, format
		  << (sal_Int32) nFlags				// Object flags
		  << (sal_Int32) 0					// Update Options
		  << (sal_Int32) 0					// reserved
		  << (sal_Int32) 0;			   		// Moniker 1
	Commit();
	return sal_Bool( GetError() == SVSTREAM_OK );
}

