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

#include <sot/stg.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>


/************** class SvStorageInfoList **********************************
*************************************************************************/
PRV_SV_IMPL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)

const SvStorageInfo * SvStorageInfoList::Get( const String & rEleName )
{
    for( sal_uLong i = 0; i < Count(); i++ )
    {
        const SvStorageInfo & rType = GetObject( i );
        if( rType.GetName() == rEleName )
            return &rType;
    }
    return NULL;
}

/************** class SvStorageInfo **************************************
*************************************************************************/
sal_uLong ReadClipboardFormat( SvStream & rStm )
{
    sal_uInt32 nFormat = 0;
    sal_Int32 nLen = 0;
    rStm >> nLen;
    if( rStm.IsEof() )
        rStm.SetError( SVSTREAM_GENERALERROR );
    if( nLen > 0 )
    {
        // get a string name
        sal_Char * p = new( ::std::nothrow ) sal_Char[ nLen ];
        if( p && rStm.Read( p, nLen ) == (sal_uLong) nLen )
        {
            // take so much from the buffer, as the string supports
            nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii( p, xub_StrLen( ( nLen - 1 ) & STRING_MAXLEN ) ) );
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
        delete [] p;
    }
    else if( nLen == -1L )
        // Windows clipboard format
        // SV und Win stimmen ueberein (bis einschl. FORMAT_GDIMETAFILE)
        rStm >> nFormat;
    else if( nLen == -2L )
    {
        rStm >> nFormat;
        // Mac clipboard format
        // ??? not implemented
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    else if( nLen != 0 )
    {
        // unknown identifier
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    return nFormat;
}

void WriteClipboardFormat( SvStream & rStm, sal_uLong nFormat )
{
    // determine the clipboard format string
    String aCbFmt;
    if( nFormat > FORMAT_GDIMETAFILE )
        aCbFmt = SotExchange::GetFormatName( nFormat );
    if( aCbFmt.Len() )
	{
		ByteString aAsciiCbFmt( aCbFmt, RTL_TEXTENCODING_ASCII_US );
        rStm << (sal_Int32) (aAsciiCbFmt.Len() + 1);
		rStm << (const char *)aAsciiCbFmt.GetBuffer();
        rStm << (sal_uInt8) 0;
	}
    else if( nFormat )
        rStm << (sal_Int32) -1         // for Windows
             << (sal_Int32) nFormat;
    else
        rStm << (sal_Int32) 0;         // no clipboard format
}


