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
#include "precompiled_sw.hxx"

// INCLUDE ---------------------------------------------------------------


#include <tools/list.hxx>
#include "wrtsh.hxx"


#include "dbui.hrc"
#include "dbui.hxx"

/*---------------------------------------------------------------------
	Beschreibung:
---------------------------------------------------------------------*/

PrintMonitor::PrintMonitor( Window *pParent, PrintMonitorType eType )
:	ModelessDialog( pParent, SW_RES(DLG_PRINTMONITOR) ),
	aDocName	(this, SW_RES( FT_DOCNAME )),
	aPrinting	(this, SW_RES( 
	    eType == MONITOR_TYPE_MAIL ? 
	        FT_SENDING : eType == MONITOR_TYPE_SAVE ? FT_SAVING : FT_PRINTING )),
	aPrinter	(this, SW_RES( FT_PRINTER 		)),
	aPrintInfo	(this, SW_RES( FT_PRINTINFO		)),
	aCancel		(this, SW_RES( PB_CANCELPRNMON	))
{
	switch (eType)
	{
		case MONITOR_TYPE_SAVE: SetText(SW_RES(STR_SAVEMON)); break;
		case MONITOR_TYPE_MAIL: SetText(SW_RES(STR_EMAILMON)); break;
		case MONITOR_TYPE_PRINT: break;
	}
	FreeResource();
}
/*---------------------------------------------------------------------
	
---------------------------------------------------------------------*/
void lcl_ResizeControl( Window* pWin, long nDiff )
{
    Size aSize( pWin->GetSizePixel() );
    aSize.Width() += nDiff; 
    pWin->SetSizePixel( aSize );   
}      
void lcl_RePosControl( Window* pWin, long nDiff )
{
    Point aPos( pWin->GetPosPixel() );
    aPos.X()  += nDiff; 
    pWin->SetPosPixel( aPos );   
}      

void PrintMonitor::ResizeControls()
{
    Size aDlgSize( GetSizePixel() );
    Size aPrinterSize( aPrinter.GetSizePixel() );
    long nPrinterTextWidth = aPrinter.GetTextWidth( aPrinter.GetText() );
    if( nPrinterTextWidth > aPrinterSize.Width() )
    {
        //increase control and dialog width if printer text is too long
        //do not increase dialog width more than three times
        long nDiff = nPrinterTextWidth - aPrinterSize.Width();
        if( nDiff > 2 * aDlgSize.Width() )
        {
            aPrinter.SetStyle( WB_RIGHT | aPrinter.GetStyle() ); 
            nDiff = 2 * aDlgSize.Width();
        }
        aDlgSize.Width() += nDiff;
        SetSizePixel(aDlgSize);
        lcl_ResizeControl( &aPrinter, nDiff );      

        nDiff /= 2;
        lcl_RePosControl( &aDocName, nDiff );      
        lcl_RePosControl( &aPrinting, nDiff );      
        lcl_RePosControl( &aPrintInfo, nDiff );
        lcl_RePosControl( &aCancel, nDiff );      
    }
}
/*---------------------------------------------------------------------
	Progress Indicator for Creation of personalized Mail Merge documents:
---------------------------------------------------------------------*/

CreateMonitor::CreateMonitor( Window *pParent )
:	ModelessDialog( pParent, SW_RES(DLG_MM_CREATIONMONITOR) ),
	m_aStatus	        (this, SW_RES( FT_STATUS )),
    m_aProgress	        (this, SW_RES( FT_PROGRESS )),
    m_aCreateDocuments	(this, SW_RES( FT_CREATEDOCUMENTS )),
    m_aCounting	        (this, SW_RES( FT_COUNTING )),
	m_aCancelButton		(this, SW_RES( PB_CANCELPRNMON	)),
    m_sCountingPattern(),
    m_sVariable_Total( String::CreateFromAscii("%Y") ),
    m_sVariable_Position( String::CreateFromAscii("%X") ),
    m_nTotalCount(0),
    m_nCurrentPosition(0)
{
	FreeResource();

    m_sCountingPattern = m_aCounting.GetText();
    m_aCounting.SetText(String::CreateFromAscii("..."));
}

void CreateMonitor::UpdateCountingText()
{
    String sText(m_sCountingPattern); 
    sText.SearchAndReplaceAll( m_sVariable_Total, String::CreateFromInt32( m_nTotalCount ) );
    sText.SearchAndReplaceAll( m_sVariable_Position, String::CreateFromInt32( m_nCurrentPosition ) );
    m_aCounting.SetText(sText);
}

void CreateMonitor::SetTotalCount( sal_Int32 nTotal )
{
    m_nTotalCount = nTotal;
    UpdateCountingText();
}

void CreateMonitor::SetCurrentPosition( sal_Int32 nCurrent )
{
    m_nCurrentPosition = nCurrent;
    UpdateCountingText();
}

void CreateMonitor::SetCancelHdl( const Link& rLink )
{
    m_aCancelButton.SetClickHdl( rLink );
}
