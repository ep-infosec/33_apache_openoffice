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

#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"

#include "tools/stream.hxx"

#include "sal/alloca.h"

using namespace psp;
using namespace rtl;

JobData& JobData::operator=(const JobData& rRight)
{
    m_nCopies				= rRight.m_nCopies;
    m_nLeftMarginAdjust		= rRight.m_nLeftMarginAdjust;
    m_nRightMarginAdjust	= rRight.m_nRightMarginAdjust;
    m_nTopMarginAdjust		= rRight.m_nTopMarginAdjust;
    m_nBottomMarginAdjust	= rRight.m_nBottomMarginAdjust;
    m_nColorDepth			= rRight.m_nColorDepth;
    m_eOrientation			= rRight.m_eOrientation;
    m_aPrinterName			= rRight.m_aPrinterName;
    m_pParser				= rRight.m_pParser;
    m_aContext				= rRight.m_aContext;
    m_nPSLevel				= rRight.m_nPSLevel;
    m_nPDFDevice            = rRight.m_nPDFDevice;
    m_nColorDevice			= rRight.m_nColorDevice;

    if( ! m_pParser && m_aPrinterName.getLength() )
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        rMgr.setupJobContextData( *this );
    }
    return *this;
}

void JobData::setCollate( bool bCollate )
{
    const PPDParser* pParser = m_aContext.getParser();
    if( pParser )
    {
        const PPDKey* pKey = pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) );
        if( pKey )
        {
            const PPDValue* pVal = NULL;
            if( bCollate )
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "True" ) ) );
            else
            {
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "False" ) ) );
                if( ! pVal )
                    pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
            }
            m_aContext.setValue( pKey, pVal );
        }
    }
}

bool JobData::setPaper( int i_nWidth, int i_nHeight )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        rtl::OUString aPaper( m_pParser->matchPaper( i_nWidth, i_nHeight ) );
    
        const PPDKey*   pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        const PPDValue* pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : NULL;
        
        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
}

bool JobData::setPaperBin( int i_nPaperBin )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        const PPDKey*   pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
        const PPDValue* pValue = pKey ? pKey->getValue( i_nPaperBin ) : NULL;
        
        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
}

bool JobData::getStreamBuffer( void*& pData, int& bytes )
{
    // consistency checks
    if( ! m_pParser )
        m_pParser = m_aContext.getParser();
    if( m_pParser != m_aContext.getParser() ||
        ! m_pParser )
        return false;

    SvMemoryStream aStream;
    ByteString aLine;

    // write header job data
    aStream.WriteLine( "JobData 1" );

    aLine = "printer=";
    aLine += ByteString( String( m_aPrinterName ), RTL_TEXTENCODING_UTF8 );
    aStream.WriteLine( aLine );

    aLine = "orientation=";
    aLine += m_eOrientation == orientation::Landscape ? "Landscape" : "Portrait";
    aStream.WriteLine( aLine );

    aLine = "copies=";
    aLine += ByteString::CreateFromInt32( m_nCopies );
    aStream.WriteLine( aLine );

    aLine = "margindajustment=";
    aLine += ByteString::CreateFromInt32( m_nLeftMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nRightMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nTopMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nBottomMarginAdjust );
    aStream.WriteLine( aLine );

    aLine = "colordepth=";
    aLine += ByteString::CreateFromInt32( m_nColorDepth );
    aStream.WriteLine( aLine );

    aLine = "pslevel=";
    aLine += ByteString::CreateFromInt32( m_nPSLevel );
    aStream.WriteLine( aLine );
    
    aLine = "pdfdevice=";
    aLine += ByteString::CreateFromInt32( m_nPDFDevice );
    aStream.WriteLine( aLine );

    aLine = "colordevice=";
    aLine += ByteString::CreateFromInt32( m_nColorDevice );
    aStream.WriteLine( aLine );

    // now append the PPDContext stream buffer
    aStream.WriteLine( "PPDContexData" );
    sal_uLong nBytes;
    void* pContextBuffer = m_aContext.getStreamableBuffer( nBytes );
    if( nBytes )
        aStream.Write( pContextBuffer, nBytes );

    // success
    pData = rtl_allocateMemory( bytes = aStream.Tell() );
    memcpy( pData, aStream.GetData(), bytes );
    return true;
}

bool JobData::constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData )
{
    SvMemoryStream aStream( pData, bytes, STREAM_READ );
    ByteString aLine;
    bool bVersion       = false;
    bool bPrinter       = false;
    bool bOrientation   = false;
    bool bCopies        = false;
    bool bContext       = false;
    bool bMargin        = false;
    bool bColorDepth    = false;
    bool bColorDevice   = false;
    bool bPSLevel       = false;
    bool bPDFDevice     = false;
    while( ! aStream.IsEof() )
    {
        aStream.ReadLine( aLine );
        if( aLine.CompareTo( "JobData", 7 ) == COMPARE_EQUAL )
            bVersion = true;
        else if( aLine.CompareTo( "printer=", 8 ) == COMPARE_EQUAL )
        {
            bPrinter = true;
            rJobData.m_aPrinterName = String( aLine.Copy( 8 ), RTL_TEXTENCODING_UTF8 );
        }
        else if( aLine.CompareTo( "orientation=", 12 ) == COMPARE_EQUAL )
        {
            bOrientation = true;
            rJobData.m_eOrientation = aLine.Copy( 12 ).EqualsIgnoreCaseAscii( "landscape" ) ? orientation::Landscape : orientation::Portrait;
        }
        else if( aLine.CompareTo( "copies=", 7 ) == COMPARE_EQUAL )
        {
            bCopies = true;
            rJobData.m_nCopies = aLine.Copy( 7 ).ToInt32();
        }
        else if( aLine.CompareTo( "margindajustment=",17 ) == COMPARE_EQUAL )
        {
            bMargin = true;
            ByteString aValues( aLine.Copy( 17 ) );
            rJobData.m_nLeftMarginAdjust = aValues.GetToken( 0, ',' ).ToInt32();
            rJobData.m_nRightMarginAdjust = aValues.GetToken( 1, ',' ).ToInt32();
            rJobData.m_nTopMarginAdjust = aValues.GetToken( 2, ',' ).ToInt32();
            rJobData.m_nBottomMarginAdjust = aValues.GetToken( 3, ',' ).ToInt32();
        }
        else if( aLine.CompareTo( "colordepth=", 11 ) == COMPARE_EQUAL )
        {
            bColorDepth = true;
            rJobData.m_nColorDepth = aLine.Copy( 11 ).ToInt32();
        }
        else if( aLine.CompareTo( "colordevice=", 12 ) == COMPARE_EQUAL )
        {
            bColorDevice = true;
            rJobData.m_nColorDevice = aLine.Copy( 12 ).ToInt32();
        }
        else if( aLine.CompareTo( "pslevel=", 8 ) == COMPARE_EQUAL )
        {
            bPSLevel = true;
            rJobData.m_nPSLevel = aLine.Copy( 8 ).ToInt32();
        }
        else if( aLine.CompareTo( "pdfdevice=", 10 ) == COMPARE_EQUAL )
        {
            bPDFDevice = true;
            rJobData.m_nPDFDevice = aLine.Copy( 10 ).ToInt32();
        }
        else if( aLine.Equals( "PPDContexData" ) )
        {
            if( bPrinter )
            {
                PrinterInfoManager& rManager = PrinterInfoManager::get();
                const PrinterInfo& rInfo = rManager.getPrinterInfo( rJobData.m_aPrinterName );
                rJobData.m_pParser = PPDParser::getParser( rInfo.m_aDriverName );
                if( rJobData.m_pParser )
                {
                    rJobData.m_aContext.setParser( rJobData.m_pParser );
                    int nBytes = bytes - aStream.Tell();
                    void* pRemain = alloca( bytes - aStream.Tell() );
                    aStream.Read( pRemain, nBytes );
                    rJobData.m_aContext.rebuildFromStreamBuffer( pRemain, nBytes );
                    bContext = true;
                }
            }
        }
    }

    return bVersion && bPrinter && bOrientation && bCopies && bContext && bMargin && bPSLevel && bPDFDevice && bColorDevice && bColorDepth;
}
