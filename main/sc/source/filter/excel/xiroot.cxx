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
#include "precompiled_scfilt.hxx"
#include "xiroot.hxx"
#include "addincol.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "xltracer.hxx"
#include "xihelper.hxx"
#include "xiformula.hxx"
#include "xilink.hxx"
#include "xiname.hxx"
#include "xistyle.hxx"
#include "xicontent.hxx"
#include "xiescher.hxx"
#include "xipivot.hxx"
#include "xipage.hxx"
#include "xiview.hxx"

#include "root.hxx"
#include "excimp8.hxx"

// Global data ================================================================

XclImpRootData::XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, false ),
    mbHasCodePage( false ),
    mbHasBasic( false )
{
}

XclImpRootData::~XclImpRootData()
{
}

// ----------------------------------------------------------------------------

XclImpRoot::XclImpRoot( XclImpRootData& rImpRootData ) :
    XclRoot( rImpRootData ),
    mrImpData( rImpRootData )
{
    mrImpData.mxAddrConv.reset( new XclImpAddressConverter( GetRoot() ) );
    mrImpData.mxFmlaComp.reset( new XclImpFormulaCompiler( GetRoot() ) );
    mrImpData.mxPalette.reset( new XclImpPalette( GetRoot() ) );
    mrImpData.mxFontBfr.reset( new XclImpFontBuffer( GetRoot() ) );
    mrImpData.mxNumFmtBfr.reset( new XclImpNumFmtBuffer( GetRoot() ) );
    mrImpData.mpXFBfr.reset( new XclImpXFBuffer( GetRoot() ) );
    mrImpData.mxXFRangeBfr.reset( new XclImpXFRangeBuffer( GetRoot() ) );
    mrImpData.mxTabInfo.reset( new XclImpTabInfo );
    mrImpData.mxNameMgr.reset( new XclImpNameManager( GetRoot() ) );
    mrImpData.mxObjMgr.reset( new XclImpObjectManager( GetRoot() ) );

    if( GetBiff() == EXC_BIFF8 )
    {
        mrImpData.mxLinkMgr.reset( new XclImpLinkManager( GetRoot() ) );
        mrImpData.mxSst.reset( new XclImpSst( GetRoot() ) );
        mrImpData.mxCondFmtMgr.reset( new XclImpCondFormatManager( GetRoot() ) );
        // TODO still in old RootData (deleted by RootData)
        GetOldRoot().pAutoFilterBuffer = new XclImpAutoFilterBuffer;
        mrImpData.mxWebQueryBfr.reset( new XclImpWebQueryBuffer( GetRoot() ) );
        mrImpData.mxPTableMgr.reset( new XclImpPivotTableManager( GetRoot() ) );
        mrImpData.mxTabProtect.reset( new XclImpSheetProtectBuffer( GetRoot() ) );
        mrImpData.mxDocProtect.reset( new XclImpDocProtectBuffer( GetRoot() ) );
    }

    mrImpData.mxPageSett.reset( new XclImpPageSettings( GetRoot() ) );
    mrImpData.mxDocViewSett.reset( new XclImpDocViewSettings( GetRoot() ) );
    mrImpData.mxTabViewSett.reset( new XclImpTabViewSettings( GetRoot() ) );
}

void XclImpRoot::SetCodePage( sal_uInt16 nCodePage )
{
    SetTextEncoding( XclTools::GetTextEncoding( nCodePage ) );
    mrImpData.mbHasCodePage = true;
}

void XclImpRoot::SetAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    if( !mrImpData.mbHasCodePage )
        SetTextEncoding( eAppFontEnc );
}

void XclImpRoot::InitializeTable( SCTAB nScTab )
{
    if( GetBiff() <= EXC_BIFF4 )
    {
        GetPalette().Initialize();
        GetFontBuffer().Initialize();
        GetNumFmtBuffer().Initialize();
        GetXFBuffer().Initialize();
    }
    GetXFRangeBuffer().Initialize();
    GetPageSettings().Initialize();
    GetTabViewSettings().Initialize();
    // delete the automatically generated codename
    GetDoc().SetCodeName( nScTab, String::EmptyString() );
}

void XclImpRoot::FinalizeTable()
{
    GetXFRangeBuffer().Finalize();
    GetOldRoot().pColRowBuff->Convert( GetCurrScTab() );
    GetPageSettings().Finalize();
    GetTabViewSettings().Finalize();
}

XclImpAddressConverter& XclImpRoot::GetAddressConverter() const
{
    return *mrImpData.mxAddrConv;
}

XclImpFormulaCompiler& XclImpRoot::GetFormulaCompiler() const
{
    return *mrImpData.mxFmlaComp;
}

ExcelToSc& XclImpRoot::GetOldFmlaConverter() const
{
    // TODO still in old RootData
    return *GetOldRoot().pFmlaConverter;
}

XclImpSst& XclImpRoot::GetSst() const
{
    DBG_ASSERT( mrImpData.mxSst.is(), "XclImpRoot::GetSst - invalid call, wrong BIFF" );
    return *mrImpData.mxSst;
}

XclImpPalette& XclImpRoot::GetPalette() const
{
    return *mrImpData.mxPalette;
}

XclImpFontBuffer& XclImpRoot::GetFontBuffer() const
{
    return *mrImpData.mxFontBfr;
}

XclImpNumFmtBuffer& XclImpRoot::GetNumFmtBuffer() const
{
    return *mrImpData.mxNumFmtBfr;
}

XclImpXFBuffer& XclImpRoot::GetXFBuffer() const
{
    return *mrImpData.mpXFBfr;
}

XclImpXFRangeBuffer& XclImpRoot::GetXFRangeBuffer() const
{
    return *mrImpData.mxXFRangeBfr;
}

_ScRangeListTabs& XclImpRoot::GetPrintAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintRanges;
}

_ScRangeListTabs& XclImpRoot::GetTitleAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintTitles;
}

XclImpTabInfo& XclImpRoot::GetTabInfo() const
{
    return *mrImpData.mxTabInfo;
}

XclImpNameManager& XclImpRoot::GetNameManager() const
{
    return *mrImpData.mxNameMgr;
}

XclImpLinkManager& XclImpRoot::GetLinkManager() const
{
    DBG_ASSERT( mrImpData.mxLinkMgr.is(), "XclImpRoot::GetLinkManager - invalid call, wrong BIFF" );
    return *mrImpData.mxLinkMgr;
}

XclImpObjectManager& XclImpRoot::GetObjectManager() const
{
    return *mrImpData.mxObjMgr;
}

XclImpSheetDrawing& XclImpRoot::GetCurrSheetDrawing() const
{
    DBG_ASSERT( !IsInGlobals(), "XclImpRoot::GetCurrSheetDrawing - must not be called from workbook globals" );
    return mrImpData.mxObjMgr->GetSheetDrawing( GetCurrScTab() );
}

XclImpCondFormatManager& XclImpRoot::GetCondFormatManager() const
{
    DBG_ASSERT( mrImpData.mxCondFmtMgr.is(), "XclImpRoot::GetCondFormatManager - invalid call, wrong BIFF" );
    return *mrImpData.mxCondFmtMgr;
}

XclImpAutoFilterBuffer& XclImpRoot::GetFilterManager() const
{
    // TODO still in old RootData
    DBG_ASSERT( GetOldRoot().pAutoFilterBuffer, "XclImpRoot::GetFilterManager - invalid call, wrong BIFF" );
    return *GetOldRoot().pAutoFilterBuffer;
}

XclImpWebQueryBuffer& XclImpRoot::GetWebQueryBuffer() const
{
    DBG_ASSERT( mrImpData.mxWebQueryBfr.is(), "XclImpRoot::GetWebQueryBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxWebQueryBfr;
}

XclImpPivotTableManager& XclImpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrImpData.mxPTableMgr.is(), "XclImpRoot::GetPivotTableManager - invalid call, wrong BIFF" );
    return *mrImpData.mxPTableMgr;
}

XclImpSheetProtectBuffer& XclImpRoot::GetSheetProtectBuffer() const
{
    DBG_ASSERT( mrImpData.mxTabProtect.is(), "XclImpRoot::GetSheetProtectBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxTabProtect;
}

XclImpDocProtectBuffer& XclImpRoot::GetDocProtectBuffer() const
{
    DBG_ASSERT( mrImpData.mxDocProtect.is(), "XclImpRoot::GetDocProtectBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxDocProtect;
}

XclImpPageSettings& XclImpRoot::GetPageSettings() const
{
    return *mrImpData.mxPageSett;
}

XclImpDocViewSettings& XclImpRoot::GetDocViewSettings() const
{
    return *mrImpData.mxDocViewSett;
}

XclImpTabViewSettings& XclImpRoot::GetTabViewSettings() const
{
    return *mrImpData.mxTabViewSett;
}

String XclImpRoot::GetScAddInName( const String& rXclName ) const
{
    String aScName;
    if( ScGlobal::GetAddInCollection()->GetCalcName( rXclName, aScName ) )
        return aScName;
    return rXclName;
}

void XclImpRoot::ReadCodeName( XclImpStream& rStrm, bool bGlobals )
{
    if( mrImpData.mbHasBasic && (GetBiff() == EXC_BIFF8) )
    {
        String aName = rStrm.ReadUniString();
        if( aName.Len() > 0 )
        {
            if( bGlobals )
            {
                GetExtDocOptions().GetDocSettings().maGlobCodeName = aName;
                GetDoc().SetCodeName( aName );
            }
            else
            {
                GetExtDocOptions().SetCodeName( GetCurrScTab(), aName );
                GetDoc().SetCodeName( GetCurrScTab(), aName );
            }
        }
    }
}

// ============================================================================
